cmake_minimum_required(VERSION 2.8)

project(sdl_fft_visualizer)

option(ASAN "Enable the address sanitizer")
option(TEST_ENABLED "Enable testing?")
option(GCOV "Compile with gcov?")

set(COMMON_CODE
    "src/data_source.cpp"
    "src/filters/clip_filter.cpp" "src/filters/peek_filter.cpp" "src/filters/sagc_filter.cpp" "src/filter.cpp"
    "src/postprocessing.cpp"

    "include/data_sources/pulseaudio.hpp"
    "include/filters/clip_filter.hpp" "include/filters/peek_filter.hpp" "include/filters/sagc_filter.hpp"
    "include/postprocessing.hpp"
    "include/filter.hpp" "include/data_source.hpp")
set(DATA_SOURCES "src/data_sources/pulseaudio.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW3 REQUIRED fftw3)
pkg_search_module(PulseAudio REQUIRED libpulse-simple)

set(COMMON_LIBS Threads::Threads ${PulseAudio_LIBRARIES} ${FFTW3_LIBRARIES} ${SDL2_LIBRARIES})
set(COMMON_INCL "include/" ${PULSEAUDIO_INCLUDE_DIRS} ${FFTW3_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} "src/main.cpp" ${COMMON_CODE} ${DATA_SOURCES})
target_link_libraries(${PROJECT_NAME} ${COMMON_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC ${COMMON_INCL})

if(ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

if(TEST_ENABLED)
    # https://github.com/google/googletest/blob/master/googletest/README.md#incorporating-into-an-existing-cmake-project
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                     ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                     EXCLUDE_FROM_ALL)

    # The gtest/gtest_main targets carry header search path
    # dependencies automatically when using CMake 2.8.11 or
    # later. Otherwise we have to add them here ourselves.
    if (CMAKE_VERSION VERSION_LESS 2.8.11)
      include_directories("${gtest_SOURCE_DIR}/include")
    endif()

    set(TEST_SRCS "tests/postprocessing.cpp" "tests/data_sources.cpp" "tests/filters.cpp")
    add_executable(${PROJECT_NAME}-test ${TEST_SRCS} ${COMMON_CODE})
    target_link_libraries(${PROJECT_NAME}-test gmock_main ${COMMON_LIBS})
    target_include_directories(${PROJECT_NAME}-test PUBLIC ${COMMON_INCL})

    if(GCOV)
        # To calculate coverage:
        #   - compile with gcov
        #   - run the visualizer
        #   - in the build directory:
        #       - run lcov: lcov -c -d ./ -o coverage.info --no-external --base-directory $CMAKE_CURRENT_SOURCE_DIR
        #       - (for html) run genhtml: genhtml  coverage.info -o coverage
        target_compile_options(${PROJECT_NAME}-test PUBLIC -g -ftest-coverage -fprofile-arcs)
        target_link_options(${PROJECT_NAME}-test PUBLIC -g -ftest-coverage -fprofile-arcs)
        target_link_libraries(${PROJECT_NAME}-test gcov)
    endif()

    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}-test)
endif()
