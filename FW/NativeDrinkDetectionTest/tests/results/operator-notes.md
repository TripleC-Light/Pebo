# Operator confirmations

All stage6 physical gestures use EMPTY direct-drink bottle at user request;
no real-water intake validation. User maintains stage5 installed direction.
No millilitre or hydration claim. Dates Asia/Taipei.

## 2026-10-25

- User supplies2.8V, same STLINK/COM5/common ground, permits flash/reset.
- train-single02: user confirms exactly one simulated sip, immediately after
  start prompt, NOT intentional shake. Initial SHAKE classification is a failure.
- train-pickup01: user confirms one normal pickup/place.
- train-shake01: user confirms one intentional shake.
- train-tilt01: user confirms one slow tilt.
- train-three01: user later says motions were not correctly executed. Exclude
  this attempt from sensitivity/miss rates; retain original trace and failed
  exploratory replay results.800ms hold trial reverted to1200ms.
- train-three02: user confirms all three performed correctly. Three candidates
  merged into one event, training only.
- validate-single-stop01: user confirms one simulated sip completed.
- validate-pickup5a and5b: user confirms five each, all no LED (ten total).
- validate-shake5a: user confirms one preliminary plus three formal shakes.
  Last formal display reached60s deadline, do not accept full3s duration there.
- Planned120s shake test did not start: tool approval review hit usage limit.

## 2026-10-26

- User reconfirms own2.8V/STLINK/common ground, empty bottle/same direction,
  UART reconfigured, permits UART/flash. Fresh single Prolific identified COM13;
  no Nordic COM6/7 access. Recalibration after ordinary reset succeeds.
- 20261026-shake5a: user explicitly confirms all five correct level5 displays
  and about3s then off. Five firmware confirmations, zero drink candidates.
- 20261026-shake5b: user explicitly confirms all five correct displays/timeout.
  Final window: five firmware confirmations, zero drink candidates/events,
  zero errors; off/debug/UART closed. Combined final-image shake cohort10/10.
- 20261026-three1: user says prompt seen late and motions not correctly
  executed, requests retry. Exclude from accuracy. Raw captured3015 samples;
  no candidate, later motion outside capture. No inference of three misses.
- Following captures use explicitly logged existing manual progress0 double
  flash as operator start cue, not automatic recognition output or new firmware.
- 20261026-three1-retry: user confirms correct three-sip flow after visible cue;
 3 candidates/1 event, exactly10s after last candidate, zero shake/errors/drops.
 Independent actual C++ replay of3015 raw samples reproduces3/0. User ready
 for second group; no confirmation yet for subsequent groups.
- 20261026-three2 /three3: user explicitly confirms all three sips in each
  group correctly performed. Each has3 candidates/1 event, exact10s defer,
  zero shake/errors/drops and3015 raw rows; actual shared C++ replay3/0 PASS.
- Operator instruction clarified:2s means HOLD tilted,1s means HOLD upright,
  not rotation duration. Pick up once at first sip, no need to place on table
  between sips, put down after third. Natural angle, do not force larger tilt
  just to pass. User chose empty simulation, no actual water ingestion.
- 20261026-mixed: user confirms one simulated sip followed by phone approach/
  removal and successful tag detection. MCU records independent motion/GPO
  STOP wake,1 candidate/1 deferred event. No inferred physical gestures added.
- Final state off/debug, COM13 closed. No additional human testing after this
  final mixed round. Expanded individual10-event acceptance remains pending.
