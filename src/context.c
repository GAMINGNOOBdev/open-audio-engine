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

void openae_context_initialize(openae_context_t* ctx)
{
    assume(ctx);
    assume(!ctx->initialized);

    memset(ctx, 0, sizeof(openae_context_t));
    ctx->volume = 1.0f;

#ifdef __PSP__
    pspAudioInit();
    pspAudioSetChannelCallback(0, openae_fill_buffer_callback, (void*)&ctx->music);
    for (size_t i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
        pspAudioSetChannelCallback(1 + i, openae_fill_buffer_callback, (void*)(&ctx->sfx[i]));
#else
    ctx->device = openae_context_choose_device();
    ctx->context = alcCreateContext(ctx->device, NULL);
    alcMakeContextCurrent(ctx->context);

    alListenerf(AL_GAIN, ctx->volume);
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    ALfloat listenerOrientation[] = {
        0, 0, 1, 1, 1, 0
    };
    alListenerfv(AL_ORIENTATION, listenerOrientation);
#endif

    ctx->initialized = 1;
}

openae_stream_t* openae_context_set_music(openae_context_t* ctx, const char* filepath)
{
    assume(ctx, NULL);
    assume(ctx->initialized, NULL);
    assume(filepath, NULL);

    openae_stream_stop(&ctx->music);
    openae_stream_dispose(&ctx->music);
    openae_stream_create(&ctx->music, filepath);
    assume(openae_stream_is_valid(&ctx->music), NULL);
    openae_stream_start(&ctx->music);
    return &ctx->music;
}

openae_stream_t* openae_context_play_sfx(openae_context_t* ctx, const char* filepath)
{
    assume(ctx, NULL);
    assume(ctx->initialized, NULL);
    assume(filepath, NULL);

    int freeIndex = -1;
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
    {
        if (!openae_stream_is_valid(&ctx->sfx[i]))
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

    openae_stream_dispose(&ctx->sfx[freeIndex]);
    openae_stream_create(&ctx->sfx[freeIndex], filepath);
    assume(openae_stream_is_valid(&ctx->sfx[freeIndex]), NULL);
    openae_stream_start(&ctx->sfx[freeIndex]);

    return &ctx->sfx[freeIndex];
}

void openae_context_update_all(openae_context_t* ctx)
{
    assume(ctx);
    assume(ctx->initialized);

#ifndef __PSP__
    alListenerf(AL_GAIN, ctx->volume);
#else
    int vol = (int)(ctx->volume * (float)PSP_VOLUME_MAX);
    pspAudioSetVolume(0, vol, vol);
#endif
    openae_stream_t* music = &ctx->music;
    if (openae_stream_is_valid(music))
        openae_stream_update(music);
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
    {
        openae_stream_t* sfx = &ctx->sfx[i];
        if (!openae_stream_is_valid(sfx))
            continue;
        openae_stream_update(sfx);

        if (!sfx->playing)
            openae_stream_dispose(sfx);
    }

    if (!music->playing)
        openae_stream_dispose(music);
}

void openae_context_dispose(openae_context_t* ctx)
{
    assume(ctx);
    assume(ctx->initialized);

    openae_stream_dispose(&ctx->music);
    for (int i = 0; i < OPENAE_AUDIO_SFX_STREAMS_MAX; i++)
        openae_stream_dispose(&ctx->sfx[i]);

#ifdef __PSP__
    pspAudioEnd();
#else
    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx->context);
    alcCloseDevice(ctx->device);
#endif
    memset(ctx, 0, sizeof(openae_context_t));
}
