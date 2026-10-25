"""Pebo event STOP bench: user supply only; never opens measurement instruments."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time
import serial
from serial.tools import list_ports

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--uart-port',required=True)
    ap.add_argument('--stlink',required=True)
    ap.add_argument('--programmer',type=Path,required=True)
    ap.add_argument('--image',type=Path,required=True)
    ap.add_argument('--results',type=Path,required=True)
    ap.add_argument('--existing',action='store_true',help='Verify matching image without reflash/reset')
    ap.add_argument('--stimulus',choices=['none','motion','rf','mixed','awake-rf'],default='none')
    ap.add_argument('--window-seconds',type=int,default=20,choices=range(10,61))
    ap.add_argument('--require-field-absent',action='store_true')
    args=ap.parse_args()
    port=next((p for p in list_ports.comports() if p.device==args.uart_port),None)
    if not port or (port.vid,port.pid)!=(0x067b,0x2303):ap.error('Expected verified Prolific UART')
    if not args.image.is_file() or not 0<args.image.stat().st_size<=32768:ap.error('Expected <=32KB image')
    args.results.mkdir(parents=True,exist_ok=False)
    records=[];result='FAIL';uart=None;runDeadline=0
    def log(kind,**data):
        row=dict(kind=kind,monotonic=time.monotonic(),**data);records.append(row);print(json.dumps(row),flush=True)
    def cube(*cmd):
        proc=subprocess.run([str(args.programmer),*cmd],capture_output=True,text=True,errors='replace',timeout=30)
        log('cube',arguments=cmd,exit=proc.returncode,output=proc.stdout+proc.stderr)
        if proc.returncode or re.search(r'^\s*Error:',proc.stdout+proc.stderr,re.M):raise RuntimeError('Cube failed')
        return proc.stdout
    def connection(mode='HOTPLUG'):
        return ['-c','port=SWD',f'sn={args.stlink}',f'mode={mode}','freq=4000']
    def line(timeout=2):
        end=time.monotonic()+timeout;buffer=bytearray()
        while time.monotonic()<end:
            c=uart.read(1);buffer.extend(c)
            if c==b'\n':
                text=buffer.decode('ascii',errors='replace').strip();log('RX',text=text);return text
        raise TimeoutError(bytes(buffer))
    def send(s,expected=None):
        data=s.encode() if isinstance(s,str) else s
        uart.write(data+b'\n');log('TX',data=repr(data));response=line()
        if expected is not None:assert response==expected,(response,expected)
        return response
    def fields(s):return dict(re.findall(r'(\w+)=(\S+)',s))
    def status():
        s=send('status');assert s.startswith('OK mode=debug'),s;return fields(s)
    def run(seconds):
        nonlocal runDeadline
        runDeadline=time.monotonic()+seconds+6
        started=time.monotonic();s=send(f'run {seconds}');assert s.startswith('RUN_BEGIN'),s
        assert line(3)=='RUN_READY rtc_probe=1'
        log('STIMULUS WINDOW READY',stimulus=args.stimulus,seconds=seconds)
        summary=line(seconds+5);assert summary.startswith('RUN_END reason=deadline'),summary
        f=fields(summary);assert int(f['elapsed'])>=seconds and int(f['stops'])>=1,f
        counts={}
        for _ in range(3):
            text=line();assert text.startswith('COUNT'),text
            row=fields(text);counts[row['source']]=row
        assert line()=='END counters'
        assert int(counts['rtc']['wake'])>=1 and int(counts['rtc']['confirmed'])>=seconds
        assert seconds-.5<=time.monotonic()-started<=seconds+5
        state=status()
        assert state['lis']=='1/0' and state['st']=='1/0',state
        log('PASS run',seconds=seconds,summary=f,counts=counts,state=state)
        runDeadline=0
        return counts,state
    try:
        log('identity',port=vars(port),image_sha256=hashlib.sha256(args.image.read_bytes()).hexdigest(),
            power='USER_CONTROLLED_2.8V',instruments='NOT_OPENED')
        probe=cube(*connection('HOTPLUG' if args.existing else 'UR'))
        assert re.search(r'Device ID\s*:\s*0x425',probe,re.I),'Unexpected target'
        if args.existing:
            copy=args.results.resolve()/'readback.bin'
            cube(*connection(),'-u','0x08000000',str(args.image.stat().st_size),str(copy),'-score','-run')
            assert copy.read_bytes()==args.image.read_bytes(),'Wrong image'
        else:
            cube(*connection('UR'),'reset=HWrst','-w',str(args.image.resolve()),'0x08000000','-v','-rst','-score','-run')
        uart=serial.Serial(args.uart_port,115200,timeout=.05,write_timeout=1)
        uart.dtr=uart.rts=False;time.sleep(.3)
        first=status()
        send('help','OK help commands=status,counters,counters clear,mode debug,run 1..60,off,help')
        send('off','OK off');send('counters clear','OK counters_clear')
        if args.require_field_absent:
            diagnostic=fields(send('probe'))
            assert diagnostic.get('ok')=='1' and diagnostic.get('field')=='0',diagnostic
            log('PASS initial RF absent',diagnostic=diagnostic)
        if args.stimulus=='none':
            before=status()
            for invalid in ['run 0','run 61','run -1','run 4294967295','run 1x','bogus','x'*80,b'run 1\x00']:
                send(invalid,'ERR too_long' if isinstance(invalid,str) and len(invalid)>31 else 'ERR invalid')
            after=status()
            for key in ('mode','rtc','lis','st'):assert before[key]==after[key]
            uart.write(b'cou');time.sleep(.05);assert uart.in_waiting==0
            uart.write(b'nters\r\n')
            for _ in range(3):assert line().startswith('COUNT')
            assert line()=='END counters'
            for seconds in (10,60):
                send('counters clear','OK counters_clear');run(seconds)
            send('mode debug','OK debug');send('off','OK off')
            stopped=status();assert stopped['rtc']=='0/0' and stopped['lis'].startswith('0/') and stopped['st'].startswith('0/')
            send('counters clear','OK counters_clear');run(1)
            log('PASS restart/off/parser',first=first)
        elif args.stimulus=='awake-rf':
            run(1)
            send('counters clear','OK counters_clear')
            log('AWAKE RF WINDOW READY',seconds=args.window_seconds)
            time.sleep(args.window_seconds)
            counts={}
            for _ in range(3):
                row=fields(send('counters') if not counts else line())
                counts[row['source']]=row
            assert line()=='END counters'
            state=status()
            assert int(counts['gpo']['isr'])>0 and int(counts['gpo']['confirmed'])>0,counts
            assert int(counts['gpo']['wake'])==0 and int(state['fieldOn'])>0,(counts,state)
            assert state['st']=='1/0',state
            log('PASS awake RF',counts=counts,state=state)
        else:
            counts,state=run(max(30,args.window_seconds) if args.stimulus=='mixed' else args.window_seconds)
            wanted=['motion','gpo'] if args.stimulus=='mixed' else ['motion' if args.stimulus=='motion' else 'gpo']
            for source in wanted:
                assert int(counts[source]['isr'])>0 and int(counts[source]['wake'])>0,(source,counts)
                if source=='motion':assert int(counts[source]['confirmed'])>0,counts
                else:assert int(state['fieldOn'])>0,state
            if 'gpo' in wanted and int(counts['gpo']['confirmed'])==0:
                log('LIMITATION',reason='GPO wake and field sample verified; read-clear FIELD cause unavailable')
        send('off','OK off');status()
        result='PASS'
    except BaseException as exc:log('ERROR',error=repr(exc))
    finally:
        if uart:
            try:
                if result!='PASS':
                    # UART cannot wake STOP. Wait only the known finite test window.
                    wait=max(0,runDeadline-time.monotonic())
                    if wait:log('bounded cleanup wait',seconds=wait);time.sleep(wait)
                    uart.reset_input_buffer();send('off','OK off')
            except Exception as exc:log('CLEANUP ERROR',error=repr(exc))
            uart.close()
        args.results.joinpath('session.json').write_text(json.dumps(dict(result=result,records=records,
            power='unchanged, user controlled',stimulus=args.stimulus),indent=2,ensure_ascii=False,default=str),encoding='utf-8')
    print('RESULT',result)
    return 0 if result=='PASS' else 1

if __name__=='__main__':raise SystemExit(main())
