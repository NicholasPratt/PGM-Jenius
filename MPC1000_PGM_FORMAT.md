# MPC1000 .pgm Program Format — Complete Parameter Reference

All offsets are in hex. All values little-endian. File size is always **0x2A04 bytes**.

**Implementation status legend:**
- ✓ — Implemented correctly in C++ port (right offset, right semantics)
- ⚠ — Named in C++ port but **at a wrong byte offset** (placeholder; reads garbage)
- ✗ — Not implemented in C++ port
- N/A — Structural / not a user-editable parameter

---

## File Layout Overview

| Region | Start | End | Size | Description |
|--------|-------|-----|------|-------------|
| Header | 0x0000 | 0x0017 | 24 B | File size word + version string |
| Pad data | 0x0018 | 0x2917 | 0x2900 B | 64 pads × 0xA4 bytes each |
| Pad→MIDI note table | 0x2918 | 0x2957 | 64 B | One byte per pad: which MIDI note triggers it |
| MIDI note→pad table | 0x2958 | 0x29D7 | 128 B | One byte per MIDI note: which pad it maps to (64=unassigned) |
| MIDI Program Change | 0x29D8 | 0x29D8 | 1 B | 0–128 (0=off) |
| Slider 1 | 0x29D9 | 0x29E5 | 13 B | Assignable slider 1 settings |
| Slider 2 | 0x29E6 | 0x29F2 | 13 B | Assignable slider 2 settings |
| Padding / unknown | 0x29F3 | 0x2A03 | 17 B | Reserved |

---

## 1. Header

| Offset | Size | Name | Values | Java | C++ |
|--------|------|------|--------|------|-----|
| 0x0000 | 2 B | File size | Always 0x2A04 | N/A | N/A |
| 0x0002 | 2 B | Padding | — | N/A | N/A |
| 0x0004 | 16 B | Version string | `"MPC1000 PGM 1.00"` | ✓ | ✓ |
| 0x0014 | 4 B | Padding | — | N/A | N/A |

---

## 2. Pad Data

64 pads. Pad *n* starts at `0x0018 + n × 0xA4` (n = 0–63).

Each pad is 0xA4 (164) bytes. Internal layout:

| Pad-relative offset | Size | Region |
|---------------------|------|--------|
| 0x00 – 0x5F | 96 B | Four layers (0x18 bytes each) |
| 0x60 – 0xA3 | 68 B | Per-pad parameters |

### 2a. Layer Data (4 layers per pad)

Layer *l* within a pad starts at pad-relative offset `l × 0x18` (l = 0–3).

All offsets below are **relative to the layer start**.

| Layer offset | Size | Parameter | Range / Values | Java | C++ |
|--------------|------|-----------|----------------|------|-----|
| 0x00 | 16 B | **Sample name** | ASCII, null-padded | ✓ | ✓ |
| 0x10 | 1 B | Padding | — | N/A | N/A |
| 0x11 | 1 B | **Level** | 0–100 | ✓ | ✓ |
| 0x12 | 1 B | **Velocity range high** | 0–127 | ✓ | ✓ |
| 0x13 | 1 B | **Velocity range low** | 0–127 | ✓ | ✓ |
| 0x14 | 2 B | **Tuning** (signed short) | −3600 to +3600 (÷100 = semitones, −36.00 to +36.00) | ✓ | ✓ |
| 0x16 | 1 B | **Play mode** | 0=One Shot, 1=Note On | ✓ | ✓ |
| 0x17 | 1 B | Padding | — | N/A | N/A |

> **Note on Tuning:** The stored value is cents × 100 as a signed 16-bit integer.  
> Example: +1.18 semitones → stored as 118. Range displayed to user is −36.00 to +36.00.

### 2b. Per-Pad Parameters

All offsets below are **relative to the pad start** (i.e., after the 4 layers at 0x00–0x5F).

#### Voice / Articulation

