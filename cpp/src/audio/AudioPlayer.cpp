#include "AudioPlayer.h"
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QtEndian>
#include <QMediaDevices>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <cstring>

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent) {}

AudioPlayer::~AudioPlayer() {
    stop();
}

namespace {
float clamp_sample(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}
}

void AudioPlayer::setup_sink(const AudioFormat& fmt) {
    QAudioFormat qfmt;
    qfmt.setSampleRate(fmt.sample_rate);
    qfmt.setChannelCount(fmt.channels);
    qfmt.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        qWarning() << "AudioPlayer: no default audio output device found";
        return;
    }

    if (!device.isFormatSupported(qfmt)) {
        qWarning() << "AudioPlayer: requested format not supported,"
                   << "resampling to device preferred format";
        qfmt = device.preferredFormat();
    }

    output_format_ = qfmt;
    sink_ = std::make_unique<QAudioSink>(device, qfmt);

    connect(sink_.get(), &QAudioSink::stateChanged, this, [this](QAudio::State state) {
        if (state == QAudio::IdleState) {
            sink_->stop();
            pcm_buffer_.close();
            return;
        }
        if (state == QAudio::StoppedState && sink_ && sink_->error() != QAudio::NoError) {
            qWarning() << "AudioPlayer: sink entered stopped state with error" << sink_->error();
        }
    });
}

float AudioPlayer::sample_at(const std::vector<float>& src, int frames, int src_channels,
                             double frame_pos, int channel, int dst_channels) const {
    if (src.empty() || frames <= 0 || src_channels <= 0) {
        return 0.0f;
    }

    int src_frame = std::clamp(static_cast<int>(frame_pos), 0, frames - 1);
    auto frame_value = [&](int src_channel) {
        int clamped_channel = std::clamp(src_channel, 0, src_channels - 1);
        return src[src_frame * src_channels + clamped_channel];
    };

    if (src_channels == dst_channels) {
        return frame_value(channel);
    }

    if (src_channels == 1) {
        return frame_value(0);
    }

    if (dst_channels == 1) {
        float mixed = 0.0f;
        for (int c = 0; c < src_channels; ++c) {
            mixed += frame_value(c);
        }
        return mixed / static_cast<float>(src_channels);
    }

    return frame_value(channel % src_channels);
}

QByteArray AudioPlayer::convert_to_output(const Sample& sample, const QAudioFormat& format) const {
    const auto& src = sample.interleaved_float();
    const AudioFormat& src_format = sample.format();
    const int src_channels = std::max(1, src_format.channels);
    const int src_frames = sample.frame_length();
    const int dst_channels = std::max(1, format.channelCount());
    const int dst_rate = std::max(1, format.sampleRate());
    const int src_rate = std::max(1, src_format.sample_rate);

    if (src_frames <= 0 || format.sampleFormat() == QAudioFormat::Unknown) {
        return {};
    }

    int dst_frames = static_cast<int>(
        std::llround(static_cast<double>(src_frames) * dst_rate / src_rate));
    dst_frames = std::max(dst_frames, 1);

    QByteArray out;
    out.resize(dst_frames * format.bytesPerFrame());
    char* dst = out.data();

    for (int frame = 0; frame < dst_frames; ++frame) {
        double src_frame = static_cast<double>(frame) * src_rate / dst_rate;
        for (int channel = 0; channel < dst_channels; ++channel) {
            float value = clamp_sample(sample_at(src, src_frames, src_channels,
                                                src_frame, channel, dst_channels));
            char* dst_sample = dst + frame * format.bytesPerFrame()
                                 + channel * format.bytesPerSample();

            switch (format.sampleFormat()) {
            case QAudioFormat::UInt8: {
                quint8 encoded = static_cast<quint8>((value * 0.5f + 0.5f) * 255.0f);
                std::memcpy(dst_sample, &encoded, sizeof(encoded));
                break;
            }
            case QAudioFormat::Int16: {
                qint16 encoded = static_cast<qint16>(std::lrint(value * 32767.0f));
                qToLittleEndian(encoded, reinterpret_cast<uchar*>(dst_sample));
                break;
            }
            case QAudioFormat::Int32: {
                qint32 encoded = static_cast<qint32>(std::llround(value * 2147483647.0f));
                qToLittleEndian(encoded, reinterpret_cast<uchar*>(dst_sample));
                break;
            }
            case QAudioFormat::Float: {
                std::memcpy(dst_sample, &value, sizeof(value));
                break;
            }
            case QAudioFormat::Unknown:
            case QAudioFormat::NSampleFormats:
                break;
            }
        }
    }

    return out;
}

void AudioPlayer::play(const Sample& sample) {
    stop();
    if (!sample.is_valid()) {
        qWarning() << "AudioPlayer::play: sample is not valid";
        return;
    }

    setup_sink(sample.format());
    if (!sink_) return;

    pcm_data_ = convert_to_output(sample, output_format_);
    if (pcm_data_.isEmpty()) {
        qWarning() << "AudioPlayer: unable to convert sample to output format";
        return;
    }

    pcm_buffer_.close();
    pcm_buffer_.setData(pcm_data_);
    pcm_buffer_.open(QIODevice::ReadOnly);
    sink_->start(&pcm_buffer_);

    if (sink_->error() != QAudio::NoError) {
        qWarning() << "AudioPlayer: sink error after start:" << sink_->error();
    }
}

void AudioPlayer::stop() {
    if (sink_) {
        sink_->stop();
        sink_.reset();
    }
    pcm_buffer_.close();
}

bool AudioPlayer::is_playing() const {
    return sink_ && (sink_->state() == QAudio::ActiveState
                  || sink_->state() == QAudio::IdleState);
}
