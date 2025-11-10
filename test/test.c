#include <openae/audio_file.h>
#include <openae/context.h>
#include <openae/logging.h>
#include <openae/stream.h>

int main(const int argc, const char* argv[])
{
    char* music_path = NULL;
    char* sfx_path = NULL;
    if (argc >= 3)
    {
        music_path = (char*)argv[1];
        sfx_path = (char*)argv[2];
    }
    else
    {
        LOGINFO("How to run: %s <path to music> <path to sfx>", *argv);
        return 0;
    }

    openae_context_initialize();
    openae_audio_file_t music = openae_audio_file_open(music_path);
    if (!openae_audio_file_is_valid(&music))
    {
        LOGERROR("Unable to open audio file, exiting test app");
        openae_context_dispose();
        return 0;
    }
    openae_audio_file_t sfx = openae_audio_file_open(sfx_path);
    if (!openae_audio_file_is_valid(&music))
    {
        LOGERROR("Unable to open audio file, exiting test app");
        openae_context_dispose();
        return 0;
    }
    openae_stream_t music_stream = openae_stream_create(&music);
    openae_stream_t sfx_stream = openae_stream_create(&sfx);
    openae_context_set_music(&music_stream);
    openae_context_play_sfx(&sfx_stream);
    while (music_stream.playing || sfx_stream.playing)
        openae_context_update_all();
    openae_stream_dispose(&sfx_stream);
    openae_stream_dispose(&music_stream);
    openae_audio_file_close(&sfx);
    openae_audio_file_close(&music);
    openae_context_dispose();
    return 0;
}

