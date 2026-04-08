#include "PadButton.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

PadButton::PadButton(int pad_index, QWidget* parent)
    : QPushButton(parent), pad_index_(pad_index)
{
    setAcceptDrops(true);
    setMinimumSize(80, 60);
    setCheckable(true);
    update_style();

    connect(this, &QPushButton::clicked, this, [this]() {
        emit clicked_pad(pad_index_);
    });
}

void PadButton::refresh(Pad& pad) {
    QString text = QString::number(pad_index_ + 1);
    Layer layer0 = pad.get_layer(0);
    std::string name = layer0.get_sample_name();
    if (!name.empty()) {
        QString qname = QString::fromStdString(name);
        if (qname.length() > 10)
            qname = qname.left(5) + "…" + qname.right(4);
        text += "\n" + qname;
    }
    setText(text);
}

void PadButton::set_selected(bool selected) {
    selected_ = selected;
    setChecked(selected);
    update_style();
}

void PadButton::update_style() {
    if (selected_) {
        setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
            "              stop:0 #ff6a4d, stop:0.55 #d12a22, stop:1 #5f0c11);"
            "  color: #fff4e4;"
            "  border: 2px solid #ffc165;"
            "  border-radius: 9px;"
            "  padding: 8px 6px;"
            "  font-weight: 700;"
            "}"
        );
    } else {
        setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
            "              stop:0 #9c251f, stop:0.58 #651218, stop:1 #28060d);"
            "  color: #ffd8b4;"
            "  border: 1px solid #b85034;"
            "  border-radius: 9px;"
            "  padding: 8px 6px;"
            "  font-weight: 700;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
            "              stop:0 #bf2d24, stop:0.58 #7f181d, stop:1 #340913);"
            "  border-color: #ff9b54;"
            "  color: #fff1de;"
            "}"
            "QPushButton:pressed {"
            "  background: #d63428;"
            "}"
        );
    }
}

void PadButton::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto& urls = event->mimeData()->urls();
        for (const auto& url : urls) {
            if (url.isLocalFile()) {
                QString path = url.toLocalFile();
                if (path.endsWith(".wav", Qt::CaseInsensitive) ||
                    path.endsWith(".WAV", Qt::CaseInsensitive)) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void PadButton::dropEvent(QDropEvent* event) {
    const auto& urls = event->mimeData()->urls();
    for (const auto& url : urls) {
        if (url.isLocalFile()) {
            QString path = url.toLocalFile();
            if (path.endsWith(".wav", Qt::CaseInsensitive)) {
                emit wav_dropped(pad_index_, path);
                event->acceptProposedAction();
                return;
            }
        }
    }
}
