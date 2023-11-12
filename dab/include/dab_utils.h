#ifndef _DAB_UTILS_H
#define _DAB_UTILS_H

#include "scpi/scpi.h"

#define INSTRUMENT_DAB_INP_COMMANDS \
    {.pattern = "ANAlog:HIres:INPut#:RAW?", .callback = SCPI_DabInputQ,},




void initDabUtils();
uint32_t dabPinCount();
void initDabPins();
uint16_t getDabPinAt(uint32_t index);

scpi_result_t SCPI_DabInputQ(scpi_t * context);

#endif // _DAB_UTILS_H