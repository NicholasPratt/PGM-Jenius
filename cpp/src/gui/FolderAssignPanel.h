#pragma once
#include <QWidget>
#include <QString>

class Program;

class FolderAssignPanel : public QWidget {
    Q_OBJECT
public:
    explicit FolderAssignPanel(Program* program, QWidget* parent = nullptr);

    void set_program(Program* program);

signals:
    void program_modified();
    void sample_dir_selected(const QString& dir);

private slots:
    void on_browse();
    void on_apply();

private:
    Program* program_;

    class QLineEdit*   folder_edit_;
    class QPushButton* apply_btn_;
    class QLabel*      status_label_;
};
