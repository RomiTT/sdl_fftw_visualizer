cmake_minimum_required(VERSION 2.8)

project(sdl_fft_visualizer)

option(ASAN "Enable the address sanitizer")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW3 REQUIRED fftw3)
pkg_search_module(PulseAudio REQUIRED libpulse-simple)
add_executable(${PROJECT_NAME} "src/main.cpp" "src/data_source.cpp" "src/data_sources/pulseaudio.cpp"
    "include/data_source.hpp")

target_link_libraries(${PROJECT_NAME} ${PulseAudio_LIBRARIES} ${FFTW3_LIBRARIES} ${SDL2_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC "include/" ${PULSEAUDIO_INCLUDE_DIRS} ${FFTW3_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
if(ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()
