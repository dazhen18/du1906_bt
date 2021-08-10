/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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

#include <string.h>
#include "driver/i2c.h"
#include "i2c_bus.h"

#include "esp_log.h"
#include "es8156.h"
#include "board_pins_config.h"

/*
 * to define the clock soure of MCLK
 */
//#define FROM_MCLK_PIN       0
//#define FROM_SCLK_PIN       1
//#define MCLK_SOURCE         0

/*
 * to define whether to reverse the clock
 */
//#define INVERT_MCLK         0 // do not invert
//#define INVERT_SCLK         0

//#define IS_DMIC             0 // Is it a digital microphone

//#define MCLK_DIV_FRE        256

/*
 * i2c default configuration
 */
static i2c_config_t es_i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000,
    //.master.clk_speed = 1000,
};

/*
 * operate function of codec
 */
audio_hal_func_t AUDIO_CODEC_ES8156_DEFAULT_HANDLE = {
    .audio_codec_initialize = ES8156_codec_init,
    .audio_codec_deinitialize = ES8156_codec_deinit,
    .audio_codec_ctrl = ES8156_codec_ctrl_state,
    .audio_codec_config_iface = ES8156_codec_config_i2s,
    .audio_codec_set_mute = ES8156_set_voice_mute,
    .audio_codec_set_volume = ES8156_codec_set_voice_volume,
    .audio_codec_get_volume = ES8156_codec_get_voice_volume,
};

/*
 * Clock coefficient structer
 */
