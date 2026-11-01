# Transient by Veff

A stereo-linked attack/sustain shaper with drive and selectable clipping for
Open-XY. Implemented as a standalone send effect (`transient`) using the public
SDK 1.8 API. No SDK modifications or firmware addresses.

## Controls

| Control | Range | Default |
| --- | --- | --- |
| Attack | −100 to +100 | 0 |
| Sustain | −100 to +100 | 0 |
| Drive | 1× to 8× | 1× |
| Output | 0× to 2× | 1× |
| Shift + E1: Response | Fast / Medium / Slow | Fast |
| Shift + E2: Clip | Off / Soft / Hard | Off |

Fast and slow peak envelopes measure the louder stereo channel. Their normalized
positive difference controls attack gain; its complement controls body/sustain.
The same smoothed gain shapes both channels, retaining their stereo relationship.
At either shaping extreme, nominal gain ranges from 0.25× to 4×. Response changes
both detector time scales; it is not an automatic transient classifier.

Signal order is linked detector → attack/sustain gain → drive → clipper → output.
Soft clipping is unity below half scale, with a continuous parabolic knee ending
at full scale. Hard clipping clamps at full scale. Off disables this intentional
clipper, but the final Q31 return still saturates safely when overloaded.
Controls glide over approximately 3 ms. Neutral settings are bit-exact passthrough.
There is no oversampling or intersample-peak detection; this is not a true-peak
limiter. Distortion can alias, and Output can overload the wet return after clipping.

`dryLevel: 0` makes full send process the source entirely. Partial send retains
some parallel dry audio. The wet clipper cannot limit that dry path or the final
master output.

## Starting settings

The four normal-control settings are also recorded in `definition.json`.
Effect presets are reference settings, not a promise of a stock effect-preset browser.

| Setting | Attack | Sustain | Drive | Output | Clip |
| --- | --- | --- | --- | --- | --- |
| Neutral | 0 | 0 | 1× | 1× | Off |
| Punch | +60 | −30 | 2.05× | 0.70× | Soft |
| Body | −20 | +50 | 1.70× | 0.60× | Soft |
| Tame | −80 | 0 | 1× | 1× | Off |

## Build and validation

Use the Open-XY SDK on staging, tested with
`7fd8fdf1757e11ab6fdca915b09f1773d7611a31`:

```sh
TMPDIR=/private/tmp python3 /path/to/open-xy/sdk.py test --machine .
```

Host tests cover exact neutral transfer, burst attack and sustained-body gain,
linked stereo, extreme samples, soft-knee continuity, silence and repeatable reset.
AddressSanitizer and UndefinedBehaviorSanitizer tests pass. State is 32 bytes,
well below the 472-byte allowance with a shift row; no scratch buffer is used.
Native compilation is checked alongside the official 2op FM fixture: the current
SDK has an effect-only startup unused-function warning, so a mixed collection is
required without changing the SDK.

The SDK host estimate is approximately 29,000 Blackfin instructions per 64-sample
block at its heaviest tested setting. This is not measured device CPU. Listening,
device timing and hardware acceptance remain pending. Effects have no SDK browser
audition. Firmware and stock data stay outside this source repository; this repo
contains no installable firmware or catalog submission.
