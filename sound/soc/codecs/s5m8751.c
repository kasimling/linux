/*
 * s5m8751.c  --  S5M8751 ALSA Soc Audio driver
 *
 * Copyright 2008 Samsung Electronics
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <asm/div64.h>

#include "s5m8751.h"

#define AUDIO_NAME "s5m8751"
#define S5M8751_VERSION "0.1"

/*
 * Debug
 */

#define S5M_DEBUG 1

#ifdef S5M_DEBUG
#define dbg(format, arg...) \
	printk(KERN_DEBUG AUDIO_NAME ": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif
#define err(format, arg...) \
	printk(KERN_ERR AUDIO_NAME ": " format "\n" , ## arg)
#define info(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#define warn(format, arg...) \
	printk(KERN_WARNING AUDIO_NAME ": " format "\n" , ## arg)

/* codec private data */
struct s5m_priv {
	unsigned int sysclk;
	unsigned int pcmclk;
};

/*
 * s5m register cache
 * We can't read the WM8990 register space when we
 * are using 2 wire for device control, so we cache them instead.
 */
static const u16 s5m_reg[] = S5M8751_REGISTER_DEFAULTS;

/*
 * read s5m register cache
 */
static inline unsigned int s5m_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	return cache[reg];
}

/*
 * write s5m register cache
 */
static inline void s5m_write_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;
	cache[reg] = value;
}

/*
 * write to the s5m register space
 */
static int s5m_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	u8 data[2];

	data[0] = reg & 0xFF;
	data[1] = value & 0xFF;

	s5m_write_reg_cache (codec, reg, value);
	if (codec->hw_write(codec->control_data, data, 2) == 2)
		return 0;
	else
		return -EIO;
}

#define S5M_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
	SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define S5M_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
	SNDRV_PCM_FMTBIT_S24_LE)

/*
 * Set's ADC and Voice DAC format.
 */
static int s5m_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;

	printk("%s,%d\n",__FUNCTION__,__LINE__);
	s5m_write(codec, 0x27, 0x14); // codec off
	s5m_write(codec, 0x27, 0x94); //sampling rate
	return 0;
}

/*
 * The WM8990 supports 2 different and mutually exclusive DAI
 * configurations.
 *
 * 1. ADC/DAC on Primary Interface
 * 2. ADC on Primary Interface/DAC on secondary
 */
struct snd_soc_dai s5m_dai = {
/* ADC/DAC on primary */
	.name = "S5M8751 Primary",
	.id = 1,
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = S5M_RATES,
		.formats = S5M_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = S5M_RATES,
		.formats = S5M_FORMATS,},
//	.ops = {
//		.hw_params = wm8990_hw_params,},
	.dai_ops = {
//		.set_pll = wm8990_set_dai_pll,
		.set_fmt = s5m_set_dai_fmt,
//		.set_sysclk = wm8990_set_dai_sysclk,
	},
};
EXPORT_SYMBOL_GPL(s5m_dai);

static int s5m_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;

	/* we only need to suspend if we are a valid card */
	if(!codec->card)
		return 0;

	//wm8990_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int s5m_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;
	int i;
	u8 data[2];
	u16 *cache = codec->reg_cache;

	/* we only need to resume if we are a valid card */
	if (!codec->card)
		return 0;

	/* Sync reg_cache with the hardware */
	for (i = 0; i < ARRAY_SIZE(s5m_reg); i++) {
		if (i + 1 == S5M_RESET)
			continue;
		data[0] = ((i + 1) << 1) | ((cache[i] >> 8) & 0x0001);
		data[1] = cache[i] & 0x00ff;
		codec->hw_write(codec->control_data, data, 2);
	}

	//wm8990_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

/*
 * initialise the WM8990 driver
 * register the mixer and dsp interfaces with the kernel
 */
static int s5m_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->codec;
	int ret = 0;

	codec->name = "S5M8751";
	codec->owner = THIS_MODULE;
	codec->read = s5m_read_reg_cache;
	codec->write = s5m_write;
	codec->dai = &s5m_dai;
	codec->num_dai = 2;
	codec->reg_cache_size = sizeof(s5m_reg);
	codec->reg_cache = kmemdup(s5m_reg, sizeof(s5m_reg), GFP_KERNEL);

	if (codec->reg_cache == NULL)
		return -ENOMEM;

	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "s5m : failed to create pcms\n");
		goto pcm_err;
	}

	/* charge output caps */
	codec->bias_level = SND_SOC_BIAS_OFF;

	/* s5m8751 codec init */
	s5m_write(codec, 0x27, 0x14); // codec off
	//wm8990_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	s5m_write(codec, 0x17, 0x3a);
	s5m_write(codec, 0x18, 0x12);
	s5m_write(codec, 0x19, 0x00);
	//s5m_write(codec, 0x1c, 0x18);
	//s5m_write(codec, 0x1d, 0x18);
	s5m_write(codec, 0x1c, 0x40);
	s5m_write(codec, 0x1d, 0x40);
	s5m_write(codec, 0x24, 0x00);
	//s5m_write(codec, 0x27, 0x94); //sampling rate
	//s5m_write(codec, 0x27, 0x14); //sampling rate
	s5m_write(codec, 0x28, 0x00);
	s5m_write(codec, 0x37, 0xc0);
	s5m_write(codec, 0x38, 0x10);
	s5m_write(codec, 0x39, 0x2a);

	s5m_write(codec, 0x27, 0x94); //sampling rate

	ret = snd_soc_register_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "s5m : failed to register card\n");
		goto card_err;
	}
	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
