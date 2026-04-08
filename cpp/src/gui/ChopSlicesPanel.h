#pragma once
#include <QWidget>
#include <QString>
#include <memory>
#include "WaveformPanel.h"
#include "../audio/AudioPlayer.h"
#include "../pgm/Program.h"

// The "Chop Slices" tab — contains the waveform display, sensitivity slider,
// export controls, and a reference to the current Program for auto-pgm export.
class ChopSlicesPanel : public QWidget {
    Q_OBJECT
public:
    explicit ChopSlicesPanel(Program* program, QWidget* parent = nullptr);

    WaveformPanel* waveform_panel() { return waveform_; }

private slots:
    void on_file_loaded(const QString& details);
    void on_marker_changed(int location);
    void on_wav_dropped(const QString& path);
    void on_load_file();
    void on_export();
    void on_play_slice();

private:
    void do_export(const QString& prefix, const QString& dir);

    Program*      program_;  // borrowed — for auto-pgm creation on export
    WaveformPanel* waveform_;
    AudioPlayer*   player_;

    // UI elements updated dynamically
    class QLabel*    info_label_;
    class QLabel*    marker_label_;
    class QLineEdit* prefix_edit_;
    class QPushButton* export_btn_;
    class QPushButton* play_btn_;
};
