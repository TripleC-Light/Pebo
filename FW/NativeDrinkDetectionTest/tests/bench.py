"""Finite stage6 HIL acquisition; exact STLINK/Prolific only, no power instruments."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time
import sys
import threading
from datetime import datetime, timezone
import serial
from serial.tools import list_ports

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--image',type=Path,required=True)
    ap.add_argument('--results',type=Path,required=True)
    ap.add_argument('--test',choices=['flash','calibration','capture','auto','run','awake','controls'],required=True)
    ap.add_argument('--seconds',type=int,default=15)
    ap.add_argument('--label',default='unlabelled')
    ap.add_argument('--split',choices=['training','validation','engineering'],default='training')
    ap.add_argument('--arm',action='store_true',help='After setup wait at most45s for controller newline')
    ap.add_argument('--progress',type=int,choices=range(9))
    ap.add_argument('--uart-port',default='COM5',help='Explicit Prolific UART, VID/PID checked')
    ap.add_argument('--cue',action='store_true',help='Manual progress0 double flash at debug capture/awake start')
    a=ap.parse_args()
    assert a.image.is_file() and 0<a.image.stat().st_size<=32768
    assert 1<=a.seconds<=150
    if a.test=='run':assert a.seconds<=60
    if a.test=='capture':assert 5<=a.seconds<=30
    if a.test=='awake':assert a.seconds<=150
    port=next((p for p in list_ports.comports() if p.device==a.uart_port),None)
    assert port and (port.vid,port.pid)==(0x067b,0x2303),'Expected explicitly selected Prolific UART'
    a.results.mkdir(parents=True,exist_ok=False)
    rows=[];samples=[];uart=None;result='FAIL';runUntil=0
    def log(kind,**kw):
        entry=dict(kind=kind,time=time.monotonic(),**kw);rows.append(entry)
        if not (kind=='RX' and kw.get('text','').startswith('RAW ')):
            print(json.dumps(entry),flush=True)
    def cube(*tail,mode='HOTPLUG'):
        cmd=['C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe',
             '-c','port=SWD','sn=56FF6C066688555210351387',f'mode={mode}','freq=4000',*map(str,tail)]
        p=subprocess.run(cmd,capture_output=True,text=True,errors='replace',timeout=35)
        log('cube',command=cmd,exit=p.returncode,output=p.stdout+p.stderr)
        assert p.returncode==0 and not re.search(r'^\s*Error:',p.stdout+p.stderr,re.M)
        return p.stdout
    buffer=bytearray()
    def receive(seconds,until=None):
        end=time.monotonic()+seconds;lines=[]
        while time.monotonic()<end:
            b=uart.read(256)
            if b:
                buffer.extend(b)
                while b'\n' in buffer:
                    one,_,remaining=buffer.partition(b'\n');buffer[:]=remaining
                    line=one.decode('ascii',errors='replace').strip();lines.append(line);log('RX',text=line)
                    if line.startswith('RAW '):samples.append(list(map(int,line.split()[1:])))
                if until and any(s.startswith(until) for s in lines):break
        return lines
    def send(cmd,seconds=.2):
        log('TX',command=cmd if isinstance(cmd,str) else repr(cmd))
        uart.write((cmd.encode() if isinstance(cmd,str) else cmd)+b'\n')
        return receive(seconds)
    def fields(line):return dict(re.findall(r'(\w+)=(\S+)',line))
    def find(lines,prefix):return next(s for s in lines if s.startswith(prefix))
    def run(n,stimulus):
        nonlocal runUntil
        lines=send(f'run {n}',0)+receive(3,'RUN_READY')
        assert 'RUN_READY rtc_probe=1' in lines,lines
        runUntil=time.monotonic()+n+3
        log('WINDOW_READY',label=stimulus,seconds=n)
        lines+=receive(n+3,'RUN_END')+receive(.5)
        summary=fields(find(lines,'RUN_END'))
        assert summary['reason']=='deadline' and int(summary['elapsed'])==n
        assert int(summary['unknownWake'])==0
        runUntil=0
        return lines
    try:
        target=cube()
        assert '0x425' in target and 'STM32L0' in target,'Wrong target'
        if a.test=='flash':cube('reset=HWrst','-w',a.image.resolve(),'0x08000000','-v','-rst','-score','-run',mode='UR')
        readback=a.results/'readback.bin'
        cube('-u','0x08000000',str(a.image.stat().st_size),readback,'-score','-run')
        assert readback.read_bytes()==a.image.read_bytes()
        uart=serial.Serial(a.uart_port,115200,timeout=.02,write_timeout=1)
        uart.dtr=uart.rts=False
        receive(.3)
        app=fields(find(send('status'),'APP '))
        assert app['version']=='NativeDrinkDetectionTest-v1'
        send('mode debug')
        # A preceding helper deliberately ended with off; re-arm bounded sources.
        run(1,'setup')
        send('counters clear')
        if a.progress is not None:
            send(f'progress {a.progress}',.05)
            receive(.5 if a.progress==0 else 3.2)
        if a.arm:
            ready=threading.Event()
            def controller():
                if sys.stdin.readline():ready.set()
            threading.Thread(target=controller,daemon=True).start()
            log('ARMED',max_wait_seconds=45)
            assert ready.wait(45),'Controller did not start finite window'
        if a.test=='calibration':
            log('WINDOW_READY',label='upright_still',seconds=3)
            lines=send('calibration start',0)+receive(4)
            assert 'CALIBRATION_OK ram_only=1' in lines,lines
            send('calibration status')
        elif a.test=='capture':
            before=fields(find(send('drink status'),'DRINK '))
            assert before['calibration']=='1','Calibrate upright first'
            log('WINDOW_READY',label=a.label,seconds=a.seconds)
            if a.cue:
                log('OPERATOR_CUE',command='progress 0; existing manual LED double flash, not recognition')
                send('progress 0',.05)
            lines=send(f'capture {a.seconds}',0)+receive(a.seconds+2,'CAPTURE_END')
            assert 'CAPTURE_END' in lines and len(samples)>=a.seconds*80
            assert samples[0][0]<=100 and all(b[0]>x[0] for x,b in zip(samples,samples[1:]))
            lines+=receive(10.5)
            send('drink status')
        elif a.test in ('run','awake'):
            probe=fields(find(send('probe'),'PROBE '))
            if 'rf' in a.label:assert probe['field']=='0',probe
            if a.test=='run':run(a.seconds,a.label)
            else:
                log('WINDOW_READY',label=a.label,seconds=a.seconds)
                if a.cue:send('progress 0',.05)
                receive(a.seconds)
            send('counters');send('drink status')
        elif a.test=='auto':
            utc=datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M:%S')
            log('UTC_SOURCE',source='Windows PC UTC; NTP accuracy unverified',utc=utc)
            assert 'OK setutc' in send('setutc '+utc)
            before=fields(find(send('clock'),'CLOCK '))
            assert before['valid']=='1'
            cube('-rst','-score','-run');receive(.3)
            after=fields(find(send('clock'),'CLOCK '))
            assert after['valid']=='1' and int(after['utc'])>=int(before['utc'])
            for key in ('bkp0','bkp1','bkp2','bkp3','bkp4'):assert before[key]==after[key],key
            assert fields(find(send('drink status'),'DRINK '))['calibration']=='0'
            anchor_time=time.monotonic();anchor=int(after['monotonicMs'])
            for level in range(9):
                send(f'progress {level}',.05);receive(.45 if level==0 else 3.1)
                state=fields(find(send('status'),'APP '))
                assert state['progress']==str(level) and state['display']==state['pwm']=='0'
            send('progress 5',.05)
            send(b'invalid',.05);send(b'x'*80,.05);send(b'sh\0ow',.05)
            assert fields(find(send('status'),'APP '))['progress']=='5'
            send('off');assert fields(find(send('status'),'APP '))['sampling']=='0'
            run(10,'still10');run(60,'still60');send('off');run(1,'restart')
            clock=fields(find(send('clock'),'CLOCK '))
            assert abs((int(clock['monotonicMs'])-anchor)-(time.monotonic()-anchor_time)*1000)<2000
        elif a.test=='controls':
            assert 'OK observe diagnostic_not_motion_wake' in send('observe',.05)
            assert fields(find(send('status'),'APP '))['sampling']=='1'
            assert any(s.startswith('ERR ') for s in send('invalid',.05))
            assert any(s.startswith('ERR too_long') for s in send(b'x'*80,.05))
            assert fields(find(send('status'),'APP '))['sampling']=='1'
            send('mode debug')
            assert fields(find(send('status'),'APP '))['sampling']=='0'
            regs=fields(find(send('lisregs'),'LISREG '))
            assert regs==dict(ok='1',ctrl1='20',ctrl2='0C',ctrl3='10',ctrl4='20',ctrl5='00',ctrl6='00')
            send('progress 5',.05);send('show',.05);send('off')
            state=fields(find(send('status'),'APP '))
            assert state['progress']=='5' and state['pwm']==state['display']==state['sampling']=='0'
            regs=fields(find(send('lisregs'),'LISREG '))
            assert regs['ctrl1']==regs['ctrl4']=='00'
            run(1,'controls_restart')
        log('EVIDENCE',status=send('status'),counters=send('counters'))
        result='PASS'
    except Exception as exc:log('ERROR',error=repr(exc))
    finally:
        if uart:
            try:
                if runUntil:receive(max(0,runUntil-time.monotonic()))
                send('off');send('status')
            except Exception as exc:log('CLEANUP_ERROR',error=repr(exc));result='FAIL'
            uart.close()
        report=dict(result=result,label=a.label,split=a.split,uart_port=a.uart_port,bottle='empty direct-drink; stage5 mounting retained',
                    image_sha256=hashlib.sha256(a.image.read_bytes()).hexdigest(),records=rows,
                    limits='Transport/acquisition PASS is not recognition acceptance; operator confirmation required')
        (a.results/'result.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
        (a.results/'raw.json').write_text(json.dumps(samples),encoding='utf-8')
    return int(result!='PASS')
if __name__=='__main__':raise SystemExit(main())
