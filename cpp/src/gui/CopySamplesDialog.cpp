#include "CopySamplesDialog.h"
#include "../pgm/Program.h"
#include "../pgm/Pad.h"
#include "../pgm/Layer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QDialogButtonBox>
#include <QProgressDialog>
#include <QMessageBox>
#include <algorithm>
#include <set>

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------
static QStringList wav_candidates(const QString& dir, const QString& name) {
    return { QDir(dir).filePath(name + ".wav"),
             QDir(dir).filePath(name + ".WAV") };
}

static QString find_in_dirs(const QString& name, const QStringList& dirs) {
    for (const QString& dir : dirs) {
        for (const QString& path : wav_candidates(dir, name)) {
            if (QFileInfo::exists(path)) return path;
        }
    }
    return {};
}

// ---------------------------------------------------------------------------
// constructor
// ---------------------------------------------------------------------------
CopySamplesDialog::CopySamplesDialog(Program* program,
                                     const QString& dest_dir,
                                     const QStringList& search_dirs,
                                     QWidget* parent)
    : QDialog(parent), dest_dir_(dest_dir)
{
    setWindowTitle("Copy Samples to Program Folder");
    setMinimumSize(680, 440);

    // Collect unique sample names from all pads × layers
    std::set<QString> seen;
    for (int p = 0; p < 64; ++p) {
        Pad pad = program->get_pad(p);
        for (int l = 0; l < Pad::LAYER_COUNT; ++l) {
            Layer layer = pad.get_layer(l);
            const std::string s = layer.get_sample_name();
            if (s.empty()) continue;
            const QString name = QString::fromStdString(s).trimmed();
            if (name.isEmpty() || seen.count(name)) continue;
            seen.insert(name);
            samples_.push_back({ name, {}, false });
        }
    }

    // ── Layout ───────────────────────────────────────────────────────────────
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);

    dest_label_ = new QLabel(
        QString("Copy sample WAV files into: <b>%1</b>").arg(dest_dir_));
    dest_label_->setWordWrap(true);
    root->addWidget(dest_label_);

    // Extra search dir row
    auto* search_row = new QHBoxLayout;
    search_row->addWidget(new QLabel("Also search in:"));
    extra_dir_edit_ = new QLineEdit;
    extra_dir_edit_->setPlaceholderText("Optional extra search folder…");
    auto* browse_btn  = new QPushButton("Browse…");
    auto* search_btn  = new QPushButton("Search");
    search_row->addWidget(extra_dir_edit_, 1);
    search_row->addWidget(browse_btn);
    search_row->addWidget(search_btn);
    root->addLayout(search_row);

    // Table: ☑ | Sample name | Source path
    table_ = new QTableWidget(0, 3);
    table_->setHorizontalHeaderLabels({"", "Sample", "Source path"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    table_->setColumnWidth(0, 28);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->verticalHeader()->setVisible(false);
    table_->setShowGrid(false);
    root->addWidget(table_, 1);

    // Selection helpers
    auto* sel_row = new QHBoxLayout;
    auto* sel_found_btn    = new QPushButton("Select found");
    auto* deselect_all_btn = new QPushButton("Deselect all");
    sel_row->addWidget(sel_found_btn);
    sel_row->addWidget(deselect_all_btn);
    sel_row->addStretch();
    root->addLayout(sel_row);

    status_label_ = new QLabel;
    status_label_->setStyleSheet("color: #aaa; font-size: 11px;");
    root->addWidget(status_label_);

    // Buttons
    auto* btn_row = new QHBoxLayout;
    copy_btn_ = new QPushButton("Copy selected files");
    copy_btn_->setDefault(true);
    auto* skip_btn = new QPushButton("Skip");
    btn_row->addStretch();
    btn_row->addWidget(copy_btn_);
    btn_row->addWidget(skip_btn);
    root->addLayout(btn_row);

    // Initial resolve with provided dirs
    resolve(search_dirs);
    populate_table();
    update_copy_button();

    connect(browse_btn,       &QPushButton::clicked, this, &CopySamplesDialog::on_browse_extra);
    connect(search_btn,       &QPushButton::clicked, this, &CopySamplesDialog::on_search);
    connect(sel_found_btn,    &QPushButton::clicked, this, &CopySamplesDialog::on_select_found);
    connect(deselect_all_btn, &QPushButton::clicked, this, &CopySamplesDialog::on_deselect_all);
    connect(copy_btn_,        &QPushButton::clicked, this, &CopySamplesDialog::on_copy);
    connect(skip_btn,         &QPushButton::clicked, this, &QDialog::reject);
}

bool CopySamplesDialog::has_copyable_samples() const {
    for (const auto& s : samples_)
        if (!s.already_there) return true;
    return false;
}

// ---------------------------------------------------------------------------
// resolve
// ---------------------------------------------------------------------------
void CopySamplesDialog::resolve(const QStringList& dirs) {
    QStringList all_dirs;
    // Always include dest_dir to detect samples already there
    if (!dest_dir_.isEmpty()) all_dirs << dest_dir_;
    all_dirs << dirs;
    // Remove duplicates and empty strings
    all_dirs.removeDuplicates();
    all_dirs.removeAll({});

    for (auto& s : samples_) {
        const QString found = find_in_dirs(s.name, all_dirs);
        s.source_path  = found;
        s.already_there = !found.isEmpty() &&
                          QFileInfo(found).absolutePath() == QFileInfo(dest_dir_).absolutePath();
    }
}

// ---------------------------------------------------------------------------
// populate_table
// ---------------------------------------------------------------------------
void CopySamplesDialog::populate_table() {
    table_->setRowCount(static_cast<int>(samples_.size()));

    for (int row = 0; row < static_cast<int>(samples_.size()); ++row) {
        const SampleCopyInfo& s = samples_[row];

        // Column 0: checkbox
        auto* chk = new QCheckBox;
        if (s.already_there) {
            chk->setChecked(false);
            chk->setEnabled(false);
        } else {
            chk->setChecked(!s.source_path.isEmpty());
        }
        auto* cell_widget = new QWidget;
        auto* cell_layout = new QHBoxLayout(cell_widget);
        cell_layout->addWidget(chk);
        cell_layout->setAlignment(Qt::AlignCenter);
        cell_layout->setContentsMargins(0, 0, 0, 0);
        table_->setCellWidget(row, 0, cell_widget);

        // Column 1: sample name
        auto* name_item = new QTableWidgetItem(s.name);
        table_->setItem(row, 1, name_item);

        // Column 2: source path / status
        QString status_text;
        if (s.already_there)       status_text = u8"\u2713 Already in destination";
        else if (!s.source_path.isEmpty()) status_text = s.source_path;
        else                       status_text = u8"\u2717 Not found";

        auto* path_item = new QTableWidgetItem(status_text);
        if (s.already_there)
            path_item->setForeground(QColor("#6a6"));
        else if (s.source_path.isEmpty())
            path_item->setForeground(QColor("#a55"));
        table_->setItem(row, 2, path_item);

        connect(chk, &QCheckBox::checkStateChanged, this, [this](int) {
            update_copy_button();
        });
    }
}

// ---------------------------------------------------------------------------
// update_copy_button
// ---------------------------------------------------------------------------
void CopySamplesDialog::update_copy_button() {
    int count = 0;
    for (int row = 0; row < table_->rowCount(); ++row) {
        auto* cw  = table_->cellWidget(row, 0);
        auto* chk = cw ? cw->findChild<QCheckBox*>() : nullptr;
        if (chk && chk->isChecked()) ++count;
    }
    copy_btn_->setText(count > 0
        ? QString("Copy %1 file%2").arg(count).arg(count == 1 ? "" : "s")
        : "Copy selected files");
    copy_btn_->setEnabled(count > 0);

    const int total   = static_cast<int>(samples_.size());
    const int found   = static_cast<int>(std::count_if(samples_.begin(), samples_.end(),
                                         [](const SampleCopyInfo& s){ return !s.source_path.isEmpty(); }));
    const int already = static_cast<int>(std::count_if(samples_.begin(), samples_.end(),
                                         [](const SampleCopyInfo& s){ return s.already_there; }));
    status_label_->setText(QString("%1 sample(s) referenced  |  %2 found  |  %3 already in destination")
        .arg(total).arg(found).arg(already));
}

// ---------------------------------------------------------------------------
// slots
// ---------------------------------------------------------------------------
void CopySamplesDialog::on_browse_extra() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Search Folder", QDir::homePath());
    if (!dir.isEmpty()) extra_dir_edit_->setText(dir);
}

