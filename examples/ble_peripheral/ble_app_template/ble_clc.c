/* Compression load cell */

#include "sdk_common.h"
#include "ble_clc.h"
#include "ble_srv_common.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static void on_write(ble_clc_t * p_clc, ble_evt_t const * p_ble_evt)
{
	
}

void ble_clc_on_ble_evt(ble_evt_t const * p_clc_evt, void * p_context)
{
    ble_clc_t * p_clc = (ble_clc_t *)p_context;

    switch (p_clc_evt->header.evt_id)
    {
        case BLE_GATTS_EVT_WRITE:
            on_write(p_clc, p_clc_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_clc_init(ble_clc_t * p_clc)
{
	uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;
	uint32_t			  initial_mass;

    // Initialize service structure.
	
    // Add service.
    ble_uuid128_t base_uuid = {CLC_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_clc->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_clc->uuid_type;
    ble_uuid.uuid = CLC_UUID_SERVICE;
	
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_clc->service_handle);
    VERIFY_SUCCESS(err_code);

    //Add load cell characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = CLC_UUID_LOAD_CELL_CHAR;
    add_char_params.uuid_type         = p_clc->uuid_type;
    add_char_params.init_len          = sizeof(uint32_t);
	add_char_params.p_init_value	  = 0;
    add_char_params.max_len           = sizeof(uint32_t);
    add_char_params.char_props.read   = 1;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_clc->service_handle,
                                  &add_char_params,
                                  &p_clc->load_cell_char_handles);	
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t ble_clc_mass_send(uint16_t conn_handle, ble_clc_t *p_clc, uint32_t mass)
{
	ble_gatts_hvx_params_t params;
	uint16_t len = sizeof(mass);

	memset(&params, 0, sizeof(params));
	params.type   = BLE_GATT_HVX_NOTIFICATION;
	params.handle = p_clc->load_cell_char_handles.value_handle;
	params.p_data = &mass;
	params.p_len  = &len;

	return sd_ble_gatts_hvx(conn_handle, &params);
}

