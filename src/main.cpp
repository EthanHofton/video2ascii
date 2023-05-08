#include <iostream>
#include <thread>
#include <opencv2/opencv.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#endif

#define ASCII_ESC 27
#define CLEAR_SCREEN (printf("%c[2J", ASCII_ESC))
#define CURSOR_HOME (printf("%c[H", ASCII_ESC))

char pixel_to_ascii(int t_pixel);
int map(int, int, int, int, int);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "usage: ./" << argv[0] << " <video path>\n";
        return EXIT_FAILURE;
    }

    cv::VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file\n";
        return EXIT_FAILURE;
    }

    // get video width and height
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    float fps = cap.get(cv::CAP_PROP_FPS);

    #ifdef _WIN32
    // get terminal width and height
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int term_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int term_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
    // get terminal width and height
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col;
    int term_height = w.ws_row;
    #endif
    
    // remap the video width (0, width) to terminal width (0, term_width)
    int animation_width = map(width, 0, width, 0, term_width);

    // remap the video height (0, height) to terminal height (0, term_height)
    int animation_height = map(height, 0, height, 0, term_height);

    cv::Mat frame, gray, resize;
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
    float delta_time = 0.f;

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // Turn off output buffering for stdout
    while (true) {
        delta_time = std::chrono::duration<float>(std::chrono::system_clock::now() - time).count();

        if (delta_time >= 1.f / fps) {
            time = std::chrono::system_clock::now();

            if (!cap.read(frame)) {
                break;
            }

            cv::resize(frame, resize, cv::Size(animation_width, animation_height));
            cv::cvtColor(resize, gray, cv::COLOR_BGR2GRAY);

            std::cout << "\033[1;1H";
            for (int i = 0; i < gray.rows; i++) {
                for (int j = 0; j < gray.cols; j++) {
                    char pixel = pixel_to_ascii(gray.at<uchar>(i, j));
                    std::cout << pixel;
                }
                std::cout << "\033[E";
            }

            CURSOR_HOME;
        }
    }

    return 0;
}

char pixel_to_ascii(int t_pixel) {
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

int map(int val, int x_min, int x_max, int y_min, int y_max) {
    // Scale the value to the range [0, 1]
    double scaled_val = static_cast<double>(val - x_min) / static_cast<double>(x_max - x_min);

    // Map the scaled value to the range [y_min, y_max]
    int mapped_val = static_cast<int>(scaled_val * (y_max - y_min) + y_min);

    // Ensure that the mapped value is within bounds
    mapped_val = std::max(y_min, std::min(mapped_val, y_max));

    return mapped_val;
}
