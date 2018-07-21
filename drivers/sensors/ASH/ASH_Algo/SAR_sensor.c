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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/wakelock.h>
#include <linux/input/ASH.h>

/**************************/
/* Debug and Log System */
/************************/
#define MODULE_NAME			"ASH_ALGO"
#define SENSOR_TYPE_NAME	"SAR_sensor"
#define OBJECT_NEAR			 1
#define OBJECT_FAR			 0
#undef dbg
#ifdef ASH_ALGO_DEBUG
	#define dbg(fmt, args...) printk(KERN_DEBUG "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)
#else
	#define dbg(fmt, args...)
#endif
#define log(fmt, args...) printk(KERN_INFO "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)
#define err(fmt, args...) do{	\
		printk(KERN_ERR "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args);	\
		sprintf(g_error_mesg, "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args);	\
	}while(0)

/******************************/
/* SAR Sensor Global Variables */
/*****************************/
static int ASUS_SAR_SENSOR_IRQ;
//static int ASUS_SAR_SENSOR_INT_STATUS;
static struct ASUS_sar_sensor_data			*g_sar_sensor_data;
static struct SAR_sensor_hw				*sar_sensor_hw_client;
static struct workqueue_struct				*SAR_sensor_workqueue;
static struct mutex 							g_sar_sensor_lock;
static struct mutex 							g_i2c_lock;
static struct wake_lock 						g_sar_sensor_wake_lock;
//static struct hrtimer 						g_sar_timer;
static struct i2c_client						*g_i2c_client;
static char *g_error_mesg;

/***********************/
/* SAR Sensor Functions*/
/**********************/
/*Device Layer Part*/
//static int 	proximity_turn_onoff(bool bOn);
//static int 	proximity_set_threshold(void);
//static void proximity_polling_adc(struct work_struct *work);

/*Interrupt Service Routine Part*/
static void SAR_sensor_ist(struct work_struct *work);

/*Initialization Part*/
static int init_data(void);

/*Work Queue*/
static DECLARE_WORK(SAR_sensor_ist_work, SAR_sensor_ist);

/********************/
/* SAR sensor data structure */
/********************/

struct ASUS_sar_sensor_data 
{
	bool Device_switch_on;					/* this var. means is turning on sensor or not */
	int Detect_channel;						/* this var. means is for chose  */
	int Device_Interrupt_detect_status;		/* this var. means is for member detect status */
};

/**********************************/
/* SAR sensor Info Type*/
/*********************************/
static SAR_sensor_info_type mSAR_sensor_info_type = {{0}};

/*=======================
 *|| I2c Stress Test Part ||
 *=======================
 */

#if 0

#include <linux/i2c_testcase.h>

#define I2C_TEST_Lsensor_FAIL (-1)
#define I2C_TEST_Psensor_FAIL (-1)

static int IRsensor_I2C_stress_test(struct i2c_client *client)
{
	int lnResult = I2C_TEST_PASS;	
	int ret = 0;

	i2c_log_in_test_case("TestIRSensorI2C ++\n");

	/* Check Hardware Support First */
	if(Psensor_hw_client->mpsensor_hw->proximity_hw_turn_onoff== NULL) {
		err("proximity_hw_turn_onoff NOT SUPPORT. \n");
		return -ENOENT;
	}
	if(Psensor_hw_client->mpsensor_hw->proximity_hw_get_adc == NULL) {
		err("proximity_hw_get_adc NOT SUPPORT. \n");
		return -ENOENT;
	}
	if(Psensor_hw_client->mpsensor_hw->proximity_hw_set_hi_threshold == NULL) {
		err("proximity_hw_set_hi_threshold NOT SUPPORT. \n");
		return -ENOENT;
	}
	if(Psensor_hw_client->mpsensor_hw->proximity_hw_set_lo_threshold == NULL) {
		err("proximity_hw_set_lo_threshold NOT SUPPORT. \n");
		return -ENOENT;
	}
	
	/* Turn on Proximity Sensor */
	if(!g_ps_data->Device_switch_on) {
		ret = Psensor_hw_client->mpsensor_hw->proximity_hw_turn_onoff(true);
	}

	/* Proximity i2c read test */
	ret = Psensor_hw_client->mpsensor_hw->proximity_hw_get_adc();
	if(ret < 0){
		i2c_log_in_test_case("IRsensor Proximity Fail to get adc\n");
		lnResult = I2C_TEST_Psensor_FAIL;
		return lnResult;	
	}

	/* Proximity i2c write test */
	ret = Psensor_hw_client->mpsensor_hw->proximity_hw_set_hi_threshold(g_ps_data->g_ps_calvalue_hi);
	if(ret < 0){
		i2c_log_in_test_case("IRsensor Proximity Fail to set high threshold.\n");
		lnResult = I2C_TEST_Psensor_FAIL;
		return lnResult;	
	}
	ret = Psensor_hw_client->mpsensor_hw->proximity_hw_set_lo_threshold(g_ps_data->g_ps_calvalue_lo);
	if(ret < 0){
		i2c_log_in_test_case("IRsensor Proximity Fail to set low threshold. \n");
		lnResult = I2C_TEST_Psensor_FAIL;
		return lnResult;	
	}

	if(!g_ps_data->HAL_switch_on) {
		ret = Psensor_hw_client->mpsensor_hw->proximity_hw_turn_onoff(false);
	}
	
	i2c_log_in_test_case("TestLSensorI2C --\n");
	return lnResult;
}

