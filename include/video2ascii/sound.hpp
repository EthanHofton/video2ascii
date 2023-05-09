#ifndef __SOUND_HPP__
#define __SOUND_HPP__

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <string>

namespace sound {

struct source {
    ALuint source;
    ALuint buffer;
};

struct context {
    ALCdevice* device;
    ALCcontext* context;
};

extern context init_sound();
extern source create_source();
extern void load_sound(const std::string&, source&);
extern void play_sound(source);
extern void delete_source(source);
extern void close_sound(context);
static void load_sound_data(const std::string&, source&);

}

#endif
