#include "stm32h7xx_hal.h"
#include "inner_main.h"
#include "spia.h"
#include "master.h"
#include "slave.h"
#include "usbh_def.h"
#include "usbh_hid_mouse.h"
#include "message.h"

uint8_t IsMaster = 0;
uint8_t LoadHALUID() {
/*
 * U1 Unique ID: 0051003E-3233510D-31373737
 * U2 Unique ID: 004D002F-3233510D-31373737
 */
#define MASTER_UID (0x0051003EU)
#define SLAVE_UID (0x004D002F)
	if (HAL_GetUIDw0() == MASTER_UID) {
		IsMaster = 1;
	} else {
		IsMaster = 0;
	}
}

typedef void(*MainPtr) ();
MainPtr MainFunc;


void EXTI15_10_IRQHandler(void) {
	uint16_t GPIO_Pin = GPIO_PIN_10;
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00U) {
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
		printf("exit 10 trigger!\n");
		Error_Handler();
	}
}

void myMain() {
	HAL_StatusTypeDef status;
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	Msg_Init();

	LoadHALUID();

	if (IsMaster) {
		printf("I am Master!\n");
		MainFunc = MasterMain;

		// 配置PA10, 设置为 GPIO_MODE_IT_FALLING 接收从机触发的中断
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* EXTI interrupt init*/
		HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	} else {
		printf("I am Slave!\n");
		MainFunc = SlaveMain;

		// 配置PA10为开漏输出，用于触发主机中断
		GPIOA->BSRR = (uint32_t)GPIO_PIN_10; // 避免初始化时输出低电平导致触发
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

	SPIA_Init();
	MainFunc();
}
