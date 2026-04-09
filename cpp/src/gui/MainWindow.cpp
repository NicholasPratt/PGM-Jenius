#include "MainWindow.h"
#include "../pgm/Pad.h"
#include "../pgm/PadMixer.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QFileInfo>
#include <QLabel>
#include <QSettings>
#include <QFont>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("PGM-Jenius");
    resize(1100, 700);
    apply_theme();
    setup_menus();
    on_new();
}

MainWindow::MainWindow(const QString& pgm_path, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("PGM-Jenius");
    resize(1100, 700);
    apply_theme();
    setup_menus();
    load_program(pgm_path);
}

MainWindow::~MainWindow() = default;

void MainWindow::apply_theme() {
    QFont ui_font("Avenir Next Condensed", 11);
    ui_font.setStyleStrategy(QFont::PreferAntialias);
    setFont(ui_font);

    setStyleSheet(
        "QMainWindow {"
        "  background-color: #0b0b0f;"
        "  color: #f3d8b8;"
        "}"
        "QWidget {"
        "  background: transparent;"
        "  color: #f3d8b8;"
        "  selection-background-color: #ff4d3c;"
        "  selection-color: #120607;"
        "}"
        "QTabWidget::pane {"
        "  border: 1px solid #57402e;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "              stop:0 #15111a, stop:0.55 #100d13, stop:1 #09090d);"
        "  top: -1px;"
        "}"
        "QTabBar::tab {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "              stop:0 #342124, stop:1 #1a1115);"
        "  color: #d3b48f;"
        "  border: 1px solid #6f4732;"
        "  padding: 8px 16px;"
        "  margin-right: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "              stop:0 #c8482d, stop:1 #631d18);"
        "  color: #fff2de;"
        "  border-color: #ff8f45;"
        "}"
        "QGroupBox {"
        "  border: 1px solid #5a4030;"
        "  border-radius: 8px;"
        "  margin-top: 14px;"
        "  padding-top: 14px;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "              stop:0 #151018, stop:1 #0d0a0f);"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 8px;"
        "  color: #ffbf73;"
        "}"
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "              stop:0 #4e2226, stop:1 #220d11);"
        "  color: #f7dbc0;"
        "  border: 1px solid #8e5036;"
        "  border-radius: 7px;"
        "  padding: 8px 14px;"
        "}"
        "QPushButton:hover {"
        "  border-color: #ff9f59;"
        "  color: #fff2e3;"
        "}"
        "QPushButton:pressed {"
        "  background: #7c241c;"
        "}"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
        "  background-color: #110d12;"
        "  color: #f7dbc0;"
        "  border: 1px solid #6f4732;"
        "  border-radius: 6px;"
        "  padding: 5px 8px;"
        "}"
        "QComboBox::drop-down {"
        "  border: 0;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #120e13;"
        "  color: #f3d8b8;"
        "  border: 1px solid #6f4732;"
        "  selection-background-color: #7c241c;"
        "  selection-color: #fff0dd;"
        "  outline: 0;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "  min-height: 24px;"
        "  padding: 4px 10px;"
        "}"
        "QScrollArea, QScrollArea > QWidget > QWidget {"
        "  background: transparent;"
        "}"
        "QMenuBar {"
        "  background-color: #120e13;"
        "  color: #e8c9a5;"
        "}"
        "QMenuBar::item:selected, QMenu::item:selected {"
        "  background-color: #7c241c;"
        "  color: #fff0dd;"
        "}"
        "QMenu {"
        "  background-color: #120e13;"
        "  border: 1px solid #6f4732;"
        "  color: #e8c9a5;"
        "}"
        "QStatusBar {"
        "  background-color: #120e13;"
        "  color: #d8b48b;"
        "  border-top: 1px solid #5a4030;"
        "}"
    );
}

