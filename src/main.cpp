#include <video2ascii/ascii_player.hpp>

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

    // get fps, and video params
    float fps;
    int animation_width, animation_height;
    calc_params(cap, animation_width, animation_height, fps);

    // print the video to the string
    print_video(cap, fps, animation_width, animation_height, mode);

    return 0;
}

