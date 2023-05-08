#include <video2ascii/sound.hpp>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}

namespace sound {

extern context init_sound() {
    context ctx;
    ctx.device = alcOpenDevice(NULL);
    ctx.context = alcCreateContext(ctx.device, NULL);
    alcMakeContextCurrent(ctx.context);
    return ctx;
}

extern source create_source() {
    source src;
    alGenSources(1, &src.source);
    alSourcei(src.source, AL_LOOPING, AL_FALSE);
    return src;
}

extern void load_sound(const std::string& t_path, source& t_source) {
    load_sound_data(t_path, t_source);
}

extern void play_sound(source s) {
    alSourcePlay(s.source);
}
extern void delete_source(source s) {
    alDeleteSources(1, &s.source);
    // alDeleteBuffers(1, &s.buffer);
}

extern void close_sound(context ctx) {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx.context);
    alcCloseDevice(ctx.device);
}

static void load_sound_data(const std::string& t_path, source& t_source) {
    avformat_network_init();
    AVFormatContext* format_context = nullptr;

    // Open the input file.
    if (avformat_open_input(&format_context, t_path.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "Error: could not open input file." << std::endl;
        return;
    }

    // Find the best audio stream.
    const AVCodec* codec = nullptr;
    int audio_stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (audio_stream_index < 0) {
        std::cerr << "Error: could not find audio stream." << std::endl;
        avformat_close_input(&format_context);
        return;
    }

    // Open the audio stream codec.
    auto codec_context = avcodec_alloc_context3(codec);
    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        std::cerr << "Error: could not open audio codec." << std::endl;
        avformat_close_input(&format_context);
        return;
    }

    // Allocate the audio frame.
    AVFrame* audio_frame = av_frame_alloc();
    if (!audio_frame) {
        std::cerr << "Error: could not allocate audio frame." << std::endl;
        avcodec_close(codec_context);
        avformat_close_input(&format_context);
        return;
    }

    // Allocate the audio packet.
    AVPacket* audio_packet = av_packet_alloc();
    if (!audio_packet) {
        std::cerr << "Error: could not allocate audio packet." << std::endl;
        av_frame_free(&audio_frame);
        avcodec_close(codec_context);
        avformat_close_input(&format_context);
        return;
    }

    while (av_read_frame(format_context, audio_packet) >= 0) {
        if (audio_packet->stream_index == audio_stream_index) {
            // Decode the audio packet.
            int response = avcodec_send_packet(codec_context, audio_packet);
            while (response >= 0) {
                response = avcodec_receive_frame(codec_context, audio_frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    std::cerr << "Error: could not decode audio frame." << std::endl;
                    av_packet_free(&audio_packet);
                    av_frame_free(&audio_frame);
                    avcodec_close(codec_context);
                    avformat_close_input(&format_context);
                    return;
                }

                // Pass the audio data to OpenAL.
                ALuint buffer;
                alGenBuffers(1, &buffer);
                alBufferData(buffer, AL_FORMAT_MONO16, audio_frame->data[0], audio_frame->linesize[0], audio_frame->sample_rate);
                alSourceQueueBuffers(t_source.source, 1, &buffer);
            }
        }

        // Free the audio packet.
        av_packet_unref(audio_packet);
    }
}

}
