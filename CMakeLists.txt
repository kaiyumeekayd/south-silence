cmake_minimum_required(VERSION 3.21)

project(SouthSilence C)

psn00bsdk_add_executable(
    south_silence
    STATIC
    src/main.c
)

psn00bsdk_add_cd_image(
    south_silence_cd
    south_silence
    ${PROJECT_SOURCE_DIR}/cd.xml
)