void CopySamplesDialog::on_search() {
    QStringList dirs;
    const QString extra = extra_dir_edit_->text().trimmed();
    if (!extra.isEmpty()) dirs << extra;
    resolve(dirs);
    populate_table();
    update_copy_button();
}

void CopySamplesDialog::on_select_found() {
    for (int row = 0; row < table_->rowCount(); ++row) {
        auto* cw  = table_->cellWidget(row, 0);
        auto* chk = cw ? cw->findChild<QCheckBox*>() : nullptr;
        if (chk && chk->isEnabled())
            chk->setChecked(!samples_[row].source_path.isEmpty());
    }
}

void CopySamplesDialog::on_deselect_all() {
    for (int row = 0; row < table_->rowCount(); ++row) {
        auto* cw  = table_->cellWidget(row, 0);
        auto* chk = cw ? cw->findChild<QCheckBox*>() : nullptr;
        if (chk && chk->isEnabled()) chk->setChecked(false);
    }
}

void CopySamplesDialog::on_copy() {
    QStringList to_copy;
    for (int row = 0; row < table_->rowCount(); ++row) {
        auto* cw  = table_->cellWidget(row, 0);
        auto* chk = cw ? cw->findChild<QCheckBox*>() : nullptr;
        if (chk && chk->isChecked() && !samples_[row].source_path.isEmpty())
            to_copy << samples_[row].source_path;
    }
    if (to_copy.isEmpty()) return;

    QProgressDialog progress("Copying samples…", "Cancel", 0, to_copy.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);

    int copied = 0, skipped = 0, failed = 0;
    for (int i = 0; i < to_copy.size(); ++i) {
        if (progress.wasCanceled()) break;
        progress.setValue(i);

        const QFileInfo src(to_copy[i]);
        const QString   dst = QDir(dest_dir_).filePath(src.fileName());

        if (QFileInfo::exists(dst)) {
            ++skipped;
            continue;
        }
        if (QFile::copy(to_copy[i], dst)) ++copied;
        else                               ++failed;
    }
    progress.setValue(to_copy.size());

    QString msg = QString("Copied %1 file(s) to:\n%2").arg(copied).arg(dest_dir_);
    if (skipped) msg += QString("\n%1 already present, skipped.").arg(skipped);
    if (failed)  msg += QString("\n%1 file(s) failed to copy.").arg(failed);
    QMessageBox::information(this, "Copy Complete", msg);

    accept();
}
