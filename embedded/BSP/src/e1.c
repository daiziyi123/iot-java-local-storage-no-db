#include "e1.h"

i2c_slave_info e1_led_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char E1_PCA9685_ADDR[] = {0xC0, 0xC2, 0xC4, 0xC6};
#else
const static unsigned char E1_PCA9685_ADDR[] = {0x60, 0x61, 0x62, 0x63};
#endif

static void e1_pca9685_init(i2c_slave_info info)
{
	i2c_reg_byte_write(info, 0x00, 0x00);
}

i2c_slave_info e1_led_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(E1_PCA9685_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], E1_PCA9685_ADDR[j]);
			if(info.flag)
			{
				e1_pca9685_init(info);
				return info;
			}
		}
	}
	return info;
}

static void e1_pca9685_pwm_set(i2c_slave_info info, unsigned char num, unsigned short on, unsigned short off)
{
	i2c_reg_byte_write(info, 0x06+4*num, on);
	i2c_reg_byte_write(info, 0x07+4*num, on>>8);
	i2c_reg_byte_write(info, 0x08+4*num, off);
	i2c_reg_byte_write(info, 0x09+4*num, off>>8);
}

void e1_led_rgb_set(i2c_slave_info info, unsigned char red, unsigned char green, unsigned char blue)
{	
	unsigned short on, off;

	on = 0x0f;
	off = on + red*0x10;
	e1_pca9685_pwm_set(info, 1, on, off);

	on = 0x0f;
	off = on + green*0x10;
	e1_pca9685_pwm_set(info, 0, on, off);

	on = 0x0f;
	off = on + blue*0x10;
	e1_pca9685_pwm_set(info, 2, on, off);
}


i2c_slave_info e1_tube_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char E1_HT16K33_ADDR[] = {0xE0, 0xE2, 0xE4, 0xE6};
#else
const static unsigned char E1_HT16K33_ADDR[] = {0x70, 0x71, 0x72, 0x73};
#endif

const static unsigned char chr_code[][2] =
{
	{0xF8, 0x01}, //0
	{0x30, 0x00}, //1
	{0xD8, 0x02}, //2
	{0x78, 0x02}, //3
	{0x30, 0x03}, //4
	{0x68, 0x03}, //5
	{0xE8, 0x03}, //6
	{0x38, 0x00}, //7
	{0xF8, 0x03}, //8
	{0x78, 0x03}, //9
	{0xB8, 0x03}, //A
	{0xE0, 0x03}, //B
	{0xC8, 0x01}, //C
	{0xF0, 0x02}, //D
	{0xC8, 0x03}, //E
	{0x88, 0x03}, //F
	{0xE8, 0x01}, //G
	{0xB0, 0x03}, //H
	{0x20, 0x00}, //I
	{0x70, 0x00}, //J
	{0xD0, 0x03}, //K
	{0xC0, 0x01}, //L
	{0xA8, 0x02}, //M
	{0xA0, 0x02}, //N
	{0xE0, 0x02}, //O
	{0x98, 0x03}, //P
	{0x38, 0x03}, //Q
	{0x80, 0x02}, //R
	{0x20, 0x03}, //S
	{0xC0, 0x03}, //T
	{0xF0, 0x01}, //U
	{0x10, 0x03}, //V
	{0x50, 0x03}, //W
	{0xB0, 0x01}, //X
	{0x70, 0x03}, //Y
	{0x48, 0x02}, //Z
	{0x00, 0x02}, //-
	{0x00, 0x00}, //NULL
};

static void e1_ht16k33_init(i2c_slave_info info)
{
	i2c_byte_write(info, 0x21);
	i2c_reg_byte_write(info, 0x02, 0x00);
	i2c_reg_byte_write(info, 0x03, 0x00);
	i2c_reg_byte_write(info, 0x04, 0x00);
	i2c_reg_byte_write(info, 0x05, 0x00);
	i2c_reg_byte_write(info, 0x06, 0x00);
	i2c_reg_byte_write(info, 0x07, 0x00);
	i2c_reg_byte_write(info, 0x08, 0x00);
	i2c_reg_byte_write(info, 0x09, 0x00);
	i2c_byte_write(info, 0x81);
}

i2c_slave_info e1_tube_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(E1_HT16K33_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], E1_HT16K33_ADDR[j]);
			if(info.flag)
			{
				e1_ht16k33_init(info);
				return info;
			}
		}
	}
	return info;
}

static void e1_ht16k33_chr_set(i2c_slave_info info, unsigned char bit, unsigned char chr, unsigned char point)
{
	unsigned char temp[2] = {chr_code[chr][0], chr_code[chr][1]};

	if(point)
	{
		temp[1] += 0x04;
	}
	
	switch(bit)
	{
		case  1:
			i2c_reg_byte_write(info, 0x02, temp[0]);
			i2c_reg_byte_write(info, 0x03, temp[1]);
			break;

		case  2:
			i2c_reg_byte_write(info, 0x04, temp[0]);
			i2c_reg_byte_write(info, 0x05, temp[1]);
			break;

		case 3:
			i2c_reg_byte_write(info, 0x06, temp[0]);
			i2c_reg_byte_write(info, 0x07, temp[1]);
			break;

		case 4:
			i2c_reg_byte_write(info, 0x08, temp[0]);
			i2c_reg_byte_write(info, 0x09, temp[1]);
			break;

		default:
			break;
	}
	i2c_byte_write(info, 0x81);
}

void e1_tube_str_set(i2c_slave_info info, char * str)
{
	char * pstr = str + strlen((char *)str) - 1;

	for(int i=4; i>=1; i--)
	{
		if(pstr >= str)
		{
			if(*pstr >= '0' && *pstr <= '9')
			{
				if(*(pstr+1) == '.')
				{
					e1_ht16k33_chr_set(info, i, *pstr-48, 1);
				}
				else
				{
					e1_ht16k33_chr_set(info, i, *pstr-48, 0);
				}
			}
			else if(*pstr >= 'a' && *pstr <= 'z')
			{
				e1_ht16k33_chr_set(info, i, *pstr-97+10, 0);
			}
			else if(*pstr >= 'A' && *pstr <= 'Z')
			{
				e1_ht16k33_chr_set(info, i, *pstr-65+10, 0);
			}
			else if(*pstr == '-')
			{
				e1_ht16k33_chr_set(info, i, 10+26+0, 0);
			}
			else
			{
				i ++;
			}
			pstr --;
		}
		else
		{
			e1_ht16k33_chr_set(info, i, 10+26+1, 0);
		}
	}
}
