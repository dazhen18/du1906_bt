/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2020 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _AUDIO_BOARD_H_
#define _AUDIO_BOARD_H_

#include "audio_hal.h"
#include "board_def.h"
#include "board_pins_config.h"
#include "esp_peripherals.h"
#include "display_service.h"
#include "periph_sdcard.h"

#ifdef __cplusplus
extern "C" {
#endif

/*for company C/C++ name format requirements*/
#define g_audio_codec_tas5805m_default_handle AUDIO_CODEC_TAS5805M_DEFAULT_HANDLE
#define g_audio_codec_es8156_default_handle AUDIO_CODEC_ES8156_DEFAULT_HANDLE
#define g_audio_codec_es7148_default_handle   AUDIO_CODEC_ES7148_DEFAULT_HANDLE
#define g_audio_codec_es7243_default_handle   AUDIO_CODEC_ES7243_DEFAULT_HANDLE
#define g_audio_codec_es7243e_default_handle   AUDIO_CODEC_ES7243E_DEFAULT_HANDLE

extern audio_hal_func_t g_audio_codec_tas5805m_default_handle;
extern audio_hal_func_t g_audio_codec_es8156_default_handle;
extern audio_hal_func_t g_audio_codec_es7148_default_handle;
extern audio_hal_func_t g_audio_codec_es7243_default_handle;
extern audio_hal_func_t g_audio_codec_es7243e_default_handle;

/**
 * @brief Audio board handle
 */
struct audio_board_handle {
    audio_hal_handle_t audio_hal;           /*!< pa hardware abstract layer handle */
    //audio_hal_handle_t adc_line_in_hal;     /*!< adc hardware abstract layer handle */
    audio_hal_handle_t adc_ref_pa_hal;      /*!< adc hardware abstract layer handle */
};

typedef struct audio_board_handle *audio_board_handle_t;

/**
 * @brief Initialize audio board
 *
 * @return The audio board handle
 */
audio_board_handle_t audio_board_init(void);

/**
 * @brief Initialize DAC chip
 *
 * @return The audio hal handle
 */
audio_hal_handle_t audio_board_dac_init(void);

/**
 * @brief Initialize ADC chip
 *
 * @return The audio hal handle
 */
audio_hal_handle_t audio_board_adc_init(void);

/**
 * @brief Query audio_board_handle
 *
 * @return The audio board handle
 */
audio_board_handle_t audio_board_get_handle(void);

/**
 * @brief Uninitialize the audio board
 *
 * @param audio_board The handle of audio board
 *
 * @return  ESP_OK,      success
 *          others,      fail
 */
esp_err_t audio_board_deinit(audio_board_handle_t audio_board);



#ifdef __cplusplus
}
#endif

#endif
