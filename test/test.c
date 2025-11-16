#include <openae/audio_file.h>
#include <openae/context.h>
#include <openae/logging.h>
#include <openae/stream.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <minimp3_ex.h>

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

    openae_context_t context = {0};
    openae_context_initialize(&context);
    openae_stream_t* music = openae_context_set_music(&context, music_path);
    openae_stream_t* sfx = openae_context_play_sfx(&context, sfx_path);

    music->end = music_end;
    sfx->end = sfx_end;

    while (music->playing || (sfx && sfx->playing))
        openae_context_update_all(&context);
    openae_context_dispose(&context);

    fclose(file);

    return 0;
}
