#include <openae/callbacks.h>
#include <openae/context.h>
#include <openae/stream.h>
#include <memory.h>

void openae_fill_buffer_callback(void* buffer, unsigned int length, void* userdata)
{
    openae_stream_t** stream_ptr = (openae_stream_t**)userdata;
    openae_stream_t* astream = *stream_ptr;
    memset(buffer, 0, sizeof(short) * length * 2);

    if (!astream)
        return;

    if (astream->processed_frames >= astream->file->samples)
        return;

    int frames = 0;
    int numFrames = length;

    if (astream->format == OPENAE_AUDIO_FORMAT_VORBIS)
    {
        short* buf = (short*)buffer;
        numFrames *= astream->file->vorbis_info.channels;
        frames = stb_vorbis_get_samples_short_interleaved(astream->file->vorbis_stream, astream->file->vorbis_info.channels, buf, numFrames);

        if (astream->volume != 1)
            for (int i = 0; i < numFrames; i++)
                buf[i] *= astream->volume;
    }
    else if (astream->format == OPENAE_AUDIO_FORMAT_MP3)
    {
        numFrames *= astream->file->mp3.info.channels;
        mp3dec_frame_info_t frameInfo;
        frames = mp3dec_ex_read(&astream->file->mp3, (mp3d_sample_t*)buffer, numFrames) / astream->file->mp3.info.channels;

        if (astream->volume != 1)
            for (int i = 0; i < numFrames; i++)
                ((mp3d_sample_t*)buffer)[i] *= astream->volume;
    }
    else if (astream->format == OPENAE_AUDIO_FORMAT_WAV)
    {
        frames = wave_read(astream->file->wav, buffer, numFrames);

        if (astream->volume != 1)
            for (int i = 0; i < numFrames; i++)
                ((WaveI16*)buffer)[i] *= astream->volume;
    }

    astream->processed_frames += frames;

    if (frames < length)
    {
        if (astream->end != NULL)
            astream->end(astream);

        openae_context_get_current()->music = NULL;
        astream->ready = 0;
    }
}
