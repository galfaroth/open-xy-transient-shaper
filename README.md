# Transient Shaper for Open-XY

**Status: planning only.** This repository currently contains a design brief, not an implemented or installable effect. No DSP, firmware, presets or measured hardware results are included.

## Goal

Shape attacks and sustain, then control peaks with a selectable soft or hard clipper. Intended for drums, percussion, bass and aggressive leads.

## Proposed MVP signal flow

Stereo input → linked transient detector → attack/sustain gain → drive → Off/Soft/Hard clipper → output gain.

Use a bounded fast/slow envelope design with smoothed gain changes. Stereo-linked detection should preserve the stereo image. Hard clipping must use defined saturating arithmetic; soft clipping should have a continuous transfer curve. Do not describe the device as true-peak limiting: oversampling and intersample peak control are not part of the initial scope.

## Proposed controls

| Primary | Purpose |
|---|---|
| Attack | Reduce or emphasize attacks |
| Sustain | Reduce or emphasize the body/tail |
| Drive | Level into the clipper |
| Output | Final level |

Proposed alternate controls: detector response and clip mode (Off / Soft / Hard). Defaults should be neutral.

Control assignments are a starting design, not a published compatibility contract. Provide at least a neutral/default sound and a few useful settings when implementation begins.

## Routing and resources

Use `dryLevel: 0` for full-send processing. At full send the source can pass entirely through the effect; with a partial send, an unprocessed path remains. A clipper on the wet return cannot limit that parallel dry path or the device's final master output.

Target a standalone Open-XY `effect` machine. The SDK supplies the adapters; this repository owns the DSP, controls, artwork and sound tests. SDK changes, if needed, belong in separate upstream PRs.

The current SDK processes stereo Q31 blocks of 64 samples at 44.1 kHz. Unbuffered state is at most 476 bytes; buffered state at most 472 bytes, with an additional reduction when using a shift row. Audio callbacks cannot allocate, block, perform I/O, log or draw. Do not assume a host estimate proves device timing.

## Implementation handoff

1. Start from a current SDK checkout beside this repository and read its authoring/UX instructions.
2. Generate the official effect starter into a fresh sibling directory, then bring its scaffold into this repository while preserving this brief:
   ```sh
   python3 ../open-xy/sdk.py new --repository --kind effect --id transient --name "Transient Shaper" --out ../open-xy-transient-shaper-starter
   ```
3. Implement the smallest signal path, controls and default settings; keep firmware addresses and platform code out of the effect.
4. Run the machine's tests with `python3 ../open-xy/sdk.py test --machine .` once the scaffold is present. Follow the generated README if its layout changes.
5. Measure memory and worst-case device CPU, then test an explicitly authorized development build before catalog submission.

No catalog entry or upstream PR is part of this planning repository. No firmware or stock content may be committed here.

## Validation plan

Impulse and burst tests for attack/sustain separation; neutral transfer; clipping bounds; extreme inputs; stereo symmetry; silence/DC behavior; parameter smoothing and reset behavior.

Host numerical tests, native build verification, listening tests and revision-bound hardware CPU measurements are separate requirements. No browser audition support is assumed for effects.

## SDK reference

Planning checked against Open-XY staging revision [`64bd090`](https://github.com/etcetc000/open-xy/tree/64bd0900a08a134e274a8b8decbc8b734ca534dd) on 2026-11-01:

- [Effect API and scratch buffers](https://github.com/etcetc000/open-xy/blob/64bd0900a08a134e274a8b8decbc8b734ca534dd/docs/api.md#effects)
- [Authoring journey](https://github.com/etcetc000/open-xy/blob/64bd0900a08a134e274a8b8decbc8b734ca534dd/docs/first-machine.md)
- [UX conventions](https://github.com/etcetc000/open-xy/blob/64bd0900a08a134e274a8b8decbc8b734ca534dd/docs/ux.md)
- [Hardware profiling](https://github.com/etcetc000/open-xy/blob/64bd0900a08a134e274a8b8decbc8b734ca534dd/docs/profiling.md)

## Next step

Assign a dedicated implementation agent after the brief is reviewed. Do not start other effects or modify the SDK as part of this repository's initial implementation.
