"""Finite HIL windows; Prolific COM5 and specified STLINK only. No power instruments."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time
import serial
from serial.tools import list_ports

def fields(text):
    return dict(re.findall(r"(\w+)=(\S+)",text))

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--programmer",type=Path,required=True)
    ap.add_argument("--stlink",required=True)
    ap.add_argument("--uart-port",default="COM5")
    ap.add_argument("--image",type=Path,required=True)
    ap.add_argument("--results",type=Path,required=True)
    ap.add_argument("--flash",action="store_true")
    ap.add_argument("--test",choices=["auto","capture","controls","motion","pickup","tilt","continuous","rf","mixed","awake-rf","awake-motion","awake-pickup"],default="auto")
    ap.add_argument("--seconds",type=int,default=30,choices=range(10,151))
    args=ap.parse_args()
    if args.seconds>60 and not args.test.startswith("awake-"):
        ap.error("Only debug awake observation permits >60s; run remains 1..60")
    port=next((p for p in list_ports.comports() if p.device==args.uart_port),None)
    assert port and (port.vid,port.pid)==(0x067b,0x2303),"Expected Prolific UART"
    assert args.image.is_file() and 0<args.image.stat().st_size<=32768
    args.results.mkdir(parents=True,exist_ok=False)
    records=[];uart=None;deadline=0;result="FAIL";clock_anchor=None
    def log(kind,**kw):
        row=dict(kind=kind,time=time.monotonic(),**kw);records.append(row);print(json.dumps(row),flush=True)
    def cube(*tail,mode="HOTPLUG"):
        cmd=[str(args.programmer),"-c","port=SWD",f"sn={args.stlink}",f"mode={mode}","freq=4000",*tail]
        p=subprocess.run(cmd,capture_output=True,text=True,errors="replace",timeout=30)
        log("cube",command=cmd,exit=p.returncode,output=p.stdout+p.stderr)
        assert not p.returncode and not re.search(r"^\s*Error:",p.stdout+p.stderr,re.M)
        return p.stdout
    def connect():
        s=serial.Serial(args.uart_port,115200,timeout=.03,write_timeout=1)
        s.dtr=s.rts=False
        return s
    def receive(seconds=.25,until=None):
        end=time.monotonic()+seconds;buf=bytearray();lines=[]
        while time.monotonic()<end:
            b=uart.read(1)
            if b:
                buf.extend(b)
                if b==b"\n":
                    s=buf.decode("ascii",errors="replace").strip();buf.clear()
                    log("RX",text=s);lines.append(s)
                    if until and s.startswith(until):break
        if buf:log("PARTIAL",data=repr(buf))
        return lines
    def send(command,wait=.25):
        log("TX",command=repr(command))
        uart.write((command.encode() if isinstance(command,str) else command)+b"\n")
        return receive(wait)
    def find(lines,prefix):
        return next(s for s in lines if s.startswith(prefix))
    def status():
        lines=send("status")
        state=fields(find(lines,"OK mode="));app=fields(find(lines,"APP "))
        assert app["version"]=="NativeShakeProgressTest-v1"
        return state,app
    def run(n,stimulus):
        nonlocal deadline
        deadline=time.monotonic()+n+6
        send("counters clear")
        before_clock=fields(find(send("clock"),"CLOCK "))
        lines=send(f"run {n}",0)
        lines+=receive(3,until="RUN_READY")
        assert any(s.startswith("RUN_BEGIN") for s in lines) and "RUN_READY rtc_probe=1" in lines,lines
        log("WINDOW_READY",stimulus=stimulus,seconds=n)
        lines+=receive(n+4,until="RUN_END")
        lines+=receive(.6)
        summary=fields(find(lines,"RUN_END"))
        assert summary["reason"]=="deadline" and int(summary["elapsed"])>=n and int(summary["stops"])>=1
        counts={fields(s)["source"]:fields(s) for s in lines if s.startswith("COUNT ")}
        assert counts["rtc"]["errors"]=="0" and int(counts["rtc"]["wake"])>=1
        assert all(v["errors"]=="0" for v in counts.values()),counts
        state,app=status()
        after_clock=fields(find(send("clock"),"CLOCK "))
        elapsed_ms=(int(after_clock["monotonicMs"])-int(before_clock["monotonicMs"]))&0xffffffff
        assert n*1000<=elapsed_ms<=(n+5)*1000,elapsed_ms
        if clock_anchor:
            host_elapsed=(time.monotonic()-clock_anchor[0])*1000
            rtc_elapsed=(int(after_clock["monotonicMs"])-clock_anchor[1])&0xffffffff
            assert abs(host_elapsed-rtc_elapsed)<1500,(host_elapsed,rtc_elapsed)
        assert state["mode"]=="debug" and state["lis"]=="1/0" and state["st"]=="1/0",state
        assert app["display"]==app["sampling"]=="0",app
        log("RUN_RESULT",summary=summary,counts=counts,shake=fields(find(lines,"SHAKE ")))
        deadline=0
        return counts,lines
    try:
        log("identity",image_sha256=hashlib.sha256(args.image.read_bytes()).hexdigest(),
            port=vars(port),power="USER_SUPPLIED",instruments="NOT_OPENED")
        probe=cube(mode="UR" if args.flash else "HOTPLUG")
        assert re.search(r"Device ID\s*:\s*0x425",probe,re.I)
        if args.flash:
            cube("reset=HWrst","-w",str(args.image.resolve()),"0x08000000","-v","-rst","-score","-run",mode="UR")
        copy=args.results.resolve()/"readback.bin"
        cube("-u","0x08000000",str(args.image.stat().st_size),str(copy),"-score","-run")
        assert copy.read_bytes()==args.image.read_bytes()
        uart=connect();receive(.3);status();send("off")
        if args.test=="auto":
            # Explicit user-authorized UTC write and ordinary reset, never backup reset.
            utc=datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
            lines=send("setutc "+utc)
            assert "OK setutc" in lines,lines
            before=fields(find(send("clock"),"CLOCK "))
            assert before["valid"]=="1" and before["bkp2"]=="544D0101"
            uart.close();uart=None
            cube("-rst","-score","-run")
            time.sleep(.3);uart=connect();receive(.3);send("off")
            after=fields(find(send("clock"),"CLOCK "))
            assert after["valid"]=="1" and after["bkp2"]==before["bkp2"] and after["bkp3"]==before["bkp3"]
            assert all(after[k]==before[k] for k in ("bkp0","bkp1","bkp4"))
            assert 0<=int(after["utc"])-int(before["utc"])<15
            clock_anchor=(time.monotonic(),int(after["monotonicMs"]))
            log("PASS reset_retention",before=before,after=after,utc_source="PC UTC clock; NTP accuracy not verified")
            for level in range(9):
                assert "OK display" in send(f"progress {level}")
                _,a=status()
                assert a["progress"]==str(level)
                if level:
                    expected=[0,1,2,6,10,26,42,106,170][level]
                    assert int(a["pattern"],16)==expected and a["pwm"]==str(level%2)
                time.sleep(3.1 if level else .5)
                _,a=status();assert a["display"]==a["pwm"]=="0" and a["progress"]==str(level)
                regs=fields(find(send("ledregs"),"LEDREG "))
                assert not int(regs["apb1"],16)&1 and not int(regs["apb2"],16)&(1<<5)
                assert not int(regs["odr"],16)&0x1e0
                assert all((int(regs["mode"],16)>>(2*p))&3==1 for p in (5,6,7,8))
            send("progress 5");time.sleep(.5);send("show");time.sleep(.5)
            assert status()[1]["display"]=="1"
            for invalid in ["progress 9","progress -1","progress 00","bad","x"*80,b"show\x00"]:
                lines=send(invalid)
                assert any(s.startswith("ERR ") for s in lines)
                assert status()[1]["progress"]=="5"
            uart.write(b"pro");time.sleep(.05);assert not uart.in_waiting
            uart.write(b"gress 7\r\n");assert "OK display" in receive(.25)
            send("off");assert status()[1]["progress"]=="7"
            for n in (10,60,1):
                run(n,"none")
                if n!=1:send("off")
            assert "OK observe diagnostic_not_motion_wake" in send("observe")
            observed=receive(2)
            assert "SHAKE_NOT_CONFIRMED" in observed,observed
            regs=fields(find(send("lisregs"),"LISREG "))
            assert regs==dict(ok="1",ctrl1="20",ctrl2="0C",ctrl3="10",ctrl4="20",ctrl5="00",ctrl6="00"),regs
            send("trace",0);trace=receive(5,until="TRACE_END")
            rows=[list(map(int,s.split()[1:])) for s in trace if s.startswith("SAMPLE ")]
            assert 100<=len(rows)<=160,len(rows)
            (args.results/"trace.json").write_text(json.dumps(rows),encoding="utf-8")
            log("PASS bounded_observation",rows=len(rows),limits="Explicit awake acquisition, not physical motion ISR/wake evidence")
            send("off")
            log("PASS auto",limits="LED optics not observed; no peak stack/power claims")
        elif args.test=="capture":
            send("trace",0);trace=receive(5,until="TRACE_END")
            rows=[list(map(int,s.split()[1:])) for s in trace if s.startswith("SAMPLE ")]
            assert rows,"No stored observation"
            (args.results/"trace.json").write_text(json.dumps(rows),encoding="utf-8")
            log("CAPTURE_ONLY",samples=len(rows),limits="Read-only stored trace; not new stimulus or wake evidence")
        elif args.test=="controls":
            run(1,"controls_setup")
            assert "OK observe diagnostic_not_motion_wake" in send("observe",.05)
            assert status()[1]["sampling"]=="1"
            assert any(s.startswith("ERR ") for s in send("invalid",.05))
            assert status()[1]["sampling"]=="1"
            send("off")
            assert status()[1]["sampling"]=="0"
            regs=fields(find(send("lisregs"),"LISREG "))
            assert regs["ctrl1"]==regs["ctrl4"]=="00"
            run(1,"controls_restart")
            assert "OK observe diagnostic_not_motion_wake" in send("observe",.05)
            assert "OK debug" in send("mode debug",.05)
            assert status()[1]["sampling"]=="0"
            regs=fields(find(send("lisregs"),"LISREG "))
            assert regs["ctrl1"]==regs["ctrl4"]=="20"
            send("progress 7",.05);assert status()[1]["display"]=="1"
            assert "OK display" in send("show",.05)
            send("off",.05)
            state,app=status()
            assert app["progress"]=="7" and app["display"]==app["sampling"]==app["pwm"]=="0"
            run(1,"controls_final_restart")
            log("PASS controls",limits="Awake diagnostic cancellation/UART; not motion-wake evidence")
        else:
            classification_error=None
            run(1,"preflight")
            diagnostic=fields(find(send("probe"),"PROBE "))
            if args.test in ("rf","mixed","awake-rf"):assert diagnostic["field"]=="0",diagnostic
            send("progress 5");time.sleep(3.2);send("counters clear")
            if args.test.startswith("awake-"):
                log("WINDOW_READY",stimulus=args.test,seconds=args.seconds)
                lines=receive(args.seconds)
                lines+=send("counters")
            else:
                counts,lines=run(args.seconds,args.test)
                if args.test in ("motion","mixed"):
                    assert int(counts["motion"]["isr"])>0 and int(counts["motion"]["wake"])>0
                    assert int(fields(find(lines,"SHAKE "))["confirmed"])>0,"No SHAKE_CONFIRMED"
                if args.test in ("rf","mixed"):
                    assert int(counts["gpo"]["isr"])>0 and int(counts["gpo"]["wake"])>0
                    assert int(counts["gpo"]["confirmed"])>0
                if args.test in ("pickup","tilt"):
                    if fields(find(lines,"SHAKE "))["confirmed"]!="0":classification_error="False positive observed"
                if args.test=="continuous":
                    if fields(find(lines,"SHAKE "))["confirmed"]!="1":classification_error="Continuous motion did not produce exactly one confirmation"
                    assert int(counts["motion"]["wake"])>0
            if args.test=="awake-rf":
                gpo=next(fields(s) for s in lines if s.startswith("COUNT source=gpo "))
                assert int(gpo["isr"])>0 and int(gpo["confirmed"])>0
            send("trace",0);trace=receive(5,until="TRACE_END")
            rows=[list(map(int,s.split()[1:])) for s in trace if s.startswith("SAMPLE ")]
            (args.results/"trace.json").write_text(json.dumps(rows),encoding="utf-8")
            if rows:assert rows[0][0]<=100,("Observation starts with stale pre-STOP elapsed clock",rows[0])
            if args.test=="awake-motion":assert int(fields(find(lines,"SHAKE "))["confirmed"])>0
            if args.test=="awake-pickup" and fields(find(lines,"SHAKE "))["confirmed"]!="0":
                classification_error="False positive observed"
            if args.test in ("rf","awake-rf") and fields(find(lines,"SHAKE "))["confirmed"]!="0":
                classification_error="RF-only window unexpectedly displayed shake progress"
            log("STIMULUS_EVIDENCE",test=args.test,lines=lines,samples=len(rows),
                limits="Operator category/trial count must be confirmed; trace is measured, not synthesized")
            regs=fields(find(send("lisregs"),"LISREG "))
            assert regs==dict(ok="1",ctrl1="20",ctrl2="0C",ctrl3="10",ctrl4="20",ctrl5="00",ctrl6="00"),regs
            assert classification_error is None,classification_error
        result="PASS"
    except Exception as exc:
        log("ERROR",error=repr(exc))
    finally:
        if uart:
            if deadline:receive(max(0,deadline-time.monotonic()))
            try:send("off");status()
            except Exception as exc:log("CLEANUP_ERROR",error=repr(exc));result="FAIL"
            finally:uart.close()
        (args.results/"result.json").write_text(json.dumps(dict(result=result,records=records),indent=2),encoding="utf-8")
    return int(result!="PASS")
if __name__=="__main__":raise SystemExit(main())
