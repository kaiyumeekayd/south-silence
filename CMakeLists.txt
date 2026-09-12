cmake_minimum_required(VERSION 3.21)

project(SouthSilence)

include($ENV{PSN00BSDK}/lib/libpsn00b/cmake/psn00bsdk.cmake)

psn00bsdk_add_executable(
    south_silence
    src/main.c
)

psn00bsdk_add_cd_image(
    south_silence
    iso.xml
)
