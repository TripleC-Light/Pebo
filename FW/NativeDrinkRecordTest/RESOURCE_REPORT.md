# Post-stage7 resource comparison (2026-10-26)

Local resource work, not committed/pushed. PC-02 TripleC, main HEAD aac139c.
STM32L031G6U6TR Flash32768/RAM8192; original linker/startup/Board unchanged.
No schedule, reminder, NFC app protocol, water intake acceptance or parameter tuning.

## Baseline and checkpoint

Rebuilt 1001ea0 stage7 source with xPack GCC14.2.1; original ELF/map and compiler
commands archived under build/resource-baseline and resource-evidence/baseline-build-report.json.
Reproduced 32712B image SHA c648aef249ce4e2f1c3bc419291a5c546f3c979febcd40c35983879e5703e958.
Actual full32K before changes SHA 1e911996c6a277b394a72e20e36189e1d7532ef0abfbedf4f973caa088c1348b:
image prefix matches, remaining56 bytes are ZERO, not FF.
Annotated checkpoint/native-drink-record-v1 object05f3aa1a61dcb2b6e05f300befa7c3bc0b51eeec
points1001ea0e34e51ccb73634a40e177a938b0ddcde0 and was pushed without force.
**Annotation correction:** this session initially asserted an FF tail incorrectly;
the assertion failed but the shell continued to the authorized tag push. Its annotation
therefore incorrectly says FF. Correct evidence is baseline-board/verification.json (00 tail).
Source commit, image and full-read hashes were correct; tag was not overwritten.
Historical stage7 empty motions partly used output-fix-predecessor9f5a869f… (32704B),
not all final c648aef…; retain original validation scope.

## Actual sections (bytes)

|Section/cost|Original|diagnostic|compact|
|---|---:|---:|---:|
|vectors|192|192|192|
|.text|28060|26432|24312|
|.rodata|4348|4440|3148|
|.ARM.exidx|8|8|8|
|.data initial values in Flash AND RAM|104|24|24|
|Flash used|32712|31096|27684|
|Flash remaining|56|1672|5084|
|.bss|5836|5820|3232|
|static RAM (.data+.bss)|5940|5844|3256|
|RAM after static (not safe object budget)|2252|2348|4936|
|minimum stack reservation|1024|1024|1024|
|heap|0|0|0|
|runtime stack peak|NOT_MEASURED|NOT_MEASURED|NOT_MEASURED|

Debug/DWARF sections are not LOAD and consume no target Flash. Reports include VMA/LMA;
.data costs both memories. Heap/stack section adds alignment (diagnostic1028, compact1024).
-Os/-flto/-ffunction-sections/-fdata-sections/-Wl,--gc-sections actually appear in archived
commands; LTO ltrans objects and discarded input sections demonstrate active LTO/GC.
Memory limits not enlarged. Compact frees5028B Flash/2684B static RAM relative original.
5084B spare is ~4.96KiB, below6144B aspiration by1060B. Stage8 can begin scoped design,
but fit is not assured; retain resource checks and avoid treating all free RAM as available.

## Separate measured steps

|Step|Flash|Static RAM|Effect vs preceding stated reference|
|---|---:|---:|---|
|Original|32712|5940|exact baseline rebuild|
|Disable inline-called-once experiment|33768|not retained|REJECTED, Flash overflow +1056; original flags restored|
|Replace stdio formatting only|30808|5844|save1904 Flash/96 RAM vs original|
|Explicit diagnostic profile/identity|31040|5844|+232 Flash vs formatting-only|
|Profile compact before UART hardening|27592|3256|save3448 Flash/2588 RAM vs diagnostic|
|Private noinline parser wrapper|27620|3260|REJECTED +28 Flash/+4 RAM vs compact; reverted|
|UART bounded response hardening diagnostic|31096|5844|+56 Flash; no static RAM change|
|UART bounded response hardening compact|27684|3256|+92 Flash; no static RAM change|

Single-feature exclusions from diagnostic31040 are experiments, **not full diagnostic**:
trace30172/static4424 (save868/1420); synthetic29940/static5836(save1100/8);
dumps30080/static5844(save960/0); verbose30508/static5844(save532/0).
These deltas include variant identity strings and LTO sharing; do not add them together.
Trace2560 and synthetic1156 share a union; removing synthetic alone cannot recover1156 RAM.
Compact removes the whole union and associated trace state. TX1024 and hardwareRX/TX64 each retained.
A smaller RAM buffer was not presented as a Flash optimization.

## Link/map attribution and retained costs

Full symbols, disassembly, selected archive members, stack frames and source string locations
are in resource-evidence/{diagnostic,compact}-*. Original library members are separately saved. Map archive selections include inputs later
removed by GC (e.g. errno/impure); final nm symbols and .data/.bss determine surviving cost.
LTO inlines parser, classifier and much records formatting into service; exact function-only
algorithm/diagnostic attribution is not additive. Controlled feature ablations above measure
actual linked marginal cost rather than guessing from function names.
Diagnostic service9460B; Console::begin1076; USART2_IRQHandler840; source initialization672;
main660; Progress::apply564; SystemClock_Config492; setUTC464; signed division460;
64-bit division408; Board::write404; Records::Pending::confirm392; I2C read392;
Text::format344. Compact actual ranking is final-compact.json, rather than assuming equality.
Largest strings: full drinkReport197B, records status181, help167, APP148, base status138
(including NUL); source-strings.json maps each to source line. rodata also includes parser
literals, error reasons, register constants and algorithm tables; strings totals are not all rodata.
Approximate instruction attribution using actual DWARF line annotations (diagnostic/compact):
DrinkLogic1612/1424B, ShakeLogic752/848B, DrinkRecords1096/1132B,
Observation960/636B, AppParser406/314B, main6370/4894B, TextFormat328+218B each.
See dwarf-source-attribution.json: inlined code and literal pools can be assigned to a
neighboring source line; these are attribution estimates, not removable standalone costs.
LTO changes attribution despite identical core source. main includes UART/control/query code;
UART HAL begin1076+ISR840 are exact named-symbol costs, with shared HAL helpers additional.
Actual feature ablations remain the useful measure of trace/test/dump savings.

