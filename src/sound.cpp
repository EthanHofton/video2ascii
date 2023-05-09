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
    // do {
    //     alGetSourcei(s.source, AL_SOURCE_STATE, &state);
    // } while (state == AL_PLAYING);
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

static void load_sound_data(const std::string& t_path, source& t_source) {
    FILE* file = fopen("output.wav", "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[fileSize];
    fread(buffer, 1, fileSize, file);
    fclose(file);

    alBufferData(t_source.buffer, AL_FORMAT_STEREO16, buffer, fileSize, 44100);
    alSourcei(t_source.source, AL_BUFFER, t_source.buffer);
}

}
