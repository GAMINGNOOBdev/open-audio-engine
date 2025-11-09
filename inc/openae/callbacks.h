#ifndef __OPENAE__CALLBACKS_H_
#define __OPENAE__CALLBACKS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Music data callback to fill a buffer
 * 
 * @param buffer Audio data buffer
 * @param length Length of samples
 * @param userdata User data
 */
void openae_fill_buffer_callback(void* buffer, unsigned int length, void* userdata);

#ifdef __cplusplus
}
#endif

#endif
