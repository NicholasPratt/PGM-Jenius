# C++ Port Plan

Phased port of PGM-Jenius from Java/Swing to C++/Qt. Each phase is independently buildable and testable. Complete phases in order - later phases depend on earlier ones.

---

## Phase 1 — Project Scaffold & Build System ✅

**Goal:** Empty but compilable CMake + Qt project.

- [x] Create `cpp/` directory structure (`src/pgm`, `src/audio`, `src/midi`, `src/gui`)
- [x] Write `cpp/CMakeLists.txt` with Qt6 and libsndfile dependencies
- [x] Write `cpp/src/main.cpp` (stub: opens a QApplication and blank QMainWindow)
- [x] Verify it compiles and launches a blank window

**Notes:** Required installing brew LLVM 22 — AppleClang 12 is too old for Qt 6.11.
Build with: `cmake -B build -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ -DCMAKE_PREFIX_PATH=/usr/local/opt/qt`

**Deliverable:** `./build/PGM-Jenius` opens a blank Qt window. ✅

---

## Phase 2 — Binary Format Layer (`pgm/`) ✅

**Goal:** Read and write `.pgm` files identically to the Java version. No GUI needed.

Port these Java classes in order:

1. **`ByteBuffer`** (`pgm/ByteBuffer.h/.cpp`)
   - Wraps `std::vector<uint8_t>`
   - Methods: `getString`, `setString`, `getShort`/`setShort` (little-endian), `getByte`/`setByte`, `getInt`/`setInt`, `getRange`/`setRange`
   - Static `open(path)` reads file into buffer, `save(path)` writes it out

2. **`Parameter`** (`pgm/Parameter.h/.cpp`)
   - Type enum: `TEXT`, `INT`, `OFF_INT`, `ENUM`, `RANGE`, `TUNING`
   - Holds: label string, byte offset, type, value range / enum values
   - Factory functions: `Parameter::integer(...)`, `Parameter::enumType(...)`, etc.

3. **`Range`** (`pgm/Range.h`)
   - Simple struct: `int low`, `int high`, `contains(double)` method

4. **`Profile`** (`pgm/Profile.h`)
   - Static instances: `Profile::MPC500` (4×3, 1 slider, 1 filter), `Profile::MPC1000` (4×4, 2 sliders, 2 filters)

5. **`BaseElement`** (`pgm/BaseElement.h/.cpp`)
   - Base class holding a reference to parent ByteBuffer + section offset + element index
   - Provides `getByte`, `setByte`, `getShort`, `setShort`, `getString`, `setString` relative to element's offset
   - `get(Parameter)` / `set(Parameter, value)` dispatch

6. **`Layer`** (`pgm/Layer.h/.cpp`)
   - Offset `0x18` per layer, 4 layers per pad
   - Fields: sample name (string), level (byte), tuning (short/100.0), range (2 bytes), play mode (byte)
   - Static `Parameter` constants matching Java originals

7. **`PadEnvelope`**, **`PadFilter1`**, **`PadFilter2`**, **`PadMixer`** (`pgm/PadEnvelope.h`, etc.)
   - Small structs with their offsets and parameters — direct port

8. **`Pad`** (`pgm/Pad.h/.cpp`)
   - Offset `0x18 + 64*0xA4`, 64 pads × `0xA4` bytes
   - Holds: 4 Layers, Envelope, Filter1, Filter2, Mixer
   - `copyFrom(Pad& other, ignoreParams)` method

9. **`Slider`** (`pgm/Slider.h/.cpp`)
   - 2 sliders per program

10. **`Program`** (`pgm/Program.h/.cpp`)
    - Fixed `0x2A04`-byte buffer
    - `static Program open(std::string path)`
    - `void save(std::string path)`
    - `Pad getPad(int index)`, `Slider getSlider(int index)`
    - MIDI program change parameter

**Testing:** Write a small CLI test that opens a real `.pgm` file, reads pad/layer names, modifies one, saves to a new file, and diffs the bytes.

**Deliverable:** Round-trip read/write of `.pgm` files with zero byte diff (except intentional edits). ✅ Verified with `test.pgm` and `chromatic.pgm`.

---

## Phase 3 — Audio Layer (`audio/`) ✅

**Goal:** Load WAV files, run beat detection, play audio.

1. **`Sample`** (`audio/Sample.h/.cpp`)
   - Use **libsndfile** (`sf_open`, `sf_readf_int`) to load WAV into `std::vector<int>` per channel
   - Store: frame count, sample rate, channel count, raw PCM data
   - Methods: `asSamples()` → `vector<vector<int>>` (channels), `subRegion(from, to)`, `save(path)`
   - Audio format conversion: normalize to 16-bit range for beat detection

2. **`Markers`** (`audio/Markers.h/.cpp`)
   - List of frame positions (beat locations)
   - `add(int frame)`, `clear()`, `size()`, `getRangeFrom(int index)` → `LocationRange`
   - Selected marker index tracking

3. **`Slicer`** (`audio/Slicer.h/.cpp`)
   - Direct port of Java `Slicer.java` — pure arithmetic, no Java-specific APIs
   - `extractMarkers()`: sliding window energy → local energy comparison → zero-crossing adjustment
   - `setSensitivity(int)` (0–255, default 130)
   - `exportSlices(path, prefix)` → writes `.wav` files via `Sample::save`

4. **`AudioPlayer`** (`audio/AudioPlayer.h/.cpp`)
   - Qt Multimedia: `QAudioOutput` + `QIODevice`
   - Queue-based playback (port of `AudioWorker` BlockingQueue pattern using `QThread` + `QMutex`)
   - `play(Sample&)`, `stop()`

