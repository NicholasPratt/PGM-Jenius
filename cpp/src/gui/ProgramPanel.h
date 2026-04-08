#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QHash>
#include <array>
#include <vector>
#include "../pgm/Program.h"
#include "../pgm/Profile.h"
#include "../audio/AudioPlayer.h"
#include "../audio/Sample.h"
#include "PadButton.h"
#include "ParameterWidget.h"

// Main program editor panel.
// Left: bank tabs (A-D) with pad button grids.
// Right: parameter tabs (Samples, Params, Envelope, Filters, Mixer, Sliders).
class ProgramPanel : public QWidget {
    Q_OBJECT
public:
    ProgramPanel(Program* program, const Profile& profile, QWidget* parent = nullptr);

    // Set the directory to search for WAV files when playing pad samples
    void set_sample_dir(const QString& dir) { sample_dir_ = dir; }

    // Reload all widgets from current program state
    void load();

    // Return the program (for MainWindow save operations)
    Program* program() const { return program_; }

    int selected_pad() const { return selected_pad_; }

signals:
    void program_modified();

private slots:
    void on_pad_clicked(int pad_index);
    void on_wav_dropped(int pad_index, const QString& path);

private:
    void build_left_panel(QWidget* parent);
    void build_right_panel(QWidget* parent);

    // Build individual right-panel tabs
    QWidget* build_samples_tab();
    QWidget* build_params_tab();
    QWidget* build_envelope_tab();
    QWidget* build_filters_tab();
    QWidget* build_mixer_tab();
    QWidget* build_sliders_tab();

    // Build a form panel for a given set of parameters on a given element kind
    QWidget* build_element_panel(ElementKind kind, int sub_index,
                                 const std::vector<Parameter>& params);

    QString normalize_sample_key(const std::string& sample_name) const;
    QString find_sample_path(const std::string& sample_name) const;
    void    remember_sample_path(const std::string& sample_name, const QString& path);

    void select_pad(int pad_index);
    void refresh_pad_button(int pad_index);
    void refresh_right_panel();

    Program*      program_;
    Profile       profile_;
    int           selected_pad_ = 0;
    QString       sample_dir_;
    QHash<QString, QString> sample_paths_;
    AudioPlayer*  player_ = nullptr;

    // Left side
    QTabWidget*                    bank_tabs_;
    std::array<PadButton*, 64>     pad_buttons_;

    // Right side
    QTabWidget*                    param_tabs_;
    std::vector<ParamWidgetBase*>  all_widgets_;  // all active param widgets
};
