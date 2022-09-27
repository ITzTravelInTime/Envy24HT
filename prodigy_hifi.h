#ifndef __SOUND_PRODIGY_HIFI_H
#define __SOUND_PRODIGY_HIFI_H

/* I2C addresses */
#define WM_DEV        0x34

/* WM8776 registers */
#define WM_HP_ATTEN_L        0x00    /* headphone left attenuation */
#define WM_HP_ATTEN_R        0x01    /* headphone left attenuation */
#define WM_HP_MASTER        0x02    /* headphone master (both channels), override LLR */
#define WM_DAC_ATTEN_L        0x03    /* digital left attenuation */
#define WM_DAC_ATTEN_R        0x04
#define WM_DAC_MASTER        0x05
#define WM_PHASE_SWAP        0x06    /* DAC phase swap */
#define WM_DAC_CTRL1        0x07
#define WM_DAC_MUTE        0x08
#define WM_DAC_CTRL2        0x09
#define WM_DAC_INT        0x0a
#define WM_ADC_INT        0x0b
#define WM_MASTER_CTRL        0x0c
#define WM_POWERDOWN        0x0d
#define WM_ADC_ATTEN_L        0x0e
#define WM_ADC_ATTEN_R        0x0f
#define WM_ALC_CTRL1        0x10
#define WM_ALC_CTRL2        0x11
#define WM_ALC_CTRL3        0x12
#define WM_NOISE_GATE        0x13
#define WM_LIMITER        0x14
#define WM_ADC_MUX        0x15
#define WM_OUT_MUX        0x16
#define WM_RESET        0x17

/* GPIO pins of envy24ht connected to wm8766 */
#define WM8766_SPI_CLK     (1<<17) /* CLK, Pin97 on ICE1724 */
#define WM8766_SPI_MD      (1<<16) /* DATA VT1724 -> WM8766, Pin96 */
#define WM8766_SPI_ML      (1<<18) /* Latch, Pin98 */

/* WM8766 registers */
#define WM8766_DAC_CTRL     0x02   /* DAC Control */
#define WM8766_INT_CTRL     0x03   /* Interface Control */
#define WM8766_DAC_CTRL2    0x09
#define WM8766_DAC_CTRL3    0x0a
#define WM8766_RESET        0x1f
#define WM8766_LDA1         0x00
#define WM8766_LDA2         0x04
#define WM8766_LDA3         0x06
#define WM8766_RDA1         0x01
#define WM8766_RDA2         0x05
#define WM8766_RDA3         0x07
#define WM8766_MUTE1        0x0C
#define WM8766_MUTE2        0x0F

#define WM_VOL_MAX    255
#define WM_VOL_MUTE    0x8000

#define DAC_0dB    0xff
#define DAC_RES    128
#define DAC_MIN    (DAC_0dB - DAC_RES)

#define VT1724_SUBDEVICE_PRODIGY_HIFI	0x38315441	/* PRODIGY 7.1 HIFI */
#define VT1724_SUBDEVICE_PRODIGY_HD2	0x37315441	/* PRODIGY HD2 */
#define VT1724_SUBDEVICE_FORTISSIMO4	0x81160100	/* Fortissimo IV */

#include "DriverData.h"
#include "misc.h"

void ProdigyHD2_Init(struct CardData *card);
int prodigy_hifi_init(struct CardData *card);

#endif /* __SOUND_PRODIGY_HIFI_H */
