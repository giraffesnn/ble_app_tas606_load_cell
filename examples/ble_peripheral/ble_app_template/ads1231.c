/**
 * @brief       ads1231.c
 * @details     24-Bit Analog-to-Digital Converter for Bridge Sensors.
 *              Functions file.
 *
 *
 * @return      NA
 *
 * @author      giraffesnn
 * @date        22/October/2019
 * @version     22/October/2019    The ORIGIN
 * @pre         NaN.
 * @warning     NaN
 * @Copyright   This code is referenced from AqueronteBlog(https://github.com/AqueronteBlog/ARM.git)
 */

#include "ads1231.h"
#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define ADS1231_PIN_HIGH        	0x01 
#define ADS1231_PIN_LOW		    	0x00

#define T_SF						2  /* Security Factor = 2 */

/* Wake-up time after Power-Down mode */
#define T_WAKEUP_TYP_US			8	 /* TYP : 7.95 */ 	 
#define T_WAKEUP_CFG_US		   	(T_SF * T_WAKEUP_TYP_US)

/* PDWN pulse width */
#define T_PDWM_MIN_US			26	/* MIN : 26us */
#define T_PDWM_CFG_US		   	(T_SF * T_PDWM_MIN_US)

/* SCLK high after DRDY/DOUT goes low to activate Standby mode */
#define T_DSS_SPEED_1_MAX_MS		13	  /* SPEED = 1, MAX : 12.44ms */
#define T_DSS_SPEED_0_MAX_MS		100   /* SPEED = 0, MAX : 99.94ms */
#define T_DSS_SPEED_1_CFG_MS		(T_SF * T_DSS_SPEED_1_MAX_MS)
#define T_DSS_SPEED_0_CFG_MS		(T_DSS_SPEED_0_MAX_MS + 10)

/* Standby mode activation time */
#define T_STANDBY_SPEED_1_MIN_MS	13    /* SPEED = 1, MIN : 12.5ms */
#define T_STANDBY_SPEED_0_MIN_MS	100   /* SPEED = 0, MIN : 100ms */
#define T_STANDBY_SPEED_1_CFG_MS	(T_SF * T_STANDBY_SPEED_1_MIN_MS)
#define T_STANDBY_SPEED_0_CFG_MS	(T_SF * T_STANDBY_SPEED_0_MIN_MS)

/* Data ready after exiting Standby mode */
#define T_S_RDY_SPEED_1_TYP_MS		53	  /* SPEED = 1, MIN : 52.6ms */
#define T_S_RDY_SPEED_0_TYP_MS		402	  /* SPEED = 0, MIN : 401.8ms */
#define T_S_RDY_SPEED_1_CFG_MS		(T_SF * T_S_RDY_SPEED_1_TYP_MS)
#define T_S_RDY_SPEED_0_CFG_MS		(T_S_RDY_SPEED_0_TYP_MS + 100)
#define TIMEOUT_TICKS_FAST_MODE		(APP_TIMER_TICKS(T_S_RDY_SPEED_1_CFG_MS))
#define TIMEOUT_TICKS_SLOW_MODE		(APP_TIMER_TICKS(T_S_RDY_SPEED_0_CFG_MS))

/* Conversion time (1/data rate) */
#define T_CONV_SPEED_1_TYP_MS		13	  /* SPEED = 1, TYP : 12.5ms */
#define T_CONV_SPEED_0_TYP_MS		100	  /* SPEED = 0, TYP : 100ms */
#define T_CONV_SPEED_1_CFG_MS		(T_SF * T_CONV_SPEED_1_TYP_MS)	  
#define T_CONV_SPEED_0_CFG_MS		(T_SF * T_CONV_SPEED_0_TYP_MS)	

#define CFG_AVGS					NUM_OF_POINTS_AVG_1

