/* 
 * Copyright (C) 2014 ASUSTek Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*******************************************************/
/* ALSPS Front RGB Sensor CM36656 Module */
/******************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/input/ASH.h>
#include "sx9310.h"

/**************************/
/* Debug and Log System */
/************************/
#define MODULE_NAME			"ASH_HW"
#define SENSOR_TYPE_NAME		"SAR_sensor"

#undef dbg
#ifdef ASH_HW_DEBUG
	#define dbg(fmt, args...) printk(KERN_DEBUG "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)
#else
	#define dbg(fmt, args...)
#endif
#define log(fmt, args...) printk(KERN_INFO "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)
#define err(fmt, args...) printk(KERN_ERR "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)

/*****************************************/
/*Global static variable and function*/
/****************************************/
static struct i2c_client	*g_i2c_client = NULL;

static int sx9310_SAR_sensor_hw_check_ID(void);
static int sx9310_SAR_sensor_hw_init(struct i2c_client* client);
static int sx9310_SAR_sensor_hw_show_allreg(void);
static int sx9310_SAR_sensor_hw_get_interrupt(void);
static int sx9310_SAR_sensor_hw_set_register(uint8_t reg, int value);
static int sx9310_SAR_sensor_hw_get_register(uint8_t reg);

static int sx9310_SAR_sensor_hw_check_ID(void)
{
	int ret = 0;
	uint8_t data_buf[2] = {0, 0};

	/* Check the Device ID */
	ret = i2c_read_reg_u16(g_i2c_client, SX9310_WHO_AM_I_REG, data_buf);
	if ((ret < 0) || (data_buf[0] != 0x01)) {
		err("%s ERROR(SX9310_WHO_AM_I_REG : 0x%02X%02X). \n", __FUNCTION__, data_buf[1], data_buf[0]);
		return -ENOMEM;
	}
	log("%s Success(SX9310_WHO_AM_I_REG : 0x%02X%02X). \n", __FUNCTION__, data_buf[1], data_buf[0]);
	return 0;
}

