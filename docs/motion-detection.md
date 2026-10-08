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

Sampling rate, interrupt configuration, classifier thresholds, reminder suppression, confidence representation, and how straw use affects product claims all require prototype data.
