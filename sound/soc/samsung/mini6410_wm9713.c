/*
 * mini6410_wm9713.c  --  SoC audio for MINI6410
 *
 * Copyright 2010 Samsung Electronics Co. Ltd.
 * Author: Jaswinder Singh Brar <jassi.brar@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/ac97_codec.h>

#include "dma.h"
#include "ac97.h"

#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)

static struct snd_soc_card mini6410;

/*
 Playback (HeadPhone):-
	$ amixer sset 'Headphone' unmute
	$ amixer sset 'Right Headphone Out Mux' 'Headphone'
	$ amixer sset 'Left Headphone Out Mux' 'Headphone'
	$ amixer sset 'Right HP Mixer PCM' unmute
	$ amixer sset 'Left HP Mixer PCM' unmute

 Capture (LineIn):-
	$ amixer sset 'Right Capture Source' 'Line'
	$ amixer sset 'Left Capture Source' 'Line'
*/

/* Machine dapm widgets */
static const struct snd_soc_dapm_widget mini6410_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("Mic (on-board)", NULL),
};

/* audio map */
static struct snd_soc_dapm_route audio_map[] = {
	{ "MIC1", NULL, "Mic Bias" },
	{ "Mic Bias", NULL, "Mic (on-board)" },
};

static int mini6410_ac97_init(struct snd_soc_pcm_runtime *rtd) {
	struct snd_soc_codec *codec = rtd->codec;
    struct snd_soc_dapm_context *dapm = &codec->dapm;
	unsigned short val;

	/* add board specific widgets */
	snd_soc_dapm_new_controls(dapm, ARRAY_AND_SIZE(mini6410_dapm_widgets));

	/* setup board specific audio path audio_map */
	snd_soc_dapm_add_routes(dapm, ARRAY_AND_SIZE(audio_map));

	/* Prepare MIC input */
	val = codec->driver->read(codec, AC97_3D_CONTROL);
	codec->driver->write(codec, AC97_3D_CONTROL, val | 0xc000);

	/* Static setup for now */
	snd_soc_dapm_enable_pin(dapm, "Mic (on-board)");
	snd_soc_dapm_sync(dapm);

	return 0;
}

static struct snd_soc_dai_link mini6410_dai = {
	.name = "AC97",
	.stream_name = "AC97 PCM",
	.platform_name = "samsung-audio",
	.cpu_dai_name = "samsung-ac97",
	.codec_dai_name = "wm9713-hifi",
	.codec_name = "wm9713-codec",
	.init = mini6410_ac97_init,
};

static struct snd_soc_card mini6410 = {
	.name = "MINI6410",
	.dai_link = &mini6410_dai,
	.num_links = 1,
};

static struct platform_device *mini6410_snd_wm9713_device;
static struct platform_device *mini6410_snd_ac97_device;

static int __init mini6410_init(void)
{
	int ret;

	mini6410_snd_wm9713_device = platform_device_alloc("wm9713-codec", -1);
	if (!mini6410_snd_wm9713_device)
		return -ENOMEM;

	ret = platform_device_add(mini6410_snd_wm9713_device);
	if (ret)
		goto err1;

	mini6410_snd_ac97_device = platform_device_alloc("soc-audio", -1);
	if (!mini6410_snd_ac97_device) {
		ret = -ENOMEM;
		goto err2;
	}

	platform_set_drvdata(mini6410_snd_ac97_device, &mini6410);

	ret = platform_device_add(mini6410_snd_ac97_device);
	if (ret)
		goto err3;

	return 0;

err3:
	platform_device_put(mini6410_snd_ac97_device);
err2:
	platform_device_del(mini6410_snd_wm9713_device);
err1:
	platform_device_put(mini6410_snd_wm9713_device);
	return ret;
}

static void __exit mini6410_exit(void)
{
	platform_device_unregister(mini6410_snd_ac97_device);
	platform_device_unregister(mini6410_snd_wm9713_device);
}

module_init(mini6410_init);
module_exit(mini6410_exit);

/* Module information */
MODULE_AUTHOR("Jaswinder Singh Brar, jassi.brar@samsung.com");
MODULE_DESCRIPTION("ALSA SoC MINI6410+WM9713");
MODULE_LICENSE("GPL");
