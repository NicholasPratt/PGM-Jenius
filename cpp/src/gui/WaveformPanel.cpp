#include "WaveformPanel.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QPainterPath>
#include <QFileInfo>
#include <algorithm>
#include <cmath>

static constexpr int CHANNEL_HEIGHT = 120;  // px per channel
static constexpr int LEFT_MARGIN    = 10;

WaveformPanel::WaveformPanel(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(CHANNEL_HEIGHT * 2 + 20);
    setMinimumWidth(400);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);

    setStyleSheet("background-color: #1a1a1a; border: 1px solid #444;");
}

bool WaveformPanel::load_file(const QString& path, QString& error_out) {
    try {
        auto s = std::make_unique<Sample>(Sample::load(path.toStdString()));
        if (!s->is_valid()) {
            error_out = "Failed to decode audio file.";
            return false;
        }
        auto sl = std::make_unique<Slicer>(*s, WINDOW_SIZE, OVERLAP_RATIO, LOCAL_ENERGY_WINDOW);
        sample_ = std::move(s);
        slicer_ = std::move(sl);
        file_path_ = path;
        update();
        emit file_loaded(file_details());
        return true;
    } catch (const std::exception& e) {
        error_out = QString::fromStdString(e.what());
        return false;
    }
}

QString WaveformPanel::file_details() const {
    if (!slicer_) return "Drag and Drop a WAV file here";
    QFileInfo fi(file_path_);
    int frames = slicer_->frame_length();
    int markers = slicer_->get_markers().size();
    float bpm   = slicer_->get_markers().get_tempo(8);
    QString s   = fi.fileName() + QString("  (%1 samples, %2 slices").arg(frames).arg(markers);
    if (bpm > 0) s += QString(", ~%1 BPM").arg(static_cast<int>(bpm));
    s += ")";
    return s;
}

// ---------------------------------------------------------------------------
// Marker manipulation
// ---------------------------------------------------------------------------
void WaveformPanel::set_sensitivity(int v) {
    if (!slicer_) return;
    slicer_->set_sensitivity(v);
    update();
    emit file_loaded(file_details());
}

void WaveformPanel::select_marker(int shift) {
    if (!slicer_) return;
    slicer_->get_markers().select_marker(shift);
    update();
    emit marker_changed(selected_marker_location());
}

void WaveformPanel::select_closest_marker(int mouse_x) {
    if (!slicer_) return;
    int frame = mouse_x_to_frame(mouse_x);
    slicer_->get_markers().select_closest(frame);
    update();
    emit marker_changed(selected_marker_location());
}

void WaveformPanel::nudge_marker(int frames) {
    if (!slicer_) return;
    slicer_->get_markers().nudge(frames, *slicer_);
    update();
    emit marker_changed(selected_marker_location());
}

void WaveformPanel::delete_selected_marker() {
    if (!slicer_) return;
    slicer_->get_markers().delete_selected();
    update();
    emit marker_changed(selected_marker_location());
}

void WaveformPanel::insert_marker() {
    if (!slicer_) return;
    slicer_->get_markers().insert_marker();
    update();
    emit marker_changed(selected_marker_location());
}

int WaveformPanel::selected_marker_location() const {
    if (!slicer_) return 0;
    return slicer_->get_markers().get_selected_location();
}

// ---------------------------------------------------------------------------
// Coordinate helpers
// ---------------------------------------------------------------------------
int WaveformPanel::frame_to_pixel_x(int frame, int total_frames, int w) const {
    if (total_frames == 0) return LEFT_MARGIN;
    return LEFT_MARGIN + static_cast<int>(
        static_cast<double>(frame) / total_frames * (w - LEFT_MARGIN * 2));
}

int WaveformPanel::mouse_x_to_frame(int mouse_x) const {
    if (!slicer_) return 0;
    int w = width() - LEFT_MARGIN * 2;
    int total = slicer_->frame_length();
    return static_cast<int>(
        static_cast<double>(mouse_x - LEFT_MARGIN) / w * total);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------
void WaveformPanel::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Background
    p.fillRect(rect(), QColor(0x1a, 0x1a, 0x1a));

    if (!slicer_) {
        p.setPen(QColor(0x88, 0x88, 0x88));
        p.drawText(rect(), Qt::AlignCenter, "Drag and Drop a WAV file here");
        return;
    }

    const auto& channels = slicer_->get_channels();
    int n_ch = std::min(static_cast<int>(channels.size()), 2);
    int total = slicer_->frame_length();
    int w     = width();
    int per_ch_h = height() / std::max(n_ch, 1);

    for (int ch = 0; ch < n_ch; ch++) {
        draw_channel(p, channels[ch], ch, w, per_ch_h);
    }
    draw_markers(p, total, w, height());
}

