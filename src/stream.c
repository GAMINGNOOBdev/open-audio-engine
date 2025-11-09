#define MINIMP3_IMPLEMENTATION
#define MINIMP3_NO_SIMD
#include <minimp3.h>
#include <minimp3_ex.h>
#undef MINIMP3_IMPLEMENTATION
#include <openae/callbacks.h>
#include <openae/logging.h>
#include <openae/context.h>
#include <openae/define.h>
#include <openae/stream.h>
#include <memory.h>


openae_stream_t openae_stream_create(openae_audio_file_t* file)
{
    openae_stream_t stream;
    memset(&stream, 0, sizeof(openae_stream_t));

    if (!file)
        return stream;

    stream.file = file;
    stream.volume = 1.0f;
    stream.format = file->format;

#ifndef __PSP__
    alGenSources(1, &stream.source);
    alGenBuffers(OPENAE_AUDIO_BUFFERS_PER_SOURCE, stream.buffers);
#endif

    return stream;
}

void openae_stream_update(openae_stream_t* stream)
{
    if (!openae_stream_is_valid(stream))
        return;

#ifndef __PSP__
    LOGDEBUG("TODO: -- implement --");
    ALint source_state = 0;
    alGetSourcei(stream->source, AL_SOURCE_STATE, &source_state);
    stream->playing = source_state == AL_PLAYING;
#endif
}

uint8_t openae_stream_is_valid(openae_stream_t* stream)
{
    if (!stream)
        return 0;

    return stream->file != NULL;
}

void openae_stream_start(openae_stream_t* stream)
{
    if (!stream)
        return;

    openae_stream_stop(stream);
    openae_stream_update(stream);
}

void openae_stream_stop(openae_stream_t* stream)
{
    if (!stream)
        return;

#ifndef __PSP__
    ALint processed_buffer_count = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed_buffer_count);
    if (processed_buffer_count > 0)
    {
        memset(openae_context_get_current()->processed_buffers, 0, sizeof(ALuint)*OPENAE_AUDIO_BUFFERS_PER_SOURCE);
        alSourceUnqueueBuffers(stream->source, OPENAE_AUDIO_BUFFERS_PER_SOURCE, openae_context_get_current()->processed_buffers);
    }
    alSourceStop(stream->source);
#endif
}

void openae_stream_dispose(openae_stream_t* stream)
{
    if (!stream)
        return;
    
#ifndef __PSP__
    alDeleteBuffers(OPENAE_AUDIO_BUFFERS_PER_SOURCE, stream->buffers);
    alDeleteSources(1, &stream->source);
#endif

    memset(stream, 0, sizeof(openae_stream_t));
}