| Pad offset | Size | Parameter | Range / Values | Java | C++ |
|------------|------|-----------|----------------|------|-----|
| 0x60 | 2 B | Padding / unknown | — | ✗ | ✗ |
| 0x62 | 1 B | **Voice overlap** | 0=Poly, 1=Mono | ✓ | ✓ |
| 0x63 | 1 B | **Mute group** | 0=Off, 1–32 | ✓ | ✓ |
| 0x64 | 2 B | Unknown | — | ✗ | ✗ |

#### Amplitude Envelope

| Pad offset | Size | Parameter | Range / Values | Java | C++ |
|------------|------|-----------|----------------|------|-----|
| 0x66 | 1 B | **Attack** | 0–100 | ✓ | ⚠ |
| 0x67 | 1 B | **Decay** | 0–100 | ✓ | ⚠ |
| 0x68 | 1 B | **Decay mode** | 0=End (decay to silence), 1=Start (decay back to start) | ✓ | ✗ |
| 0x69 | 2 B | Unknown | — | ✗ | ✗ |
| 0x6B | 1 B | **Velocity to level** | 0–100 | ✓ | ✗ |
| 0x6C | 5 B | Unknown | — | ✗ | ✗ |

> **Decay Mode:** "End" decays the sample to silence from its current position.  
> "Start" decays the volume back toward the sample start (loop-friendly behaviour).

#### Filter 1

| Pad offset | Size | Parameter | Range / Values | Java | C++ |
|------------|------|-----------|----------------|------|-----|
| 0x71 | 1 B | **Filter 1 type** | 0=Off, 1=Lowpass, 2=Bandpass, 3=Highpass | ✓ | ✗ |
| 0x72 | 1 B | **Filter 1 frequency** | 0–100 | ✓ | ⚠ |
| 0x73 | 1 B | **Filter 1 resonance** | 0–100 | ✓ | ⚠ |
| 0x74 | 4 B | Unknown | — | ✗ | ✗ |
| 0x78 | 1 B | **Filter 1 velocity→freq** | 0–100 | ✓ | ✗ |

#### Filter 2

| Pad offset | Size | Parameter | Range / Values | Java | C++ |
|------------|------|-----------|----------------|------|-----|
| 0x79 | 1 B | **Filter 2 type** | 0=Off, 1=Lowpass, 2=Bandpass, 3=Highpass, 4=Link | ✓ | ✗ |
| 0x7A | 1 B | **Filter 2 frequency** | 0–100 | ✓ | ⚠ |
| 0x7B | 1 B | **Filter 2 resonance** | 0–100 | ✓ | ⚠ |
| 0x7C | 4 B | Unknown | — | ✗ | ✗ |
| 0x80 | 1 B | **Filter 2 velocity→freq** | 0–100 | ✓ | ✗ |
| 0x81 | 14 B | Unknown | — | ✗ | ✗ |

> **Filter 2 "Link"**: Links Filter 2 to Filter 1 so they track together.  
> **Pre-attenuation** (below) applies to the signal before both filters.

#### Mixer / Output

| Pad offset | Size | Parameter | Range / Values | Java | C++ |
|------------|------|-----------|----------------|------|-----|
| 0x8F | 1 B | **Mixer level** | 0–100 | ✓ | ⚠ |
| 0x90 | 1 B | **Pan** | 0–49=Left, 50=Center, 51–100=Right | ✓ | ⚠ |
| 0x91 | 1 B | **Output** | 0=Stereo, 1=1-2, 2=3-4 | ✓ | ✗ |
| 0x92 | 1 B | **FX send** | 0=Off, 1=Send 1, 2=Send 2 | ✓ | ✗ |
| 0x93 | 1 B | **FX send level** | 0–100 | ✓ | ✗ |
| 0x94 | 1 B | **Pre-filter attenuation** | 0=0 dB, 1=−6 dB, 2=−12 dB | ✓ | ✗ |
| 0x95 | 15 B | Unknown / padding | — | ✗ | ✗ |

> **Output routing:** "Stereo" routes to the main stereo output. "1-2" and "3-4" route to  
> the discrete stereo output pairs on the MPC1000 (used for hardware mixing/effects).

---

## 3. MIDI Tables (Global)

### 3a. Pad → MIDI Note Table

