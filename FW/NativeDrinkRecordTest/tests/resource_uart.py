"""Finite awake query/deadline interleaving; does not send RX into STOP."""
from pathlib import Path
import argparse,json,time
from records_bench import Bench,P

a=argparse.ArgumentParser();a.add_argument('--results',type=Path,required=True);x=a.parse_args();b=Bench(x.results);result='FAIL'
try:
 b.open();b.send('off');b.send('run 1',.05);b.send('progress 5',.05)
 rows=b.rx(.75)
 # Progress display keeps CPU awake until after deadline; queries exercise TX arbitration.
 for _ in range(6):
  b.uart.write(b'status\nrecords today\ncounters\nrecords list\n');rows+=b.rx(.25)
 rows+=b.rx(3)
 assert sum(s.startswith('APP version=') for s in rows)==6
 assert sum(s.startswith('DRINK calibration=') for s in rows)==6
 assert sum(s.startswith('TODAY REAL ') for s in rows)==6
 assert sum(s.startswith('LIST_BEGIN REAL ') for s in rows)==6
 assert sum(s=='LIST_END REAL' for s in rows)==6
 assert sum(s=='END counters' for s in rows)>=6
 assert any(s.startswith('RUN_END reason=deadline') for s in rows)
 assert not any('ERR' in s for s in rows)
 result='PASS'
finally:
 b.close();(b.out/'result.json').write_text(json.dumps(dict(result=result,queries=24,configuration='compact',scope='awake finite responses and deadline, no UART wake-from-STOP claim'),indent=2))
print(result)
