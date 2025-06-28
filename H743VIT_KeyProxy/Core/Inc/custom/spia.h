#ifndef SPIA_H
#define SPIA_H

#include "stm32h7xx_hal.h"

void SPIA_TxData(uint8_t *buff, uint8_t size);
void SPIA_Init();
#endif
