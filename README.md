# video2ascii

Play videos in your terminal displayed as ASCII charicters

![](https://github.com/EthanHofton/video2ascii/blob/main/res/preview.gif)

### Features

- [x] play video files in terminal converted to ascii charicters
  - [x] RGB colors
  - [x] grayscale
  - [x] RGB ascii colors
  - [x] grayscale ascii
- [ ] customise ascii charicters used (`v1.2.0`)
- [ ] image preview (`v1.2.0`)
- [ ] play video audio (`v1.3.0`)
- [ ] play/pause (`v1.4.0`)

# Requirements

- `CMake`
- `OpenCV`
- (From `v1.3.0`) `OpenAL`
- (From `v1.3.0`) `FFMpeg`

# Install

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**Optionally** do `cp bin/video2ascii /usr/local/bin/` (or OS equivilent) to use as a command

# Usage

```
./bin/video2ascii [-m ascii|color|color_ascii|grayscale|grayscale_ascii] <video_name>
```

### Arguemnts:

- `<video_name>` must be a releative path to the video from the directory you execute the program in, or an absolute path
- `m` the video mode (default ascii):
  - `ascii` ascii mode, display the image in ascii charicters `[' ','.',':','-','=','+','*','#','%','@']`
  - `color` color mode. display the video using each `' '` charicters with a background color the same RGB as the video pixel.
  - `color_ascii` color ascii mode. display the video using the ascii charicters colored the same as there corrisponding RGB pixel
  - `grayscale` grayscale mode. same as color mode, except converted to grayscale
  - `grayscale_ascii` grayscale ascii mode. The same as color ascii mode but in grayscale
