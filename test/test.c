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
    openae_stream_t* music_stream = openae_context_set_music(music_path);
    openae_stream_t* sfx_stream = openae_context_play_sfx(sfx_path);
    while (music_stream->playing || (sfx_stream && sfx_stream->playing))
        openae_context_update_all();
    openae_context_dispose();
    return 0;
}
