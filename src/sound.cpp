#include <video2ascii/sound.hpp>
#include <vector>
#include <iostream>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
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
    alGenBuffers(1, &src.buffer);
    return src;
}

extern void load_sound(const std::string& t_path, source& t_source) {
    load_sound_data(t_path, t_source);
}

extern void play_sound(source s) {
    alSourcePlay(s.source);

    ALint state;
    do {
        alGetSourcei(s.source, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);
}
extern void delete_source(source s) {
    alDeleteSources(1, &s.source);
    alDeleteBuffers(1, &s.buffer);
}

extern void close_sound(context ctx) {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx.context);
    alcCloseDevice(ctx.device);
}

static int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type, std::string src_filename) {
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;
 
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n", av_get_media_type_string(type), src_filename.c_str());
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
 
        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
 
        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }
 
        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }
 
        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
 
    return 0;
}


static int output_audio_frame(AVFrame* frame, AVCodecContext* codec_ctx, source& t_source) {
    int err = 0;

    // determine OpenAL format for audio data
    ALenum al_format = AL_FORMAT_STEREO16;
    switch (frame->format) {
        case AV_SAMPLE_FMT_U8:
            al_format = (frame->ch_layout.nb_channels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
            break;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_FLTP:
            al_format = (frame->ch_layout.nb_channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            break;
        default:
            std::cerr << "Error: Unsupported audio data format" << std::endl;
            return -1;
    }

    // convert audio data to OpenAL format
    int sample_count = frame->nb_samples * frame->ch_layout.nb_channels;
    std::vector<int16_t> data(sample_count);
    float* src_data = reinterpret_cast<float*>(frame->data[0]);
    int16_t* dst_data = data.data();

    const float scaling_factor = 32767.0f;
    for (int i = 0; i < sample_count; ++i) {
        float value = src_data[i] * scaling_factor;
        if (value < -32768.0f) {
            value = -32768.0f;
        } else if (value > 32767.0f) {
            value = 32767.0f;
        }
        dst_data[i] = static_cast<int16_t>(value);
    }

    ALuint src;
    ALuint buf;
    alGenSources(1, &src);
    alGenBuffers(1, &buf);
    // fill OpenAL buffer with audio data
    alBufferData(buf, al_format, dst_data, sample_count * sizeof(int16_t), frame->sample_rate);
    if ((err = alGetError()) != AL_NO_ERROR) {
        std::cerr << "Error filling OpenAL buffer with audio data: " << err << std::endl;
        return -1;
    }

    // alSourceQueueBuffers(t_source.source, 1, &al_buffer);
    alSourcei(src, AL_BUFFER, buf);
    alSourcePlay(src);

    ALint state;
    do {
        alGetSourcei(src, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);

    alDeleteSources(1, &src);
    alDeleteBuffers(1, &buf);

    return 0;
}

static int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame *frame, source& t_source)
{
    int ret = 0;
 
    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }
 
    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
 
            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }
 
        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_AUDIO)
            ret = output_audio_frame(frame, dec, t_source);
 
        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }
 
    return 0;
}

static void load_sound_data(const std::string& t_path, source& t_source) {
    AVFormatContext* format_ctx = NULL;
    AVCodecContext* audio_dec_ctx = NULL;
    AVFrame* frame = NULL;
    AVPacket* pkt = NULL;

    AVStream* audio_stream = NULL;
    int audio_stream_idx = -1;
    avformat_network_init();

    // open and allocate context
    if (avformat_open_input(&format_ctx, t_path.c_str(), NULL, NULL) != 0) {
        std::cerr << "Error: Could not open file for audio" << std::endl;
        return;
    }

    // get stream info
    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        std::cerr << "Error: Could not find stream info" << std::endl;
        return;
    }

    // open codec context
    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, format_ctx, AVMEDIA_TYPE_AUDIO, t_path) >= 0) {
        audio_stream = format_ctx->streams[audio_stream_idx];
    }

    // allocate frame
    if ((frame = av_frame_alloc()) == NULL) {
        std::cerr << "Error: Could not allocate frame" << std::endl;
        return;
    }

    // allocate packet
    if ((pkt = av_packet_alloc()) == NULL) {
        std::cerr << "Error: Could not allocate packet" << std::endl;
        return;
    }

    int ret;
    while (av_read_frame(format_ctx, pkt) >= 0) {
        if (pkt->stream_index == audio_stream_idx) {
            ret = decode_packet(audio_dec_ctx, pkt, frame, t_source);
        }
        av_packet_unref(pkt);
    }

    // Step 3: Clean up
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&audio_dec_ctx);
    avformat_close_input(&format_ctx);
}

}
