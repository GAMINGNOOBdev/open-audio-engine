#ifndef __OPENAE__AUDIO_FILE_H_
#define __OPENAE__AUDIO_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define STB_VORBIS_HEADER_ONLY
#include <openae/define.h>
#include <stb_vorbis.c>
#define MINIMP3_NO_SIMD
#include <minimp3_ex.h>
#include <wave.h>

typedef enum openae_audio_format_t
{
    OPENAE_AUDIO_FORMAT_INVALID,
    OPENAE_AUDIO_FORMAT_VORBIS,
    OPENAE_AUDIO_FORMAT_MP3,
    OPENAE_AUDIO_FORMAT_WAV,
} openae_audio_format_t;

typedef struct openae_audio_file_t
{
    uint32_t freq;
    int channels;
    float length;
    int length_ms;
    uint32_t samples;

    struct stb_vorbis* vorbis_stream;
    stb_vorbis_info vorbis_info;
    mp3dec_ex_t mp3;
    WaveFile* wav;

    openae_audio_format_t format;
} openae_audio_file_t;

/**
 * @brief Open an audio file
 * 
 * @param filepath Path to the file
 * @returns An audio file handle
 */
openae_audio_file_t openae_audio_file_open(const char* filepath);

/**
 * @brief Check if the audio file is valid (data loaded, etc.)
 * 
 * @param file Audio file
 * @returns 1 if valid, 0 if not
 */
uint8_t openae_audio_file_is_valid(openae_audio_file_t* file);

/**
 * @brief Seek the audio to a specific position/frame
 * 
 * @param file Audio file
 * @param pos Position
 */
void openae_audio_file_seek(openae_audio_file_t* file, uint64_t pos);

/**
 * @brief Closes an audio file
 * 
 * @param file The audio file
 */
void openae_audio_file_close(openae_audio_file_t* file);

#ifdef __cplusplus
}
#endif

#endif