static struct i2c_test_case_info IRSensorTestCaseInfo[] =	{
	__I2C_STRESS_TEST_CASE_ATTR(IRsensor_I2C_stress_test),
};
#endif

/*====================
 *|| Device Layer Part ||
 *====================
 */
 #if 0
static int SAR_sensor_turn_onoff(bool bOn)
{	
	return 0;
}

static int SAR_sensor_set_threshold(void)
{
	return 0;
}
#endif

/*******************/
/*BMMI Function*/
/*******************/
bool mSAR_sensor_show_atd_test(void)
{
	int ret = 0;
	int round = 0;

	/* Check SAR sensor HW ID */
	ret = sar_sensor_hw_client->SAR_sensor_hw_check_ID();
	if (ret < 0)	{
		err("SAR sensor ATD test check ID ERROR\n");
		goto SAR_sensor_atd_test_fail;
	}

	/* Trun on SAR sensor */
	ret = sar_sensor_hw_client->SAR_sensor_hw_turn_onoff(true, g_sar_sensor_data->Detect_channel);
	if (ret < 0)	{
		err("SAR sensor ATD test turn on ERROR\n");
		goto SAR_sensor_atd_test_fail;
	}
 
	/* Read SAR sensor adc value 5 time */
	for(; round < 5; round++){
		ret = sar_sensor_hw_client->SAR_sensor_hw_get_adc();
		if (ret < 0)	{
			err("SAR sensor ATD test get adc ERROR\n");
			goto SAR_sensor_atd_test_fail;
		}
		msleep(100);
	}

	/* Trun off SAR sensor */
	ret = sar_sensor_hw_client->SAR_sensor_hw_turn_onoff(false, g_sar_sensor_data->Detect_channel);
	if(ret < 0){
		err("SAR sensor ATD test turn off ERROR\n");
		goto SAR_sensor_atd_test_fail;
	}

	return true;
SAR_sensor_atd_test_fail:
	return false;
}

int mSAR_sensor_show_adc_value(void)
{
	int ret = 0;
	/* Read SAR sensor adc value */
	ret = sar_sensor_hw_client->SAR_sensor_hw_get_adc();
	if (ret < 0)
		err("SAR sensor ATD test get adc ERROR\n");
	return ret;
}

int mSAR_sensor_show_proxavg_value(void)
{
	int ret = 0;
	/* Read SAR sensor PROXAVG value */
	ret = sar_sensor_hw_client->SAR_sensor_hw_get_proxavg_value();
	if (ret < 0)
		err("SAR sensor ATD test get adc ERROR\n");
	return ret;
}

int mSAR_sensor_show_proxdiff_value(void)
{
	int ret = 0;
	/* Read SAR sensor PROXDIFF value */
	ret = sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value();
	if (ret < 0)
		err("SAR sensor ATD test get adc ERROR\n");
	return ret;
}

/*****************************/
/* AP Interface Function*/
/*****************************/

bool mSAR_sensor_show_switch_onoff(void)
{	
	return g_sar_sensor_data->Device_switch_on;
}