| File offset | Size | Description |
|-------------|------|-------------|
| 0x2918 + *n* | 1 B | MIDI note number assigned to pad *n* (0–127). Pad *n* ∈ 0–63. |

The default mapping starts pad 0 at note 35 (B1). Java original and C++ both implement this. ✓

### 3b. MIDI Note → Pad Table

| File offset | Size | Description |
|-------------|------|-------------|
| 0x2958 + *n* | 1 B | Pad number triggered by MIDI note *n* (0–63), or 64 = unassigned. *n* ∈ 0–127. |

Inverse lookup of the table above. Commented out in Java original; not in C++. ✗

---

## 4. Global Program Parameters

| File offset | Size | Parameter | Range / Values | Java | C++ |
|-------------|------|-----------|----------------|------|-----|
| 0x29D8 | 1 B | **MIDI program change** | 0=Off, 1–128 | ✓ | ✓ |

---

## 5. Slider Configuration (2 Sliders)

Slider *s* starts at `0x29D9 + s × 0x0D` (s = 0 or 1). Each slider is 13 bytes.

All offsets below are **absolute file offsets** for slider 0; add 0x0D for slider 1.

| File offset | Size | Parameter | Range / Values | Java | C++ |
|-------------|------|-----------|----------------|------|-----|
| 0x29D9 | 1 B | **Assigned pad** | 0–63 or 64=Off | ✓ | ⚠ |
| 0x29DA | 1 B | Unknown | — | ✗ | ✗ |
| 0x29DB | 1 B | **Assigned parameter** | 0=Tune, 1=Filter, 2=Layer, 3=Attack, 4=Decay | ✓ | ⚠ |
| 0x29DC | 2 B | **Tune range** (signed short pair: lo, hi) | −120 to +120 | ✓ | ✗ |
| 0x29DE | 2 B | **Filter range** (signed short pair: lo, hi) | −50 to +50 | ✓ | ✗ |
| 0x29E0 | 2 B | **Layer range** (lo, hi) | 0–127 | ✓ | ✗ |
| 0x29E2 | 2 B | **Attack range** (lo, hi) | 0–100 | ✓ | ✗ |
| 0x29E4 | 2 B | **Decay range** (lo, hi) | 0–100 | ✓ | ✗ |

> Each slider can be assigned to one pad and one parameter. The range fields define  
> what minimum/maximum value the slider sweeps over for each parameter type.

---

## 6. C++ Port — Known Issues

The C++ subsection classes (`PadEnvelope`, `PadFilter1`, `PadFilter2`, `PadMixer`) were ported with **incorrect section offsets**. Because these classes use their own `SECTION_OFFSET` value that is added to the pad's base address, the actual bytes they access in the file are wrong — they fall inside Layer 0's data rather than the actual envelope/filter/mixer region.

| C++ Class | C++ SECTION_OFFSET | Correct offset (Java) | Delta |
|-----------|--------------------|-----------------------|-------|
| `PadEnvelope` | 0x04 | 0x66 (Attack) | off by 0x62 |
| `PadFilter1` | 0x0C | 0x71 (Type) | off by 0x65 |
| `PadFilter2` | 0x10 | 0x79 (Type) | off by 0x69 |
| `PadMixer` | 0x14 | 0x8F (Level) | off by 0x7B |

Additionally, `PadEnvelope` in C++ has a `Hold` parameter (0x01 relative) that does not exist in the real format. `PadFilter1`/`PadFilter2` are missing the Filter Type selector. `PadMixer` is missing Output routing, FX Send, FX Send Level, and Pre-attenuation.

The `Slider` C++ class has a simplified 3-field layout (Assign/Low/High) that does not match the Java original's 7-field layout with per-parameter range pairs.

**To fix:** Each subsection's `SECTION_OFFSET` must match the Java reference offsets, and missing parameters must be added.

---

## 7. JJOS — Variants, History, and Format Differences

JJOS ("JJ OS") is a third-party replacement OS for the MPC1000 and MPC2500 written by a Japanese developer ("JJ"). There are several distinct variants with different levels of .pgm compatibility.

---

