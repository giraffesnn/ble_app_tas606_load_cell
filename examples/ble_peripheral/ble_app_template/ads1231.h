#ifndef __ADS1231_H__
#define __ADS1231_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

typedef enum{
	DATA_RARE_SLOW_SPEED_MODE  = 0,	/* SPEED PIN = 0, 10SPS, better noise Performance */
	DATA_RARE_FAST_SPEED_MODE  = 1,	/* SPEED PIN = 1, 80SPS */
}ADS1231_DATA_RATE;

/* Number of points to use when reading weight.
 * The choices available are 2, 4, 8, 10, 16, 32,
 * 50, 64, 100, and 128. The default is 50.
 */
typedef enum{
	NUM_OF_POINTS_AVG_1 = 1,
	NUM_OF_POINTS_AVG_2 = 2,
	NUM_OF_POINTS_AVG_4 = 4,
	NUM_OF_POINTS_AVG_8 = 8,
	NUM_OF_POINTS_AVG_10 = 10,
	NUM_OF_POINTS_AVG_16 = 16,
	NUM_OF_POINTS_AVG_32 = 32,
	NUM_OF_POINTS_AVG_50 = 50,
	NUM_OF_POINTS_AVG_64 = 64,
	NUM_OF_POINTS_AVG_100 = 100,
	NUM_OF_POINTS_AVG_128 = 128,
}ADS1231_ADC_CODE_AVERAGES;

struct ads1231_desc{
	uint32_t data;					/* ~DRDY/DOUT : OUTPUT*/
	uint32_t sclk;					/* SCLK  : INPUT  */
	uint32_t pdwn;					/* ~PDWN : INPUT  */
	uint32_t speed;					/* SPEED : INPUT  */
	ADS1231_DATA_RATE data_rate;	/* SPEED = 0(10SPS), SPEED = 1(80SPS) */
};

void ads1231_init(struct ads1231_desc *ads1231_desc);
int ads1231_power_down_mode_enter(struct ads1231_desc *ads1231_desc);
int ads1231_power_down_mode_exit(struct ads1231_desc *ads1231_desc);
int ads1231_reset(struct ads1231_desc *ads1231_desc);
int ads1231_standby_mode_enter(struct ads1231_desc *ads1231_desc);
int ads1231_standby_mode_exit(struct ads1231_desc *ads1231_desc);
int ads1231_get_adc_code(struct ads1231_desc *ads1231_desc, uint32_t *adc_code);
int ads1231_calculate_mass(struct ads1231_desc *ads1231_desc, uint32_t *mass);

#endif /* __ADS1231_H__ */