int mSAR_sensor_store_switch_onoff(bool bOn)
{
	int ret = 0;
	mutex_lock(&g_sar_sensor_lock);

	if((g_sar_sensor_data->Device_switch_on != bOn))	{
		if (bOn)	{
			/* Trun on SAR sensor */
			ret = sar_sensor_hw_client->SAR_sensor_hw_turn_onoff(true, g_sar_sensor_data->Detect_channel);
			if (ret < 0)	{
				err("SAR sensor turn on ERROR\n");
			} else	{
				g_sar_sensor_data->Device_switch_on = bOn;
				err("Enable SAR Sensor irq\n");
				enable_irq(ASUS_SAR_SENSOR_IRQ);
			}
		} else	{
			/* Trun off SAR sensor */
			ret = sar_sensor_hw_client->SAR_sensor_hw_turn_onoff(false, g_sar_sensor_data->Detect_channel);
			if (ret < 0)	{
				err("SAR sensor turn off ERROR\n");
			} else	{
				g_sar_sensor_data->Device_switch_on = bOn;
				err("Disable SAR Sensor irq\n");
				disable_irq_nosync(ASUS_SAR_SENSOR_IRQ);
			}
		}
	} else	{
		log("SAR Sensor is already %s\n", bOn?"ON":"OFF");
	}

	mutex_unlock(&g_sar_sensor_lock);
	
	return 0;
}

int mSAR_sensor_show_switch_channel(void)
{	
	return g_sar_sensor_data->Detect_channel;
}

int mSAR_sensor_store_switch_channel(int channel)
{
	mutex_lock(&g_sar_sensor_lock);

	if (channel == 0 || channel == 2)
		g_sar_sensor_data->Detect_channel = channel;
	else
		log("Get wrong channel number : %d \n", channel);

	mutex_unlock(&g_sar_sensor_lock);
	
	return 0;
}

int mSAR_sensor_show_Interrupt_detect_status(void)
{
	return g_sar_sensor_data->Device_Interrupt_detect_status;
}

/*************************/
/*Hardware Function*/
/*************************/
int mSAR_sensor_show_reg(uint8_t addr)
{
	int value;
	if (sar_sensor_hw_client->SAR_sensor_hw_get_register == NULL) {
		err("SAR_sensor_hw_get_register NOT SUPPORT. \n");
		return -EINVAL;
	}

	value = sar_sensor_hw_client->SAR_sensor_hw_get_register(addr);
	log("mSAR_sensor_show_reg, addr=%02X, value=%02X.\n", addr, value);
	return value;
}

int mSAR_sensor_store_reg(uint8_t addr, int value)
{	
	if (sar_sensor_hw_client->SAR_sensor_hw_set_register == NULL) {
		err("SAR_sensor_hw_set_register NOT SUPPORT. \n");
		return -EINVAL;
	}

	sar_sensor_hw_client->SAR_sensor_hw_set_register(addr, value);
	log("mSAR_sensor_store_reg, addr=%02X, value=%02X.\n", addr, value);
	return 0;
}

int mSAR_sensor_show_proxthresh0_reg(void)
{
	int value = 0;
	if (sar_sensor_hw_client->SAR_sensor_hw_get_proxthresh0_value == NULL) {
		err("SAR_sensor_hw_get_proxthresh0_value NOT SUPPORT. \n");
		return -EINVAL;
	}

	value = sar_sensor_hw_client->SAR_sensor_hw_get_proxthresh0_value();
	log("SAR_sensor_hw_get_proxthresh0_value, value=%02X.\n", value);
	return value;
}

int mSAR_sensor_store_proxthresh0_reg(int value)
{	
	if (sar_sensor_hw_client->SAR_sensor_hw_set_proxthresh0_value == NULL) {
		err("SAR_sensor_hw_set_proxthresh0_value NOT SUPPORT. \n");
		return -EINVAL;
	}

	sar_sensor_hw_client->SAR_sensor_hw_set_proxthresh0_value(value);
	log("SAR_sensor_hw_set_proxthresh0_value, value=%02X.\n", value);
	return 0;
}

/**************************/
/* Extension Function */
/**************************/
bool mSAR_sensor_show_allreg(void)
{
	if(sar_sensor_hw_client->SAR_sensor_hw_show_allreg == NULL) {
		err("SAR_sensor_hw_show_allreg NOT SUPPORT. \n");
		return false;
	}
	sar_sensor_hw_client->SAR_sensor_hw_show_allreg();
	return true;
}

