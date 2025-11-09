#include <openae/callbacks.h>
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

static openae_context_t openae_current_context;
static uint8_t openae_initialized = 0;

void openae_context_initialize(void)
{
    if (openae_initialized)
        return;

    openae_initialized = 1;
    memset(&openae_current_context, 0, sizeof(openae_context_t));
    openae_current_context.audio_volume = 1.0f;

#ifdef __PSP__
    pspAudioInit();
    pspAudioSetChannelCallback(0, openae_fill_buffer_callback, (void*)&openae_current_context.music);
    // for (size_t i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
    //     pspAudioSetChannelCallback(1 + i, openae_fill_buffer_callback, (void*)(openae_current_context.sfx +i));
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
    return &openae_current_context;
}

void openae_context_set_music(openae_stream_t* stream)
{
    if (!openae_initialized)
        return;

    openae_stream_stop(openae_current_context.music);
    openae_current_context.music = stream;
    #ifndef __PSP__
    if (stream == NULL)
        return;

    openae_stream_start(openae_current_context.music);

    #endif
}

void openae_context_play_sfx(openae_stream_t* stream)
{
    if (!openae_initialized || !stream)
        return;

    //
}

void openae_context_update_all(void)
{
    if (!openae_initialized)
        return;

    openae_stream_update(openae_current_context.music);
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
        openae_stream_update(openae_current_context.sfx[i]);
}

void openae_context_dispose(void)
{
    if (!openae_initialized)
        return;

    openae_initialized = 0;

#ifndef __PSP__
    alcMakeContextCurrent(NULL);
    alcDestroyContext(openae_current_context.context);
    alcCloseDevice(openae_current_context.device);
#endif
    memset(&openae_current_context, 0, sizeof(openae_context_t));
}
