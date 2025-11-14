#ifndef __OPENAE__STREAM_H_
#define __OPENAE__STREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <openae/audio_file.h>

#ifdef __PSP__
#   include <pspaudiolib.h>
#   include <pspkernel.h>
#   include <pspaudio.h>
#else
#   include <AL/al.h>
#   include <AL/alc.h>
#endif

typedef void(*openae_audio_stream_end_callback_t)(void* stream);

typedef struct openae_stream_t
{
    float volume;
    float seconds; // current position in seconds
    uint8_t playing;
    int processed_frames;
    openae_audio_file_t file;
    openae_audio_format_t format;
    openae_audio_stream_end_callback_t end; // Only useful for music lol

    #ifndef __PSP__
        ALuint source;
        ALuint buffers[OPENAE_AUDIO_BUFFERS_PER_SOURCE];
        short data[OPENAE_AUDIO_FRAME_SIZE];
    #endif
} openae_stream_t;

/**
 * @brief Create an audio stream
 * 
 * @param file File path
 * @returns Audio stream
 */
void openae_stream_create(openae_stream_t* stream, const char* filepath);

/**
 * @brief Update the stream
 * 
 * @param stream Audio stream
 */
void openae_stream_update(openae_stream_t* stream);

/**
 * @brief Update a buffer using this stream
 * 
 * @param stream Audio stream
 * @param buffer Audio buffer
 * @param bytes Audio buffer size
 */
void openae_stream_update_buffer(openae_stream_t* stream, void* buffer, int bytes);

/**
 * @brief Check whether stream is valid
 */
uint8_t openae_stream_is_valid(openae_stream_t* stream);

/**
 * @brief Start the audio stream
 */
void openae_stream_start(openae_stream_t* stream);

/**
 * @brief Stop the stream from continuing
 */
void openae_stream_stop(openae_stream_t* stream);

/**
 * @brief Dispose an audio stream
 * 
 * @param stream Audio stream
 */
void openae_stream_dispose(openae_stream_t* stream);

#ifdef __cplusplus
}
#endif

#endif