/******************  ATTR Structure ****************/
static SAR_sensor_ATTR_BMMI mSAR_sensor_ATTR_BMMI = {
	.SAR_sensor_show_atd_test				 = mSAR_sensor_show_atd_test,
	.SAR_sensor_show_adc_value				 = mSAR_sensor_show_adc_value,
	.SAR_sensor_show_proxavg_value			 = mSAR_sensor_show_proxavg_value,
	.SAR_sensor_show_proxdiff_value			 = mSAR_sensor_show_proxdiff_value,
};

static SAR_sensor_ATTR_HAL mSAR_sensor_ATTR_HAL = {
	.SAR_sensor_show_switch_onoff			 = mSAR_sensor_show_switch_onoff,
	.SAR_sensor_store_switch_onoff				 = mSAR_sensor_store_switch_onoff,
	.SAR_sensor_show_switch_channel			 = mSAR_sensor_show_switch_channel,
	.SAR_sensor_store_switch_channel			 = mSAR_sensor_store_switch_channel,
	.SAR_sensor_show_Interrupt_detect_status	 = mSAR_sensor_show_Interrupt_detect_status,
};

static SAR_sensor_ATTR_Hardware mSAR_sensor_ATTR_Hardware = {
	.SAR_show_reg							 = mSAR_sensor_show_reg,
	.SAR_store_reg							 = mSAR_sensor_store_reg,
	.SAR_show_proxthresh0_reg				 = mSAR_sensor_show_proxthresh0_reg,
	.SAR_store_proxthresh0_reg					 = mSAR_sensor_store_proxthresh0_reg,
};

static SAR_sensor_ATTR_Extension mSAR_sensor_ATTR_Extension = {
	.SAR_sensor_show_allreg					 = mSAR_sensor_show_allreg,
};

static SAR_sensor_ATTR mSAR_sensor_ATTR = {
	.info_type		 = &mSAR_sensor_info_type,
	.ATTR_BMMI		 = &mSAR_sensor_ATTR_BMMI,
	.ATTR_HAL		 = &mSAR_sensor_ATTR_HAL,
	.ATTR_Hardware	 = &mSAR_sensor_ATTR_Hardware,
	.ATTR_Extension	 = &mSAR_sensor_ATTR_Extension,
};


/*==========================
 *|| Interrupt Service Routine Part ||
 *===========================
 */
static void SAR_sensor_work(int sar_sensor_status)
{
	/* Check Interrupt status for channel 0 or channel 2 */
	if (g_sar_sensor_data->Detect_channel == 0)	{
		if ((sar_sensor_status & 0x01) == 0x00)	{
			g_sar_sensor_data->Device_Interrupt_detect_status = OBJECT_FAR;
			err("SAR sensor Detect channel 0 object Far (adc = %d, Diff = %d).\n",
				sar_sensor_hw_client->SAR_sensor_hw_get_adc(),
				sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value());
		} else if ((sar_sensor_status & 0x01) == 0x01)	{
			g_sar_sensor_data->Device_Interrupt_detect_status = OBJECT_NEAR;
			err("SAR sensor Detect channel 0 object Near (adc = %d, Diff = %d).\n",
				sar_sensor_hw_client->SAR_sensor_hw_get_adc(),
				sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value());
		} else
			err("SAR sensor Detect channel 0 object Non : Sensor Status = 0x%02X (adc = %d, Diff = %d).\n", 	sar_sensor_status,
				sar_sensor_hw_client->SAR_sensor_hw_get_adc(),
				sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value());
	} else	{
		if ((sar_sensor_status & 0x04) == 0x00)	{
			g_sar_sensor_data->Device_Interrupt_detect_status = OBJECT_FAR;
			err("SAR sensor Detect channel 2 object Far (adc = %d, Diff = %d).\n",
				sar_sensor_hw_client->SAR_sensor_hw_get_adc(),
				sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value());
		} else if ((sar_sensor_status & 0x04) == 0x04)	{
			g_sar_sensor_data->Device_Interrupt_detect_status = OBJECT_NEAR;
			err("SAR sensor Detect channel 2 object Near (adc = %d, Diff = %d).\n",
				sar_sensor_hw_client->SAR_sensor_hw_get_adc(),
				sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value());
		} else
			err("SAR sensor Detect channel 2 object Non : Sensor Status = 0x%02X (adc = %d, Diff = %d).\n", 	sar_sensor_status,
				sar_sensor_hw_client->SAR_sensor_hw_get_adc(),
				sar_sensor_hw_client->SAR_sensor_hw_get_proxdiff_value());
	}
}

