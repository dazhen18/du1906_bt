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
#include "string.h"
#include "esp_log.h"
#include "board.h"
#include "audio_mem.h"

#include "periph_sdcard.h"
#include "led_indicator.h"
#include "periph_adc_button.h"
#include "led_bar_ws2812.h"
#include "display_service.h"
#ifdef CONFIG_MARS_ADC_ES7243
    #include "es7243.h"
#elif CONFIG_MARS_ADC_ES7243E
    #include "es7243e.h"
#endif  

static const char *TAG = "AUDIO_BOARD";

static audio_board_handle_t board_handle;
static void init_cs_pin(void)
{
	gpio_config_t  io_conf;
    memset(&io_conf, 0, sizeof(io_conf));
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = BIT(DAC_CS_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(DAC_CS_GPIO, 0);
    //gpio_set_level(get_pa_enable_gpio(), 1);
}
audio_board_handle_t audio_board_init(void)
{
    if (board_handle) {
        ESP_LOGW(TAG, "The board has already been initialized!");
        return board_handle;
    }
    board_handle = (audio_board_handle_t) audio_calloc(1, sizeof(struct audio_board_handle));
    AUDIO_MEM_CHECK(TAG, board_handle, return NULL);
    
	board_handle->audio_hal = audio_board_dac_init();
#ifdef CONFIG_MARS_ADC_ES7243
    es7243e_adc_set_addr(0x26);
#elif CONFIG_MARS_ADC_ES7243E

    es7243e_adc_set_addr(0x20);
#endif
    board_handle->adc_ref_pa_hal = audio_board_adc_init(); 
    return board_handle;
}

audio_hal_handle_t audio_board_dac_init(void)
{
    audio_hal_handle_t dac_hal = NULL;
    audio_hal_codec_config_t audio_codec_cfg = AUDIO_CODEC_DEFAULT_CONFIG();
#ifdef CONFIG_MARS_DAC_TAS5805M
    dac_hal = audio_hal_init(&audio_codec_cfg, &AUDIO_CODEC_TAS5805M_DEFAULT_HANDLE);
#elif CONFIG_MARS_DAC_ES8156
	init_cs_pin();
    dac_hal = audio_hal_init(&audio_codec_cfg, &AUDIO_CODEC_ES8156_DEFAULT_HANDLE);
	printf("application audio_board_adc_init CONFIG_MARS_DAC_ES8156\r\n\r\n\r\n");
    //i2s_mclk_gpio_select(I2S_NUM_0, GPIO_NUM_0);
#endif

    AUDIO_NULL_CHECK(TAG, dac_hal, return NULL);
    return dac_hal;
}

audio_hal_handle_t audio_board_adc_init(void)
{
    audio_hal_handle_t adc_hal = NULL;
#ifdef CONFIG_MARS_ADC_ES7243
	printf("application audio_board_adc_init CONFIG_MARS_ADC_ES7243 \r\n\r\n\r\n");

    audio_hal_codec_config_t audio_codec_cfg = AUDIO_CODEC_DEFAULT_CONFIG();
    adc_hal = audio_hal_init(&audio_codec_cfg, &AUDIO_CODEC_ES7243_DEFAULT_HANDLE);
    AUDIO_NULL_CHECK(TAG, adc_hal, return NULL);
#elif CONFIG_MARS_ADC_ES7243E
    printf("application audio_board_adc_init CONFIG_MARS_ADC_ES7243e \r\n\r\n\r\n");

    audio_hal_codec_config_t audio_codec_cfg = AUDIO_CODEC_DEFAULT_CONFIG();
    adc_hal = audio_hal_init(&audio_codec_cfg, &AUDIO_CODEC_ES7243E_DEFAULT_HANDLE);

#endif

    return adc_hal;
}

audio_board_handle_t audio_board_get_handle(void)
{
    return board_handle;
}

esp_err_t audio_board_deinit(audio_board_handle_t audio_board)
{
    esp_err_t ret = ESP_OK;
    ret |= audio_hal_deinit(audio_board->audio_hal);
    audio_free(audio_board);
    board_handle = NULL;
    return ret;
}
