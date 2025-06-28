#ifndef MESSAGE_H
#define MESSAGE_H

#include "stm32h7xx_hal.h"

typedef enum
{
  MSG_FROM_USB    = 0x00,
  MSG_FROM_SPI    = 0x01,
} Msg_SourceTypeDef;

void Msg_Init();
uint8_t Msg_Add(const uint8_t* data, Msg_SourceTypeDef source);
uint8_t Msg_IsEmpty();
uint8_t Msg_Consume(void (*cb)(uint8_t *data, Msg_SourceTypeDef source));
#endif
