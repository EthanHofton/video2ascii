# ================= OPENCV =================
find_package(OpenCV REQUIRED)
# ===========================================

# ================= OPENAL =================
find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})
link_directories(${OPENAL_LIBRARY_DIRS})
# ===========================================

# ================= ffmpg =================
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavutil)
# ===========================================

# ================= SOURCES =================
set(EXE_NAME "video2ascii")

set(SOURCES
    ascii_player.cpp
    sound.cpp
    main.cpp
)
# ============================================

# ================= EXECUTABLE =================
add_executable(${EXE_NAME} ${SOURCES})
# ===============================================

# ================= INCLUDES =================
target_include_directories(${EXE_NAME} PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
# =============================================

# ================= LINKS =================
target_link_libraries(${EXE_NAME} PUBLIC
    ${OpenCV_LIBS}
    ${OPENAL_LIBRARY}
    PkgConfig::FFMPEG
)
# =========================================
