#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>

#include <openae/audio_file.h>
#include <openae/context.h>
#include <openae/logging.h>
#include <openae/stream.h>

PSP_MODULE_INFO("OpenAE test", 0, 0, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define psprintf pspDebugScreenPrintf

int ExitRequest = 0;

void stop_running()
{
    ExitRequest = 1;
}

int is_running()
{
    return ExitRequest == 0;
}

int exit_callback(int arg1, int arg2, void* common)
{
    stop_running();
    return 0;
}

int callback_thread(SceSize args, void* argsPtr)
{
    int callBackID;
    callBackID = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(callBackID);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks()
{
    int threadID = sceKernelCreateThread("Exit Callback Thread", callback_thread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
    sceKernelStartThread(threadID, 0, 0);

    return threadID;
}

void exit_game()
{
    sceKernelExitGame();
}

void end()
{
    LOGDEBUG("yo hi from psp end !!");
    ExitRequest = 1;
}

void end_sfx()
{
    LOGDEBUG("yo hi from psp sfx end !!");
}

void loghandlepsp(const char* msg)
{
    psprintf("%s", msg);
}

int main()
{
    setup_callbacks();
    pspDebugScreenInit();

    FILE* file = fopen("log.log", "wb+");
    log_set_stream(file);
    log_set_msg_handler(loghandlepsp);
    log_enable_debug_msgs(1);

    openae_context_initialize();
    openae_stream_t* music_stream = openae_context_set_music("snd/0.ogg");
    openae_stream_t* sfx_stream = openae_context_play_sfx("snd/1.mp3");

    music_stream->end = end;
    sfx_stream->end = end_sfx;

    while (is_running())
    {
        openae_context_update_all(); // technically not needed but wtv, consistency
    }
    openae_context_dispose();

    fclose(file);

    exit_game();
}
