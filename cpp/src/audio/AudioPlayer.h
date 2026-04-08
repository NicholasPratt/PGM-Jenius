#pragma once
#include "Sample.h"
#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QObject>
#include <QByteArray>
#include <memory>

// Simple in-process sample playback backed by Qt Multimedia.
class AudioPlayer : public QObject {
    Q_OBJECT
public:
    explicit AudioPlayer(QObject* parent = nullptr);
    ~AudioPlayer();

    void play(const Sample& sample);
    void stop();
    bool is_playing() const;

private:
    QByteArray  convert_to_output(const Sample& sample, const QAudioFormat& format) const;
    float       sample_at(const std::vector<float>& src, int frames, int src_channels,
                          double frame_pos, int channel, int dst_channels) const;
    void        setup_sink(const AudioFormat& format);

    std::unique_ptr<QAudioSink> sink_;
    QAudioFormat                output_format_;
    QByteArray                  pcm_data_;
    QBuffer                     pcm_buffer_;
};
