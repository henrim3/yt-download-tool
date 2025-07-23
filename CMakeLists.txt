cmake_minimum_required(VERSION 3.24)
project(yt-download-tool C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# get yt-dlp
FetchContent_Declare(
    YT_DLP
    GIT_REPOSITORY https://github.com/yt-dlp/yt-dlp.git
    GIT_TAG 035b1ece8f382358f5503bf5011ca098f6c9eaf9
)
FetchContent_MakeAvailable(YT_DLP)

add_executable(yt-download-tool src/main.c)

# Make sure yt-dlp is available at runtime
add_custom_command(TARGET yt-download-tool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${yt_dlp_SOURCE_DIR}
            $<TARGET_FILE_DIR:yt-download-tool>/yt-dlp
)