struct _coeff_div {
    uint32_t mclk;        /* mclk frequency */
    uint32_t rate;        /* sample rate */
    uint8_t pre_div;      /* the pre divider with range from 1 to 8 */
    uint8_t pre_multi;    /* the pre multiplier with x1, x2, x4 and x8 selection */
    uint8_t adc_div;      /* adcclk divider */
    uint8_t dac_div;      /* dacclk divider */
    uint8_t fs_mode;      /* double speed or single speed, =0, ss, =1, ds */
    uint8_t lrck_h;       /* adclrck divider and daclrck divider */
    uint8_t lrck_l;
    uint8_t bclk_div;     /* sclk divider */
    uint8_t adc_osr;      /* adc osr */
    uint8_t dac_osr;      /* dac osr */
};
static i2c_bus_handle_t     i2c_handler;

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
    //mclk     rate   pre_div  mult  adc_div dac_div fs_mode lrch  lrcl  bckdiv osr
    /* 8k */
    {12288000, 8000 , 0x06, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 8000 , 0x03, 0x02, 0x03, 0x03, 0x00, 0x05, 0xff, 0x18, 0x10, 0x10},
    {16384000, 8000 , 0x08, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 8000 , 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 8000 , 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 8000 , 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 8000 , 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000 , 8000 , 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 11.025k */
    {11289600, 11025, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 11025, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 11025, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 11025, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 12k */
    {12288000, 12000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 12000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 12000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 12000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 16k */
    {12288000, 16000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 16000, 0x03, 0x02, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 16000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 16000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 16000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 16000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 16000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 16000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 16000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000 , 16000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 22.05k */
    {11289600, 22050, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 22050, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 22050, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 24k */
    {12288000, 24000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 24000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 24000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 24000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 24000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 32k */
    {12288000, 32000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 32000, 0x03, 0x04, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 32000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 32000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 32000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 32000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 32000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
    {1024000 , 32000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 44.1k */
    {11289600, 44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 44100, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 44100, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 44100, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 48k */
    {12288000, 48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 48000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 48000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 48000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 48000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 64k */
    {12288000, 64000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 64000, 0x03, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {16384000, 64000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 64000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 64000, 0x01, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {4096000 , 64000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 64000, 0x01, 0x08, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {2048000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0xbf, 0x03, 0x18, 0x18},
    {1024000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 88.2k */
    {11289600, 88200, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 88200, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 88200, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 88200, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 96k */
    {12288000, 96000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 96000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 96000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
};

static char *TAG = "es8156";

#define ES_ASSERT(a, format, b, ...) \
    if ((a) != 0) { \
        ESP_LOGE(TAG, format, ##__VA_ARGS__); \
        return b;\
    }

static int ES8156_write_reg(uint8_t reg_addr, uint8_t data)
{
	int res = 0;

 #if 0   
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, ES8156_ADDR, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, reg_addr, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, data, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    ES_ASSERT(res, "ES8156 Write Reg error", -1);
 #endif   

	res = i2c_bus_write_bytes(i2c_handler, ES8156_ADDR, &reg_addr, 1, &data, 1);

	return res;
}

static int ES8156_read_reg(uint8_t reg_addr)
{
    uint8_t data = 0xff;
    int res = 0;
#if 0	
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, ES8156_ADDR, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, reg_addr, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, ES8156_ADDR | 0x01, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_read_byte(cmd, &data, 0x01 /*NACK_VAL*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
#endif


	res = i2c_bus_read_bytes(i2c_handler, ES8156_ADDR, &reg_addr, 1, &data, 1);


    ES_ASSERT(res, "ES8156 Read Reg error", -1);
    return (int)data;
}

static int i2c_init()
{
#if 0
    int res = 0;
    get_i2c_pins(I2C_NUM_0, &es_i2c_cfg);
    res |= i2c_param_config(I2C_NUM_0, &es_i2c_cfg);
    res |= i2c_driver_install(I2C_NUM_0, es_i2c_cfg.mode, 0, 0, 0);
    ES_ASSERT(res, "i2c_init error", -1);
    return res;
#endif
	int res = 0;
    res = get_i2c_pins(I2C_NUM_0, &es_i2c_cfg);
    ES_ASSERT(res, "getting i2c pins error", -1);
    i2c_handler = i2c_bus_create(I2C_NUM_0, &es_i2c_cfg);
    return res;
}

/*
* look for the coefficient in coeff_div[] table
*/
int get_coeff(uint32_t mclk, uint32_t rate)
{
    for (int i = 0; i < (sizeof(coeff_div) / sizeof(coeff_div[0])); i++) {
        if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
            return i;
    }
    return -1;
}

/*
* set ES8156 dac mute or not
* if mute = 0, dac un-mute
* if mute = 1, dac mute
*/
static void ES8156_mute(int mute)
{
    uint8_t regv;
    ESP_LOGI(TAG, "Enter into ES8156_mute(), mute = %d\n", mute);
    regv = ES8156_read_reg(0x11) & 0xf7;
    if (mute) {
        ES8156_write_reg(0x11, regv | 0x08);
    } else {
        ES8156_write_reg(0x11, regv & 0xF7);
    }
}

/*
* set ES8156 into suspend mode
*/
static void ES8156_suspend(void)
{
    ESP_LOGI(TAG, "Enter into ES8156_suspend()");

    ES8156_write_reg(0x14, 0x00);
    ES8156_write_reg(0x19, 0x02);
    ES8156_write_reg(0x21, 0x1f);
    ES8156_write_reg(0x22, 0x02);
    ES8156_write_reg(0x25, 0x21);
    ES8156_write_reg(0x25, 0x87);
    ES8156_write_reg(0x18, 0x01);
    ES8156_write_reg(0x09, 0x02);
    ES8156_write_reg(0x09, 0x01);
    ES8156_write_reg(0x08, 0x00);
}

/*
* enable pa power
*/
void ES8156_pa_power(bool enable)
{
    gpio_config_t  io_conf;
    memset(&io_conf, 0, sizeof(io_conf));
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = BIT(get_pa_enable_gpio());
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    if (enable) {
        gpio_set_level(get_pa_enable_gpio(), 1);
    } else {
        gpio_set_level(get_pa_enable_gpio(), 0);
    }
}

esp_err_t ES8156_codec_init(audio_hal_codec_config_t *codec_cfg)
{
//    uint8_t datmp, regv;
//    int coeff;
    esp_err_t ret = ESP_OK;
    i2c_init(); // ESP32 in master mode
/*	*/	
	ret |= ES8156_write_reg(0x02, 0x04);
    ret |= ES8156_write_reg(0x20, 0x2a);
    ret |= ES8156_write_reg(0x21, 0x3c);
    ret |= ES8156_write_reg(0x22, 0x00);
    ret |= ES8156_write_reg(0x24, 0x07);
    ret |= ES8156_write_reg(0x23, 0x3a);
    ret |= ES8156_write_reg(0x0a, 0x01);
    ret |= ES8156_write_reg(0x0b, 0x01);
    ret |= ES8156_write_reg(0x14, 0xBF);
    ret |= ES8156_write_reg(0x01, 0x21);
    //ret |= ES8156_write_reg(0x02, 0x04);
    ret |= ES8156_write_reg(0x0d, 0x14);
    ret |= ES8156_write_reg(0x18, 0x00);
    ret |= ES8156_write_reg(0x08, 0x3f);
    ret |= ES8156_write_reg(0x00, 0x02);
    ret |= ES8156_write_reg(0x00, 0x03); 
    ret |= ES8156_write_reg(0x25, 0x20); 
/*
    ret |= ES8156_write_reg(0x18, 0x00);
    ret |= ES8156_write_reg(0x20, 0x2a);
    ret |= ES8156_write_reg(0x21, 0x16);
    ret |= ES8156_write_reg(0x22, 0x08);
    ret |= ES8156_write_reg(0x23, 0x8a);
    ret |= ES8156_write_reg(0x0a, 0x01);
    ret |= ES8156_write_reg(0x0b, 0x01);
    ret |= ES8156_write_reg(0x14, 0xBF);
    ret |= ES8156_write_reg(0x01, 0xe1);
    ret |= ES8156_write_reg(0x09, 0x02);
    ret |= ES8156_write_reg(0x02, 0x84);
    ret |= ES8156_write_reg(0x0d, 0x14);
    ret |= ES8156_write_reg(0x08, 0x3f);
    ret |= ES8156_write_reg(0x00, 0x02);
    ret |= ES8156_write_reg(0x00, 0x03); 
    ret |= ES8156_write_reg(0x25, 0x20);   
*/
/*
    int sample_fre = 0;
    int mclk_fre = 0;
    switch (i2s_cfg->samples) {
        case AUDIO_HAL_08K_SAMPLES:
            sample_fre = 8000;
            break;
        case AUDIO_HAL_11K_SAMPLES:
            sample_fre = 11025;
            break;
        case AUDIO_HAL_16K_SAMPLES:
            sample_fre = 16000;
            break;
        case AUDIO_HAL_22K_SAMPLES:
            sample_fre = 22050;
            break;
        case AUDIO_HAL_24K_SAMPLES:
            sample_fre = 24000;
            break;
        case AUDIO_HAL_32K_SAMPLES:
            sample_fre = 32000;
            break;
        case AUDIO_HAL_44K_SAMPLES:
            sample_fre = 44100;
            break;
        case AUDIO_HAL_48K_SAMPLES:
            sample_fre = 48000;
            break;
        default:
            ESP_LOGE(TAG, "Unable to configure sample rate %dHz", sample_fre);
            break;
    }
*/
	ESP_LOGI(TAG, "ADC uesed es8156\r\n");
    ES8156_pa_power(true);
    return ESP_OK;
}

esp_err_t ES8156_codec_deinit()
{
//TODO
    return ESP_OK;
}

esp_err_t ES8156_config_fmt(es_i2s_fmt_t fmt)
{
    esp_err_t ret = ESP_OK;
    uint8_t dac_iface = 0;
    dac_iface = ES8156_read_reg(0x11);
    switch (fmt) {
        case AUDIO_HAL_I2S_NORMAL:
            ESP_LOGD(TAG, "ES8156 in I2S Format");
            dac_iface &= 0xFC;
            break;
        case AUDIO_HAL_I2S_LEFT:
        case AUDIO_HAL_I2S_RIGHT:
            ESP_LOGD(TAG, "ES8156 in LJ Format");
            dac_iface &= 0xFC;
            dac_iface |= 0x01;
            break;
        case AUDIO_HAL_I2S_DSP:
            ESP_LOGD(TAG, "ES8156 in DSP-A Format");
            dac_iface &= 0xf8;
            dac_iface |= 0x03;
            break;
        default:
            dac_iface &= 0xFC;
            break;
    }
    ret |= ES8156_write_reg(0x11, dac_iface);

    return ret;
}

esp_err_t ES8156_set_bits_per_sample(audio_hal_iface_bits_t bits)
{
    esp_err_t ret = ESP_OK;
    uint8_t dac_iface = 0;
    dac_iface = ES8156_read_reg(0x11);
    switch (bits) {
        case AUDIO_HAL_BIT_LENGTH_16BITS:
            dac_iface &= 0x8f;
            dac_iface |= 0x30;
            break;
        case AUDIO_HAL_BIT_LENGTH_24BITS:
        	dac_iface &= 0x8f;
            break;
        case AUDIO_HAL_BIT_LENGTH_32BITS:
            dac_iface &= 0x8f;
            dac_iface |= 0x40;
            break;
        default:
            dac_iface &= 0x8f;
            dac_iface |= 0x30;
            break;

    }
    ret |= ES8156_write_reg(0x11, dac_iface);
    return ret;
}

esp_err_t ES8156_codec_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface)
{
    int ret = ESP_OK;
    ret |= ES8156_set_bits_per_sample(iface->bits);
    ret |= ES8156_config_fmt(iface->fmt);
    return ret;
}

esp_err_t ES8156_codec_ctrl_state(audio_hal_codec_mode_t mode, audio_hal_ctrl_t ctrl_state)
{
    esp_err_t ret = ESP_OK;
    es_module_t es_mode = ES_MODULE_MIN;

    switch (mode) {
        case AUDIO_HAL_CODEC_MODE_ENCODE:
            es_mode  = ES_MODULE_ADC;
            break;
        case AUDIO_HAL_CODEC_MODE_LINE_IN:
            es_mode  = ES_MODULE_LINE;
            break;
        case AUDIO_HAL_CODEC_MODE_DECODE:
            es_mode  = ES_MODULE_DAC;
            break;
        case AUDIO_HAL_CODEC_MODE_BOTH:
            es_mode  = ES_MODULE_ADC_DAC;
            break;
        default:
            es_mode = ES_MODULE_DAC;
            ESP_LOGW(TAG, "Codec mode not support, default is decode mode");
            break;
    }

    if (ctrl_state == AUDIO_HAL_CTRL_START) {
        ret |= ES8156_start(es_mode);
    } else {
        ESP_LOGW(TAG, "The codec is about to stop");
        ret |= ES8156_stop(es_mode);
    }

    return ret;
}

esp_err_t ES8156_start(es_module_t mode)
{
    esp_err_t ret = ESP_OK;

    if (mode == ES_MODULE_LINE) {
        ESP_LOGE(TAG, "The codec ES8156 doesn't support ES_MODULE_LINE mode");
        return ESP_FAIL;
    }
    if (mode == ES_MODULE_ADC || mode == ES_MODULE_ADC_DAC) {
     
    }
    if (mode == ES_MODULE_DAC || mode == ES_MODULE_ADC_DAC) {
      
    }

    ret |= ES8156_write_reg(0x02, 0x04);
    ret |= ES8156_write_reg(0x20, 0x2a);
    ret |= ES8156_write_reg(0x21, 0x3c);
    ret |= ES8156_write_reg(0x22, 0x00);
    ret |= ES8156_write_reg(0x24, 0x07);
    ret |= ES8156_write_reg(0x23, 0x3a);
    ret |= ES8156_write_reg(0x0a, 0x01);
    ret |= ES8156_write_reg(0x0b, 0x01);
    ret |= ES8156_write_reg(0x14, 0xBF);
    ret |= ES8156_write_reg(0x01, 0x21);
    //ret |= ES8156_write_reg(0x02, 0x04);
    ret |= ES8156_write_reg(0x0d, 0x14);
    ret |= ES8156_write_reg(0x18, 0x00);
    ret |= ES8156_write_reg(0x08, 0x3f);
    ret |= ES8156_write_reg(0x00, 0x02);
    ret |= ES8156_write_reg(0x00, 0x03); 
    ret |= ES8156_write_reg(0x25, 0x20);   
    return ret;
}

esp_err_t ES8156_stop(es_module_t mode)
{
    esp_err_t ret = ESP_OK;
    ES8156_suspend();
    return ret;
}

esp_err_t ES8156_codec_set_voice_volume(int volume)
{
    esp_err_t res = ESP_OK;
    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }
    int vol = (volume) * 1910 / 1000;
    ESP_LOGI(TAG, "SET: volume:%d", vol);
    ES8156_write_reg(0x14, vol);
    return res;
}

esp_err_t ES8156_codec_get_voice_volume(int *volume)
{
    esp_err_t res = ESP_OK;
    int regv = 0;
    regv = ES8156_read_reg(0x14);
    if (regv == ESP_FAIL) {
        *volume = 0;
        res = ESP_FAIL;
    } else {
        *volume = regv * 100 / 191;
    }
    ESP_LOGD(TAG, "GET: res:%d, volume:%d", regv, *volume);
    return res;
}
#if 0
esp_err_t ES8156_codec_set_voice_volume(int volume)
{
    esp_err_t res = ESP_OK;
    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }
    int vol = (volume) * 2550 / 1000;
    ESP_LOGD(TAG, "SET: volume:%d", vol);
    ES8156_write_reg(0x14, vol);
    return res;
}

esp_err_t ES8156_codec_get_voice_volume(int *volume)
{
    esp_err_t res = ESP_OK;
    int regv = 0;
    regv = ES8156_read_reg(0x14);
    if (regv == ESP_FAIL) {
        *volume = 0;
        res = ESP_FAIL;
    } else {
        *volume = regv;
    }
    ESP_LOGI(TAG, "GET: res:%d, volume:%d", regv, *volume);
    return res;
}
#endif
esp_err_t ES8156_set_voice_mute(bool enable)
{
    ESP_LOGW(TAG, "ES8156SetVoiceMute volume:%d", enable);
    ES8156_mute(enable);
    return ESP_OK;
}

esp_err_t ES8156_get_voice_mute(int *mute)
{
    esp_err_t res = ESP_OK;
    uint8_t reg = 0;
    res = ES8156_read_reg(0x11);
    if (res != ESP_FAIL) {
        reg = (res & 0x08) >> 3;
    }
    *mute = reg;
    return res;
}

void ES8156_read_all()
{
    for (int i = 0; i < 0x4A; i++) {
        uint8_t reg = ES8156_read_reg(i);
        ets_printf("REG:%02x, %02x\n", reg, i);
    }
}
