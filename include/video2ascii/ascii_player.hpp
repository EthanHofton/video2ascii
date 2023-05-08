#ifndef __ASCII_PLAYER_HPP__
#define __ASCII_PLAYER_HPP__

#include <video2ascii/sound.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#define ASCII_ESC 27
#define CURSOR_HOME (printf("%c[H", ASCII_ESC))
#define SET_COLOR(R, G, B) (printf("%c[38;2;%d;%d;%dm", ASCII_ESC, R, G, B))
#define SET_BACKGROUND_COLOR(R, G, B) (printf("%c[48;2;%d;%d;%dm", ASCII_ESC, R, G, B))

extern sound::source s_source;

enum display_mode {
    ASCII,
    COLOR,
    COLOR_ASCII,
    GRAYSCALE,
    GRAYSCALE_ASCII,
    NONE,
};

static int map(int, int, int, int, int);
static char pixel_to_ascii(int t_pixel);
static void get_terminal_size(int&, int&);

extern void calc_params(cv::VideoCapture, int&, int&, float&);
extern void print_video(cv::VideoCapture, int, int, int, display_mode);
extern display_mode get_display_mode(int argc, char** argv);

#endif
