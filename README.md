# Super Arp Module for Move Everything

Advanced MIDI arpeggiator module for Ableton Move, built for Move Everything.

## Features

Super Arp is a chainable MIDI FX module (`midi_fx`) for Move Everything. It turns held notes into deterministic or randomized arpeggiated phrases, with independent control over timing, note order, rhythm, and variation.

### Global

Global controls define transport/timing behavior and base output characteristics for every generated note.

| Parameter | What it does |
|---------|--------|
| `latch` | Holds notes after release when enabled. |
| `max_voices` | Limits simultaneous output voices (`1-64`), stealing oldest first. |
| `rate` | Sets arp step division (`1/32`, `1/16`, `1/8`, `1/4`). |
| `triplet` | Enables triplet timing for step duration. |
| `swing` | Applies swing amount (`0-100`) to internal timing. |
| `gate` | Sets note length (`0-1600`), including overlaps beyond one step. |
| `velocity` | Base velocity override (`0` = as played, `1-127` = fixed base). |
| `octave_range` | Applies deterministic octave movement (`0`, `+1`, `-1`, `+2`, `-2`). |
| `sync` | Chooses timing source (`internal` or incoming MIDI `clock`). |
| `bpm` | Internal tempo (`40-240`) when `sync=internal`. |

### Progression

Progression controls decide which held notes are selected on each trigger step, including ordered, patterned, and seeded random behavior.

| Parameter | What it does |
|---------|--------|
| `progression_mode` (`mode`) | Selects note order engine: `up`, `down`, `as_played`, `leap_inward`, `leap_outward`, `chord`, `pattern`, `random_pattern`. |
| `pattern_preset` (`pattern`) | Selects one of 40 progression presets for `pattern` mode. |
| `progression_trigger` (`trigger`) | Sets reset policy: `retrigger` or `continuous`. |
| `missing_note_policy` (`note policy`) | Handles out-of-range pattern indices: `fold`, `wrap`, `clamp`, `skip`. |
| `progression_seed` (`rand seed`) | Main seed for random progression generation. |
| `random_pattern_length` (`rand length`) | Sets random pattern length (`1-32`). For random_pattern mode only. |
| `random_pattern_chords` (`rand chords`) | Increases chord density in random pattern (`0-100`). For random_pattern mode only. |
| `random_pattern_chord_seed` (`rand ch seed`) | Additional seed used for random chord voicing choices. For random_pattern mode only. |

Pattern notation for progression presets:
- Numbers (`1`, `2`, `3`, ...) reference held notes sorted by pitch (lowest to highest).
- `-` separates time steps.
- Parentheses group simultaneous notes into a chord step, for example `(1-3)`.

### Rhythm

Rhythm controls define trigger/rest pulses that gate progression advancement.

Pattern notation for rhythm presets:
- `0` means trigger/play a step.
- `x` means rest/pause (no trigger on that step).
- Each character is one step in sequence order from left to right.

| Parameter | What it does |
|---------|--------|
| `rhythm_preset` (`pattern`) | Selects one of 40 rhythm strings (`0` = trigger, `x` = rest). |
| `rhythm_trigger` (`trigger`) | Sets rhythm reset policy: `retrigger` or `continuous`. |

### Modifiers

Modifiers add deterministic variation on top of selected notes. Each modifier has independent seeded behavior, and all can follow a shared loop length.

| Parameter | What it does |
|---------|--------|
| `modifier_loop_length` (`mod loop`) | Loop length for modifier randomness (`0-128`, `0` = no loop). |
| `drop_amount` (`drop`) | Probability-like amount to skip triggered steps (`0-100`). |
| `drop_seed` | Seed for drop decisions. |
| `velocity_random_amount` (`vel rand`) | Randomizes outgoing velocity (`0-127`). |
| `velocity_seed` (`vel seed`) | Seed for velocity randomization. |
| `gate_random_amount` (`gate rand`) | Randomizes gate length (`0-1600`). |
| `gate_seed` | Seed for gate randomization. |
| `random_octave_amount` (`oct rand`) | Amount of octave randomization (`0-100`). |
| `random_octave_range` (`oct range`) | Octave randomization set: `+1`, `-1`, `+-1`, `+2`, `-2`, `+-2`. |
| `random_octave_seed` (`oct seed`) | Seed for octave randomization. |
| `random_note_amount` (`note rand`) | Replaces selected notes with other held notes (`0-100`). |
| `random_note_seed` (`note seed`) | Seed for note replacement decisions. |

