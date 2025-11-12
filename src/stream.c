#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <minimp3_ex.h>
#undef MINIMP3_IMPLEMENTATION
#include <openae/audio_file.h>
#include <openae/logging.h>
#include <openae/context.h>
#include <openae/define.h>
#include <openae/stream.h>
#include <memory.h>

#ifndef __PSP__
#   include <AL/al.h>
#endif


openae_stream_t openae_stream_create(const char* filepath)
{
    openae_stream_t stream;
    memset(&stream, 0, sizeof(openae_stream_t));

    stream.file = openae_audio_file_open(filepath);
    assume(openae_audio_file_is_valid(&stream.file), stream);

    stream.volume = 1.0f;
    stream.format = stream.file.format;

#ifndef __PSP__
    alGenSources(1, &stream.source);
    alGenBuffers(OPENAE_AUDIO_BUFFERS_PER_SOURCE, stream.buffers);
#endif

    return stream;
}

void openae_stream_update(openae_stream_t* stream)
{
    assume(openae_stream_is_valid(stream));
    if (!stream->playing)
        return;

    if (stream->processed_frames >= stream->file.samples)
    {
        if (stream->end != NULL)
            stream->end(stream);

        stream->playing = 0;
        return;
    }

#ifndef __PSP__
    ALint source_state = 0;
    alGetSourcei(stream->source, AL_SOURCE_STATE, &source_state);
    stream->playing = source_state == AL_PLAYING;

    ALint processed_buffer_count = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed_buffer_count);
    if (processed_buffer_count <= 0)
        return;

    ALuint* buffers = openae_context_get_current()->processed_buffers;
    alSourceUnqueueBuffers(stream->source, processed_buffer_count, buffers);

    for (int i = 0; i < processed_buffer_count; i++)
    {
        ALuint buffer = buffers[i];
        openae_stream_update_buffer(stream, stream->data, OPENAE_AUDIO_FRAME_SIZE);
        alBufferData(buffer, AL_FORMAT_STEREO16, stream->data, OPENAE_AUDIO_FRAME_SIZE, stream->file.freq);
    }

    alSourceQueueBuffers(stream->source, processed_buffer_count, buffers);
#endif

    if (stream->file.format == OPENAE_AUDIO_FORMAT_VORBIS)
        stream->seconds = (float)stb_vorbis_get_sample_offset(stream->file.vorbis_stream) / (float)stream->file.samples * 1000.f * stream->file.length;
    else if (stream->file.format == OPENAE_AUDIO_FORMAT_MP3)
        stream->seconds = (float)stream->processed_frames / (float)stream->file.samples * 1000.f * stream->file.length;
    else
        stream->seconds = (float)stream->processed_frames / (float)stream->file.samples * 1000.f;
}

void openae_stream_update_buffer(openae_stream_t* stream, void* buffer, int bytes)
{
    assume(openae_stream_is_valid(stream));
    assume(buffer);
    assume(bytes);

    if (!stream->playing)
        return;

    if (stream->processed_frames >= stream->file.samples)
    {
        if (stream->end != NULL)
            stream->end(stream);

        stream->playing = 0;
        return;
    }

    int frames = 0;
    int expectedFrames = bytes / sizeof(short);
    memset(buffer, 0, bytes);

    if (stream->format == OPENAE_AUDIO_FORMAT_VORBIS)
    {
        expectedFrames /= stream->file.vorbis_info.channels;
        frames = stb_vorbis_get_samples_short_interleaved(stream->file.vorbis_stream, stream->file.vorbis_info.channels, (short*)buffer, expectedFrames*sizeof(short));
    }
    else if (stream->format == OPENAE_AUDIO_FORMAT_WAV)
    {
        expectedFrames /= wave_get_sample_size(stream->file.wav);
        frames = wave_read(stream->file.wav, buffer, expectedFrames);
    }
    else if (stream->format == OPENAE_AUDIO_FORMAT_MP3)
        frames = mp3dec_ex_read(&stream->file.mp3, (mp3d_sample_t*)buffer, expectedFrames);

    for (int i = 0; i < expectedFrames; i++)
        ((short*)buffer)[i] *= stream->volume;

    stream->processed_frames += frames;

    if (frames < expectedFrames)
    {
        guaranteed(stream->end) stream->end(stream);

        stream->playing = 0;
        return;
    }
}

uint8_t openae_stream_is_valid(openae_stream_t* stream)
{
    assume(stream, 0);

    return openae_audio_file_is_valid(&stream->file);
}

void openae_stream_start(openae_stream_t* stream)
{
    assume(stream);

    openae_stream_stop(stream);
    stream->playing = 1;

#ifndef __PSP__
    ALuint* buffers = stream->buffers;
    for (int i = 0; i < OPENAE_AUDIO_BUFFERS_PER_SOURCE; i++)
    {
        ALuint buffer = buffers[i];
        openae_stream_update_buffer(stream, stream->data, OPENAE_AUDIO_FRAME_SIZE);
        alBufferData(buffer, AL_FORMAT_STEREO16, stream->data, OPENAE_AUDIO_FRAME_SIZE, stream->file.freq);
    }

    alSourceQueueBuffers(stream->source, OPENAE_AUDIO_BUFFERS_PER_SOURCE, buffers);

    alSourcePlay(stream->source);
#endif
}

void openae_stream_stop(openae_stream_t* stream)
{
    assume(stream);

    openae_audio_file_seek(&stream->file, 0);
    stream->playing = 0;

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
    assume(stream);

    openae_audio_file_close(&stream->file);

#ifndef __PSP__
    ALint processed_buffer_count = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed_buffer_count);
    if (processed_buffer_count > 0)
    {
        memset(openae_context_get_current()->processed_buffers, 0, sizeof(ALuint)*OPENAE_AUDIO_BUFFERS_PER_SOURCE);
        alSourceUnqueueBuffers(stream->source, OPENAE_AUDIO_BUFFERS_PER_SOURCE, openae_context_get_current()->processed_buffers);
    }

    alDeleteBuffers(OPENAE_AUDIO_BUFFERS_PER_SOURCE, stream->buffers);
    alDeleteSources(1, &stream->source);
#endif

    memset(stream, 0, sizeof(openae_stream_t));
}