void MainWindow::setup_menus() {
    QMenu* file_menu = menuBar()->addMenu("&File");

    auto* new_action = file_menu->addAction("&New");
    new_action->setShortcut(QKeySequence::New);
    connect(new_action, &QAction::triggered, this, &MainWindow::on_new);

    auto* open_action = file_menu->addAction("&Open…");
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::on_open);

    auto* save_action = file_menu->addAction("&Save");
    save_action->setShortcut(QKeySequence::Save);
    connect(save_action, &QAction::triggered, this, &MainWindow::on_save);

    auto* save_as_action = file_menu->addAction("Save &As…");
    save_as_action->setShortcut(QKeySequence::SaveAs);
    connect(save_as_action, &QAction::triggered, this, &MainWindow::on_save_as);

    file_menu->addSeparator();

    auto* quit_action = file_menu->addAction("&Quit");
    quit_action->setShortcut(QKeySequence::Quit);
    connect(quit_action, &QAction::triggered, this, &QWidget::close);

    // MPC model menu
    QMenu* model_menu = menuBar()->addMenu("&MPC Model");
    auto* group = new QActionGroup(this);

    auto add_model = [&](const QString& name) {
        auto* action = model_menu->addAction(name);
        action->setCheckable(true);
        action->setChecked(name == "MPC1000");
        group->addAction(action);
        connect(action, &QAction::triggered, this, [this, name]() {
            on_switch_profile(name);
        });
    };
    add_model("MPC1000");

    // macOS native menu bar
    menuBar()->setNativeMenuBar(true);
}

void MainWindow::on_new() {
    program_ = std::make_unique<Program>();
    current_path_.clear();
    show_program_panel();
    setWindowTitle("PGM-Jenius — Untitled");
    statusBar()->showMessage("New program created.");
}

void MainWindow::load_program(const QString& path) {
    try {
        program_ = std::make_unique<Program>(Program::open(path.toStdString()));
        current_path_ = path;

        // Remember last-opened directory
        QSettings settings;
        settings.setValue("last_pgm_dir", QFileInfo(path).absolutePath());

        show_program_panel();
        setWindowTitle("PGM-Jenius — " + QFileInfo(path).fileName());
        statusBar()->showMessage("Loaded: " + path);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
    }
}

void MainWindow::show_program_panel() {
    if (!program_) return;

    tabs_                  = new QTabWidget(this);
    pgm_panel_             = new ProgramPanel(program_.get(), profile_, tabs_);
    chop_panel_            = new ChopSlicesPanel(program_.get(), tabs_);
    multisample_panel_     = new MultisamplePanel(program_.get(), tabs_);
    batch_panel_           = new BatchCreatePanel(tabs_);
    folder_assign_panel_   = new FolderAssignPanel(program_.get(), tabs_);
    jjos_instrument_panel_ = new JJOSInstrumentPanel(program_.get(), tabs_);

    tabs_->addTab(pgm_panel_,             "Program Editor");
    tabs_->addTab(chop_panel_,            "Chop Slices");
    tabs_->addTab(multisample_panel_,     "Multisample");
    tabs_->addTab(folder_assign_panel_,   "Folder Assign");
    tabs_->addTab(jjos_instrument_panel_, "JJOS Instrument");
    tabs_->addTab(batch_panel_,           "Batch Create");

    // Let the pad grid play samples from the same directory as the .pgm file
    if (!current_path_.isEmpty())
        pgm_panel_->set_sample_dir(QFileInfo(current_path_).absolutePath());

    setCentralWidget(tabs_);

    connect(pgm_panel_, &ProgramPanel::program_modified, this, [this]() {
        QString title = windowTitle();
        if (!title.startsWith("*"))
            setWindowTitle("* " + title);
    });

    auto update_sample_dir = [this](const QString& dir) {
        sample_dir_ = dir;
        if (pgm_panel_) pgm_panel_->set_sample_dir(dir);
    };
    auto mark_modified = [this]() {
        if (pgm_panel_) pgm_panel_->load();
        QString title = windowTitle();
        if (!title.startsWith("*")) setWindowTitle("* " + title);
    };

    connect(folder_assign_panel_,   &FolderAssignPanel::sample_dir_selected,   this, update_sample_dir);
    connect(folder_assign_panel_,   &FolderAssignPanel::program_modified,       this, mark_modified);
    connect(jjos_instrument_panel_, &JJOSInstrumentPanel::sample_dir_selected,  this, update_sample_dir);
    connect(jjos_instrument_panel_, &JJOSInstrumentPanel::program_modified,     this, mark_modified);
}

