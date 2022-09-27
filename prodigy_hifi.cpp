#include "prodigy_hifi.h"
#include "misc.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) \
(sizeof(array) / sizeof(*array))
#endif

/* Analog Recording Source :- Mic, LineIn, CD/Video, */

/* implement capture source select control for WM8776 */

#define WM_AIN1 "AIN1"
#define WM_AIN2 "AIN2"
#define WM_AIN3 "AIN3"
#define WM_AIN4 "AIN4"
#define WM_AIN5 "AIN5"


/*
 * Prodigy HD2
 */
#define AK4396_ADDR    0x00
#define AK4396_CSN    (1 << 8)    /* CSN->GPIO8, pin 75 */
#define AK4396_CCLK   (1 << 9)    /* CCLK->GPIO9, pin 76 */
#define AK4396_CDTI   (1 << 10)   /* CDTI->GPIO10, pin 77 */

/* ak4396 registers */
#define AK4396_CTRL1	    0x00
#define AK4396_CTRL2	    0x01
#define AK4396_CTRL3	    0x02
#define AK4396_LCH_ATT	  0x03
#define AK4396_RCH_ATT	  0x04

/*
 * write data in the SPI mode
 */

static void SetGPIOBits(struct CardData *card, unsigned int bit, int val)
{
	unsigned int tmp = GetGPIOData(card->pci_dev, card->iobase);
	if (val)
		tmp |= bit;
	else
		tmp &= ~bit;
	SetGPIOData(card->pci_dev, card->iobase, tmp);
}

/*
 * serial interface for ak4396 - only writing supported, no readback
 */

static void ak4396_write_word(struct CardData *card, unsigned int data)
{
	int i;
	for (i = 0; i < 16; i++) {
		SetGPIOBits(card, AK4396_CCLK, 0);
		MicroDelay(1);
		SetGPIOBits(card, AK4396_CDTI, data & 0x8000);
		MicroDelay(1);
		SetGPIOBits(card, AK4396_CCLK, 1);
		MicroDelay(1);
		data <<= 1;
	}
}

static void ak4396_write(struct CardData *card, unsigned int reg,
			 unsigned int data)
{
	unsigned int block;

    SaveGPIO(card->pci_dev, card);
	SetGPIODir(card->pci_dev, card, AK4396_CSN|AK4396_CCLK|AK4396_CDTI);
	SetGPIOMask(card->pci_dev, card->iobase, ~(AK4396_CSN|AK4396_CCLK|AK4396_CDTI));
	/* latch must be low when writing */
	SetGPIOBits(card, AK4396_CSN, 0); 
	block =  ((AK4396_ADDR & 0x03) << 14) | (1 << 13) |
			((reg & 0x1f) << 8) | (data & 0xff);
	ak4396_write_word(card, block); /* REGISTER ADDRESS */
	/* release latch */
	SetGPIOBits(card, AK4396_CSN, 1);
	MicroDelay(1);
	/* restore */
    RestoreGPIO(card->pci_dev, card);
}


/*
 * initialize the chip
 */

void ProdigyHD2_Init(struct CardData *card)
{
	static unsigned short ak4396_inits[] = {
		AK4396_CTRL1,	   0x87,   // I2S Normal Mode, 24 bit
		AK4396_CTRL2,	   0x02,
		AK4396_CTRL3,	   0x00, 
		AK4396_LCH_ATT,	 0xFF,
		AK4396_RCH_ATT,	 0xFF,
	};
    int i;

	//initialize ak4396 codec
	// reset codec
	ak4396_write(card, AK4396_CTRL1, 0x86);
	MicroDelay(100);
	ak4396_write(card, AK4396_CTRL1, 0x87);
			
	for (i = 0; i < 10; i += 2)
		ak4396_write(card, ak4396_inits[i], ak4396_inits[i+1]);
}

/*
 * SPI implementation for WM8766 codec - only writing supported, no readback
 */

static void wm8766_spi_send_word(struct CardData *card, unsigned int data)
{
    int i;
    for (i = 0; i < 16; i++) {
        SetGPIOBits(card, WM8766_SPI_CLK, 0);
        MicroDelay(1);
        SetGPIOBits(card, WM8766_SPI_MD, data & 0x8000);
        MicroDelay(1);
        SetGPIOBits(card, WM8766_SPI_CLK, 1);
        MicroDelay(1);
        data <<= 1;
    }
}

