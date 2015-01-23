/*
 * ASoC driver for the Sondbox (with a WM8737)
 * connected to a Raspberry Pi
 *
 * Author:  Maric Michaud, <maric@lesondier.com>
 *          Copyright 2014
 * Inspired by the same for wm8731
 *          Florian Meier, <koalo@koalo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>

static int sysclk = 12288000;
module_param_named(sysclk, sysclk , int, 0444);
MODULE_PARM_DESC(sysclk, "Input clock to wm8737");

static const unsigned int wm8737_rates_12288000[] = {
    8000, 12000, 16000, 24000, 32000, 48000, 96000,
};

static struct snd_pcm_hw_constraint_list wm8737_constraints_12288000 = {
    .list = wm8737_rates_12288000,
    .count = ARRAY_SIZE(wm8737_rates_12288000),
};

static const unsigned int wm8737_rates_11289600[] = {
    8000, 11025, 22050, 44100, 88200,
};

static struct snd_pcm_hw_constraint_list wm8737_constraints_11289600 = {
    .list = wm8737_rates_11289600,
    .count = ARRAY_SIZE(wm8737_rates_11289600),
};

static const unsigned int wm8737_rates_18432000[] = {
    8000, 12000, 16000, 24000, 32000, 48000, 96000,
};

static struct snd_pcm_hw_constraint_list wm8737_constraints_18432000 = {
    .list = wm8737_rates_18432000,
    .count = ARRAY_SIZE(wm8737_rates_18432000),
};

static const unsigned int wm8737_rates_16934400[] = {
    8000, 11025, 22050, 44100, 88200,
};

static struct snd_pcm_hw_constraint_list wm8737_constraints_16934400 = {
    .list = wm8737_rates_16934400,
    .count = ARRAY_SIZE(wm8737_rates_16934400),
};

static const unsigned int wm8737_rates_12000000[] = {
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 88200, 96000,
};

static struct snd_pcm_hw_constraint_list wm8737_constraints_12000000 = {
    .list = wm8737_rates_12000000,
    .count = ARRAY_SIZE(wm8737_rates_12000000),
};

static int sondbox_startup(struct snd_pcm_substream *substream)
{
    snd_pcm_hw_constraint_list(substream->runtime, 0,
                SNDRV_PCM_HW_PARAM_RATE,
                &wm8737_constraints_12288000);

    // start the bcm gpc


    printk("Sondbox started !\n");
    return 0;
}

static int sondbox_hw_params(struct snd_pcm_substream *substream,
                       struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    unsigned int sample_bits = snd_pcm_format_physical_width(
                                                  params_format(params));

    /* Set proto bclk */
    int ret = snd_soc_dai_set_bclk_ratio(cpu_dai, 64);
    if (ret < 0){
        dev_err(substream->pcm->dev, "Failed to set BCLK ratio %d\n", ret);
        return ret;
    }

    /* Set proto sysclk */
    ret = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, SND_SOC_CLOCK_OUT);
    if (ret < 0) {
        dev_err(substream->pcm->dev,
                "Failed to set WM8737 SYSCLK: %d\n", ret);
        return ret;
    }

    return 0;
}

/* machine stream operations */
static struct snd_soc_ops sondbox_ops = {
    .startup = sondbox_startup,
    .hw_params = sondbox_hw_params,
};

static struct snd_soc_dai_link sondbox_dai[] = {
{
    .name = "WM8737",
    .stream_name = "Capture",
    .cpu_dai_name = "bcm2708-i2s.0",
    .codec_dai_name = "wm8737",
    .platform_name = "bcm2708-i2s.0",
    .codec_name = "wm8737.1-001a",
    .dai_fmt = SND_SOC_DAIFMT_I2S
             | SND_SOC_DAIFMT_NB_NF
             | SND_SOC_DAIFMT_CBM_CFM,
    .ops = &sondbox_ops,
},
};

/* audio machine driver */
static struct snd_soc_card sondbox = {
    .name = "sondbox",
    .dai_link = sondbox_dai,
    .num_links = ARRAY_SIZE(sondbox_dai),
};

static int sondbox_probe(struct platform_device *pdev) {
    int ret = 0;

    sondbox.dev = &pdev->dev;
    ret = snd_soc_register_card(&sondbox);
    if (ret)
        dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);

    return ret;
}


static int sondbox_remove(struct platform_device *pdev)
{
    return snd_soc_unregister_card(&sondbox);
}

static struct platform_driver sondbox_driver = {
    .driver = {
        .name = "snd-sondbox-adc",
        .owner = THIS_MODULE,
    },
    .probe = sondbox_probe,
    .remove = sondbox_remove,
};

module_platform_driver(sondbox_driver);

MODULE_ALIAS("platform:snd-sondbox-adc");
MODULE_AUTHOR("Maric Michaud <maric@lesondier.com>");
MODULE_DESCRIPTION("ASoC Driver for Raspberry Pi connected to WM8737");
MODULE_LICENSE("GPL");
