#pragma once
#include <QWidget>
#include <QString>
#include <memory>
#include "../audio/Sample.h"
#include "../audio/Slicer.h"

// Draws a PCM waveform with beat marker overlays.
// Owns the Sample and Slicer for the currently loaded audio file.
// Keyboard and mouse interaction for marker manipulation.
class WaveformPanel : public QWidget {
    Q_OBJECT
public:
    static constexpr int WINDOW_SIZE         = 1024;
    static constexpr int OVERLAP_RATIO       = 1;
    static constexpr int LOCAL_ENERGY_WINDOW = 43;

    explicit WaveformPanel(QWidget* parent = nullptr);

    // Load a WAV file and run beat detection
    bool load_file(const QString& path, QString& error_out);

    bool is_ready() const { return slicer_ != nullptr; }

    const Slicer* slicer() const { return slicer_.get(); }
    Slicer*       slicer()       { return slicer_.get(); }

    QString file_details() const;
    QString file_path()    const { return file_path_; }

    // Marker manipulation (call repaint after these)
    void set_sensitivity(int v);
    void select_marker(int shift);
    void select_closest_marker(int mouse_x);
    void nudge_marker(int frames);
    void delete_selected_marker();
    void insert_marker();
    int  selected_marker_location() const;

signals:
    void file_loaded(QString details);
    void marker_changed(int location);
    void wav_dropped(QString path);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private:
    void draw_channel(QPainter& p, const std::vector<int>& samples,
                      int channel, int width, int height);
    void draw_markers(QPainter& p, int total_frames, int width, int height);

    // Convert mouse x → frame position
    int  mouse_x_to_frame(int mouse_x) const;
    // Convert frame position → pixel x
    int  frame_to_pixel_x(int frame, int total_frames, int width) const;

    std::unique_ptr<Sample> sample_;
    std::unique_ptr<Slicer> slicer_;
    QString file_path_;
};
