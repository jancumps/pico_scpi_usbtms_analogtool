#include "dab_utils.h"
#include "hardware/i2c.h"
#include "ads111x.h"

// address of first DAB found
// if none found, this will be stay 0
uint8_t ads_addr = 0;

uint8_t confreg[2]; // config register

void initDabUtils() {
    uint8_t buf[3];
    // is the ADC chip installed?
    // we search for all possible ADS111x addresses, starting from lowest
    for (int i = 0; i < 4; i++) {
        buf[0] = ADS1115_REG_CONFIG;
        i2c_write_blocking(i2c_default, ADS_START_ADDR + i, buf, 1, false);
        if (i2c_read_blocking(i2c_default, ADS_START_ADDR + i, buf, 2, false) != PICO_ERROR_GENERIC) {
            ads_addr = ADS_START_ADDR + i; // chip found
            break;
        }
    }

    if (ads_addr) {
        // set config register to desired values
        confreg[0] = 0x44; // MUX set to AIN0, and PGA set to +-2.048V and continuous conversion mode
        confreg[1] = 0x43; // rate = 32 SPS
        buf[0] = ADS1115_REG_CONFIG;
        buf[1] = confreg[0];
        buf[2] = confreg[1];
        i2c_write_blocking(i2c_default, ads_addr, buf, 3, false);
    }

}

uint32_t dabPinCount() {
    // dab (if installed supports 2 inputs)
    return ads_addr ? 2 : 0;
}

void initDabPins() {
    // nothing to do
    return;
}

uint16_t getDabPinAt(uint32_t index) {
    // TODO: implement
      return 0;
}

scpi_result_t SCPI_DabInputQ(scpi_t * context) {
  int32_t numbers[1];

  // retrieve the adc index
  SCPI_CommandNumbers(context, numbers, 1, 0);
  if (! ((numbers[0] > -1) && (numbers[0] < dabPinCount()))) {
    SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
    return SCPI_RES_ERR;
  }

  SCPI_ResultUInt16(context, getDabPinAt(numbers[0]));
  return SCPI_RES_OK;
}