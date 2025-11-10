#ifndef __OPENAE__CONTEXT_H_
#define __OPENAE__CONTEXT_H_

#ifdef __PSP__
#   include <pspaudiolib.h>
#   include <pspkernel.h>
#   include <pspaudio.h>
#else
#   include <AL/al.h>
#   include <AL/alc.h>
#endif

#include <stdint.h>

#include <openae/stream.h>

typedef struct openae_context_t
{
#ifndef __PSP__
    ALCdevice* device;
    ALCcontext* context;
    ALuint processed_buffers[OPENAE_AUDIO_BUFFERS_PER_SOURCE];
#endif

    uint8_t initialized;
    openae_stream_t sfx[OPENAE_AUDIO_SFX_STREAMS_MAX];
    openae_stream_t music;
    float audio_volume;
} openae_context_t;

/**
 * @brief Initialize the audio context
 */
void openae_context_initialize(void);

/**
 * @brief Get current context
 */
openae_context_t* openae_context_get_current(void);

/**
 * @brief Set the currently playing music (immediately playing it)
 * 
 * @param ctx Context
 * @param filepath Audio file path
 * 
 * @returns The created stream
 */
openae_stream_t* openae_context_set_music(const char* filepath);

/**
 * @brief Add a sfx to the queue (and immediately playing it)
 * 
 * @param ctx Context
 * @param filepath Audio file path
 * 
 * @returns The created stream
 */
openae_stream_t* openae_context_play_sfx(const char* filepath);

/**
 * @brief Update all and every sounds
 */
void openae_context_update_all(void);

/**
 * @brief Dispose a context
 */
void openae_context_dispose(void);

#endif
