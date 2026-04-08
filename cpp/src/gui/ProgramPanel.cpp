#include "ProgramPanel.h"
#include "../pgm/Pad.h"
#include "../pgm/Layer.h"
#include "../pgm/PadEnvelope.h"
#include "../pgm/PadFilter.h"
#include "../pgm/PadMixer.h"
#include "../pgm/Slider.h"
#include <QSplitter>
#include <QLabel>
#include <QFormLayout>
#include <QScrollArea>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QMessageBox>

namespace {
Range full_velocity_range() {
    return {0, 127};
}
}

ProgramPanel::ProgramPanel(Program* program, const Profile& profile, QWidget* parent)
    : QWidget(parent), program_(program), profile_(profile)
{
    pad_buttons_.fill(nullptr);
    player_ = new AudioPlayer(this);

    auto* root_layout = new QHBoxLayout(this);
    root_layout->setContentsMargins(8, 8, 8, 8);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    root_layout->addWidget(splitter);

    // Left panel: bank tabs with pad grids
    auto* left = new QWidget(this);
    build_left_panel(left);
    splitter->addWidget(left);

    // Right panel: parameter tabs
    auto* right = new QWidget(this);
    build_right_panel(right);
    splitter->addWidget(right);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    // Select pad 0 by default
    select_pad(0);
}

// ---------------------------------------------------------------------------
// Left panel: 4 bank tabs, each with a grid of pad buttons
// ---------------------------------------------------------------------------
void ProgramPanel::build_left_panel(QWidget* parent) {
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    bank_tabs_ = new QTabWidget(parent);
    layout->addWidget(bank_tabs_);

    const int rows = profile_.row_number;
    const int cols = profile_.col_number;
    const int bank_size = profile_.pad_number();

    for (int bank = 0; bank < 4; bank++) {
        auto* grid_widget = new QWidget();
        auto* grid = new QGridLayout(grid_widget);
        grid->setSpacing(6);

        const int bank_offset = bank_size * bank;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                // Java original fills from bottom-left, i.e. row 0 = bottom row
                int pad_id = bank_offset + (rows - 1 - row) * cols + col;
                auto* btn = new PadButton(pad_id, grid_widget);
                pad_buttons_[pad_id] = btn;
                grid->addWidget(btn, row, col);

                connect(btn, &PadButton::clicked_pad, this, &ProgramPanel::on_pad_clicked);
                connect(btn, &PadButton::wav_dropped,  this, &ProgramPanel::on_wav_dropped);
            }
        }

        QString bank_label = QString("  %1  ").arg(QChar('A' + bank));
        bank_tabs_->addTab(grid_widget, bank_label);
    }

    // Refresh all pad button labels
    for (int i = 0; i < 64; i++) {
        if (pad_buttons_[i]) refresh_pad_button(i);
    }
}

// ---------------------------------------------------------------------------
// Right panel: parameter tabs
// ---------------------------------------------------------------------------
void ProgramPanel::build_right_panel(QWidget* parent) {
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    param_tabs_ = new QTabWidget(parent);
    layout->addWidget(param_tabs_);

    param_tabs_->addTab(build_samples_tab(),  "Samples");
    param_tabs_->addTab(build_params_tab(),   "Params");
    param_tabs_->addTab(build_envelope_tab(), "Envelope");
    param_tabs_->addTab(build_filters_tab(),  "Filters");
    param_tabs_->addTab(build_mixer_tab(),    "Mixer");
    param_tabs_->addTab(build_sliders_tab(),  "Sliders");
}

// ---------------------------------------------------------------------------
// Samples tab: 2x2 grid of layer group boxes
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_samples_tab() {
    auto* container = new QWidget();
    auto* grid = new QGridLayout(container);
    grid->setSpacing(8);
    grid->setContentsMargins(6, 6, 6, 6);

    for (int layer = 0; layer < 4; layer++) {
        ElementRef ref;
        ref.kind      = ElementKind::LAYER;
        ref.pad_index = selected_pad_;
        ref.sub_index = layer;

        auto* box = new QGroupBox(QString("Layer %1").arg(layer + 1), container);
        auto* form = new QFormLayout(box);
        form->setRowWrapPolicy(QFormLayout::WrapLongRows);

        const std::vector<Parameter> layer_params = {
            Layer::SAMPLE_NAME, Layer::LEVEL, Layer::TUNING_PARAM,
            Layer::PLAY_MODE, Layer::RANGE_PARAM
        };

        for (const auto& p : layer_params) {
            auto* w = make_param_widget(program_, ref, p, box);
            if (w) {
                form->addRow(w);
                all_widgets_.push_back(w);
            }
        }
        grid->addWidget(box, layer / 2, layer % 2);
    }
    return container;
}

