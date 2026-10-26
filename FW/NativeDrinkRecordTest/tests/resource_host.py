"""Execute actual ARM formatter and pure policies in a host Unicorn emulator.
No MCU registers, physical stack peak or real-drinking validation is claimed.
"""
import argparse,ast,json,re,struct,subprocess,sys,hashlib
from pathlib import Path
P=Path(__file__).resolve().parents[1]
def main():
 a=argparse.ArgumentParser();a.add_argument('--toolchain-bin',type=Path,required=True);a.add_argument('--config',choices=['diagnostic','compact'],required=True);x=a.parse_args()
 sys.path.insert(0,str(P/'build/host-deps'))
 import unicorn
 from unicorn.arm_const import UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_R0
 out=P/'build'/('host-'+x.config);out.mkdir(exist_ok=True)
 def tool(n):return str(x.toolchain_bin/('arm-none-eabi-'+n+'.exe'))
 def run(c):
  r=subprocess.run(list(map(str,c)),capture_output=True,text=True,timeout=90)
  if r.returncode:raise RuntimeError(r.stdout+r.stderr)
  return r.stdout
 maintext=(P/'src/main.cpp').read_text(encoding='utf8')
 formats=sorted(set(ast.literal_eval('"'+s+'"') for s in re.findall(r'Text::format\(\w+,sizeof\(\w+\),"((?:\\.|[^"\\])*)"',maintext)))
 formats+=['%d %ld %u %lu %X %08lX %04d','%s %% %02u','%4d %04d']
 cases=[]
 for fmt in formats:
  for mode in range(3):
   args=[]
   for m in re.finditer(r'%(0?)(\d*)(l?)([sduX])',fmt):
    kind=m[4];wide=bool(m[3]);signed=kind=='d'
    if kind=='s':args.append('"SYNTHETIC"');continue
    v=['0','(-2147483647-1)','2147483647'][mode] if signed else ['0','4294967295','305419896'][mode]
    args.append('static_cast<'+('long' if signed and wide else 'int' if signed else 'unsigned long' if wide else 'unsigned')+'>('+v+')')
   arg=','+','.join(args) if args else ''
   cases.append('for(unsigned cap: {0u,1u,2u,7u,31u,63u,128u,256u,384u}){\n'+
    'for(unsigned i=0;i<512;++i){a[i]=b[i]=char(0x55);}\n'+
    'int u=Text::format(a+1,cap,'+json.dumps(fmt)+arg+');int v=snprintf(b+1,cap,'+json.dumps(fmt)+arg+');\n'+
    '++cases;if(u!=v)return 1000+cases;for(unsigned i=0;i<512;++i)if(a[i]!=b[i])return 2000+cases;}\n')
 source='#include <stdio.h>\n#include <initializer_list>\n#include "'+(P/'src/TextFormat.h').as_posix()+'"\n'
 source+='#define main core_main\n#include "'+(P/'tests/RecordChecks.cpp').as_posix()+'"\n#undef main\n'
 source+='#include "'+(P/'tests/LogicChecks.cpp').as_posix()+'"\n#include "'+(P/'tests/ConfigChecks.cpp').as_posix()+'"\n'
 source+='extern "C" void *_sbrk(int){return (void*)-1;}\nextern "C" unsigned test_entry(){if(core_main()||!configCommands())return 1;char a[512],b[512];unsigned cases=0;\n'
 source+=''.join(cases)
 source+='for(const char *fmt:{"%f","%n","%*u","%17u","%"}){a[0]=a[2]=char(0x55);if(Text::format(a+1,1,fmt)!=-1||a[0]!=char(0x55)||a[1]!=0||a[2]!=char(0x55))return 3;}\n'
 source+='if(Text::format(nullptr,0,"%lu",4294967295ul)!=10||Text::format(nullptr,1,"x")!=-1)return 4;return 0;}\n'
 src=out/'runner.cpp';src.write_text(source,encoding='utf8');elf=out/'runner.elf'
 run([tool('g++'),'-mcpu=cortex-m0plus','-mthumb','-std=c++17','-Os','-fno-exceptions','-fno-rtti','-nostartfiles',
  '-DPEBO_DIAGNOSTIC='+str(int(x.config=='diagnostic')),'-DPEBO_BUILD_NAME="'+x.config+'"',src,P/'src/TextFormat.cpp',
  '--specs=nano.specs','--specs=nosys.specs','-Wl,-Ttext=0x08000000,-Tdata=0x20000000,-e,test_entry','-o',elf])
 data=elf.read_bytes();phoff=struct.unpack_from('<I',data,28)[0];phsize,phnum=struct.unpack_from('<HH',data,42)
 emu=unicorn.Uc(unicorn.UC_ARCH_ARM,unicorn.UC_MODE_THUMB|unicorn.UC_MODE_MCLASS)
 emu.mem_map(0x08000000,0x100000);emu.mem_map(0x20000000,0x100000)
 for i in range(phnum):
  typ,offset,vaddr,paddr,size,memsz,flags,align=struct.unpack_from('<8I',data,phoff+i*phsize)
  if typ==1 and size:emu.mem_write(vaddr,data[offset:offset+size])
 nm=run([tool('nm'),elf]);entry=int(re.search(r'(?m)^([0-9a-f]+) T test_entry$',nm)[1],16)
 emu.reg_write(UC_ARM_REG_SP,0x200ff000);emu.reg_write(UC_ARM_REG_LR,0x080ff001)
 emu.emu_start(entry|1,0x080ff000,count=50000000)
 result=emu.reg_read(UC_ARM_REG_R0);assert result==0,('runtime mismatch',result)
 report=dict(result='PASS',config=x.config,method='actual ARM executable on host Unicorn, compared against linked newlib-nano snprintf; no physical hardware/stack peak',unicorn=unicorn.__version__,formats=len(formats),golden_cases=len(formats)*3*9,invalid_formats=5,core='same RecordChecks/LogicChecks/ConfigChecks, plus runtime pure-policy entry',format_source_sha256=hashlib.sha256((P/'src/TextFormat.cpp').read_bytes()).hexdigest())
 (P/'resource-evidence'/('host-'+x.config+'.json')).write_text(json.dumps(report,indent=2));print(json.dumps(report))
if __name__=='__main__':main()
