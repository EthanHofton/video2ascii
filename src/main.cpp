#include <video2ascii/ascii_player.hpp>

void reset_cursor() {
    SHOW_CURSOR;
    CURSOR_HOME;
}

void sigint_handler(int) {
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    // get the mode
    display_mode mode = get_display_mode(argc, argv);

    if (mode == display_mode::NONE) {
        return EXIT_FAILURE;
    }

    // create the video capture
    cv::VideoCapture cap(argv[optind]);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream or file\n";
        return EXIT_FAILURE;
    }

    atexit(reset_cursor);
    signal(SIGINT, sigint_handler);

    // get fps, and video params
    float fps;
    int animation_width, animation_height, x_offset, y_offset;
    calc_params(cap, animation_width, animation_height, fps, x_offset, y_offset);

    // print the video to the string
    print_video(cap, fps, animation_width, animation_height, x_offset, y_offset, mode);

    return 0;
}

