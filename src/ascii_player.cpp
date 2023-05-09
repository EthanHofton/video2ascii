#include <video2ascii/ascii_player.hpp>
#include <video2ascii/sound.hpp>

sound::source s_source;

extern display_mode get_display_mode(int argc, char** argv) {
    display_mode mode = display_mode::ASCII;
    int opt;

    while ((opt = getopt(argc, argv, ":m:")) != -1) {
        switch (opt) {
            case 'm':
                if (strcmp(optarg, "ascii") == 0) {
                    mode = display_mode::ASCII;
                } else if (strcmp(optarg, "color") == 0) {
                    mode = display_mode::COLOR;
                } else if (strcmp(optarg, "color_ascii") == 0) {
                    mode = display_mode::COLOR_ASCII;
                } else if (strcmp(optarg, "grayscale") == 0) {
                    mode = display_mode::GRAYSCALE;
                } else if (strcmp(optarg, "grayscale_ascii") == 0) {
                    mode = display_mode::GRAYSCALE_ASCII;
                } else {
                    std::cout << "Invalid mode: " << optarg << std::endl;
                    return display_mode::NONE;
                }
                break;
            case ':':
                std::cout << "Option needs a value: " << optopt << std::endl;
                return display_mode::NONE;
            case '?':
                std::cout << "Unknown option: " << optopt << std::endl;
                return display_mode::NONE;
        }
    }

    return mode;
}

extern void print_video(cv::VideoCapture t_cap, int t_fps, int t_animation_width, int t_animation_height, int x_offset, int y_offset, display_mode t_mode) {
    cv::Mat frame, gray, resize;
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
    float delta_time = 0.f;

    // Turn off output buffering for stdout
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // clear the screen
    HIDE_CURSOR;
    CLEAR_SCREEN;

    while (true) {
        delta_time = std::chrono::duration<float>(std::chrono::system_clock::now() - time).count();

        if (delta_time >= 1.f / t_fps) {
            time = std::chrono::system_clock::now();

            if (!t_cap.read(frame)) {
                break;
            }

            cv::resize(frame, resize, cv::Size(t_animation_width, t_animation_height));
            if (t_mode == GRAYSCALE || t_mode == ASCII || t_mode == GRAYSCALE_ASCII) {
                cv::cvtColor(resize, resize, cv::COLOR_BGR2GRAY);
            } else if (t_mode == COLOR_ASCII) {
                cv::cvtColor(resize, gray, cv::COLOR_BGR2GRAY);
            }

            // move the cursor to the animiation start
            CURSOR_HOME;

            MOVE_CURSOR_N_DOWN(y_offset);

            for (int i = 0; i < resize.rows; i++) {
                MOVE_CURSOR_N_RIGHT(x_offset);
                for (int j = 0; j < resize.cols; j++) {
                    switch (t_mode) {
                        case ASCII:
                            std::cout << pixel_to_ascii(resize.at<uchar>(i, j));
                            break;
                        case COLOR:
                            {
                                auto pixel_color = resize.at<cv::Vec3b>(i, j);
                                SET_BACKGROUND_COLOR(pixel_color[2], pixel_color[1], pixel_color[0]);
                            }
                            std::cout << ' ';
                            break;
                        case COLOR_ASCII:
                            {
                                auto pixel_color = resize.at<cv::Vec3b>(i, j);
                                SET_COLOR(pixel_color[2], pixel_color[1], pixel_color[0]);
                                std::cout << pixel_to_ascii(gray.at<uchar>(i, j));
                            }

                            break;
                        case GRAYSCALE:
                            {
                                auto pixel_color = resize.at<uchar>(i, j);
                                SET_BACKGROUND_COLOR(pixel_color, pixel_color, pixel_color);
                            }
                            std::cout << ' ';
                            break;
                        case GRAYSCALE_ASCII:
                            {
                                auto pixel_color = resize.at<uchar>(i, j);
                                SET_COLOR(pixel_color, pixel_color, pixel_color);
                                std::cout << pixel_to_ascii(resize.at<uchar>(i, j));
                            }
                            break;
                        default:
                            return;
                    }
                }
                CURSOR_NEXT_LINE;
            }
            CURSOR_HOME;
        }
    }
    SHOW_CURSOR;
}

extern void calc_params(cv::VideoCapture t_cap, int& t_animation_width, int& t_animation_height, float& t_fps, int& t_x_offset, int& t_y_offset) {
    // get video width and height
    int width = t_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = t_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    t_fps = t_cap.get(cv::CAP_PROP_FPS);

    int term_width, term_height;
    get_terminal_size(term_width, term_height);
    int char_width, char_height;
    get_char_size(char_width, char_height);
    
    // Calculate adjusted character aspect ratio
    float char_aspect_ratio = static_cast<float>(char_height) / char_width;

    // Calculate video aspect ratio, adjusted for character aspect ratio
    float video_aspect_ratio = static_cast<float>(width) / height * char_aspect_ratio;

    // calculate animation dimensions based on video aspect ratio
    if (video_aspect_ratio > static_cast<float>(term_width) / term_height) {
        // limit width
        t_animation_width = term_width;
        t_animation_height = static_cast<int>(term_width / video_aspect_ratio);
    } else {
        // limit height
        t_animation_width = static_cast<int>(term_height * video_aspect_ratio);
        t_animation_height = term_height;
    }

    // Calculate x and y offsets for centering the video
    t_x_offset = std::max(0, (term_width - t_animation_width) / 2);
    t_y_offset = std::max(0, (term_height - t_animation_height) / 2);
}

static char pixel_to_ascii(int t_pixel) {
    // Define an array of ASCII characters to map the pixel value to
    const char ascii_chars[] = {' ', '.', ':', '-', '=', '+', '*', '#', '%', '@'};
    // Determine the number of ASCII characters in the array
    const int num_chars = sizeof(ascii_chars) / sizeof(char);

    // Map the pixel value to an index in the ASCII character array
    int char_index = static_cast<int>((t_pixel / 255.0) * num_chars);

    // Ensure that the char_index is within bounds
    char_index = std::max(0, std::min(char_index, num_chars - 1));

    // Return the corresponding ASCII character
    return ascii_chars[char_index];
}

static void get_terminal_size(int& t_term_width, int& t_term_height) {
    #ifdef _WIN32
    // get terminal width and height
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    t_term_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    t_term_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
    // get terminal width and height
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    t_term_width = w.ws_col;
    t_term_height = w.ws_row;
    #endif
}

static void get_char_size(int& char_width, int& char_height) {
    #ifdef _WIN32
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    char_width = cfi.dwFontSize.X;
    char_height = cfi.dwFontSize.Y;
    #else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    char_width = w.ws_xpixel / w.ws_col;
    char_height = w.ws_ypixel / w.ws_row;
    #endif
}
