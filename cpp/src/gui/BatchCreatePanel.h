#pragma once
#include <QWidget>
#include <QString>

class BatchCreatePanel : public QWidget {
    Q_OBJECT
public:
    explicit BatchCreatePanel(QWidget* parent = nullptr);

private slots:
    void on_browse_source();
    void on_browse_dest();
    void on_run();

private:
    void log(const QString& msg);

    class QLineEdit*   source_edit_;
    class QLineEdit*   dest_edit_;
    class QPushButton* run_btn_;
    class QTextEdit*   log_edit_;
};