// ---------------------------------------------------------------------------
// Params tab: pad-level parameters
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_params_tab() {
    ElementRef ref{ ElementKind::PAD, selected_pad_, 0 };
    return build_element_panel(ElementKind::PAD, 0,
        { Pad::MIDI_NOTE, Pad::VOICE_OVERLAP, Pad::MUTE_GROUP });
}

// ---------------------------------------------------------------------------
// Envelope tab
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_envelope_tab() {
    return build_element_panel(ElementKind::ENVELOPE, 0,
        { PadEnvelope::ATTACK, PadEnvelope::DECAY,
          PadEnvelope::DECAY_MODE, PadEnvelope::VELOCITY_TO_LEVEL });
}

// ---------------------------------------------------------------------------
// Filters tab: sub-tabs for Filter1 and Filter2
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_filters_tab() {
    auto* tabs = new QTabWidget();

    auto add_filter = [&](ElementKind kind, const QString& label) {
        if (kind == ElementKind::FILTER1) {
            tabs->addTab(build_element_panel(kind, 0,
                { PadFilter1::TYPE, PadFilter1::CUTOFF, PadFilter1::RESONANCE,
                  PadFilter1::VELOCITY_TO_FREQ, PadFilter1::PRE_ATTENUATION }), label);
        } else {
            tabs->addTab(build_element_panel(kind, 0,
                { PadFilter2::TYPE, PadFilter2::CUTOFF, PadFilter2::RESONANCE,
                  PadFilter2::VELOCITY_TO_FREQ }), label);
        }
    };

    add_filter(ElementKind::FILTER1, "Filter 1");
    if (profile_.filter_number > 1)
        add_filter(ElementKind::FILTER2, "Filter 2");

    return tabs;
}

// ---------------------------------------------------------------------------
// Mixer tab
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_mixer_tab() {
    return build_element_panel(ElementKind::MIXER, 0,
        { PadMixer::VOLUME, PadMixer::PAN, PadMixer::OUTPUT,
          PadMixer::FX_SEND, PadMixer::FX_SEND_LEVEL });
}

// ---------------------------------------------------------------------------
// Sliders tab: sub-tabs per slider (not pad-specific)
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_sliders_tab() {
    auto* tabs = new QTabWidget();

    for (int s = 0; s < profile_.slider_number; s++) {
        ElementRef ref{ ElementKind::SLIDER, 0, s };
        auto* panel = new QWidget();
        auto* form  = new QFormLayout(panel);

        const std::vector<Parameter> slider_params = {
            Slider::PAD, Slider::PARAMETER_ASSIGN, Slider::TUNE_RANGE,
            Slider::FILTER_RANGE, Slider::LAYER_RANGE,
            Slider::ATTACK_RANGE, Slider::DECAY_RANGE
        };
        for (const auto& p : slider_params) {
            auto* w = make_param_widget(program_, ref, p, panel);
            if (w) {
                form->addRow(w);
                all_widgets_.push_back(w);
            }
        }
        tabs->addTab(panel, QString("Slider %1").arg(s + 1));
    }
    return tabs;
}

// ---------------------------------------------------------------------------
// Generic element panel builder
// ---------------------------------------------------------------------------
QWidget* ProgramPanel::build_element_panel(ElementKind kind, int sub_index,
                                            const std::vector<Parameter>& params) {
    auto* panel = new QWidget();
    auto* form  = new QVBoxLayout(panel);
    form->setAlignment(Qt::AlignTop);

    ElementRef ref{ kind, selected_pad_, sub_index };

    for (const auto& p : params) {
        auto* w = make_param_widget(program_, ref, p, panel);
        if (w) {
            form->addWidget(w);
            all_widgets_.push_back(w);
        }
    }
    return panel;
}

