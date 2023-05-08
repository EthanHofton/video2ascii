# ================= OPENCV =================
find_package(OpenCV REQUIRED)
# ===========================================

# ================= SOURCES =================
set(EXE_NAME "video2ascii")

set(SOURCES
    main.cpp
)
# ============================================

# ================= EXECUTABLE =================
add_executable(${EXE_NAME} ${SOURCES})
# ===============================================

# ================= INCLUDES =================
target_include_directories(${EXE_NAME} PUBLIC 
    ${OpenCV_INCLUDE_DIRS}
)
# =============================================

# ================= LINKS =================
target_link_libraries(${EXE_NAME} PUBLIC
    ${OpenCV_LIBS}
)
# =========================================
