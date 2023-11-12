#include "dab_utils.h"
#include "hardware/i2c.h"
#include "ads111x.h"

// address of first DAB found
// if none found, this will be stay 0
uint8_t ads_addr = 0;
double adc_range_value[8] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
uint32_t conv_time_ms[8] = {125, 63, 32, 16, 8, 4, 3, 2}; // conversion time in us


uint8_t adc_confreg[2]; // config register
uint8_t adc_data_rate = DR_128;
uint8_t adc_gain_bits[2] = {GAIN_2_048, GAIN_2_048}; 
double adc_range[2] = {2.048, 2.048};
double opamp_gain[2] = {0.38298, 0.38298};

void build_data_rate(uint8_t dr); // sets up the local config for data rate (e.g. DR_128)
void build_gain(uint8_t chan, uint8_t gain); // sets up the local config for gain (e.g. GAIN_2_048)

// adc_set_mux: programs the config and sets the channel (e.g. AIN1) and also starts the conversion
// if in continuous mode. Note: first result after changing mux in continuous mode will be invalid
// If do_single_conversion is 1, then the ADC is
// set to single-shot mode, and the conversion is started.
int adc_set_mux(uint8_t chan, uint8_t do_single_conversion);

int16_t adc_raw_diff_result(); // returns the raw differential result (16-bit signed integer)
double to_volts(uint8_t chan, int16_t raw); // converts the raw result to volts


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
/*         // set config register to desired values
        adc_confreg[0] = 0x44; // MUX set to AIN0, and PGA set to +-2.048V and continuous conversion mode
        adc_confreg[1] = 0x43; // rate = 32 SPS
        buf[0] = ADS1115_REG_CONFIG;
        buf[1] = adc_confreg[0];
        buf[2] = adc_confreg[1];
        i2c_write_blocking(i2c_default, ads_addr, buf, 3, false); */

        build_data_rate(DR_128);
        build_gain(AIN1, GAIN_2_048); // +- 2.048V
        build_gain(AIN2, GAIN_2_048); // +- 2.048V

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

    adc_set_mux(index, DO_SINGLE_CONVERSION);
    // wait for conversion to complete on both boards
    sleep_ms(conv_time_ms[adc_data_rate]);
    sleep_ms(5); // wait a little longer for margin TODO: check
    return adc_raw_diff_result();
}

scpi_result_t SCPI_DabInputRawQ(scpi_t * context) {
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

scpi_result_t SCPI_DabInputQ(scpi_t * context) {
  int32_t numbers[1];

  // retrieve the adc index
  SCPI_CommandNumbers(context, numbers, 1, 0);
  if (! ((numbers[0] > -1) && (numbers[0] < dabPinCount()))) {
    SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
    return SCPI_RES_ERR;
  }

  SCPI_ResultDouble(context, to_volts(numbers[0], getDabPinAt(numbers[0])));
  return SCPI_RES_OK;
}

void build_data_rate(uint8_t dr) {
    if (dr > 7) {
        return;
    }
    adc_data_rate = dr;
    adc_confreg[1] &= ~0xE0; // clear the DR bits
    adc_confreg[1] |= (dr<<5); // set the DR bits
}
void build_gain(uint8_t chan, uint8_t gain) {
    if (gain > 5) {
        return;
    }
    adc_range[chan] = adc_range_value[gain];
    adc_gain_bits[chan] = gain;
    adc_confreg[0] &= ~0x0E; // clear the PGA bits
    adc_confreg[0] |= (gain<<1); // set the PGA bits
}

// selects either the first or second channel on the ADC board
// this ends up programming the entire config register
// returns 1 if successful, 0 otherwise.
// if do_single_conversion is 1 then a single conversion will be immediately started.
int adc_set_mux(uint8_t chan, uint8_t do_single_conversion) {
    uint8_t mux;
    uint8_t buf[3];
    switch(chan) {
        case AIN1:
            mux = ADS1115_DIFF_0_1;
            break;
        case AIN2:
            mux = ADS1115_DIFF_2_3;
            break;
        default:
            break;
    }
    adc_confreg[0] &= ~0x70; // clear the MUX bits
    adc_confreg[0] |= (mux<<4); // set the MUX bits
    buf[0] = ADS1115_REG_CONFIG;
    buf[1] = adc_confreg[0];
    buf[2] = adc_confreg[1];
    if (do_single_conversion) {
        buf[1] |= 0x01; // set MODE bit to single-shot mode
        buf[1] |= 0x80; // set START bit to start conversion
    }
    i2c_write_blocking(i2c_default, ads_addr, buf, 3, false);
    return(1);
}


// read the conversion register
// result is 16-bit signed integer since the input can be negative or positive
int16_t adc_raw_diff_result() {
    uint16_t meas;
    uint16_t buf;
    uint8_t* buf8_ptr = (uint8_t*)&buf;
    *buf8_ptr = ADS1115_REG_CONVERSION;
    i2c_write_blocking(i2c_default, ads_addr, buf8_ptr, 1, false);
    i2c_read_blocking(i2c_default, ads_addr, buf8_ptr, 2, false);
    meas = buf>>8 | buf<<8; // swap bytes
    return(meas);
}

double to_volts(uint8_t chan, int16_t raw) {
    double v;
    v = (double)raw * (adc_range[chan] / 32768.0); // convert the raw value to volts present at the ADC input
    v = v / opamp_gain[chan]; // adjust for opamp gain
    return(v);
}
