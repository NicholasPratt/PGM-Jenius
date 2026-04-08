#pragma once
#include <QWidget>
#include <QString>
#include <vector>
#include "../pgm/Program.h"
#include "../pgm/MultisampleBuilder.h"

class MultisamplePanel : public QWidget {
    Q_OBJECT
public:
    explicit MultisamplePanel(Program* program, QWidget* parent = nullptr);

    void set_program(Program* program);

private slots:
    void on_browse();
    void on_scan();
    void on_apply();

private:
    void update_table(const std::vector<SampleEntry>& entries);

    Program* program_;
    std::vector<SampleEntry> entries_;

    class QLineEdit*   folder_edit_;
    class QTableWidget* table_;
    class QPushButton* scan_btn_;
    class QPushButton* apply_btn_;
    class QLabel*      status_label_;
};
