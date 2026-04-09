#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QString>
#include <memory>
#include "../pgm/Program.h"
#include "../pgm/Profile.h"
#include "ProgramPanel.h"
#include "ChopSlicesPanel.h"
#include "MultisamplePanel.h"
#include "BatchCreatePanel.h"
#include "FolderAssignPanel.h"
#include "JJOSInstrumentPanel.h"
#include "CopySamplesDialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    explicit MainWindow(const QString& pgm_path, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_open();
    void on_new();
    void on_save();
    void on_save_as();
    void on_switch_profile(const QString& profile_name);

private:
    void apply_theme();
    void setup_menus();
    void load_program(const QString& path);
    void show_program_panel();
    void show_copy_samples_dialog();

    std::unique_ptr<Program> program_;
    Profile  profile_      = Profile::MPC1000;
    QString  current_path_;
    QString  sample_dir_;   // last known directory containing WAV samples

    QTabWidget*        tabs_            = nullptr;
    ProgramPanel*      pgm_panel_       = nullptr;
    ChopSlicesPanel*   chop_panel_      = nullptr;
    MultisamplePanel*  multisample_panel_ = nullptr;
    BatchCreatePanel*  batch_panel_     = nullptr;
    FolderAssignPanel*     folder_assign_panel_   = nullptr;
    JJOSInstrumentPanel*   jjos_instrument_panel_ = nullptr;
};
