#include "JJOSInstrumentPanel.h"
#include "../pgm/Program.h"
#include "../pgm/Pad.h"
#include "../pgm/Layer.h"
#include "../pgm/PadMixer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <algorithm>

// JJOS chromatic mapping: pad 0 → MIDI note 36 (C2)
static constexpr int JJOS_PAD0_NOTE = 36;

static const char* NOTE_NAMES[12] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

static QString midi_note_label(int note) {
    const int octave = (note / 12) - 2;
    return QString("%1%2 (%3)").arg(NOTE_NAMES[note % 12]).arg(octave).arg(note);
}

JJOSInstrumentPanel::JJOSInstrumentPanel(Program* program, QWidget* parent)
    : QWidget(parent), program_(program)
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(10);

    // ── Instructions ────────────────────────────────────────────────────────
    auto* intro = new QLabel(
        "Build a JJOS instrument program from a single sample.\n"
        "The sample is assigned to all 64 pads. Each pad receives a tuning offset "
        "so it plays at its JJOS chromatic MIDI note (C2\u2013Eb7). "
        "Set the root note to match the pitch at which the sample was recorded.");
    intro->setWordWrap(true);
    intro->setStyleSheet("color: #888; font-size: 11px;");
    root->addWidget(intro);

    // ── WAV file ─────────────────────────────────────────────────────────────
    auto* file_box = new QGroupBox("Sample");
    auto* file_layout = new QHBoxLayout(file_box);
    wav_edit_ = new QLineEdit;
    wav_edit_->setPlaceholderText("Path to WAV sample (root sample)…");
    auto* browse_btn = new QPushButton("Browse…");
    file_layout->addWidget(wav_edit_, 1);
    file_layout->addWidget(browse_btn);
    root->addWidget(file_box);

    // ── Options ──────────────────────────────────────────────────────────────
    auto* opts_box = new QGroupBox("Options");
    auto* form = new QFormLayout(opts_box);

    root_note_combo_ = new QComboBox;
    for (int note = 0; note < 128; ++note)
        root_note_combo_->addItem(midi_note_label(note), note);
    // Default root = C4 (60), which falls in the middle of the JJOS pad range.
    root_note_combo_->setCurrentIndex(60);

    play_mode_combo_ = new QComboBox;
    play_mode_combo_->addItem("Note On  (sustain while key held)", 1);
    play_mode_combo_->addItem("One Shot (play to end)", 0);

    form->addRow("Root note:", root_note_combo_);
    form->addRow("Play mode:", play_mode_combo_);
    root->addWidget(opts_box);

    // ── Build + status ───────────────────────────────────────────────────────
    build_btn_ = new QPushButton("Build Instrument Program");
    build_btn_->setEnabled(program_ != nullptr);
    root->addWidget(build_btn_);

    status_label_ = new QLabel("No instrument built yet.");
    status_label_->setWordWrap(true);
    root->addWidget(status_label_);

    root->addStretch();

    connect(browse_btn, &QPushButton::clicked, this, &JJOSInstrumentPanel::on_browse_wav);
    connect(build_btn_, &QPushButton::clicked, this, &JJOSInstrumentPanel::on_build);
}

void JJOSInstrumentPanel::set_program(Program* program) {
    program_ = program;
    build_btn_->setEnabled(program_ != nullptr);
}

void JJOSInstrumentPanel::on_browse_wav() {
    QSettings settings;
    QString start = settings.value("last_wav_dir", QDir::homePath()).toString();
    QString path = QFileDialog::getOpenFileName(
        this, "Select Root Sample", start, "WAV files (*.wav *.WAV);;All Files (*)");
    if (!path.isEmpty()) {
        wav_edit_->setText(path);
        settings.setValue("last_wav_dir", QFileInfo(path).absolutePath());
    }
}

void JJOSInstrumentPanel::on_build() {
    if (!program_) return;

    const QString wav_path = wav_edit_->text().trimmed();
    if (wav_path.isEmpty()) {
        QMessageBox::warning(this, "No Sample", "Please select a WAV file first.");
        return;
    }

    const QFileInfo fi(wav_path);
    std::string sample_name = fi.completeBaseName().toStdString();
    if (sample_name.size() > 16)
        sample_name = sample_name.substr(0, 16);

    const int root_note = root_note_combo_->currentData().toInt();
    const int play_mode = play_mode_combo_->currentData().toInt(); // 0=One Shot, 1=Note On

    for (int pad_index = 0; pad_index < 64; ++pad_index) {
        const int midi_note = JJOS_PAD0_NOTE + pad_index;
        double tuning = static_cast<double>(midi_note - root_note);
        tuning = std::max(-36.0, std::min(36.0, tuning));

        Pad pad = program_->get_pad(pad_index);

        // Layer 0: root sample with tuning offset
        Layer layer0 = pad.get_layer(0);
        layer0.set_sample_name(sample_name);
        layer0.set_level(100);
        layer0.set_range({0, 127});
        layer0.set_tuning(tuning);
        if (play_mode == 0) layer0.set_one_shot();
        else                layer0.set_note_on();

        // Layers 1-3: clear
        for (int l = 1; l < Pad::LAYER_COUNT; ++l) {
            Layer layer = pad.get_layer(l);
            layer.set_sample_name("");
            layer.set_level(100);
            layer.set_range({0, 127});
            layer.set_tuning(0.0);
        }

        pad.set(Pad::VOICE_OVERLAP, 0); // Poly

        PadMixer mixer = pad.get_mixer();
        mixer.set(PadMixer::VOLUME, 100);
        mixer.set(PadMixer::PAN, 50);
    }

    const QString root_label = root_note_combo_->currentText();
    status_label_->setText(
        QString("Built instrument: \"%1\" across 64 pads (C2\u2013Eb7), root = %2.")
            .arg(fi.completeBaseName())
            .arg(root_label));

    emit sample_dir_selected(fi.absolutePath());
    emit program_modified();

    QMessageBox::information(this, "Instrument Built",
        QString("Sample \"%1\" assigned to all 64 pads with chromatic tuning.\n"
                "Root note: %2\n\nRemember to Save the program.")
            .arg(fi.completeBaseName())
            .arg(root_label));
}