QString ProgramPanel::normalize_sample_key(const std::string& sample_name) const {
    return QString::fromStdString(sample_name).trimmed().toLower();
}

void ProgramPanel::remember_sample_path(const std::string& sample_name, const QString& path) {
    const QString key = normalize_sample_key(sample_name);
    if (key.isEmpty() || path.isEmpty()) return;

    sample_paths_.insert(key, path);
}

QString ProgramPanel::find_sample_path(const std::string& sample_name) const {
    const QString key = normalize_sample_key(sample_name);
    if (key.isEmpty()) return {};

    auto known = sample_paths_.constFind(key);
    if (known != sample_paths_.cend() && QFileInfo::exists(known.value()))
        return known.value();

    if (sample_dir_.isEmpty()) return {};

    QDir dir(sample_dir_);
    QString stem = QString::fromStdString(sample_name);

    const QString wav_path = dir.filePath(stem + ".wav");
    if (QFileInfo::exists(wav_path))
        return wav_path;

    const QString upper_wav_path = dir.filePath(stem + ".WAV");
    if (QFileInfo::exists(upper_wav_path))
        return upper_wav_path;

    const QFileInfoList candidates = dir.entryInfoList(
        QStringList() << "*.wav" << "*.WAV",
        QDir::Files | QDir::Readable);

    for (const QFileInfo& candidate : candidates) {
        if (candidate.completeBaseName().compare(stem, Qt::CaseInsensitive) == 0)
            return candidate.absoluteFilePath();
    }

    return {};
}

// ---------------------------------------------------------------------------
// Pad selection
// ---------------------------------------------------------------------------
void ProgramPanel::select_pad(int pad_index) {
    // Deselect previous
    if (pad_buttons_[selected_pad_])
        pad_buttons_[selected_pad_]->set_selected(false);

    selected_pad_ = pad_index;

    if (pad_buttons_[selected_pad_])
        pad_buttons_[selected_pad_]->set_selected(true);

    // Propagate new pad index to all param widgets and reload
    for (auto* w : all_widgets_) {
        if (w) w->set_pad(pad_index);
    }
}

void ProgramPanel::refresh_pad_button(int pad_index) {
    if (!pad_buttons_[pad_index]) return;
    Pad pad = program_->get_pad(pad_index);
    pad_buttons_[pad_index]->refresh(pad);
}

void ProgramPanel::load() {
    for (auto* w : all_widgets_)
        if (w) w->load();
    for (int i = 0; i < 64; i++)
        refresh_pad_button(i);
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------
void ProgramPanel::on_pad_clicked(int pad_index) {
    select_pad(pad_index);

    Pad pad = program_->get_pad(pad_index);
    for (int layer_index = 0; layer_index < Pad::LAYER_COUNT; ++layer_index) {
        Layer layer = pad.get_layer(layer_index);
        const std::string name = layer.get_sample_name();
        if (name.empty())
            continue;

        const QString sample_path = find_sample_path(name);
        if (sample_path.isEmpty())
            continue;

        try {
            Sample sample = Sample::load(sample_path.toStdString());
            remember_sample_path(name, sample_path);
            player_->play(sample);
            return;
        } catch (...) {
        }
    }
}

void ProgramPanel::on_wav_dropped(int pad_index, const QString& path) {
    // Assign the WAV filename (without extension) to layer 0 of the target pad
    select_pad(pad_index);

    QFileInfo fi(path);
    if (sample_dir_.isEmpty())
        sample_dir_ = fi.absolutePath();

    std::string name = fi.completeBaseName().toStdString();
    if (name.size() > 16) name = name.substr(0, 16);

    Pad pad = program_->get_pad(pad_index);
    Layer layer = pad.get_layer(0);
    layer.set_sample_name(name);
    layer.set_level(100);
    layer.set_range(full_velocity_range());
    layer.set_one_shot();
    PadMixer mixer = pad.get_mixer();
    mixer.set(PadMixer::VOLUME, 100);
    mixer.set(PadMixer::PAN, 50);
    remember_sample_path(name, fi.absoluteFilePath());

    refresh_pad_button(pad_index);
    load();

    emit program_modified();
}

void ProgramPanel::refresh_right_panel() {
    for (auto* w : all_widgets_)
        if (w) w->load();
}