static void wm8766_spi_write(struct CardData *card, unsigned int reg,
                             unsigned int data)
{
    unsigned int block;
    
    SaveGPIO(card->pci_dev, card);
    SetGPIODir(card->pci_dev, card, WM8766_SPI_MD|WM8766_SPI_CLK|WM8766_SPI_ML);
    SetGPIOMask(card->pci_dev, card->iobase, ~(WM8766_SPI_MD|WM8766_SPI_CLK|WM8766_SPI_ML));
    
    SetGPIOBits(card, WM8766_SPI_ML, 0);
    block = (reg << 9) | (data & 0x1ff);
    wm8766_spi_send_word(card, block);
    
    SetGPIOBits(card, WM8766_SPI_ML, 1);
    MicroDelay(1);
    
    RestoreGPIO(card->pci_dev, card);
}

static void wm_put_nocache(struct CardData *card, int reg, unsigned short val)
{
    unsigned short cval;
    cval = (reg << 9) | val;
    //snd_vt1724_write_i2c(ice, WM_DEV, cval >> 8, cval & 0xff);
    WriteI2C(card->pci_dev, card, WM_DEV, cval >> 8, cval & 0xff);
}

static void wm_put(struct CardData *card, int reg, unsigned short val)
{
    wm_put_nocache(card, reg, val);
    reg <<= 1;
    //card->ProdigyHiFiCodec
    //ice->akm[0].images[reg] = val >> 8;
    //ice->akm[0].images[reg + 1] = val;
    
    //TODO: Implement codec caching stuff
}

static void wm_set_vol(struct CardData *card, unsigned int index,
                       unsigned short vol, unsigned short master)
{
    unsigned char nvol;
    
    if ((master & WM_VOL_MUTE) || (vol & WM_VOL_MUTE))
        nvol = 0;
    else {
        nvol = (((vol & ~WM_VOL_MUTE) * (master & ~WM_VOL_MUTE)) / 128)
        & WM_VOL_MAX;
        nvol = (nvol ? (nvol + DAC_MIN) : 0) & 0xff;
    }
    
    wm_put(card, index, nvol);
    wm_put_nocache(card, index, 0x100 | nvol);
}

static void wm8766_set_vol(struct CardData *card, unsigned int index,
                           unsigned short vol, unsigned short master)
{
    unsigned char nvol;
    
    if ((master & WM_VOL_MUTE) || (vol & WM_VOL_MUTE))
        nvol = 0;
    else {
        nvol = (((vol & ~WM_VOL_MUTE) * (master & ~WM_VOL_MUTE)) / 128)
        & WM_VOL_MAX;
        nvol = (nvol ? (nvol + DAC_MIN) : 0) & 0xff;
    }
    
    wm8766_spi_write(card, index, (0x0100 | nvol));
}

static void wm8766_init(struct CardData *card)
{
    static const unsigned short wm8766_inits[] = {
        WM8766_RESET,       0x0000,
        WM8766_DAC_CTRL,    0x0120,
        WM8766_INT_CTRL,    0x0022, /* I2S Normal Mode, 24 bit */
        WM8766_DAC_CTRL2,       0x0001,
        WM8766_DAC_CTRL3,       0x0080,
        WM8766_LDA1,        0x0100,
        WM8766_LDA2,        0x0100,
        WM8766_LDA3,        0x0100,
        WM8766_RDA1,        0x0100,
        WM8766_RDA2,        0x0100,
        WM8766_RDA3,        0x0100,
        WM8766_MUTE1,       0x0000,
        WM8766_MUTE2,       0x0000,
    };
    unsigned int i;
    
    for (i = 0; i < ARRAY_SIZE(wm8766_inits); i += 2)
        wm8766_spi_write(card, wm8766_inits[i], wm8766_inits[i + 1]);
}

static void wm8776_init(struct CardData *card)
{
    static const unsigned short wm8776_inits[] = {
        /* These come first to reduce init pop noise */
        WM_ADC_MUX,    0x0003,    /* ADC mute */
        /* 0x00c0 replaced by 0x0003 */
        
        WM_DAC_MUTE,    0x0001,    /* DAC softmute */
        WM_DAC_CTRL1,    0x0000,    /* DAC mute */
        
        WM_POWERDOWN,    0x0008,    /* All power-up except HP */
        WM_RESET,    0x0000,    /* reset */
    };
    unsigned int i;
    
    for (i = 0; i < ARRAY_SIZE(wm8776_inits); i += 2)
        wm_put(card, wm8776_inits[i], wm8776_inits[i + 1]);
}