/* Mass is calculated from ADC code using the formula: 
 * w = m * c + wzs -wt
 * w = mass
 * c = the ADC code
 * wt = tare weight
 * m = wfs / (Cfs - Czs)
 * wzs = -m * czs
 *
 * calibration process:
 * user-specified calibration mass : 100g
 * Cfs = 19600, the ADC code taken with the calibration mass applied
 * Czs = 12500, the ADC measurement taken with no load.
 * wt = 0
 * m = 100g / (19600 - 12500) = (1 / 71) g
 * w = m * c - m * czs = m * (c - czs) = (1 / 71) * (c - 12500)g
 */
#define CAL_CONST_M	 			71
#define ZERO_SCALE_ADC_CODE 	12500

/* Waiting for data ready after starting the conversion */
static int wait_data_ready(struct ads1231_desc *ads1231_desc)
{ 
	uint32_t timeout_ticks = 0;
		
	if(DATA_RARE_FAST_SPEED_MODE)
		timeout_ticks = TIMEOUT_TICKS_FAST_MODE;
	else
		timeout_ticks = TIMEOUT_TICKS_SLOW_MODE;

	while((nrf_gpio_pin_read(ads1231_desc->data) == ADS1231_PIN_HIGH)
		&&(--timeout_ticks));

	if(timeout_ticks < 1)
		return -1;

	return timeout_ticks;
}

/* SCLK high after DRDY/DOUT goes low to activate Standby mode. */
static void wait_sclk_to_high(void)
{
	if(DATA_RARE_FAST_SPEED_MODE)
		nrf_delay_ms(T_DSS_SPEED_1_CFG_MS);
	else
		nrf_delay_ms(T_DSS_SPEED_0_CFG_MS);
}

/* Standby mode activation time. */
static void wait_standby_mode_activate(void)
{
	if(DATA_RARE_FAST_SPEED_MODE)
		nrf_delay_ms(T_STANDBY_SPEED_1_CFG_MS);
	else
		nrf_delay_ms(T_STANDBY_SPEED_0_CFG_MS);
}

/* POWER-DOWN MODE:
 * It power down the entire converter and resets the ADC.
 * It reduces the total power consumption close to zero.
 */
int ads1231_power_down_mode_enter(struct ads1231_desc *ads1231_desc)
{
	/* To enter Power-Down mode, simply hold the PDWN pin low. */
	nrf_gpio_pin_clear(ads1231_desc->pdwn);
	nrf_delay_us(T_PDWM_CFG_US);

	if(nrf_gpio_pin_read(ads1231_desc->pdwn) == ADS1231_PIN_LOW)
		return 0;
	else
		return -1;
}

int ads1231_power_down_mode_exit(struct ads1231_desc *ads1231_desc)
{
	nrf_gpio_pin_set(ads1231_desc->pdwn);
	nrf_delay_us(T_WAKEUP_CFG_US);
	
	if(nrf_gpio_pin_read(ads1231_desc->pdwn) == ADS1231_PIN_HIGH)
		return 0;
	else
		return -1;	
}

int ads1231_reset(struct ads1231_desc *ads1231_desc)
{
	nrf_gpio_pin_clear(ads1231_desc->pdwn);
	nrf_delay_us(T_PDWM_CFG_US);
	if(nrf_gpio_pin_read(ads1231_desc->pdwn) != ADS1231_PIN_LOW)
		return -1;

	ads1231_power_down_mode_exit(ads1231_desc);
	
	if(DATA_RARE_FAST_SPEED_MODE)
		nrf_delay_ms(T_S_RDY_SPEED_1_CFG_MS);
	else
		nrf_delay_ms(T_S_RDY_SPEED_0_CFG_MS);
}

/* STANDBY MODE:
 * It dramatically reduces power consumption by shutting down most of the circuitry.
 */
int ads1231_standby_mode_enter(struct ads1231_desc *ads1231_desc)
{
	/* To enter Standby mode, simply hold SCLK high after DRDY/DOUT goes low. */
	while(nrf_gpio_pin_read(ads1231_desc->data) == ADS1231_PIN_HIGH)
	wait_sclk_to_high();
	nrf_gpio_pin_set(ads1231_desc->sclk);
	wait_standby_mode_activate();

	if(nrf_gpio_pin_read(ads1231_desc->sclk) == ADS1231_PIN_HIGH)
		return 0;
	else
		return -1;	
}

