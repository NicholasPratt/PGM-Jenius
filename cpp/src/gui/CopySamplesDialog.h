#pragma once
#include <QDialog>
#include <QString>
#include <QStringList>
#include <vector>

class Program;
class QTableWidget;
class QLabel;
class QLineEdit;
class QPushButton;

struct SampleCopyInfo {
    QString name;           // basename without extension
    QString source_path;    // resolved full path (empty = not found)
    bool    already_there;  // file already exists in dest_dir
};

// Modal dialog shown after saving a program.
// Collects all sample names referenced by the program, attempts to resolve
// their WAV paths from a set of search directories, and copies selected ones
// into dest_dir so the MPC can find them next to the .pgm file.
class CopySamplesDialog : public QDialog {
    Q_OBJECT
public:
    // dest_dir  : folder where the .pgm was just saved (copy target)
    // search_dirs: candidate dirs to search for WAV files
    CopySamplesDialog(Program* program,
                      const QString& dest_dir,
                      const QStringList& search_dirs,
                      QWidget* parent = nullptr);

    // True if there is anything to copy (samples not already in dest_dir).
    bool has_copyable_samples() const;

private slots:
    void on_browse_extra();
    void on_search();
    void on_select_found();
    void on_deselect_all();
    void on_copy();

private:
    void resolve(const QStringList& dirs);
    void populate_table();
    void update_copy_button();

    QString dest_dir_;
    std::vector<SampleCopyInfo> samples_;

    QLabel*       dest_label_;
    QLineEdit*    extra_dir_edit_;
    QTableWidget* table_;
    QPushButton*  copy_btn_;
    QLabel*       status_label_;
};