Largest RAM objects: realRecords1156, TX1024, diagnostic union2560 only diagnostic,
UART HAL handle136, I2C handle84, sources84, shake detector76, drink detector64,
hardwareRX64/TX64, PWM HAL handle64, RTC36, parser35, snapshot32, pending32.
No reduction of32 events x32B or four24B summaries; Store1156B. No unsafe packed accesses.

Original snprintf pulled nano stdio, reentrancy/impure state and allocator support
(_svfprintf_r/_printf_i, malloc/realloc/free family), despite heap0. Board::_sbrk returns
-1/ENOMEM; heap0 never promised those link dependencies absent. New bounded integer
Text::format removes stdio/allocator references; both final ELF allocation-symbol lists empty,
no printf/snprintf. Division/memcpy/memset/strlen support remains where actually needed.
Formatter supports only project formats %s/%d/%u/%X, optional l/width0..16/zero padding/%%;
no floats, locale, %n or general serialization. Signed minimum and tiny/zero buffers tested
against actual newlib snprintf in ARM emulation:621 cases per profile plus invalid-format guards.

## UART safety and costs

TX remains1024 (usable1023), command admission requires900 free bytes. Fixed parser35B,
record page snapshot1 event32B, at most one list row per service iteration; RTC/GPO/application
processing continues every iteration. Enlarged transient drinkReport buffer256->288: full
boundary response needs up to275 bytes; no intentional truncation. No blocking bulk dump.
Automatic counters previously could append to a simultaneous query/run-end. They are now
pending until900B free, also preventing STOP before flushing. Explicit counters consumes pending
flag. Bounded blocks: status+APP+full DRINK608B, full counters555B; largest four-summary query
~651B plus run-end122/cancel reason fits admission. This is capacity reasoning, not runtime
stack measurement. Existing UART RX does NOT wake STOP; commands sent asleep may be lost,
which must not be mistaken for a TX regression or claimed concurrent-query coverage.

## Stack analysis: estimates, not a measured bound

Compiler -fstack-usage ltrans frames and actual BL call edges archived. Diagnostic frames:
main32/service488/counters416/drinkReport368/clock232/format56/number72.
Compact main32/service448/counters216/drinkReport336/clock232/format56/number72.
Representative diagnostic main->service->counters->format->number sum1064B;
compact main->service->drinkReport->format->number sum944B. These are partial chain
estimates, excluding division/library frames, alignment and interrupts; NOT peak or safety proof.
I2C path includes Board wrappers, HAL read40, error-check40, wait32/24; query path may
also call RTC validity88, epoch48. Do not sum mutually exclusive branches.
USART ISR software frame32; EXTI/RTC8 plus record16. Hardware exception entry32B each
(with possible alignment). RTC priority0 can preempt UART/EXTI/SysTick priority3 (actual HAL TICK_INT_PRIORITY3); callback/handler
and error-path library frames also need inclusion. Compiler frames alone do not prove a
complete worst case;1024 reservations are not runtime measurements.
Diagnostic partial1064 already exceeds minimum1024, though2348 RAM after static remains;
reserve must not be treated as a proven bound. No stack painting was added: pinned startup
has no independently validated early unused-stack paint hook, and painting active stack is unsafe.
Both normal images have no watermark instrumentation; runtime peak remains NOT_MEASURED.
Future options: validated early-startup watermark + stress, targeted UART HAL setup/IRQ cost,
or bounded protocol text redesign with separately measured deltas. No broad driver rewrite here.

## Configuration contract

Build requires explicit --config diagnostic|compact; invalid/missing options fail.
Same DrinkLogic/ShakeLogic/DrinkRecords/TimeSupport/ElapsedLogic and Board/linker sources.
Feature masks diagnostic15/compact0 in boot and status. Single application main, no forked algorithms.
Both: status,counters,mode debug,run1..60,off,progress0..8,show,clock,setutc,
calibration start/status,drink status/counters,records status/today/list0..31/clear confirm.
Diagnostic additionally: capture5..30,trace,observe,probe,ledregs,lisregs,counters clear,
records test enter/add/day/exit. Compact excludes literals/code/trace buffer/synthetic store
at compile/link time; rejected commands cannot enter synthetic paths. Compact counters retain
RTC/motion/GPO source statistics, omitting detailed SHAKE line; drink status is shortened.
Clock/BKP diagnostics retained for RTC reset verification. Compact is a development configuration,
not a validated production release. Observation counters are not intake or task-completion measures.

See RESOURCE_VALIDATION.md for exact physical coverage, final image and remaining limits.