static int prodigy_hifi_resume(struct CardData *card)
{
    static const unsigned short wm8776_reinit_registers[] = {
        WM_MASTER_CTRL,
        WM_DAC_INT,
        WM_ADC_INT,
        WM_OUT_MUX,
        WM_HP_ATTEN_L,
        WM_HP_ATTEN_R,
        WM_PHASE_SWAP,
        WM_DAC_CTRL2,
        WM_ADC_ATTEN_L,
        WM_ADC_ATTEN_R,
        WM_ALC_CTRL1,
        WM_ALC_CTRL2,
        WM_ALC_CTRL3,
        WM_NOISE_GATE,
        WM_ADC_MUX,
        /* no DAC attenuation here */
    };
    //struct prodigy_hifi_spec *spec = ice->spec;
    
    
    //mutex_lock(&ice->gpio_mutex);
    
    /* reinitialize WM8776 and re-apply old register values */
    //wm8776_init(card);
    
    //schedule_timeout_uninterruptible(1);
    /*
     int i, ch;
     
    for (i = 0; i < ARRAY_SIZE(wm8776_reinit_registers); i++)
        wm_put(card, wm8776_reinit_registers[i],
               wm_get(card, wm8776_reinit_registers[i]));
    */
    /* reinitialize WM8766 and re-apply volumes for all DACs */
    
    wm8766_init(card);
    
    /*
    for (ch = 0; ch < 2; ch++) {
        wm_set_vol(card, WM_DAC_ATTEN_L + ch,
                   spec->vol[2 + ch], spec->master[ch]);
        
        wm8766_set_vol(card, WM8766_LDA1 + ch,
                       spec->vol[0 + ch], spec->master[ch]);
        
        wm8766_set_vol(card, WM8766_LDA2 + ch,
                       spec->vol[4 + ch], spec->master[ch]);
        
        wm8766_set_vol(card, WM8766_LDA3 + ch,
                       spec->vol[6 + ch], spec->master[ch]);
    }*/
    
    //TODO: Volume restore for both codecs
    
    /* unmute WM8776 DAC */
    wm_put(card, WM_DAC_MUTE, 0x00);
    wm_put(card, WM_DAC_CTRL1, 0x90);
    
    //mutex_unlock(&ice->gpio_mutex);
    return 0;
}

int prodigy_hifi_init(struct CardData *card)
{
    
    static const unsigned short wm8776_defaults[] = {
        WM_MASTER_CTRL,  0x0022, /* 256fs, slave mode */
        WM_DAC_INT,    0x0022,    /* I2S, normal polarity, 24bit */
        WM_ADC_INT,    0x0022,    /* I2S, normal polarity, 24bit */
        WM_DAC_CTRL1,    0x0090,    /* DAC L/R */
        WM_OUT_MUX,    0x0001,    /* OUT DAC */
        WM_HP_ATTEN_L,    0x0179,    /* HP 0dB */
        WM_HP_ATTEN_R,    0x0179,    /* HP 0dB */
        WM_DAC_ATTEN_L,    0x0000,    /* DAC 0dB */
        WM_DAC_ATTEN_L,    0x0100,    /* DAC 0dB */
        WM_DAC_ATTEN_R,    0x0000,    /* DAC 0dB */
        WM_DAC_ATTEN_R,    0x0100,    /* DAC 0dB */
        WM_PHASE_SWAP,    0x0000,    /* phase normal */
#if 0
        WM_DAC_MASTER,    0x0100,    /* DAC master muted */
#endif
        WM_DAC_CTRL2,    0x0000,    /* no deemphasis, no ZFLG */
        WM_ADC_ATTEN_L,    0x0000,    /* ADC muted */
        WM_ADC_ATTEN_R,    0x0000,    /* ADC muted */
#if 1
        WM_ALC_CTRL1,    0x007b,    /* */
        WM_ALC_CTRL2,    0x0000,    /* */
        WM_ALC_CTRL3,    0x0000,    /* */
        WM_NOISE_GATE,    0x0000,    /* */
#endif
        WM_DAC_MUTE,    0x0000,    /* DAC unmute */
        WM_ADC_MUX,    0x0003,    /* ADC unmute, both CD/Line On */
    };
    
    //unsigned int i;
    
    //TODO: init codec cache here
    
    /* initialize WM8776 codec */
    //wm8776_init(card);
    //schedule_timeout_uninterruptible(1);
    //for (i = 0; i < ARRAY_SIZE(wm8776_defaults); i += 2)
    //    wm_put(card, wm8776_defaults[i], wm8776_defaults[i + 1]);
    
    wm8766_init(card);
    
    return 0;
}
