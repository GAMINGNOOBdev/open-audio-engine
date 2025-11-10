#include <openae/context.h>
#include <openae/logging.h>
#include <openae/define.h>
#include <openae/stream.h>
#include <memory.h>

#ifndef __PSP__
static ALCdevice* openae_context_choose_device(void)
{
    ALCdevice* device = NULL;

    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_FALSE)
    {
        LOGWARNING("Cannot find multiple devices, choosing default device");
        return alcOpenDevice(NULL);
    }

    const char* defaultDevice = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    const char* devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    LOGDEBUG("Default device name: '%s'", defaultDevice);

    char* deviceNamePtr = (char*)devices;
    while (*deviceNamePtr != 0)
    {
        LOGDEBUG("Device name: '%s'", deviceNamePtr);
        deviceNamePtr += strlen(deviceNamePtr)+1;
    }

    device = alcOpenDevice(defaultDevice);
    LOGINFO("Found suitable device");
    return device;
}
#endif // #ifndef __PSP__

#ifdef __PSP__
static void openae_fill_buffer_callback(void* buffer, unsigned int samples, void* userdata)
{
    uint32_t bytes = sizeof(short) * samples * 2;
    memset(buffer, 0, bytes);
    openae_stream_t* stream = (openae_stream_t*)userdata;
    if (stream == NULL || !openae_stream_is_valid(stream))
        return;

    openae_stream_update_buffer(stream, buffer, bytes);
    openae_stream_update(stream);

    if (!stream->playing)
        openae_stream_dispose(stream);
}
#endif

static openae_context_t openae_current_context;
static uint8_t openae_initialized = 0;

void openae_context_initialize(void)
{
    assume(!openae_initialized);

    openae_initialized = 1;
    memset(&openae_current_context, 0, sizeof(openae_context_t));
    openae_current_context.audio_volume = 1.0f;

#ifdef __PSP__
    pspAudioInit();
    pspAudioSetChannelCallback(0, openae_fill_buffer_callback, (void*)&openae_current_context.music);
    for (size_t i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
        pspAudioSetChannelCallback(1 + i, openae_fill_buffer_callback, (void*)(&openae_current_context.sfx[i]));
#else
    openae_current_context.device = openae_context_choose_device();
    openae_current_context.context = alcCreateContext(openae_current_context.device, NULL);
    alcMakeContextCurrent(openae_current_context.context);

    alListenerf(AL_GAIN, 1.0f);
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    ALfloat listenerOrientation[] = {
        0, 0, 1, 1, 1, 0
    };
    alListenerfv(AL_ORIENTATION, listenerOrientation);
#endif

    openae_current_context.initialized = 1;
}

openae_context_t* openae_context_get_current(void)
{
    assume(openae_initialized, NULL);

    return &openae_current_context;
}

openae_stream_t* openae_context_set_music(const char* filepath)
{
    assume(openae_initialized, NULL);
    assume(filepath, NULL);

    openae_stream_stop(&openae_current_context.music);
    openae_stream_dispose(&openae_current_context.music);
    openae_current_context.music = openae_stream_create(filepath);
    assume(openae_stream_is_valid(&openae_current_context.music), NULL);
    openae_stream_start(&openae_current_context.music);
    return &openae_current_context.music;
}

openae_stream_t* openae_context_play_sfx(const char* filepath)
{
    assume(openae_initialized, NULL);
    assume(filepath, NULL);

    int freeIndex = -1;
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
    {
        if (!openae_stream_is_valid(&openae_current_context.sfx[i]))
        {
            freeIndex = i;
            break;
        }
    }

    if (freeIndex == -1)
    {
        LOGERROR("Cannot play stream 0x%x: insufficient available playback slots");
        return NULL;
    }

    openae_stream_dispose(&openae_current_context.sfx[freeIndex]);
    openae_current_context.sfx[freeIndex] = openae_stream_create(filepath);
    assume(openae_stream_is_valid(&openae_current_context.sfx[freeIndex]), NULL);
    openae_stream_start(&openae_current_context.sfx[freeIndex]);

    return &openae_current_context.sfx[freeIndex];
}

void openae_context_update_all(void)
{
    assume(openae_initialized);

    openae_stream_t* music = &openae_current_context.music;

    openae_stream_update(music);
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
    {
        openae_stream_t* sfx = &openae_current_context.sfx[i];
        if (!openae_stream_is_valid(sfx))
            continue;
        openae_stream_update(sfx);

        if (!sfx->playing)
            openae_stream_dispose(sfx);
    }

    if (!music->playing)
        openae_stream_dispose(music);
}

void openae_context_dispose(void)
{
    assume(openae_initialized);

    openae_stream_dispose(&openae_current_context.music);
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
        openae_stream_dispose(&openae_current_context.sfx[i]);

    openae_initialized = 0;

#ifdef __PSP__
    pspAudioEnd();
#else
    alcMakeContextCurrent(NULL);
    alcDestroyContext(openae_current_context.context);
    alcCloseDevice(openae_current_context.device);
#endif
    memset(&openae_current_context, 0, sizeof(openae_context_t));
}