**Deliverable:** CLI tool that loads a WAV, detects beats, prints marker positions, exports slices. ✅

---

## Phase 4 — MIDI Export (`midi/`) ✅

**Goal:** Generate Standard MIDI Files from slice markers.

1. **`MidiSequenceBuilder`** (`midi/MidiSequenceBuilder.h/.cpp`)
   - Hand-rolled SMF format 0 writer
   - Input: list of beat positions + sample rate → MIDI note-on/off events at corresponding ticks
   - Output: valid `.mid` file
   - SMF header: `MThd`, track chunk `MTrk`, variable-length delta times
   - One note per slice at a fixed pitch (or pad-assigned pitch)

**Deliverable:** Exports a `.mid` groove file from sliced markers, playable in a DAW. ✅ Verified output at `/tmp/test_groove.mid`.

---

## Phase 5 — Core GUI (`gui/`) ✅

**Goal:** Main window with pad grid, parameter editing, file open/save. No audio/slice tab yet.

1. **`MainWindow`** (`gui/MainWindow.h/.cpp`) — `QMainWindow`
   - Menu bar: File (Open, Save, Save As, New), Help, MPC model selector
   - Tab widget: "Pads" tab (only tab for now)
   - Loads `.pgm` on startup if path given on command line
   - Multi-window support: each `MainWindow` instance is independent

2. **`PadButton`** (`gui/PadButton.h/.cpp`) — `QPushButton` subclass
   - Displays pad number + sample name of layer 1
   - Click → selects pad, updates parameter panel on right
   - Highlighted when selected

3. **`ProgramPanel`** (`gui/ProgramPanel.h/.cpp`) — `QWidget`
   - Grid layout of `PadButton`s (4×3 or 4×4 based on profile)
   - Right panel: layer tabs (Layer 1–4), each with parameter widgets
   - Connects pad selection to parameter display update

4. **`ParameterWidget`** (`gui/ParameterWidget.h/.cpp`) — `QWidget`
   - Generic editor that adapts to `Parameter` type:
     - TEXT → `QLineEdit`
     - INT / TUNING → `QSpinBox` or `QSlider` + value label
     - ENUM → `QComboBox`
     - RANGE → dual `QSpinBox` (low/high)
   - Emits `valueChanged` signal → writes back to model

5. **`FileDragHandler`** — drag `.wav` files onto pads
   - Subclass `QWidget`, override `dragEnterEvent` / `dropEvent`
   - Dropped WAV → assign to layer 1 sample name of target pad

**Deliverable:** Open a `.pgm`, see all pads, click a pad, edit parameters, save. Drag WAV files onto pads. ✅

---

## Phase 6 — Waveform & Slicer Tab (`gui/`) ✅

**Goal:** Visual beat detection and slice export.

1. **`WaveformPanel`** (`gui/WaveformPanel.h/.cpp`) — `QWidget`
   - `paintEvent`: draw PCM waveform + beat marker lines
   - Click on waveform → move nearest marker
   - Sensitivity slider → re-runs `Slicer::setSensitivity`
   - Play button → plays selected slice via `AudioPlayer`

2. **Slicer tab in `MainWindow`**
   - Load WAV button → opens file dialog, loads into `Sample` + `Slicer`
   - Shows `WaveformPanel`
   - Export slices button → `Slicer::exportSlices()`
   - Export MIDI button → `MidiSequenceBuilder`

**Deliverable:** Full chop/slice workflow works in GUI. ✅

---

## Phase 7 — Multisample Builder & Batch Create (`gui/`) ✅

**Goal:** Remaining tabs from the original app.

1. **`MultisampleBuilder`** (`pgm/MultisampleBuilder.h/.cpp`)
   - Port of Java `MultisampleBuilder.java`
   - Parse note names from filenames (regex: `[A-G]#?[0-9]`)
   - Map samples to pads with pitch transposition across layers
   - Fill gaps with interpolated tuning values

2. **Multisample tab in `MainWindow`**
   - Folder picker → scans for WAV files with note names
   - Preview assignment → confirm → writes to `Program`

3. **`BatchCreateCommand`** — port of Java batch folder processor
   - Walk directory tree, create one `.pgm` per subfolder of samples

4. **Batch Create tab in `MainWindow`**
   - Source folder + destination folder pickers
   - Progress display
   - Run batch

**Deliverable:** All three original tabs functional. ✅

---

## Phase 8 — Polish & Platform Integration ✅

- App icon + splash screen image (reuse original `mpcmaidlogo400_400.png`)
- macOS: native menu bar (`QMenuBar` with `menuBar()->setNativeMenuBar(true)`)
- Preferences persistence: `QSettings` → save last opened directory, preferred profile, audio device
- Command-line: `pgm-jenius [-n] [program.pgm]` (`-n` = no splash)
- Package: macOS `.app` bundle via `macdeployqt`, Windows installer via CPack

---

## Dependency Summary

| Dependency | Use | How to get |
|---|---|---|
| Qt 6.5+ | GUI, audio output, settings | `brew install qt` / Qt Installer |
| libsndfile | WAV read/write | `brew install libsndfile` |
| CMake 3.20+ | Build | `brew install cmake` |

No other external dependencies needed.

---

## File Count Estimate

~35–40 `.h`/`.cpp` file pairs. The Java original was 48 files at 6,200 lines; the C++ port will be similar in line count, longer in some areas (Qt boilerplate) and shorter in others (no reflection-based parameter dispatch).