int ads1231_standby_mode_exit(struct ads1231_desc *ads1231_desc)
{
	/* To exit Standby mode(wake up), set SCLK low. */
	nrf_gpio_pin_clear(ads1231_desc->sclk);
	nrf_delay_us(T_WAKEUP_CFG_US);

	if(nrf_gpio_pin_read(ads1231_desc->sclk) == ADS1231_PIN_LOW)
		return 0;
	else
		return -1;	
}

/* Configure the pins of nrf to control the ads1231. */
void ads1231_init(struct ads1231_desc *ads1231_desc)
{
	/* nrf INPUT: ~DRDY/DOUT PIN */
	nrf_gpio_cfg_input(ads1231_desc->data, NRF_GPIO_PIN_NOPULL);
	
	/* nrf OUTPUT: SCLK PIN, ~PDWN PIN, ~SPEED PIN */
	nrf_gpio_cfg_output(ads1231_desc->sclk);
	nrf_gpio_cfg_output(ads1231_desc->pdwn);
	nrf_gpio_cfg_output(ads1231_desc->speed);

	/* Configure the data rate */
	if(DATA_RARE_FAST_SPEED_MODE)
		nrf_gpio_pin_set(ads1231_desc->speed);
	else
		nrf_gpio_pin_clear(ads1231_desc->speed);
}

/* ADS1131REF and ADS1231REF User's Guide(sbau175a.pdf)
 * 
 * 1.7.2  Measuring Mass
 * The following items are required to measure mass:
 * A load cell and An object of known mass within the load cell range.
 * 
 * NOTE: Save the calibration settings to flash memory to 
 * avoid performing calibration on each power-up.
 * 
 * 
 */

int ads1231_get_adc_code(struct ads1231_desc *ads1231_desc, uint32_t *adc_code)
{
	unsigned char i, j; 
	uint32_t data;
	uint32_t sum = 0;
	int ret;
	
	for(i = 0;i < CFG_AVGS; i++)
	{
		data = 0;

		if(DATA_RARE_FAST_SPEED_MODE)	
			nrf_delay_ms(T_CONV_SPEED_1_CFG_MS);
		else
			nrf_delay_ms(T_CONV_SPEED_0_CFG_MS);
		
		for(j = 0; j < 24; j++)
		{	
			/* SCLK positive or negative pulse width: MIN. 100ns */
			nrf_gpio_pin_set(ads1231_desc->sclk);
			data <<= 1;
			/* SCLK positive or negative pulse width: MIN. 100ns */
			nrf_gpio_pin_clear(ads1231_desc->sclk);
		
			if(nrf_gpio_pin_read(ads1231_desc->data) == ADS1231_PIN_HIGH)
				data++;
		}	
		
		/* The 25th SCLK can be applied to force DRDY/DOUT high for data retrieval */
		nrf_gpio_pin_set(ads1231_desc->sclk);
		nrf_gpio_pin_clear(ads1231_desc->sclk);

		sum += data;
	}

	*adc_code = sum / CFG_AVGS;

	NRF_LOG_DEBUG("ads1231 adc_code = %d", *adc_code);

	return 0;
}

int ads1231_calculate_mass(struct ads1231_desc *ads1231_desc, uint32_t *mass)
{
      uint32_t w;
      uint32_t adc_code;

      ads1231_get_adc_code(ads1231_desc, &adc_code);

      if(adc_code >= ZERO_SCALE_ADC_CODE)
          w = (adc_code - ZERO_SCALE_ADC_CODE) / CAL_CONST_M;
      else
          w = 0;

      *mass = w;

      NRF_LOG_DEBUG("ads1231 mass = %d", *mass);

      return 0;
}