void MainWindow::on_open() {
    QSettings settings;
    QString start = settings.value("last_pgm_dir", QDir::homePath()).toString();
    QString path = QFileDialog::getOpenFileName(
        this, "Open Program", start, "MPC Program (*.pgm);;All Files (*)");
    if (!path.isEmpty()) load_program(path);
}

void MainWindow::on_save() {
    if (!program_) return;
    if (current_path_.isEmpty()) { on_save_as(); return; }
    try {
        // Repair legacy broken pan encoding from older builds that wrote signed
        // values directly instead of MPC/JJOS 0..100 pan values.
        for (int i = 0; i < 64; ++i) {
            Pad pad = program_->get_pad(i);
            PadMixer mixer = pad.get_mixer();
            int pan = mixer.get(PadMixer::PAN);
            if (pan > 100) {
                const int signed_pan = static_cast<int>(static_cast<int8_t>(pan));
                const int repaired = std::clamp(signed_pan + 50, 0, 100);
                mixer.set(PadMixer::PAN, repaired);
            }
        }

        program_->save(current_path_.toStdString());
        QString title = windowTitle();
        if (title.startsWith("* ")) setWindowTitle(title.mid(2));
        statusBar()->showMessage("Saved: " + current_path_);
        show_copy_samples_dialog();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
    }
}

void MainWindow::on_save_as() {
    if (!program_) return;
    QSettings settings;
    QString start = settings.value("last_pgm_dir", QDir::homePath()).toString();
    QString path = QFileDialog::getSaveFileName(
        this, "Save Program As", start, "MPC Program (*.pgm);;All Files (*)");
    if (path.isEmpty()) return;

    QFileInfo info(path);
    if (info.suffix().compare("pgm", Qt::CaseInsensitive) != 0) {
        const QString dir = info.dir().absolutePath();
        const QString base_name = info.completeBaseName().isEmpty() ? info.fileName() : info.completeBaseName();
        path = QDir(dir).filePath(base_name + ".pgm");
    }

    current_path_ = path;
    settings.setValue("last_pgm_dir", QFileInfo(path).absolutePath());
    on_save();
}

void MainWindow::on_switch_profile(const QString& profile_name) {
    profile_ = Profile::get(profile_name.toStdString());
    if (program_) show_program_panel();
}

void MainWindow::show_copy_samples_dialog() {
    if (!program_ || current_path_.isEmpty()) return;

    const QString dest_dir = QFileInfo(current_path_).absolutePath();

    // Build search dirs: the .pgm's own directory plus any known sample dir
    QStringList search_dirs;
    search_dirs << dest_dir;
    if (!sample_dir_.isEmpty() && sample_dir_ != dest_dir)
        search_dirs << sample_dir_;
    // Also try the dir the .pgm was loaded from (may differ after Save As)
    const QString pgm_dir = QFileInfo(current_path_).absolutePath();
    if (!pgm_dir.isEmpty() && !search_dirs.contains(pgm_dir))
        search_dirs << pgm_dir;
    search_dirs.removeDuplicates();

    CopySamplesDialog dlg(program_.get(), dest_dir, search_dirs, this);

    // Skip the dialog entirely when there's nothing to copy
    if (!dlg.has_copyable_samples()) return;

    dlg.exec();
}
