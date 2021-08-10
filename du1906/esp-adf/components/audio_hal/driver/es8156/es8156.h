/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#ifndef __ES8156_H__
#define __ES8156_H__

#include "esp_types.h"
#include "audio_hal.h"
#include "driver/i2c.h"
#include "esxxx_common.h"
/* ES8156 address
 * 0x12:CE=1;0x10:CE=0
 */
#define ES8156_ADDR         0x10

esp_err_t ES8156_codec_init(audio_hal_codec_config_t *codec_cfg);
esp_err_t ES8156_codec_deinit();
esp_err_t ES8156_codec_ctrl_state(audio_hal_codec_mode_t mode, audio_hal_ctrl_t ctrl_state);
esp_err_t ES8156_codec_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface);
esp_err_t ES8156_set_voice_mute(bool enable);
esp_err_t ES8156_codec_set_voice_volume(int volume);
esp_err_t ES8156_codec_get_voice_volume(int *volume);
esp_err_t ES8156_start(es_module_t mode);
esp_err_t ES8156_stop(es_module_t mode);


#endif //__ES8388_H__
