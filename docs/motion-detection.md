# Drinking motion detection — experimental

The accelerometer observes bottle motion. It cannot directly detect water intake, especially when a child drinks through a straw while the bottle remains upright. Present results as **detected drinking-like events** until field testing supports stronger claims.

## First implementation to evaluate

- Wake on meaningful motion, capture a short time series, and compute tilt from the gravity estimate when movement is low enough for that estimate to be useful.
- Use a state machine such as resting → pickup → candidate use → return/rest, with timeouts and a minimum separation between events.
- Consider tilt, duration, acceleration magnitude, motion variability, and return-to-rest together; tilt alone is insufficient.
- Record compact diagnostic features in development builds and optionally an internal confidence/quality score. A numeric score is not automatically a calibrated probability.

## Training and validation data

Collect labeled use cases across bottle shapes and mounting positions: normal drinking, straw drinking, carrying, shaking, setting down, backpack movement, and classroom handling. Measure false positives and missed drinks separately. Set thresholds using real samples, then replay the same traces after firmware changes.

## Decisions pending

### Stage6 prototype evidence (2026-10-26)

Implemented independently in FW/NativeDrinkDetectionTest, native HAL only;
see [validation](../FW/NativeDrinkDetectionTest/VALIDATION.md). RAM upright
calibration, shared motion sampling, integer tilt/hold/return, shake exclusion
and10s deferred sip merging are engineering prototype behavior, not confirmed
product thresholds. First cohort uses EMPTY direct-drink bottle simulations.
Three independently confirmed three-sip groups each produced3 candidates/1
event;10 shakes produced no drink candidates. Actual C++ measured-data replay
and STOP merge/RF/RTC regressions are recorded. Do not infer actual intake,
volume, pour rejection or upright straw detection. Single-event10-trial cohort,
real-water/population/fill/mount robustness and power remain pending.

Sampling rate, interrupt configuration, classifier thresholds, reminder suppression, confidence representation, and how straw use affects product claims all require prototype data.

### Stage7 record integration (2026-10-26)

Classifier/thresholds remain frozen from NativeDrinkDetectionTest. Independent
NativeDrinkRecordTest stores only confirmed10s groups, using FIRST VALID CANDIDATE
COMPLETION (not tilt start) for event occurrence/local day; last completion and
delayed output remain distinct. Candidate count is not sips/ml. User-confirmed
empty single1/1,three candidates/one record,shake/no record,cancelled/no record
are integration-image evidence; invalid trials excluded. A final UART-buffer-only
fix and final automatic/RF checks have separate SHA scopes in VALIDATION.md.
No water/intake acceptance, retuning or task/progress interpretation is implied.
