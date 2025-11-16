cmake_minimum_required(VERSION 3.10)
project(openae)

string(TIMESTAMP NOW "%Y %m %d")
string(REPLACE " " ";" DATE_LIST ${NOW})
list(GET DATE_LIST 0 YEAR)
list(GET DATE_LIST 1 MONTH)
list(GET DATE_LIST 2 DAY)

math(EXPR YEAR ${YEAR})
math(EXPR MONTH ${MONTH})
math(EXPR DAY ${DAY})

add_compile_definitions(__YEAR__=${YEAR})
add_compile_definitions(__MONTH__=${MONTH})
add_compile_definitions(__DAY__=${DAY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

# OpenAL Options
set(LIBTYPE STATIC)
set(ALSOFT_UTILS off)
set(ALSOFT_RTKIT off)
set(ALSOFT_INSTALL off)
set(ALSOFT_EXAMPLES off)
set(ALSOFT_INSTALL_UTILS off)
set(ALSOFT_INSTALL_CONFIG off)
set(ALSOFT_INSTALL_EXAMPLES off)
set(ALSOFT_INSTALL_HRTF_DATA off)
set(ALSOFT_INSTALL_AMBDEC_PRESETS off)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/libwave/include
    ${PROJECT_SOURCE_DIR}/minimp3
    ${PROJECT_SOURCE_DIR}/stb
    ${PROJECT_SOURCE_DIR}/openal-soft/include
)

if(NOT PSP)
    add_subdirectory(openal-soft)
endif()

set(library_sources
    libwave/src/wave.c
    stb/stb_vorbis.c
)

set(project_sources
    src/audio_file.c
    src/context.c
    src/logging.c
    src/stream.c
)

if (UNIX)
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

    add_compile_options(
        "-Wall" "-Wextra" "-Wpedantic" "-fexceptions" "-Wl,--export-dynamic"
    )
endif()

add_compile_definitions(NOMINMAX AL_LIBTYPE_STATIC MINIMP3_NO_SIMD)
add_library(${PROJECT_NAME} STATIC ${project_sources})
if(PSP)
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

    add_compile_options(
        "-Wall" "-Wextra" "-Wpedantic" "-fexceptions" "-Wl,--export-dynamic"
    )

    add_compile_definitions(__PSP__)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pspaudiolib
        pspaudio
        pspaudiocodec
    )

    add_executable(${PROJECT_NAME}_test test/test_psp.c ${library_sources})
    target_link_libraries(${PROJECT_NAME}_test
        ${PROJECT_NAME}
        pspdisplay
        pspdebug
        pspge
        pspgu
    )

    create_pbp_file(TARGET ${PROJECT_NAME}_test TITLE "${CMAKE_PROJECT_NAME}"
        # ICON_PATH
        # BACKGROUND_PATH
        # PREVIEW_PATH
    )

else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenAL
        alsoft.common
    )

    add_executable(${PROJECT_NAME}_test test/test.c ${library_sources})
    target_link_libraries(${PROJECT_NAME}_test
        ${PROJECT_NAME}
    )
endif()


if( MSVC )
    set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME} )
endif()
