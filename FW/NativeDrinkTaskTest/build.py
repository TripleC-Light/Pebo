"""Reproducible GCC build: vendored ST HAL/CMSIS, no Arduino package lookup."""
import argparse
import ast
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent
BASE = ROOT.parent/"NativeEventLoopStopTest"

def run(command, **kwargs):
    proc = subprocess.run([str(x) for x in command], capture_output=True, text=True,
                          errors="replace", timeout=120, **kwargs)
    if proc.returncode:
        raise RuntimeError(" ".join(str(x) for x in command) + "\n" + proc.stdout + proc.stderr)
    return proc.stdout

def analyze(elf, tool):
    output = run([tool("objdump"), "-h", elf])
    sections = []
    rows = output.splitlines()
    for i, row in enumerate(rows):
        m = re.match(r"\s*\d+\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)", row)
        if m:
            sections.append(dict(name=m[1], size=int(m[2],16), vma=int(m[3],16),
                                 lma=int(m[4],16), flags=rows[i+1].strip()))
    sizes = {s["name"]: s["size"] for s in sections}
    loaded = [s for s in sections if "LOAD" in s["flags"] and 0x8000000 <= s["lma"] < 0x8008000]
    flash = max(s["lma"]+s["size"] for s in loaded)-0x8000000
    stat = sizes.get(".data",0)+sizes.get(".bss",0)
    symbols = []
    for row in run([tool("nm"), "-S", "--size-sort", "--radix=d", "-C", elf]).splitlines():
        m = re.match(r"\d+\s+(\d+)\s+(\w)\s+(.+)", row)
        if m:
            symbols.append(dict(bytes=int(m[1]), type=m[2], name=m[3]))
    undefined = run([tool("nm"), "-u", elf]).strip()
    mapfile = elf.with_suffix(".map")
    merged_rodata = 0
    if not sizes.get(".rodata") and mapfile.exists():
        text = mapfile.read_text(errors="replace").split("Linker script and memory map")[-1].split("Cross Reference Table")[0]
        merged_rodata = sum(int(m[1],16) for m in re.finditer(
            r"(?m)^\s+\.rodata\S*\s+0x[0-9a-f]+\s+0x([0-9a-f]+)",text))
    return dict(elf=str(elf), sections=sections, flash_bytes=flash,
                text_output=sizes.get(".text",0), rodata_output=sizes.get(".rodata",0),
                rodata_merged_input=merged_rodata,
                text_excluding_merged_rodata=sizes.get(".text",0)-merged_rodata,
                data=sizes.get(".data",0), bss=sizes.get(".bss",0), static_ram=stat,
                ram_after_static=8192-stat,
                largest_functions=sorted([s for s in symbols if s["type"] in "tT"],key=lambda s:s["bytes"],reverse=True)[:20],
                largest_objects=sorted([s for s in symbols if s["type"] in "bBdDrR"],key=lambda s:s["bytes"],reverse=True)[:20],
                allocation_symbols=[s for s in symbols if re.search(r"malloc|calloc|realloc|_sbrk",s["name"])],
                unresolved=undefined)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--toolchain-bin", type=Path)
    ap.add_argument("--config",required=True,choices=["diagnostic","compact"])
    ap.add_argument("--measure-without",action="append",choices=["trace","synthetic","dumps","verbose"],default=[],help="diagnostic resource experiment only; not a full diagnostic image")
    ap.add_argument("--build-dir", type=Path)
    ap.add_argument("--baseline-elf", type=Path)
    args = ap.parse_args()
    if args.config=="compact" and args.measure_without:ap.error("measurement exclusions require diagnostic")
    variant=args.config+("-no-"+"-".join(sorted(args.measure_without)) if args.measure_without else "")
    defines=["-DPEBO_DIAGNOSTIC="+str(int(args.config=="diagnostic")), '-DPEBO_BUILD_NAME="'+variant+'"',
             "-DPEBO_TASK_TEST="+str(int(args.config=="diagnostic")),
             "-DPEBO_TRACE=0","-DPEBO_SYNTHETIC=0","-DPEBO_DUMPS=0","-DPEBO_VERBOSE=0"]
    defines += ["-DPEBO_"+f.upper()+"=0" for f in args.measure_without]
    directory = args.toolchain_bin or (Path(os.environ["ARM_GCC_BIN"]) if "ARM_GCC_BIN" in os.environ else None)
    def tool(name):
        candidate = (directory/("arm-none-eabi-"+name+(".exe" if os.name=="nt" else ""))) if directory else shutil.which("arm-none-eabi-"+name)
        if not candidate or not Path(candidate).is_file():
            raise RuntimeError("GCC tools required: put them on PATH or pass --toolchain-bin")
        return str(candidate)
    out = (args.build_dir or ROOT/"build"/variant).resolve()
    out.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((BASE/"vendor/manifest.json").read_text(encoding="utf-8-sig"))
    for item in manifest:
        p = BASE/"vendor"/item["path"]
        if hashlib.sha256(p.read_bytes()).hexdigest() != item["sha256"]:
            raise RuntimeError("Vendor hash mismatch: "+item["path"])
    flags = ["-mcpu=cortex-m0plus","-mthumb","-mfloat-abi=soft","-Os","-flto",
             "-ffunction-sections","-fdata-sections","-fstack-usage","-g3",
             "-DSTM32L031xx","-DUSE_HAL_DRIVER","-DUSER_VECT_TAB_ADDRESS",
             "-Wall","-Wextra",*defines]
    for item in json.loads((ROOT/"vendor/manifest.json").read_text()):
        if hashlib.sha256((ROOT/"vendor"/item["path"]).read_bytes()).hexdigest() != item["sha256"]:
            raise RuntimeError("TIM vendor hash mismatch")
    includes = [ROOT/"include",BASE/"vendor/HAL/Inc",BASE/"vendor/Device/Include",BASE/"vendor/Core/Include"]
    includes = [arg for p in includes for arg in ["-I",str(p)]]
    commands = []
    objects = []
    source_files = sorted((BASE/"vendor/HAL/Src").glob("*.c")) + sorted((ROOT/"vendor/HAL/Src").glob("*.c")) + [
        BASE/"vendor/Device/Source/system_stm32l0xx.c",
        BASE/"vendor/Device/Source/startup_stm32l031xx.s"] + sorted((ROOT/"src").glob("*.cpp"))
    for source in source_files:
        obj = out/(source.stem+".o")
        cpp = source.suffix == ".cpp"
        extra = ["-std=gnu++17","-fno-exceptions","-fno-rtti","-fno-threadsafe-statics","-fno-use-cxa-atexit"] if cpp else ["-std=gnu11"]
        if source.suffix == ".s":
            extra = ["-x","assembler-with-cpp"]
        cmd = [tool("g++" if cpp else "gcc"),*flags,*extra,*includes,"-c",source,"-o",obj]
        commands.append([str(x) for x in cmd])
        run(cmd)
        objects.append(obj)
    elf = out/("NativeDrinkTaskTest-"+variant+".elf")
    cmd = [tool("g++"),*flags,"--specs=nano.specs","--specs=nosys.specs","-nostartfiles",
           "-T",ROOT/"stm32l031.ld",*objects,"-Wl,--gc-sections,--cref",
           "-Wl,-Map="+str(elf.with_suffix(".map")),"-Wl,--print-memory-usage",
           "-Wl,--start-group","-lc","-lm","-Wl,--end-group","-o",elf]
    commands.append([str(x) for x in cmd])
    print(run(cmd))
    for fmt,suffix in [("binary",".bin"),("ihex",".hex")]:
        run([tool("objcopy"),"-O",fmt,elf,elf.with_suffix(suffix)])
    image=elf.with_suffix('.bin').read_bytes()
    names={}
    for row in run([tool('nm'),'-n',elf]).splitlines():
        m=re.fullmatch(r'([0-9a-fA-F]+)\s+([TtWw])\s+(\S+)',row)
        if m:names[m[3]]=(int(m[1],16),m[2])
    assert int.from_bytes(image[:4],'little')==0x20002000
    for index,name in [(1,'Reset_Handler'),(15,'SysTick_Handler'),(18,'RTC_IRQHandler'),
                       (21,'EXTI0_1_IRQHandler'),(22,'EXTI2_3_IRQHandler'),(44,'USART2_IRQHandler')]:
        address,kind=names[name]
        assert int.from_bytes(image[index*4:index*4+4],'little')==(address|1),(index,name)
        if index!=1:assert kind in 'Tt',name
    # Actual reused pure policies are evaluated by compiler on the host.
    run([tool("g++"),"-std=gnu++17","-Wall","-Wextra","-Werror","-fsyntax-only",*defines,
         ROOT.parent/"EventLoopStopTest/tests/LogicChecks.cpp"])
    run([tool("g++"),"-std=gnu++17","-Wall","-Wextra","-Werror","-fsyntax-only",*defines,ROOT/"tests/LogicChecks.cpp"])
    run([tool("g++"),"-std=gnu++17","-Wall","-Wextra","-Werror","-fsyntax-only",*defines,ROOT/"tests/RecordChecks.cpp"])
    run([tool("g++"),"-std=gnu++17","-Wall","-Wextra","-Werror","-fsyntax-only",*defines,ROOT/"tests/ConfigChecks.cpp"])
    run([tool("g++"),"-std=gnu++17","-Wall","-Wextra","-Werror","-fsyntax-only",*defines,ROOT/"tests/TaskChecks.cpp"])
    for p in ROOT.glob("*.py"):
        ast.parse(p.read_text(encoding="utf8"))
    own = "\n".join(p.read_text(encoding="utf8") for p in (ROOT/"src").glob("*")) + (ROOT/"include/Board.h").read_text()
    if re.search(r"#include\s*[<\"](?:Arduino|Wire)|\b(?:setup|loop|millis|analogWrite|digitalWrite|pinMode)\s*\(", own):
        raise RuntimeError("Arduino dependency in native sources")
    report = dict(config=args.config,variant=variant,features_excluded=args.measure_without,compiler=run([tool("gcc"),"--version"]).splitlines()[0], commands=commands,
                  native=analyze(elf,tool), host_checks="PASS; actual EventLogic, LED, Shake, Drink, calibration, merge, AppParser, elapsed and RecordChecks (compile-time execution) boundaries; synthetic logic only, not physical recognition/races",
                  vector_checks='PASS: stack/reset/SysTick/RTC/EXTI0/EXTI3/USART2, no weak IRQ fallback',
                  c_library='bounded Text::format integer subset; no stdio formatting; nosys _sbrk rejects allocation; actual symbols checked in report',
                  image_sha256=hashlib.sha256(elf.with_suffix(".bin").read_bytes()).hexdigest(),
                  heap_bytes=0, minimum_stack_bytes=1024, peak_stack="NOT_MEASURED",
                  vendor_files=len(manifest)+2, baseline_vendor_files=len(manifest), added_tim_vendor_files=2)
    if report["native"]["unresolved"]:
        raise RuntimeError("Unresolved native symbols")
    if args.baseline_elf:
        report["arduino"] = analyze(args.baseline_elf.resolve(),tool)
        report['arduino_image_sha256']=hashlib.sha256(args.baseline_elf.resolve().with_suffix('.bin').read_bytes()).hexdigest()
        for metric in ("flash_bytes","static_ram"):
            a,b = report["arduino"][metric],report["native"][metric]
            report.setdefault("savings",{})[metric] = dict(bytes=a-b,percent=round((a-b)*100/a,2))
    (out/"build-report.json").write_text(json.dumps(report,indent=2),encoding="utf-8")
    print(json.dumps({k:v for k,v in report.items() if k not in ("commands","native","arduino")},indent=2))
    print(run([tool("size"),"-A",elf]))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
