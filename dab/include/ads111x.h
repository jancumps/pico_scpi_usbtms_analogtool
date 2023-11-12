#ifndef _ADS111X_UTILS_H
#define _ADS111X_UTILS_H

// ADS1115 definitions
#define ADS_START_ADDR 0x48

// ADS1115 registers (4)
#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG 0x01
#define ADS1115_REG_LO_THRESH 0x02
#define ADS1115_REG_HI_THRESH 0x03

// ADS1115 mux values
#define ADS1115_CH0 0x04
#define ADS1115_CH1 0x05
#define ADS1115_CH2 0x06
#define ADS1115_CH3 0x07
#define ADS1115_DIFF_0_1 0x00
#define ADS1115_DIFF_2_3 0x03
#define DR_8 0
#define DR_16 1
#define DR_32 2
#define DR_64 3
#define DR_128 4
#define DR_250 5
#define DR_475 6
#define DR_860 7
#define GAIN_6_144 0
#define GAIN_4_096 1
#define GAIN_2_048 2
#define GAIN_1_024 3
#define GAIN_0_512 4
#define GAIN_0_256 5
#define AIN1 0
#define AIN2 1
#define BOARD0 0
#define BOARD1 1
#define DO_SINGLE_CONVERSION 1

#endif // _ADS111X_UTILS_H