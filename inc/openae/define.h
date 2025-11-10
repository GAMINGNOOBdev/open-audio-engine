#ifndef __OPENAE__DEFINE_H_
#define __OPENAE__DEFINE_H_

#include <stdint.h>

#define OPENAE_PLATFORM_UNDEFINED  0
#define OPENAE_PLATFORM_WINDOWS    1
#define OPENAE_PLATFORM_LINUX      2
#define OPENAE_PLATFORM_APPLE      3
#define OPENAE_PLATFORM_PSP        4

#define OPENAE_PLATFORM_NAME_UNKNOWN   "Unknown"
#define OPENAE_PLATFORM_NAME_WINDOWS   "Windows"
#define OPENAE_PLATFORM_NAME_LINUX     "Linux"
#define OPENAE_PLATFORM_NAME_APPLE     "Apple MacOS"
#define OPENAE_PLATFORM_NAME_PSP       "PlayStationPortable (PSP)"

#ifndef __PSP__
#   define OPENAE_AUDIO_FRAME_SIZE 0x1000
#endif

#define OPENAE_AUDIO_STREAMS_MAX 8
#define OPENAE_AUDIO_SFX_STREAMS_MAX 7
#define OPENAE_AUDIO_BUFFERS_PER_SOURCE 4

#if defined(_WIN32) || defined(WINAPI_FAMILY)
#   define OPENAE_PLATFORM         OPENAE_PLATFORM_WINDOWS
#   define OPENAE_PLATFORM_NAME    OPENAE_PLATFORM_NAME_WINDOWS
#elif defined(__linux__)
#   define OPENAE_PLATFORM         OPENAE_PLATFORM_LINUX
#   define OPENAE_PLATFORM_NAME    OPENAE_PLATFORM_NAME_LINUX
#elif defined(__APPLE__)
#   define OPENAE_PLATFORM         OPENAE_PLATFORM_APPLE
#   define OPENAE_PLATFORM_NAME    OPENAE_PLATFORM_NAME_APPLE
#elif defined(__psp__)
#   define OPENAE_PLATFORM         OPENAE_PLATFORM_PSP
#   define OPENAE_PLATFORM_NAME    OPENAE_PLATFORM_NAME_PSP
#else
#   define OPENAE_PLATFORM         OPENAE_PLATFORM_UNDEFINED
#   define OPENAE_PLATFORM_NAME    OPENAE_PLATFORM_NAME_UNKNOWN
#   error Could not detect your operating system
#endif

#define null NULL
#define guaranteed(value) if (value != null)

#endif
