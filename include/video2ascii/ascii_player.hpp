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

// ascii escape code
#define ASCII_ESC 27

// cursor display macros
#define HIDE_CURSOR (printf("%c[?25l", ASCII_ESC))
#define SHOW_CURSOR (printf("%c[?25h", ASCII_ESC))

// screen clear macros
#define CLEAR_SCREEN (printf("%c[2J", ASCII_ESC))

// cursor position macros
#define CURSOR_HOME (printf("%c[H", ASCII_ESC))
#define CURSOR_NEXT_LINE (printf("%c[E", ASCII_ESC))
#define SET_CURSOR_POS(x, y) (printf("%c[%d;%dH", ASCII_ESC, y, x))
#define MOVE_CURSOR_N_UP(n) if (n > 0) { printf("%c[%dA", ASCII_ESC, n); }
#define MOVE_CURSOR_N_DOWN(n) if (n > 0) { printf("%c[%dB", ASCII_ESC, n); }
#define MOVE_CURSOR_N_RIGHT(n) if (n > 0) { printf("%c[%dC", ASCII_ESC, n); }
#define MOVE_CURSOR_N_LEFT(n) if (n > 0) { printf("%c[%dD", ASCII_ESC, n); }

// color macros
#define SET_COLOR(R, G, B) (printf("%c[38;2;%d;%d;%dm", ASCII_ESC, R, G, B))
#define SET_BACKGROUND_COLOR(R, G, B) (printf("%c[48;2;%d;%d;%dm", ASCII_ESC, R, G, B))

// cursor movement macros

extern sound::source s_source;

enum display_mode {
    ASCII,
    COLOR,
    COLOR_ASCII,
    GRAYSCALE,
    GRAYSCALE_ASCII,
    NONE,
};

static char pixel_to_ascii(int t_pixel);
static void get_terminal_size(int&, int&);
static void get_char_size(int& char_width, int& char_height);

extern void calc_params(cv::VideoCapture, int&, int&, float&, int&, int&);
extern void print_video(cv::VideoCapture, int, int, int, int, int, display_mode);
extern display_mode get_display_mode(int argc, char** argv);

#endif
