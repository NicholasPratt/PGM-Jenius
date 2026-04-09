#pragma once
#include <QWidget>
#include <QString>

class Program;

// Builds a JJOS "instrument" program from a single sample.
// Each of the 64 pads is assigned the same sample with a tuning offset so it
// plays at the correct pitch for its JJOS chromatic MIDI note (note 36 + pad_index).
class JJOSInstrumentPanel : public QWidget {
    Q_OBJECT
public:
    explicit JJOSInstrumentPanel(Program* program, QWidget* parent = nullptr);

    void set_program(Program* program);

signals:
    void program_modified();
    void sample_dir_selected(const QString& dir);

private slots:
    void on_browse_wav();
    void on_build();

private:
    Program* program_;

    class QLineEdit*   wav_edit_;
    class QComboBox*   root_note_combo_;
    class QComboBox*   play_mode_combo_;
    class QPushButton* build_btn_;
    class QLabel*      status_label_;
};