pcm_err:
	kfree(codec->reg_cache);
	return ret;
}

/* If the i2c layer weren't so broken, we could pass this kind of data
   around */
static struct snd_soc_device *s5m_socdev;

#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)

/*
 * WM8912 wire address is determined by GPIO5
 * state during powerup.
 *    low  = 0x34
 *    high = 0x36
 */
static unsigned short normal_i2c[] = { 0, I2C_CLIENT_END };

/* Magic definition of all other variables and things */
I2C_CLIENT_INSMOD;

static struct i2c_driver s5m_i2c_driver;
static struct i2c_client client_template;

static int s5m_codec_probe(struct i2c_adapter *adap, int addr, int kind)
{
	struct snd_soc_device *socdev = s5m_socdev;
	struct s5m_setup_data *setup = socdev->codec_data;
	struct snd_soc_codec *codec = socdev->codec;
	struct i2c_client *i2c;
	int ret;

	if (addr != setup->i2c_address)
		return -ENODEV;

	client_template.adapter = adap;
	client_template.addr = addr;

	i2c =  kmemdup(&client_template, sizeof(client_template), GFP_KERNEL);
	if (i2c == NULL){
		kfree(codec);
		return -ENOMEM;
	}
	i2c_set_clientdata(i2c, codec);
	codec->control_data = i2c;

	ret = i2c_attach_client(i2c);
	if (ret < 0) {
		err("failed to attach codec at addr %x\n", addr);
		goto err;
	}

	dbg("codec probe ret = %d .\n", ret);
	ret = s5m_init(socdev);
	if (ret < 0) {
		err("failed to initialise WM8990\n");
		goto err;
	}
	return ret;

err:
	kfree(codec);
	kfree(i2c);
	return ret;
}

static int s5m_i2c_detach(struct i2c_client *client)
{
	struct snd_soc_codec *codec = i2c_get_clientdata(client);
	i2c_detach_client(client);
	kfree(codec->reg_cache);
	kfree(client);
	return 0;
}

static int s5m_i2c_attach(struct i2c_adapter *adap)
{
	return i2c_probe(adap, &addr_data, s5m_codec_probe);
}

/* corgi i2c codec control layer */
static struct i2c_driver s5m_i2c_driver = {
	.driver = {
		.name = "S5M8751 I2C Codec",
		.owner = THIS_MODULE,
	},
	.id =             I2C_DRIVERID_WM8753,
	.attach_adapter = s5m_i2c_attach,
	.detach_client =  s5m_i2c_detach,
	.command =        NULL,
};

static struct i2c_client client_template = {
	.name =   "S5M8751",
	.driver = &s5m_i2c_driver,
};
#endif

static int s5m_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct s5m_setup_data *setup;
	struct snd_soc_codec *codec;
	struct s5m_priv *s5m;
	int ret = 0;

	info("S5M8751 Audio Codec %s", S5M8751_VERSION);

	setup = socdev->codec_data;
	codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

	s5m = kzalloc(sizeof(struct s5m_priv), GFP_KERNEL);
	if (s5m == NULL) {
		kfree(codec);
		return -ENOMEM;
	}

	codec->private_data = s5m;
	socdev->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);
	s5m_socdev = socdev;

#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)
	if (setup->i2c_address) {
		normal_i2c[0] = setup->i2c_address;
		codec->hw_write = (hw_write_t)i2c_master_send;
		ret = i2c_add_driver(&s5m_i2c_driver);
		if (ret != 0)
			printk(KERN_ERR "can't add i2c driver");
	}
#else
		/* Add other interfaces here */
#endif
	return ret;
}

/* power down chip */
static int s5m_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)
	i2c_del_driver(&s5m_i2c_driver);
#endif
	kfree(codec->private_data);
	kfree(codec);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_s5m= {
	.probe =	s5m_probe,
	.remove =	s5m_remove,
	.suspend =	s5m_suspend,
	.resume =	s5m_resume,
};

EXPORT_SYMBOL_GPL(soc_codec_dev_s5m);

MODULE_DESCRIPTION("ASoC S5M8751 driver");
MODULE_AUTHOR("Ryu Euiyoul");
MODULE_LICENSE("GPL");