static void SAR_sensor_ist(struct work_struct *work)
{
	int SAR_sensor_status = 0;
mutex_lock(&g_sar_sensor_lock);
	if (g_sar_sensor_data->Device_switch_on == false)	{
		err("SAR sensor are disabled and ignore IST.\n");
		msleep(100);
		goto ist_err;
	}

	if (sar_sensor_hw_client == NULL)	{
		dbg("sar_sensor_hw_client is NULL \n");
		msleep(100);
		goto ist_err;
	}

	/**************************************************/
	/* Check IRQ Status and refresh status    */
	/* Read INT_FLAG will clean the interrupt */
	/**************************************************/
	SAR_sensor_status = sar_sensor_hw_client->SAR_sensor_hw_get_interrupt();
	if (SAR_sensor_status < 0)	{
		err("SAR_sensor_status ERROR\n");
		msleep(100);
		goto ist_err;
	}

	SAR_sensor_work(SAR_sensor_status);
ist_err:	
	wake_unlock(&g_sar_sensor_wake_lock);
	if (g_sar_sensor_data->Device_switch_on == true )	{
		dbg("[IRQ] Enable irq !! \n");
		enable_irq(ASUS_SAR_SENSOR_IRQ);	
	}
mutex_unlock(&g_sar_sensor_lock);
}

void SAR_sensor_irq_handler(void)
{
	dbg("[IRQ] Disable irq !! \n");
	disable_irq_nosync(ASUS_SAR_SENSOR_IRQ);
	
	if(sar_sensor_hw_client->SAR_sensor_hw_get_interrupt == NULL) {
		err("SAR_sensor_hw_get_interrupt NOT SUPPORT. \n");
		goto irq_err;
	}

	/*Queue work will enbale IRQ and unlock wake_lock*/
	queue_work(SAR_sensor_workqueue, &SAR_sensor_ist_work);
	wake_lock(&g_sar_sensor_wake_lock);
	return;
irq_err:
	dbg("[IRQ] Enable irq !! \n");
	enable_irq(ASUS_SAR_SENSOR_IRQ);
}

static SAR_sensor_GPIO mSAR_sensor_GPIO = {
	.SAR_sensor_isr = SAR_sensor_irq_handler,
};

/*====================
 *|| I2C mutex lock ||
 *====================
 */
void lock_i2c_bus6(void) {
	mutex_lock(&g_i2c_lock);
}
EXPORT_SYMBOL(lock_i2c_bus6);

void unlock_i2c_bus6(void) {
	mutex_unlock(&g_i2c_lock);
}
EXPORT_SYMBOL(unlock_i2c_bus6);

/*====================
 *|| Initialization Part ||
 *====================
 */
static int init_data(void)
{
	int ret = 0;
	/* Reset ASUS_SAR_sensor_data */
	g_sar_sensor_data = kmalloc(sizeof(struct ASUS_sar_sensor_data), GFP_KERNEL);
	if (!g_sar_sensor_data) {
		err("g_sar_sensor_data kmalloc ERROR\n");
		ret = -ENOMEM;
		goto init_data_err;
	}
	memset(g_sar_sensor_data, 0, sizeof(struct ASUS_sar_sensor_data));
	g_sar_sensor_data->Device_switch_on = false;
	g_sar_sensor_data->Detect_channel = 2;
	g_sar_sensor_data->Device_Interrupt_detect_status = -EBUSY;
	return 0;
init_data_err:
	err("Init Data ERROR\n");
	return ret;
}
 
void mSAR_sensor_algo_probe(struct i2c_client *client)
{	
	log("Driver PROBE +++\n");

	/*check i2c client*/
	if (client == NULL) {
		err("i2c Client is NUll\n");
		goto probe_err;
	}	

	/*link driver data to i2c client*/
	strlcpy(client->name, SENSOR_TYPE_NAME, I2C_NAME_SIZE);
	i2c_set_clientdata(client, g_sar_sensor_data);	

	/* i2c client */
	g_i2c_client = client;
	if (ASUS_SAR_SENSOR_IRQ < 0)		
		goto probe_err;	

	/* I2c stress test */
#if 0
	i2c_add_test_case(client, "IRSensorTest", ARRAY_AND_SIZE(IRSensorTestCaseInfo));	
#endif

	log("Driver PROBE ---\n");
	return ;
probe_err:
	err("Driver PROBE ERROR ---\n");
	return;

}

