#include <openae/audio_file.h>
#include <openae/context.h>
#include <openae/logging.h>
#include <openae/stream.h>

int main(const int argc, const char* argv[])
{
    char* path = NULL;
    if (argc >= 2)
        path = (char*)argv[1];
    else
    {
        LOGINFO("How to run: %s <path to audio file>", *argv);
        return 0;
    }

    openae_context_initialize();
    openae_audio_file_t file = openae_audio_file_open(path);
    if (!openae_audio_file_is_valid(&file))
    {
        LOGERROR("Unable to open audio file, exiting test app");
        openae_context_dispose();
        return 0;
    }
    openae_stream_t stream = openae_stream_create(&file);
    openae_context_set_music(&stream);
    while (stream.playing)
        openae_context_update_all();
    openae_stream_dispose(&stream);
    openae_audio_file_close(&file);
    openae_context_dispose();
    return 0;
}

