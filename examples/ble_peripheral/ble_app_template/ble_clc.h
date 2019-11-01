#ifndef __BLE_CLC_H__
#define __BLE_CLC_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#define BLE_CLC_DEF(_name)                         \
static ble_clc_t _name;                            \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                \
                     BLE_BPS_BLE_OBSERVER_PRIO,    \
                     ble_clc_on_ble_evt, &_name)

/* nRFgo Studio UUID : A49Bxxxx-2B67-4666-B8A9-491A583C4EE0 */
#define CLC_UUID_BASE        {0xE0, 0x4E, 0x3C, 0x58, 0x1A, 0x49, 0xA9, 0xB8, \
                              0x66, 0x46, 0x67, 0x2B, 0x00, 0x00, 0x9B, 0xA4}
#define CLC_UUID_SERVICE   			0x0001
#define CLC_UUID_LOAD_CELL_CHAR		0x0002


typedef struct ble_clc_s ble_clc_t;

struct ble_clc_s{
	uint16_t service_handle;
	ble_gatts_char_handles_t load_cell_char_handles;
	uint8_t uuid_type;
};

void ble_clc_on_ble_evt(ble_evt_t const * p_clc_evt, void * p_context);
uint32_t ble_clc_init(ble_clc_t * p_clc);
uint32_t ble_clc_mass_send(uint16_t conn_handle, ble_clc_t *p_clc, uint32_t mass);

#endif /* __BLE_CLC_H__ */
