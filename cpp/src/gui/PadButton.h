#pragma once
#include <QPushButton>
#include <QString>
#include "../pgm/Pad.h"

// A pad button in the pad grid.
// Displays the pad number and up to 4 layer sample names.
// Accepts WAV file drops — emits wav_dropped signal.
class PadButton : public QPushButton {
    Q_OBJECT
public:
    explicit PadButton(int pad_index, QWidget* parent = nullptr);

    int pad_index() const { return pad_index_; }

    // Refresh label from the given Pad's layer names
    void refresh(Pad& pad);

    // Highlight as selected
    void set_selected(bool selected);

signals:
    void clicked_pad(int pad_index);
    void wav_dropped(int pad_index, QString path);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    int  pad_index_;
    bool selected_ = false;

    void update_style();
};