void SAR_sensor_algo_remove(void)
{
	log("Driver REMOVE +++\n");
	log("Driver REMOVE ---\n");
	
	return;
}

void mSAR_sensor_algo_shutdown(void)
{
	log("Driver SHUTDOWN +++\n");

	/* Disable sensor */

	log("Driver SHUTDOWN ---\n");
	
	return;
}

void mSAR_sensor_algo_suspend(void)
{
	log("Driver SUSPEND +++\n");
	log("Driver SUSPEND ---\n");
	
	return;
}

void mSAR_sensor_algo_resume(void)
{
	log("Driver RESUME +++\n");
	log("Driver RESUME ---\n");
	
	return;
}

static SAR_sensor_I2C mSAR_sensor_I2C = {
	.SAR_sensor_probe = mSAR_sensor_algo_probe,
	.SAR_sensor_remove = SAR_sensor_algo_remove,
	.SAR_sensor_shutdown = mSAR_sensor_algo_shutdown,
	.SAR_sensor_suspend = mSAR_sensor_algo_suspend,
	.SAR_sensor_resume = mSAR_sensor_algo_resume,
};

static int __init SAR_sensor_init(void)
{
	int ret = 0;
	log("Driver INIT +++\n");

	/*Record the error message*/
	g_error_mesg = kzalloc(sizeof(char [ERROR_MESG_SIZE]), GFP_KERNEL);
	
	/* Work Queue */
	SAR_sensor_workqueue = create_singlethread_workqueue(SENSOR_TYPE_NAME"_wq");
	//SAR_sensor_delay_workqueue = create_singlethread_workqueue(SENSOR_TYPE_NAME"_delay_wq");	

	/* Initialize the Mutex */
	mutex_init(&g_sar_sensor_lock);
	mutex_init(&g_i2c_lock);

	/* Initialize the wake lock */
	wake_lock_init(&g_sar_sensor_wake_lock, WAKE_LOCK_SUSPEND, "SAR_sensor_wake_lock");
#if 0
	/*Initialize high resolution timer*/
	hrtimer_init(&g_ir_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	g_ir_timer.function = proximity_timer_function;
#endif
	/* i2c Registration for probe/suspend/resume */				
	ret = SAR_sensor_i2c_register(&mSAR_sensor_I2C);
	if (ret < 0)
		goto init_err;
	
	/* Hardware Register Initialization */
	sar_sensor_hw_client = SAR_sensor_hw_getHardware();
	if(sar_sensor_hw_client == NULL)
		goto init_err;

	/* driver data structure initialize */
	ret = init_data();
	if (ret < 0)
		goto init_err;

	/* string copy the character of vendor and module number */
	strcpy(mSAR_sensor_ATTR.info_type->vendor, sar_sensor_hw_client->vendor);
	strcpy(mSAR_sensor_ATTR.info_type->module_number, sar_sensor_hw_client->module_number);

	/* Attribute */
	ret = SAR_sensor_ATTR_register(&mSAR_sensor_ATTR);
	if (ret < 0)
		goto init_err;

#if 0	
	/* Input Device */
	ret = SAR_sensor_report_register();
	if (ret < 0)
		goto init_err;	
#endif
	ASUS_SAR_SENSOR_IRQ = SAR_sensor_gpio_register(g_i2c_client, &mSAR_sensor_GPIO);
	if (ASUS_SAR_SENSOR_IRQ < 0)
		goto init_err;	

	log("Driver INIT ---\n");
	return 0;

init_err:
	err("Driver INIT ERROR ---\n");
	return ret;
}

static void __exit SAR_sensor_exit(void)
{
	log("Driver EXIT +++\n");

	/* i2c Unregistration */	
	SAR_sensor_i2c_unregister();
#if 0
	sar_sensor_report_unregister();
	sar_sensor_ATTR_unregister();	
	
	wake_lock_destroy(&g_ir_wake_lock);
#endif
	mutex_destroy(&g_sar_sensor_lock);
	mutex_destroy(&g_i2c_lock);
	kfree(g_sar_sensor_data);
	
	log("Driver EXIT ---\n");
}

module_init(SAR_sensor_init);
module_exit(SAR_sensor_exit);

MODULE_AUTHOR("Peter <Peter_Lu@asus.com>");
MODULE_DESCRIPTION("SAR sensor Sensor");
MODULE_LICENSE("GPL");