## Prerequisites

- [Move Everything](https://github.com/charlesvestal/move-anything) installed on your Ableton Move
- SSH access enabled: http://move.local/development/ssh

## Installation

### Via Module Store

If Super Arp is published in Module Store:

1. Launch Move Everything on your Move
2. Select **Module Store**
3. Navigate to **MIDI FX** -> **Super Arp**
4. Select **Install**

### Manual Installation

```bash
./scripts/build.sh
./scripts/install.sh
```

## Usage

Super Arp is a MIDI FX module:

1. Insert **Super Arp** in a chain's MIDI FX slot
2. Feed MIDI notes/chords into the chain
3. Route output to a synth/sampler module
4. Shape timing, progression, rhythm, and modifiers in Shadow UI

## Parameters

In Shadow UI, parameters are organized into categories.

### Global

- `latch` (`off`/`on`) - Hold notes after release
- `max_voices` (`1-64`) - Max simultaneous output voices
- `rate` (`1/32`, `1/16`, `1/8`, `1/4`) - Step rate
- `triplet` (`off`/`on`) - Triplet timing
- `swing` (`0-100`) - Swing amount
- `gate` (`0-1600`) - Gate length (100 = one step)
- `velocity` (`0-127`) - Base velocity (`0` = as played)
- `octave_range` (`0`, `+1`, `-1`, `+2`, `-2`) - Deterministic octave progression
- `sync` (`internal`, `clock`) - Timing source
- `bpm` (`40-240`) - Internal tempo

### Progression

- `mode` - Progression algorithm
- `pattern` - Progression preset (used in `pattern` mode)
- `trigger` (`retrigger`, `continuous`) - Progression reset behavior
- `note policy` (`fold`, `wrap`, `clamp`, `skip`) - Out-of-range index handling
- `rand seed` (`0-65535`) - Random progression seed
- `rand length` (`1-32`) - Random pattern length
- `rand chords` (`0-100`) - Chord density for random pattern
- `rand ch seed` (`0-65535`) - Chord generation seed

### Rhythm

- `pattern` - Rhythm preset (`0` = trigger, `x` = rest)
- `trigger` (`retrigger`, `continuous`) - Rhythm reset behavior

### Modifiers

- `mod loop` (`0-128`) - Loop length for deterministic mod repetition (`0` = free-running)
- `drop` (`0-100`) + `drop seed` - Probability of dropping a triggered step
- `vel rand` (`0-127`) + `vel seed` - Velocity variation
- `gate rand` (`0-1600`) + `gate seed` - Gate variation
- `oct rand` (`0-100`) + `oct range` (`+1`, `-1`, `+-1`, `+2`, `-2`, `+-2`) + `oct seed`
- `note rand` (`0-100`) + `note seed` - Replace selected notes with other held notes

## Troubleshooting

**No arpeggio output:**
- Verify notes are reaching Super Arp (check chain MIDI routing)
- Check rhythm pattern is not resting (`x`) at the current step
- In `clock` sync mode, ensure external MIDI clock start/tick is present

**Too many overlapping notes:**
- Lower `gate`
- Lower `max_voices`

**Timing sounds wrong:**
- Verify `sync` mode matches your setup (`internal` vs `clock`)
- Check `triplet` and `swing` settings

**Need debug logs:**
- This build writes debug logs to `/data/UserData/move-anything/superarp.log`

## Building from Source

```bash
./scripts/build.sh
```

The build script supports local cross-compilers or Docker and outputs:

- `dist/superarp/`
- `dist/superarp-module.tar.gz`

## Credits

- Move Everything framework and host APIs: Charles Vestal and contributors
- Super Arp implementation: move-anything-superarp project contributors

## AI Assistance Disclaimer

This module is part of Move Everything and was developed with AI assistance, including Claude, Codex, and other AI assistants.

All architecture, implementation, and release decisions are reviewed by human maintainers.  
AI-assisted content may still contain errors, so please validate functionality, security, and license compatibility before production use.
