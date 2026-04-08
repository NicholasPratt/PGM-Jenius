#include "BatchCreatePanel.h"
#include "../pgm/Program.h"
#include "../pgm/MultisampleBuilder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <filesystem>

namespace fs = std::filesystem;

BatchCreatePanel::BatchCreatePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);

    // --- Source folder row ---
    auto* src_row = new QHBoxLayout;
    source_edit_ = new QLineEdit;
    source_edit_->setPlaceholderText("Source folder (contains subfolders of WAV files)…");
    auto* src_btn = new QPushButton("Browse…");
    src_row->addWidget(new QLabel("Source:"));
    src_row->addWidget(source_edit_, 1);
    src_row->addWidget(src_btn);
    root->addLayout(src_row);

    // --- Destination folder row ---
    auto* dst_row = new QHBoxLayout;
    dest_edit_ = new QLineEdit;
    dest_edit_->setPlaceholderText("Destination folder for .pgm files…");
    auto* dst_btn = new QPushButton("Browse…");
    dst_row->addWidget(new QLabel("Output:"));
    dst_row->addWidget(dest_edit_, 1);
    dst_row->addWidget(dst_btn);
    root->addLayout(dst_row);

    // --- Instructions ---
    auto* hint = new QLabel(
        "Each subfolder becomes one .pgm file. WAV files must contain note names (e.g. \"Piano_C4.wav\").");
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888; font-size: 11px;");
    root->addWidget(hint);

    // --- Log output ---
    log_edit_ = new QTextEdit;
    log_edit_->setReadOnly(true);
    log_edit_->setPlaceholderText("Output log will appear here…");
    log_edit_->setFontFamily("Courier");
    root->addWidget(log_edit_, 1);

    // --- Run button ---
    run_btn_ = new QPushButton("Run Batch Create");
    run_btn_->setFixedHeight(32);
    root->addWidget(run_btn_);

    connect(src_btn,  &QPushButton::clicked, this, &BatchCreatePanel::on_browse_source);
    connect(dst_btn,  &QPushButton::clicked, this, &BatchCreatePanel::on_browse_dest);
    connect(run_btn_, &QPushButton::clicked, this, &BatchCreatePanel::on_run);
}

void BatchCreatePanel::on_browse_source() {
    QSettings settings;
    QString start = settings.value("last_batch_src_dir", QDir::homePath()).toString();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Source Folder", start);
    if (!dir.isEmpty()) {
        source_edit_->setText(dir);
        settings.setValue("last_batch_src_dir", dir);
    }
}

void BatchCreatePanel::on_browse_dest() {
    QSettings settings;
    QString start = settings.value("last_batch_dst_dir", QDir::homePath()).toString();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Folder", start);
    if (!dir.isEmpty()) {
        dest_edit_->setText(dir);
        settings.setValue("last_batch_dst_dir", dir);
    }
}

void BatchCreatePanel::on_run() {
    QString src = source_edit_->text().trimmed();
    QString dst = dest_edit_->text().trimmed();
    if (src.isEmpty() || dst.isEmpty()) {
        log("ERROR: Please set both source and output folders.");
        return;
    }

    log_edit_->clear();
    log("Starting batch create…");
    log(QString("Source: %1").arg(src));
    log(QString("Output: %1").arg(dst));
    log("---");

    int created = 0;
    int skipped = 0;

    try {
        fs::path src_path = src.toStdString();
        fs::path dst_path = dst.toStdString();

        if (!fs::is_directory(src_path)) {
            log("ERROR: Source is not a valid directory.");
            return;
        }
        fs::create_directories(dst_path);

        for (const auto& entry : fs::directory_iterator(src_path)) {
            if (!entry.is_directory()) continue;

            std::string subfolder_name = entry.path().filename().string();
            log(QString("Processing: %1").arg(QString::fromStdString(subfolder_name)));

            auto samples = MultisampleBuilder::scan(entry.path().string());
            if (samples.empty()) {
                log("  -> No note-named WAV files found, skipping.");
                skipped++;
                continue;
            }

            // Create a fresh default program
            Program program;
            MultisampleBuilder::apply(program, samples);

            fs::path pgm_path = dst_path / (subfolder_name + ".pgm");
            program.save(pgm_path.string());

            log(QString("  -> Wrote %1 (%2 samples)")
                .arg(QString::fromStdString(pgm_path.filename().string()))
                .arg(samples.size()));
            created++;
        }
    } catch (const std::exception& e) {
        log(QString("ERROR: %1").arg(QString::fromStdString(e.what())));
    }

    log("---");
    log(QString("Done. %1 program(s) created, %2 folder(s) skipped.").arg(created).arg(skipped));
}

void BatchCreatePanel::log(const QString& msg) {
    log_edit_->append(msg);
}