### 7a. Variant Overview

| Variant | MPC target | Format compat. with stock OS | Notes |
|---------|-----------|------------------------------|-------|
| **Akai stock OS** (v2.13, v3.16) | MPC1000 | Reference | Baseline |
| **Free JJOS** (v3.15–3.16) | MPC1000/2500 | ✓ 100% identical | Drop-in replacement; no .pgm changes |
| **JJOS1** | MPC1000/2500 | ✓ Compatible | Extends UI/features; same file format |
| **JJOS2** (discontinued) | MPC1000/2500 | ✗ Incompatible | Introduced DRUM/INST types; format changed |
| **JJOS2XL** | MPC1000 | ✗ Loads but silent | Chromatic note order; NDC chop data; different defaults |
| **JJOS-XL** | MPC2500 | ✗ Loads but silent | Same OS as JJOS2XL, different hardware |
| **JJOS3 / OS128XL** | MPC1000+large LCD | ✗ Incompatible | Most advanced; real-time time-stretch; 32 audio tracks |

---

### 7b. Free JJOS

Free JJOS is a direct binary-compatible replacement for the stock Akai OS. It fixes bugs, improves pad sensitivity (range 1–25 vs stock 1–16), and adds audio tracks, but **does not change the .pgm file format in any way**. Programs load and save identically to stock OS. This is the only JJOS variant that a .pgm editor can treat as equivalent to stock OS.

**Key OS-level additions (not stored in .pgm):**
- Improved pad sensitivity scaling
- Audio track recording into sequences
- Fixed data corruption bug present in Akai v2.x

---

### 7c. JJOS1

JJOS1 extends the UI considerably (better piano roll, XOX editor, directory system) but retains the same .pgm binary format as stock OS. Programs are fully interchangeable.

---

### 7d. JJOS2 / JJOS2XL — Format Changes

JJOS2 (and its successor JJOS2XL) introduced several **breaking changes** to the DRUM program format. Programs created in JJOS2+ will not load correctly in JJOS1 or stock OS, and vice-versa.

#### File Identifier / Version String

Community tools (e.g. Maschine import) report that JJOS2XL writes a different version string in the header (offset 0x04, 16 bytes). Stock OS writes `"MPC1000 PGM 1.00"`; JJOS2XL reportedly writes `"MPC1000 PGM 4.0\0"` (with a different minor version). Changing this string back to `"MPC1000 PGM 1.00"` in a hex editor is sometimes used to force compatibility with stock-OS tools, though it may corrupt data stored in JJOS-specific bytes.

> **Confidence:** community-reported. Not confirmed by official documentation.

#### Chromatic MIDI Note Ordering

The most visible format difference is the default content of the pad→MIDI-note table (0x2918–0x2957) and MIDI-note→pad table (0x2958–0x29D7).

**Stock OS default mapping** follows the General MIDI drum layout (non-sequential):

| Pad | Default MIDI note |
|-----|-----------------|
| A01 | 37 (Side Stick) |
| A02 | 36 (Bass Drum) |
| A03 | 42 (Closed Hi-Hat) |
| A04 | 40 (Electric Snare) |
| … | … |

**JJOS2XL default mapping** is strictly chromatic from C1:

| Pad | Default MIDI note |
|-----|-----------------|
| A01 | 36 (C1) |
| A02 | 37 (C#1) |
| A03 | 38 (D1) |
| A04 | 39 (D#1) |
| … | … |
| D16 | 99 (D#6) |

When JJOS2XL **loads a stock OS program**, it automatically remaps the MIDI note tables to its chromatic layout (pad assignments stay the same, notes shift). When that program is **saved back** from JJOS2XL and then loaded in stock OS or Free JJOS, the note table contains the chromatic values — the pads exist and samples are assigned, but the note numbers no longer match any incoming MIDI note the sequencer expects, so **the pads produce no audio on playback**.

This is the primary reason a JJOS2XL .pgm is "silent" when loaded in stock OS: the binary file is otherwise valid, but 0x2918–0x29D7 holds foreign values.

#### Non-Destructive Chop (NDC) Data

JJOS2XL's non-destructive chop feature allows a sample to be divided into slices without duplicating audio data. Slice boundary references are stored inside the .pgm file by reusing the **padding byte at layer offset 0x17** (the byte immediately after `Play Mode` at 0x16, previously unused). Each layer's byte 0x17 holds the chop slice index or reference for that layer's assignment.

In stock OS programs this byte is always 0x00. In JJOS2XL programs using NDC, this byte is non-zero for chopped layers. A .pgm editor that preserves this byte (by not zeroing it on save) is NDC-safe even without understanding the feature.

#### Round Robin Playback

JJOS2XL adds **round robin** layer cycling: multiple layers on a pad can cycle through (sequential) or pick randomly on each hit, rather than all playing simultaneously or being selected by velocity range. The `Play Mode` byte (layer offset 0x16) may encode extended values for this:

| Value | Meaning (JJOS2XL) |
|-------|-------------------|
| 0 | One Shot |
| 1 | Note On |
| 2 | Round Robin (cycle) *(reported)* |
| 3 | Round Robin (random) *(reported)* |

> **Confidence:** community-reported. The exact byte values are not confirmed by official documentation.

#### Q-Link Slider Behaviour (OS-level, not .pgm)

The slider section of the .pgm file (0x29D9–0x29F2) is structurally the same in JJOS2XL as in stock OS. However the **runtime behaviour** differs:

| Behaviour | Stock OS | JJOS2XL |
|-----------|----------|---------|
| Cutoff slider | Adds to program's base filter value | **Replaces** program's filter value (ignores base) |
| Multiple Q-Links | All work independently | Only the **lowest-numbered** assigned parameter works |
| Slider while recording | Works normally | Disabled if AFTER key is active |

This affects playback feel but does not change what is stored in the .pgm binary.

---

### 7e. JJOSXL — INST (Instrument / Keygroup) Programs

JJOS2XL introduces a second program type alongside DRUM: **INST** (instrument) programs. INST programs are analogous to keygroup programs on the MPC4000 — one program spans the entire keyboard chromatically, with multiple velocity zones per key.

#### Identification

INST programs use the `.pgm` file extension but carry a **different program type flag** (the exact byte and offset are not publicly documented; it likely sits in the currently-unknown pad region or the file header). Loading an INST program in stock OS, Free JJOS, or JJOS1 produces an error or garbage output.

#### Structure Differences (known)

| Feature | DRUM program | INST program |
|---------|-------------|--------------|
| Pad model | 64 discrete pads | Chromatic keygroup mapping |
| Layers per zone | 4 | Up to 4 (velocity-split) |
| Pitch | Per-layer tuning offset | Root note + chromatic transpose per zone |
| MIDI trigger | Per-pad MIDI note | Key range (lo note, hi note, root note) per zone |
| Channel pressure | Not used | Triggers aftertouch modulation |
| Compatibility | All OS variants | JJOS2XL / JJOSXL only |

The full binary layout of INST programs has **not been publicly reverse-engineered**. The unknown bytes in the standard pad structure (see below) may overlap with how INST data is packed.

---

### 7f. JJOS3 / OS128XL

The most advanced variant, requiring a large LCD hardware modification. Adds real-time time-stretching, pitch-shifting, 32 audio tracks, an arpeggiator, and looping recorder mode. Program format is incompatible with stock OS and all earlier JJOS versions. Binary format is not documented publicly.

---

### 7g. Unknown Bytes — Potential JJOS Storage

The following byte ranges within each pad structure are zero in stock OS programs but may carry JJOS2XL-specific data. A safe .pgm editor should preserve (round-trip) these bytes unchanged rather than zeroing them.

| Pad offset | Size | Stock OS | Possible JJOS2XL use |
|------------|------|----------|----------------------|
| 0x60–0x61 | 2 B | 0x00 | Unknown |
| 0x64–0x65 | 2 B | 0x00 | Unknown |
| 0x69–0x6A | 2 B | 0x00 | Unknown |
| 0x6C–0x70 | 5 B | 0x00 | Unknown |
| 0x74–0x77 | 4 B | 0x00 | Unknown |
| 0x7C–0x7F | 4 B | 0x00 | Unknown |
| 0x81–0x8E | 14 B | 0x00 | Unknown; possibly pad scene or round robin data |
| 0x95–0xA3 | 15 B | 0x00 | Unknown |

Additionally, **layer offset 0x17** (1 byte per layer, normally 0x00 in stock OS) is used by JJOS2XL for NDC chop slice references.

---

### 7h. Compatibility Matrix

| Program created in → | Akai stock OS | Free JJOS | JJOS1 | JJOS2XL | JJOSXL INST |
|---------------------|:---:|:---:|:---:|:---:|:---:|
| **Akai stock OS** | ✓ | ✓ | ✓ | ✓ (remapped) | ✗ |
| **Free JJOS** | ✓ | ✓ | ✓ | ✓ (remapped) | ✗ |
| **JJOS1** | ✓ | ✓ | ✓ | ✓ (remapped) | ✗ |
| **JJOS2XL DRUM** | silent¹ | silent¹ | silent¹ | ✓ | ✗ |
| **JJOSXL INST** | ✗ | ✗ | ✗ | ✓ | ✓ |

¹ File loads without error but pads produce no audio because the MIDI note tables contain chromatic values that don't match any incoming note.

---

### 7i. Implications for PGM-Jenius C++ Port

| Concern | Recommendation |
|---------|---------------|
| Reading JJOS2XL DRUM programs | Will load (same file size, valid header); MIDI note table will differ from stock — display it as-is, do not remap |
| Writing JJOS2XL programs | Preserve the version string and all unknown bytes read from file; do not overwrite with stock defaults |
| Layer byte 0x17 | Always preserve on read/write; do not zero it |
| INST programs | Detect unknown program type flag; refuse to parse rather than silently corrupting |
| Round robin Play Mode values | Treat any `Play Mode` byte > 1 as "unknown / JJOS extension" rather than clamping to 0 or 1 |

---

## 8. Implementation Summary

| Section | Total parameters | In Java original | In C++ (correct) | In C++ (wrong offset) | Missing from C++ |
|---------|-----------------|------------------|-------------------|-----------------------|-----------------|
| Layer | 5 | 5 | 5 | 0 | 0 |
| Pad base | 3 | 3 | 3 | 0 | 0 |
| Envelope | 4 | 4 | 0 | 2 (Attack, Decay) | 2 (Decay Mode, Vel→Level) |
| Filter 1 | 5 | 5 | 0 | 2 (Freq, Res) | 3 (Type, Vel→Freq, Pre-Atten) |
| Filter 2 | 4 | 4 | 0 | 2 (Freq, Res) | 2 (Type, Vel→Freq) |
| Mixer | 5 | 5 | 0 | 2 (Level, Pan) | 3 (Output, FX Send, FX Send Level) |
| MIDI tables | 2 | 1.5 (note→pad commented) | 1 | 0 | 1 |
| Global | 1 | 1 | 1 | 0 | 0 |
| Sliders | 7 per slider | 7 | 0 | 2 (Pad, Parameter) | 5 (range pairs) |

---

## 9. Reference Sources

- **Stephen Norum**, "MPC1000 PGM File Format" — mybunnyhug.com (primary format spec)
- **Cyrille Martraire**, original Java source — github.com/cyriux/mpcmaid (this repo's `PGM-Jenius/src/`)
- **JJ OS Comparison Chart** — http://www7a.biglobe.ne.jp/~mpc1000/chart.htm (feature matrix across all variants)
- **JJOS2XL Operations Manual** — audiofanzine.com (official JJOS2XL documentation)
- **JJOS128XL / OS3 Manual** — audiofanzine.com
- **Community research** — mpc-forums.com, mpc-tutor.com JJOS guides
- **JJOS compatibility matrix** — mpc-samples.com MPC File Compatibility Guide
- **pympc1000** — github.com/stephenn/pympc1000 (Python struct-based parser)
- **mpc1k-node** — github.com/stigi/mpc1k-node (JavaScript parser/generator)
