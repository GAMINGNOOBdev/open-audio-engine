#include <openae/audio_file.h>
#include <openae/logging.h>
#include <memory.h>

#ifdef __PSP__
#   include <pspaudiolib.h>
#   include <pspkernel.h>
#   include <pspaudio.h>
#endif

static int strlpos(const char* str, char c)
{
    char* s = (char*)str;
    int idx = 0;
    int resIdx = -1;
    while (*s != 0)
    {
        if (*s == c)
            resIdx = idx;

        s++;
        idx++;
    }
    return resIdx;
}

openae_audio_file_t openae_audio_file_open(const char* filepath)
{
    openae_audio_file_t file = {};
    memset(&file, 0, sizeof(openae_audio_file_t));
    file.format = OPENAE_AUDIO_FORMAT_INVALID;

    if (!filepath)
        return file;

    int lastDot = strlpos(filepath, '.');
    int vorbis = strcmp(&filepath[lastDot+1], "ogg");
    int mp3 = strcmp(&filepath[lastDot+1], "mp3");
    int wav = strcmp(&filepath[lastDot+1], "wav");

    if (vorbis == 0)
    {
        LOGINFO("loading ogg vorbis file '%s'", filepath);
        file.vorbis_stream = stb_vorbis_open_filename(filepath, NULL, NULL);
        if (file.vorbis_stream == NULL)
        {
            LOGERROR("could not load audio file '%s': Unknown error", filepath);
            return file;
        }
        file.vorbis_info = stb_vorbis_get_info(file.vorbis_stream);
        file.length = stb_vorbis_stream_length_in_seconds(file.vorbis_stream);
        file.samples = stb_vorbis_stream_length_in_samples(file.vorbis_stream);
        file.freq = file.vorbis_info.sample_rate;
        file.channels = file.vorbis_info.channels;

        LOGINFO("sample rate: '%d' | channels: '%d' | max_frame_size: '%d' | length: '%f' | samples: '%d'", file.freq, file.vorbis_info.channels, file.vorbis_info.max_frame_size, file.length, file.samples);

        file.format = OPENAE_AUDIO_FORMAT_VORBIS;
    }
    else if (mp3 == 0)
    {
        LOGINFO("loading mp3 file '%s'", filepath);
        if (mp3dec_ex_open(&file.mp3, filepath, MP3D_SEEK_TO_SAMPLE))
        {
            LOGERROR("could not load audio file '%s': Unknown error", filepath);
            return file;
        }

        file.samples = file.mp3.samples;
        file.freq = file.mp3.info.hz;
        file.length = (float)file.samples / (float)file.freq;
        file.channels = file.mp3.info.channels;

        LOGINFO("sample rate: '%d' | channels: '%d' | frame_bytes: '%d' | length: '%f' | samples: '%d'", file.freq, file.mp3.info.channels, file.mp3.info.frame_bytes, file.length, file.samples);

        file.format = OPENAE_AUDIO_FORMAT_MP3;
    }
    else if (wav == 0)
    {
        LOGINFO("loading wav file '%s'", filepath);
        file.wav = wave_open(filepath, WAVE_OPEN_READ);
        if (wave_get_format(file.wav) != WAVE_FORMAT_PCM)
        {
            LOGERROR("could not load wav file '%s': expected int16 format", filepath);
            wave_close(file.wav);
            return file;
        }
        file.samples = wave_get_length(file.wav);
        file.freq = wave_get_sample_rate(file.wav);
        file.length = (float)file.samples / (float)file.freq;
        file.channels = wave_get_num_channels(file.wav);

        file.format = OPENAE_AUDIO_FORMAT_WAV;

        LOGINFO("sample rate: '%ld' | channels: '%d' | length: '%f'", file.freq, file.channels, file.length);
    }
    else
    {
        LOGERROR("could not load audio file '%s': Invalid format", filepath);
    }

    file.length_ms = file.length * 1000.0f;

    #ifdef __PSP__
    sceKernelDcacheWritebackInvalidateAll();
    #endif

    return file;
}

uint8_t openae_audio_file_is_valid(openae_audio_file_t* file)
{
    if (!file)
        return 0;

    return file->format != OPENAE_AUDIO_FORMAT_INVALID;
}

void openae_audio_file_close(openae_audio_file_t* file)
{
    if (!file)
        return;

    if (file->format == OPENAE_AUDIO_FORMAT_VORBIS)
        stb_vorbis_close(file->vorbis_stream);
    else if (file->format == OPENAE_AUDIO_FORMAT_MP3)
        mp3dec_ex_close(&file->mp3);
    else if (file->format == OPENAE_AUDIO_FORMAT_WAV)
        wave_close(file->wav);

    memset(file, 0, sizeof(openae_audio_file_t));
}

