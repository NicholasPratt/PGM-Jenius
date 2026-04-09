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
#include <QRegularExpression>
#include <algorithm>

// Returns a sort priority for a sample filename.
// Lower = earlier in pad assignment order:
//   0 Kick  1 Snare  2 Tom  3 Hat  4 Cymbal  5 Percussion  6 Misc
static int drum_category(const QString& base_name)
{
    const QString n = base_name.toLower();

    auto has = [&](const char* s) { return n.contains(QLatin1String(s)); };
    auto word = [&](const char* pattern) {
        return QRegularExpression(QLatin1String(pattern),
            QRegularExpression::CaseInsensitiveOption).match(n).hasMatch();
    };

    // ── KICK ──────────────────────────────────────────────────────────────
    if (has("kick") || has("kik") || has("bassdrum") || has("bass_drum") ||
        has("bass-drum") || word(R"(\bkd\b)") || word(R"(\bbd\b)") ||
        word(R"(\b808\b)") || word(R"(\b909k\b)") ||
        word(R"((?:^|[_\-\s])k(?:[_\-\s]|\d|$))") ||   // _k_ / k1 / -k
        word(R"((?:^|[_\-\s])sub(?:[_\-\s]|$))"))       // sub-kick
        return 0;

    // ── SNARE ─────────────────────────────────────────────────────────────
    if (has("snare") || has("snr") || word(R"(\bsd\b)") ||
        word(R"((?:^|[_\-\s])sn(?:[_\-\s\d]|$))") ||   // sn, sn1, _sn_
        has("rimsnare") || has("brush"))
        return 1;

    // ── TOM ───────────────────────────────────────────────────────────────
    if (has("tom") || has("floor") || has("rack") ||
        word(R"(\b[hml]t\b)") ||                        // ht mt lt
        word(R"(\bft\b)") ||                            // floor tom
        word(R"((?:^|[_\-\s])t[123](?:[_\-\s]|$))"))   // t1 t2 t3
        return 2;

    // ── HI-HAT ────────────────────────────────────────────────────────────
    if (has("hihat") || has("hi_hat") || has("hi-hat") ||
        word(R"(\bhh\b)") || word(R"(\bhat\b)") ||
        word(R"(\bchh?\b)") || word(R"(\bohh?\b)") ||  // ch chh oh ohh
        word(R"((?:^|[_\-\s])oh(?:[_\-\s]|$))") ||
        word(R"((?:^|[_\-\s])ch(?:[_\-\s]|$))") ||
        (has("closed") && !has("snare")) || (has("open") && !has("snare")))
        return 3;

    // ── CYMBAL ────────────────────────────────────────────────────────────
    if (has("cymbal") || has("cym") || has("crash") || has("splash") ||
        word(R"(\bcrs\b)") || word(R"(\bspl\b)") ||
        word(R"((?:^|[_\-\s])ride(?:[_\-\s]|$))") ||   // ride (not "override")
        word(R"(\brd\b)") ||
        has("bell") || has("china"))
        return 4;

    // ── PERCUSSION ────────────────────────────────────────────────────────
    if (has("perc") || has("clap") || word(R"(\bclp\b)") ||
        has("rimshot") || word(R"(\brim\b)") || word(R"(\brm\b)") ||
        has("cowbell") || word(R"(\bcb\b)") ||
        has("tamb") || has("shaker") || word(R"(\bshk\b)") ||
        has("clave") || word(R"(\bclv\b)") ||
        has("conga") || has("bongo") || has("cabasa") ||
        has("woodblock") || has("wood_block") || word(R"(\bwb\b)") ||
        has("agogo") || has("guiro") || has("marac"))
        return 5;

    // ── MISC / FX ─────────────────────────────────────────────────────────
    return 6;
}

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
        "Assigns WAV files to layer 1 of pads 1-64, sorted by drum category: "
        "kicks \u2192 snares \u2192 toms \u2192 hats \u2192 cymbals \u2192 percussion \u2192 misc. "
        "Detection uses common abbreviations (kd, bd, snr, sn, hh, ch, cym\u2026). "
        "Files beyond 64 are skipped.");
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

    QFileInfoList files = dir.entryInfoList(
        QStringList() << "*.wav" << "*.WAV",
        QDir::Files | QDir::Readable,
        QDir::Name | QDir::IgnoreCase);

    // Sort by drum category, then alphabetically within each category.
    std::stable_sort(files.begin(), files.end(),
        [](const QFileInfo& a, const QFileInfo& b) {
            int ca = drum_category(a.completeBaseName());
            int cb = drum_category(b.completeBaseName());
            if (ca != cb) return ca < cb;
            return a.completeBaseName().toLower() < b.completeBaseName().toLower();
        });

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
