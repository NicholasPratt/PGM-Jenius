#include "MultisamplePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

MultisamplePanel::MultisamplePanel(Program* program, QWidget* parent)
    : QWidget(parent), program_(program)
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);

    // --- Folder row ---
    auto* folder_row = new QHBoxLayout;
    folder_edit_ = new QLineEdit;
    folder_edit_->setPlaceholderText("Folder containing WAV files…");
    auto* browse_btn = new QPushButton("Browse…");
    scan_btn_ = new QPushButton("Scan");
    folder_row->addWidget(new QLabel("Folder:"));
    folder_row->addWidget(folder_edit_, 1);
    folder_row->addWidget(browse_btn);
    folder_row->addWidget(scan_btn_);
    root->addLayout(folder_row);

    // --- Instructions ---
    auto* hint = new QLabel(
        "WAV files must contain a note name in their filename (e.g. \"Piano_C4.wav\", \"Strings_A#3.wav\").");
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888; font-size: 11px;");
    root->addWidget(hint);

    // --- Results table ---
    table_ = new QTableWidget(0, 3);
    table_->setHorizontalHeaderLabels({"Filename", "Note", "MIDI #"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->verticalHeader()->setVisible(false);
    root->addWidget(table_, 1);

    // --- Bottom row ---
    auto* bottom_row = new QHBoxLayout;
    status_label_ = new QLabel("No folder scanned.");
    apply_btn_ = new QPushButton("Apply to Program");
    apply_btn_->setEnabled(false);
    bottom_row->addWidget(status_label_, 1);
    bottom_row->addWidget(apply_btn_);
    root->addLayout(bottom_row);

    connect(browse_btn, &QPushButton::clicked, this, &MultisamplePanel::on_browse);
    connect(scan_btn_,  &QPushButton::clicked, this, &MultisamplePanel::on_scan);
    connect(apply_btn_, &QPushButton::clicked, this, &MultisamplePanel::on_apply);
}

void MultisamplePanel::set_program(Program* program) {
    program_ = program;
}

void MultisamplePanel::on_browse() {
    QSettings settings;
    QString start = settings.value("last_wav_dir", QDir::homePath()).toString();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Sample Folder", start);
    if (!dir.isEmpty()) {
        folder_edit_->setText(dir);
        settings.setValue("last_wav_dir", dir);
    }
}

void MultisamplePanel::on_scan() {
    QString dir = folder_edit_->text().trimmed();
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "No Folder", "Please choose a folder first.");
        return;
    }
    try {
        entries_ = MultisampleBuilder::scan(dir.toStdString());
        update_table(entries_);
        if (entries_.empty()) {
            status_label_->setText("No note-named WAV files found.");
            apply_btn_->setEnabled(false);
        } else {
            status_label_->setText(QString("%1 sample(s) found.").arg(entries_.size()));
            apply_btn_->setEnabled(program_ != nullptr);
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Scan Error", QString::fromStdString(e.what()));
    }
}

void MultisamplePanel::on_apply() {
    if (!program_ || entries_.empty()) return;
    MultisampleBuilder::apply(*program_, entries_);
    QMessageBox::information(this, "Applied",
        QString("%1 samples assigned to pads.\n\nRemember to Save the program.").arg(entries_.size()));
}

void MultisamplePanel::update_table(const std::vector<SampleEntry>& entries) {
    table_->setRowCount(static_cast<int>(entries.size()));
    for (int i = 0; i < static_cast<int>(entries.size()); i++) {
        table_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(entries[i].filename)));
        table_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(entries[i].note_name)));
        table_->setItem(i, 2, new QTableWidgetItem(QString::number(entries[i].midi_note)));
    }
}