void WaveformPanel::draw_channel(QPainter& p, const std::vector<int>& samples,
                                  int channel, int w, int ch_height)
{
    int n    = static_cast<int>(samples.size());
    int y0   = channel * ch_height + ch_height / 2;

    // Center line
    p.setPen(QColor(0x44, 0x44, 0x44));
    p.drawLine(LEFT_MARGIN, y0, w - LEFT_MARGIN, y0);

    if (n == 0) return;

    // Decimate: aim for one sample per 2 pixels
    int draw_w   = w - LEFT_MARGIN * 2;
    int increment = std::max(1, n / (draw_w * 2));
    double x_scale = static_cast<double>(draw_w) / n;
    double y_scale = -static_cast<double>(ch_height / 2 - 4) / 32767.0;

    p.setPen(QColor(0x44, 0xaa, 0x44));  // green waveform

    int old_x = LEFT_MARGIN;
    int old_y = y0;
    for (int t = 0; t < n; t += increment) {
        int x = LEFT_MARGIN + static_cast<int>(t * x_scale);
        int y = y0 + static_cast<int>(samples[t] * y_scale);
        p.drawLine(old_x, old_y, x, y);
        old_x = x;
        old_y = y;
    }
}

void WaveformPanel::draw_markers(QPainter& p, int total_frames, int w, int h) {
    if (!slicer_ || total_frames == 0) return;
    const Markers& markers = slicer_->get_markers();
    int selected_idx = markers.get_selected_index();

    for (int i = 0; i < markers.size(); i++) {
        int frame = markers.get(i);
        int x     = frame_to_pixel_x(frame, total_frames, w);

        if (i == selected_idx) {
            p.setPen(QPen(QColor(0xff, 0x88, 0x00), 2));  // orange for selected
        } else {
            p.setPen(QPen(QColor(0xff, 0x22, 0x22), 1));  // red for others
        }
        p.drawLine(x, 0, x, h - 16);

        // Draw downward triangle pointer at bottom for selected
        if (i == selected_idx) {
            int ty = h - 14;
            QPolygon tri;
            tri << QPoint(x, ty + 10) << QPoint(x - 6, ty) << QPoint(x + 6, ty);
            p.setBrush(QColor(0xff, 0x88, 0x00));
            p.setPen(Qt::NoPen);
            p.drawPolygon(tri);
            p.setPen(QPen(QColor(0xff, 0x88, 0x00), 2));
        }
    }
}

// ---------------------------------------------------------------------------
// Input events
// ---------------------------------------------------------------------------
void WaveformPanel::mousePressEvent(QMouseEvent* e) {
    if (!slicer_) return;
    setFocus();
    select_closest_marker(e->pos().x());
}

void WaveformPanel::wheelEvent(QWheelEvent* e) {
    if (!slicer_) return;
    int delta = e->angleDelta().y();
    nudge_marker(delta > 0 ? -100 : 100);
    e->accept();
}

void WaveformPanel::keyPressEvent(QKeyEvent* e) {
    if (!slicer_) { QWidget::keyPressEvent(e); return; }
    switch (e->key()) {
        case Qt::Key_Left:
            if (e->modifiers() & Qt::ShiftModifier)    nudge_marker(-1000);
            else if (e->modifiers() & Qt::AltModifier) nudge_marker(-100);
            else                                        select_marker(-1);
            break;
        case Qt::Key_Right:
            if (e->modifiers() & Qt::ShiftModifier)    nudge_marker(+1000);
            else if (e->modifiers() & Qt::AltModifier) nudge_marker(+100);
            else                                        select_marker(+1);
            break;
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
            delete_selected_marker();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            insert_marker();
            break;
        default:
            QWidget::keyPressEvent(e);
            return;
    }
    e->accept();
}

void WaveformPanel::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasUrls()) {
        for (const auto& url : e->mimeData()->urls()) {
            if (url.isLocalFile()) {
                QString p = url.toLocalFile();
                if (p.endsWith(".wav", Qt::CaseInsensitive)) {
                    e->acceptProposedAction();
                    return;
                }
            }
        }
    }
    e->ignore();
}

void WaveformPanel::dropEvent(QDropEvent* e) {
    for (const auto& url : e->mimeData()->urls()) {
        if (url.isLocalFile()) {
            QString path = url.toLocalFile();
            if (path.endsWith(".wav", Qt::CaseInsensitive)) {
                emit wav_dropped(path);
                e->acceptProposedAction();
                return;
            }
        }
    }
}
