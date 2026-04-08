#include "ChopSlicesPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include "../pgm/Program.h"
#include "../pgm/Layer.h"
#include "../pgm/PadMixer.h"
#include "../midi/MidiSequenceBuilder.h"

ChopSlicesPanel::ChopSlicesPanel(Program* program, QWidget* parent)
    : QWidget(parent), program_(program)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    // ---- Info label (top) ----
    info_label_ = new QLabel("Drag and Drop a WAV file onto the waveform area", this);
    info_label_->setAlignment(Qt::AlignCenter);
    root->addWidget(info_label_);

    // ---- Waveform + sensitivity slider (middle, resizable) ----
    auto* wave_row = new QHBoxLayout();
    root->addLayout(wave_row, 1);

    waveform_ = new WaveformPanel(this);
    wave_row->addWidget(waveform_, 1);

    auto* slider = new QSlider(Qt::Vertical, this);
    slider->setRange(50, 200);
    slider->setValue(130);
    slider->setToolTip("Beat detection sensitivity");
    slider->setFixedWidth(28);
    wave_row->addWidget(slider);

    // ---- Marker location label ----
    marker_label_ = new QLabel(" – ", this);
    marker_label_->setAlignment(Qt::AlignCenter);
    root->addWidget(marker_label_);

    // ---- Keyboard shortcut hint ----
    auto* hint = new QLabel(
        "← → : prev/next marker  |  Shift+← → : nudge ±1000  |  Alt+← → : nudge ±100  "
        "|  Del : delete  |  Enter : insert  |  Space : play slice",
        this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #888; font-size: 10px;");
    root->addWidget(hint);

    // ---- Export controls (bottom) ----
    auto* ctrl_row = new QHBoxLayout();
    root->addLayout(ctrl_row);

    auto* load_btn = new QPushButton("Load WAV…", this);
    ctrl_row->addWidget(load_btn);

    ctrl_row->addWidget(new QLabel("Prefix:", this));
    prefix_edit_ = new QLineEdit("Slice", this);
    prefix_edit_->setMaxLength(12);
    prefix_edit_->setFixedWidth(80);
    ctrl_row->addWidget(prefix_edit_);

    export_btn_ = new QPushButton("Export Slices + MIDI + PGM", this);
    export_btn_->setEnabled(false);
    ctrl_row->addWidget(export_btn_);

    play_btn_ = new QPushButton("▶ Play Slice", this);
    play_btn_->setEnabled(false);
    ctrl_row->addWidget(play_btn_);

    ctrl_row->addStretch();

    // ---- Audio player ----
    player_ = new AudioPlayer(this);

    // ---- Connections ----
    connect(waveform_, &WaveformPanel::file_loaded,   this, &ChopSlicesPanel::on_file_loaded);
    connect(waveform_, &WaveformPanel::marker_changed, this, &ChopSlicesPanel::on_marker_changed);
    connect(waveform_, &WaveformPanel::wav_dropped,   this, &ChopSlicesPanel::on_wav_dropped);
    connect(load_btn,  &QPushButton::clicked,         this, &ChopSlicesPanel::on_load_file);
    connect(export_btn_, &QPushButton::clicked,       this, &ChopSlicesPanel::on_export);
    connect(play_btn_, &QPushButton::clicked,         this, &ChopSlicesPanel::on_play_slice);

    connect(slider, &QSlider::valueChanged, this, [this](int v) {
        waveform_->set_sensitivity(v);
    });

    // Space bar on the panel → play slice
    setFocusPolicy(Qt::StrongFocus);
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------
void ChopSlicesPanel::on_file_loaded(const QString& details) {
    info_label_->setText(details);
    bool ready = waveform_->is_ready() && waveform_->slicer()->get_markers().has_beat();
    export_btn_->setEnabled(ready);
    play_btn_->setEnabled(waveform_->is_ready());
}

void ChopSlicesPanel::on_marker_changed(int location) {
    marker_label_->setText(QString("Marker location: %1 samples").arg(location));
}

void ChopSlicesPanel::on_wav_dropped(const QString& path) {
    QString err;
    if (!waveform_->load_file(path, err)) {
        QMessageBox::warning(this, "Could not load file", err);
    }
}

void ChopSlicesPanel::on_load_file() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open WAV File", "", "WAV Files (*.wav *.WAV);;All Files (*)");
    if (!path.isEmpty()) {
        QString err;
        if (!waveform_->load_file(path, err)) {
            QMessageBox::warning(this, "Could not load file", err);
        } else {
            // Auto-fill prefix from filename stem
            QFileInfo fi(path);
            QString stem = fi.completeBaseName();
            prefix_edit_->setText(stem.left(12));
        }
    }
}

void ChopSlicesPanel::on_export() {
    if (!waveform_->is_ready()) return;

    QString wav_path = waveform_->file_path();
    QFileInfo fi(wav_path);
    QString dir = fi.absolutePath();

    QString dest = QFileDialog::getExistingDirectory(
        this, "Choose export directory", dir);
    if (dest.isEmpty()) return;

    do_export(prefix_edit_->text(), dest);
}

void ChopSlicesPanel::on_play_slice() {
    if (!waveform_->is_ready()) return;
    Sample slice = waveform_->slicer()->get_selected_slice();
    player_->play(slice);
}

// ---------------------------------------------------------------------------
// Export: slices + MIDI + auto-PGM
// ---------------------------------------------------------------------------
void ChopSlicesPanel::do_export(const QString& prefix, const QString& dir) {
    Slicer* slicer = waveform_->slicer();
    if (!slicer) return;

    std::string dir_s    = dir.toStdString();
    std::string prefix_s = prefix.toStdString();

    try {
        // 1. Export WAV slices
        std::vector<std::string> slice_files =
            slicer->export_slices(dir_s, prefix_s);

        // 2. Export MIDI groove
        const Markers& markers = slicer->get_markers();
        std::vector<int> positions = markers.positions();
        int sample_rate = slicer->get_slice(0).format().sample_rate;

        MidiSequenceBuilder midi_builder(sample_rate, 120, 480);
        std::string midi_path = dir_s + "/" + prefix_s + ".mid";
        midi_builder.build(midi_path, positions, 36);

        // 3. Create auto PGM: new empty program, assign slice names to pads
        Program auto_pgm;
        int n = static_cast<int>(slice_files.size());
        for (int i = 0; i < n && i < 64; i++) {
            QFileInfo fi(QString::fromStdString(slice_files[i]));
            std::string name = fi.completeBaseName().toStdString();
            if (name.size() > 16) name = name.substr(0, 16);
            Pad pad = auto_pgm.get_pad(i);
            Layer layer = pad.get_layer(0);
            layer.set_sample_name(name);
            layer.set_level(100);
            layer.set_range({0, 127});
            PadMixer mixer = pad.get_mixer();
            mixer.set(PadMixer::VOLUME, 100);
            mixer.set(PadMixer::PAN, 50);
            layer.set_one_shot();
        }
        std::string pgm_path = dir_s + "/" + prefix_s + ".pgm";
        auto_pgm.save(pgm_path);

        QMessageBox::information(this, "Export complete",
            QString("Exported %1 slices, MIDI groove, and PGM to:\n%2")
                .arg(n).arg(dir));

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Export failed", QString::fromStdString(e.what()));
    }
}