static int sx9310_SAR_sensor_hw_check_HW_address(void)
{
	int ret = 0;
	uint8_t data_buf[2] = {0, 0};

	/* Check the Device HW address */
	ret = i2c_read_reg_u16(g_i2c_client, SX9310_I2C_ADDR_REG, data_buf);
	if ((ret < 0) || (data_buf[0] != 0x28)) {
		err("%s ERROR(SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__, data_buf[1], data_buf[0]);
		return -ENOMEM;
	}
	log("%s Success(SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__, data_buf[1], data_buf[0]);
	return 0;
}

static int sx9310_SAR_sensor_hw_init(struct i2c_client* client)
{
	int ret = 0;
	int i = 0;
	g_i2c_client = client;

	//sx9310_SAR_sensor_hw_show_allreg();

	/* Reset Interrupt status by SX9310_IRQSTAT_REG register */
	ret = i2c_read_reg_u8(g_i2c_client, SX9310_IRQSTAT_REG);
	if (ret < 0) 
	{
		err("%s ERROR Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__, SX9310_IRQSTAT_REG , ret);
		return ret;
	}
	log("%s Success Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__, SX9310_IRQSTAT_REG , ret);

	/* Check the Device ID */
	ret = sx9310_SAR_sensor_hw_check_ID();

	/* Check the HW address */
	ret = sx9310_SAR_sensor_hw_check_HW_address();

	for (i = 0; i < SX9310_SETTING_REGISTER_MAX; i++)	{
		ret = i2c_write_reg_u8(g_i2c_client, sx9310_i2c_reg_setup[i].reg, sx9310_i2c_reg_setup[i].val);
		if (ret < 0)	{
			err("%s ERROR Write (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
				sx9310_i2c_reg_setup[i].reg, sx9310_i2c_reg_setup[i].val);
			return ret;
		}
		log("%s Success Write (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
				sx9310_i2c_reg_setup[i].reg, sx9310_i2c_reg_setup[i].val);
	}
	return 0;
}

static int sx9310_SAR_sensor_hw_show_allreg(void)
{
	int i = 0;
	uint8_t data_buf = 0;

	/* Check the Device ID */
	sx9310_SAR_sensor_hw_check_ID();

	/* Check the HW address */
	sx9310_SAR_sensor_hw_check_HW_address();

	for (i = 0; i < SX9310_SETTING_REGISTER_MAX; i++)	{
		data_buf = i2c_read_reg_u8(g_i2c_client, sx9310_i2c_reg_setup[i].reg );
		if (data_buf < 0)	{
			err("%s ERROR Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
				sx9310_i2c_reg_setup[i].reg, data_buf );
			return data_buf;
		}
		log("%s Success Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
				sx9310_i2c_reg_setup[i].reg, data_buf);
	}

	for (i = 0; i < SX9310_DATA_READBACK_REGISTER_MAX; i++)	{
		data_buf = i2c_read_reg_u8(g_i2c_client, sx9310_i2c_data_readback_reg[i].reg );
		if (data_buf < 0)	{
			err("%s ERROR Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
				sx9310_i2c_data_readback_reg[i].reg, data_buf );
			return data_buf;
		}
		log("%s Success Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
				sx9310_i2c_data_readback_reg[i].reg, data_buf);
	}
	return 0;
}

static int sx9310_SAR_sensor_hw_get_interrupt(void)
{
	uint8_t data_buf = 0, sensor_status = 0;

	/* Read detect status */
	sensor_status = i2c_read_reg_u8( g_i2c_client, SX9310_STAT0_REG );

	if (sensor_status < 0)	{
		err("%s ERROR Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
			SX9310_STAT0_REG, sensor_status );
		return sensor_status;
	}
	log("%s Success Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
		SX9310_STAT0_REG, sensor_status);

	/* Clean interrupt */
	data_buf = i2c_read_reg_u8( g_i2c_client, SX9310_IRQSTAT_REG );

	if (data_buf < 0)	{
		err("%s ERROR Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
			SX9310_IRQSTAT_REG, data_buf );
		return data_buf;
	}
	log("%s Success Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__,
		SX9310_IRQSTAT_REG, data_buf);

	return sensor_status;
}

static int sx9310_SAR_sensor_hw_set_register(uint8_t reg, int value)
{
	int ret = 0;
	uint8_t buf[2] = {0};

	buf[1] = value/256;
	buf[0] = value%256;
	
	ret = i2c_write_reg_u16(g_i2c_client, reg, buf);
	log("Set Register Value (0x%X) = 0x%02X%02X\n", reg, buf[1], buf[0]);
	if(ret < 0) 
	{
		err("Set Register Value ERROR. (REG:0x%X)\n", reg);
		return ret;
	}

	return 0;
}

static int sx9310_SAR_sensor_hw_get_register(uint8_t reg)
{
	int ret = 0;	
	uint8_t buf[2] = {0};
	int value = 0;
	
	ret = i2c_read_reg_u16(g_i2c_client, reg, buf);
	log("Get Register Value (0x%X) = 0x%02X%02X\n", reg, buf[1], buf[0]);
	if(ret < 0) 
	{
		err("ALSPS FRGB Get Register Value ERROR. (REG:0x%X)\n", reg);
		return ret;
	}

	value =  buf[1]*256 + buf[0];

	return value;
}

static int sx9310_SAR_sensor_hw_turn_onoff(bool bOn, int channel)
{
	int ret = 0;
	uint8_t data_buf = 0;

	log("sx9310 use channel %d \n", channel);

	if (channel == 0)	{
		/* Use channel 0 */
		/* Reset Interrupt status by SX9310_IRQSTAT_REG register */
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_IRQSTAT_REG, 0x00);
		log("Set Register Value (0x%X) = 0x%02X\n", SX9310_IRQSTAT_REG, 0x00);
		if (ret < 0) 
		{
			err("Set Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_IRQSTAT_REG, ret);
			return ret;
		}

		/* Write channel 0 Detect bit to the SX9310_CPS_CTRL5_REG register */
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_CPS_CTRL5_REG, 0xC1);
		log("Set channel 0 Register Value (0x%X) = 0x%02X\n", SX9310_CPS_CTRL5_REG, 0xC1);
		if(ret < 0) 
		{
			err("Set channel 0 Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPS_CTRL5_REG, ret);
			return ret;
		}

		/* Write channel 0 data readback to the SX9310_CPS_CTRL5_REG register */
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_CPSRD, 0x00);
		log("Set channel 0 Register Value (0x%X) = 0x%02X\n", SX9310_CPSRD, 0x00);
		if(ret < 0) 
		{
			err("Set channel 0 Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPSRD, ret);
			return ret;
		}

		/* Read the SX9310_CPS_CTRL0_REG value */
		data_buf = i2c_read_reg_u8(g_i2c_client, SX9310_CPS_CTRL0_REG);
		if (data_buf < 0) {
			err("Read Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPS_CTRL0_REG, data_buf);
			return ret;
		}
		log("%s Success Read (SX9310_CPS_CTRL0_REG : 0x%02X). \n", __FUNCTION__, data_buf);

		/* Write channel 0 enable bit to the SX9310_CPS_CTRL0_REG register */
		if (bOn)
			data_buf = data_buf |0x01;
		else
			data_buf = data_buf & 0xFE;
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_CPS_CTRL0_REG, data_buf);
		log("Set channel 0 Register Value (0x%X) = 0x%02X\n", SX9310_CPS_CTRL0_REG, data_buf);
		if(ret < 0) 
		{
			err("Set channel 0 Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPS_CTRL0_REG, ret);
			return ret;
		}
	} else	{
		/* Default Use channel 2 */
		/* Reset Interrupt status by SX9310_IRQSTAT_REG register */
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_IRQSTAT_REG, 0x00);
		log("Set Register Value (0x%X) = 0x%02X\n", SX9310_IRQSTAT_REG, 0x00);
		if (ret < 0) 
		{
			err("Set Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_IRQSTAT_REG, ret);
			return ret;
		}

		/* Write channel 2 Detect bit to the SX9310_CPS_CTRL5_REG register */
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_CPS_CTRL5_REG, 0xC9);
		log("Set channel 0 Register Value (0x%X) = 0x%02X\n", SX9310_CPS_CTRL5_REG, 0xC9);
		if(ret < 0) 
		{
			err("Set channel 0 Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPS_CTRL5_REG, ret);
			return ret;
		}

		/* Write channel 0 data readback to the SX9310_CPS_CTRL5_REG register */
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_CPSRD, 0x02);
		log("Set channel 0 Register Value (0x%X) = 0x%02X\n", SX9310_CPSRD, 0x02);
		if(ret < 0) 
		{
			err("Set channel 0 Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPSRD, ret);
			return ret;
		}

		/* Read the SX9310_CPS_CTRL0_REG value */
		data_buf = i2c_read_reg_u8(g_i2c_client, SX9310_CPS_CTRL0_REG);
		if (data_buf < 0) {
			err("Read Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPS_CTRL0_REG, data_buf);
			return ret;
		}
		log("%s Success Read (SX9310_CPS_CTRL0_REG : 0x%02X). \n", __FUNCTION__, data_buf);

		/* Write channel 2 enable bit to the SX9310_CPS_CTRL0_REG register */
		if (bOn)
			data_buf = data_buf |0x04;
		else
			data_buf = data_buf & 0xFB;
		ret = i2c_write_reg_u8(g_i2c_client, SX9310_CPS_CTRL0_REG, data_buf);
		log("Set channel 2 Register Value (0x%X) = 0x%02X\n", SX9310_CPS_CTRL0_REG, data_buf);
		if(ret < 0) 
		{
			err("Set channel 2 Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_CPS_CTRL0_REG, ret);
			return ret;
		}
	}

	return 0;
}

static int sx9310_SAR_sensor_hw_get_adc(void)
{
	int ret = 0;
	uint8_t data_buf[2] = {0, 0};
	int value;

	/* Check the Device HW address */
	ret = i2c_read_reg_u16(g_i2c_client, SX9310_USEMSB, data_buf);
	if (ret < 0) {
		err("%s ERROR(SX9310_ADC_Value : 0x%02X, %02X). \n", __FUNCTION__, data_buf[0], data_buf[1]);
		return ret;
	}
	value =  data_buf[0]*256 + data_buf[1];
	
	log("%s Success(SX9310_ADC_Value  = %d [0x%02X, %02X]). \n", __FUNCTION__, value, data_buf[0], data_buf[1]);

	return value;
}

static int sx9310_SAR_sensor_hw_get_proxavg_value(void)
{
	int ret = 0;
	uint8_t data_buf[2] = {0, 0};
	int value;

	/* Check the Device HW address */
	ret = i2c_read_reg_u16(g_i2c_client, SX9310_AVGMSB, data_buf);
	if (ret < 0) {
		err("%s ERROR(SX9310_PROXAVG_Value : 0x%02X, %02X). \n", __FUNCTION__, data_buf[0], data_buf[1]);
		return ret;
	}
	value =  data_buf[0]*256 + data_buf[1];
	
	log("%s Success(SX9310_PROXAVG_Value  = %d [0x%02X, %02X]). \n", __FUNCTION__, value, data_buf[0], data_buf[1]);

	return value;
}

static int sx9310_SAR_sensor_hw_get_proxdiff_value(void)
{
	int ret = 0;
	uint8_t data_buf[2] = {0, 0};
	int value;

	/* Check the Device HW address */
	ret = i2c_read_reg_u16(g_i2c_client, SX9310_DIFFMSB, data_buf);
	if (ret < 0) {
		err("%s ERROR(SX9310_PROXDIFF_Value : 0x%02X, %02X). \n", __FUNCTION__, data_buf[0], data_buf[1]);
		return ret;
	}
	value =  data_buf[0]*256 + data_buf[1];
	
	log("%s Success(SX9310_PROXDIFF_Value  = %d [0x%02X, %02X]). \n", __FUNCTION__, value, data_buf[0], data_buf[1]);

	return value;
}

static int sx9310_SAR_sensor_hw_get_proxthresh0_value(void)
{
	int data_buf = 0;
	data_buf = i2c_read_reg_u8( g_i2c_client, SX9310_CPS_CTRL9_REG );

	if (data_buf < 0)	{
		err("%s ERROR Read (SX9310_I2C_ADDR_REG : 0x%02X, error = %d). \n", __FUNCTION__,	SX9310_CPS_CTRL9_REG, data_buf );
		return data_buf;
	}
	log("%s Success Read (SX9310_I2C_ADDR_REG : 0x%02X = %02X). \n", __FUNCTION__, SX9310_CPS_CTRL9_REG, data_buf);

	return data_buf;
}

static int sx9310_SAR_sensor_hw_set_proxthresh0_value(int value)
{
	int ret = 0;
	ret = i2c_write_reg_u8(g_i2c_client, SX9310_IRQSTAT_REG, value);
	log("Set Register Value (0x%X) = 0x%02X\n", SX9310_IRQSTAT_REG, value);
	if (ret < 0) 
	{
		err("Set Register Value ERROR. (REG:0x%X, error = %d)\n", SX9310_IRQSTAT_REG, ret);
		return ret;
	}
	return 0;
}

static struct SAR_sensor_hw SAR_sensor_hw_sx9310 = {	
	.vendor = "Semtech",
	.module_number = "sx9310",

	.SAR_sensor_hw_check_ID				 = sx9310_SAR_sensor_hw_check_ID,
	.SAR_sensor_hw_init					 = sx9310_SAR_sensor_hw_init,
	.SAR_sensor_hw_get_interrupt			 = sx9310_SAR_sensor_hw_get_interrupt,
	.SAR_sensor_hw_show_allreg			 = sx9310_SAR_sensor_hw_show_allreg,
	.SAR_sensor_hw_set_register			 = sx9310_SAR_sensor_hw_set_register,
	.SAR_sensor_hw_get_register			 = sx9310_SAR_sensor_hw_get_register,
	.SAR_sensor_hw_turn_onoff			 = sx9310_SAR_sensor_hw_turn_onoff,
	.SAR_sensor_hw_get_adc				 = sx9310_SAR_sensor_hw_get_adc,
	.SAR_sensor_hw_get_proxavg_value		 = sx9310_SAR_sensor_hw_get_proxavg_value,
	.SAR_sensor_hw_get_proxdiff_value		 = sx9310_SAR_sensor_hw_get_proxdiff_value,
	.SAR_sensor_hw_get_proxthresh0_value	 = sx9310_SAR_sensor_hw_get_proxthresh0_value,
	.SAR_sensor_hw_set_proxthresh0_value	 = sx9310_SAR_sensor_hw_set_proxthresh0_value,
};

SAR_sensor_hw* SAR_sensor_hw_sx9310_getHardware(void)
{
	SAR_sensor_hw* SAR_sensor_hw_client = NULL;
	SAR_sensor_hw_client = &SAR_sensor_hw_sx9310;
	return SAR_sensor_hw_client;
}

