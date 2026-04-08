#include "FolderAssignPanel.h"
#include "../pgm/Program.h"
#include "../pgm/Pad.h"
#include "../pgm/Layer.h"
#include "../pgm/PadMixer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QFileInfoList>
#include <algorithm>

FolderAssignPanel::FolderAssignPanel(Program* program, QWidget* parent)
    : QWidget(parent), program_(program)
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);

    auto* folder_row = new QHBoxLayout;
    folder_edit_ = new QLineEdit;
    folder_edit_->setPlaceholderText("Folder containing WAV files to assign to pads 1-64...");
    auto* browse_btn = new QPushButton("Browse...");
    folder_row->addWidget(new QLabel("Folder:"));
    folder_row->addWidget(folder_edit_, 1);
    folder_row->addWidget(browse_btn);
    root->addLayout(folder_row);

    auto* hint = new QLabel(
        "Assigns WAV files in folder order to layer 1 of pads 1-64. If there are more than 64 files, only the first 64 are added.");
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888; font-size: 11px;");
    root->addWidget(hint);

    status_label_ = new QLabel("No folder selected.");
    root->addWidget(status_label_);

    apply_btn_ = new QPushButton("Assign Folder to Pads");
    apply_btn_->setEnabled(program_ != nullptr);
    root->addWidget(apply_btn_);
    root->addStretch();

    connect(browse_btn, &QPushButton::clicked, this, &FolderAssignPanel::on_browse);
    connect(apply_btn_, &QPushButton::clicked, this, &FolderAssignPanel::on_apply);
}

void FolderAssignPanel::set_program(Program* program) {
    program_ = program;
    apply_btn_->setEnabled(program_ != nullptr);
}

void FolderAssignPanel::on_browse() {
    QSettings settings;
    QString start = settings.value("last_folder_assign_dir", QDir::homePath()).toString();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Sample Folder", start);
    if (!dir.isEmpty()) {
        folder_edit_->setText(dir);
        status_label_->setText(QString("Ready to assign samples from %1").arg(dir));
        settings.setValue("last_folder_assign_dir", dir);
    }
}

void FolderAssignPanel::on_apply() {
    if (!program_) return;

    const QString dir_path = folder_edit_->text().trimmed();
    if (dir_path.isEmpty()) {
        QMessageBox::warning(this, "No Folder", "Please choose a folder first.");
        return;
    }

    QDir dir(dir_path);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Invalid Folder", "Selected folder does not exist.");
        return;
    }

    const QFileInfoList files = dir.entryInfoList(
        QStringList() << "*.wav" << "*.WAV",
        QDir::Files | QDir::Readable,
        QDir::Name | QDir::IgnoreCase);

    if (files.isEmpty()) {
        QMessageBox::information(this, "No Samples", "No WAV files found in the selected folder.");
        return;
    }

    const int assign_count = std::min(static_cast<int>(files.size()), 64);
    QString last_assigned_name;

    for (int i = 0; i < assign_count; ++i) {
        const QFileInfo& file = files[i];
        std::string sample_name = file.completeBaseName().toStdString();
        if (sample_name.size() > 16)
            sample_name = sample_name.substr(0, 16);

        Pad pad = program_->get_pad(i);
        Layer layer = pad.get_layer(0);
        layer.set_sample_name(sample_name);
        layer.set_level(100);
        layer.set_range({0, 127});
        layer.set_one_shot();
        PadMixer mixer = pad.get_mixer();
        mixer.set(PadMixer::VOLUME, 100);
        mixer.set(PadMixer::PAN, 50);

        last_assigned_name = file.completeBaseName();
    }

    status_label_->setText(QString("%1 sample(s) assigned to pads 1-%2.")
        .arg(assign_count)
        .arg(assign_count));

    emit sample_dir_selected(dir_path);
    emit program_modified();

    if (files.size() > 64) {
        QMessageBox::information(this, "Folder Assign", QString("last sample added %1").arg(last_assigned_name));
    } else {
        QMessageBox::information(this, "Folder Assign",
            QString("%1 sample(s) assigned to layer 1 of pads 1-%2.")
                .arg(assign_count)
                .arg(assign_count));
    }
}
