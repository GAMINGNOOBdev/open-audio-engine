#include <openae/audio_file.h>
#include <openae/context.h>
#include <openae/logging.h>
#include <openae/stream.h>

void music_end(void* _)
{
    LOGDEBUG("music ended !!");
}

void sfx_end(void* _)
{
    LOGDEBUG("sfx ended !!");
}

int main(const int argc, const char* argv[])
{
    char* music_path = NULL;
    char* sfx_path = NULL;
    if (argc < 3)
    {
        LOGINFO("How to run: %s <path to music> <path to sfx>", *argv);
        return 0;
    }
    music_path = (char*)argv[1];
    sfx_path = (char*)argv[2];

    FILE* file = fopen("log.log", "wb+");
    log_set_stream(file);
    log_enable_debug_msgs(1);

    openae_context_initialize();
    openae_stream_t* music = openae_context_set_music(music_path);
    openae_stream_t* sfx = openae_context_play_sfx(sfx_path);

    music->end = music_end;
    sfx->end = sfx_end;

    while (music->playing || (sfx && sfx->playing))
        openae_context_update_all();
    openae_context_dispose();

    fclose(file);

    return 0;
}
