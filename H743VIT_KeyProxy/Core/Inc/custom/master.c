#include "stm32h7xx_hal.h"
#include "message.h"
#include "master.h"
#include "usb_host.h"
#include "spia.h"
#include "usba.h"

// #define CL_DEBUG

uint8_t mtxBuff[32];
uint8_t lastTx = 0;
uint8_t rx = 1;
uint32_t totalRecv = 0;
uint32_t mainLastActive;

__attribute__((section(".dtcm_ram"))) volatile uint8_t Active = 1;
__attribute__((section(".dtcm_ram"))) volatile uint8_t keyFlagSub = 0;
__attribute__((section(".dtcm_ram"))) volatile uint8_t keyFlagMain = 0;


uint32_t GetRand() {
	extern RNG_HandleTypeDef hrng;
	uint32_t random_num = 0;
	if (HAL_RNG_GenerateRandomNumber(&hrng, &random_num) == HAL_OK) {
		return random_num;
	}
	return 0;
}

void testSend() {
	if (rx == 0) {
		return;
	}
	rx = 0;
	uint8_t count = 0;
	for (uint8_t i = 0; i < 9; i++) {
		mtxBuff[i] = GetRand();
		count += mtxBuff[i];
	}
	mtxBuff[9] = count;
	lastTx = mtxBuff[0];
//	printf("tx: %u\n", lastTx);
	SPIA_TxData((uint8_t*)mtxBuff, 10);
}

void SendToSlave(uint8_t *data) {
	// 前8字节为HID 报文，第9字节为键鼠区分，第10字节为CRC
	uint8_t crc = 0;
	for (uint8_t i = 0; i < 9; i++) {
		mtxBuff[i] = data[i];
		crc += data[i];
	}

	mtxBuff[9] = crc;
	SPIA_TxData((uint8_t*)mtxBuff, 10);
}

uint8_t CheckActive(uint8_t *data) {
	// data 为收到的 HID 报文, 前8字节为原始报文，第9字节为键鼠标记。data 必须为9字节
	// 在这里检查按键的键值，并判断是否改变Active

	if (data[8] != 1) {
		#ifdef CL_DEBUG
			printf("is not kb.\n");
		#endif
		return 0;
	}
	if ((data[0] != 0) || \
		(data[1] != 0) || \
		(data[3] != 0) || \
		(data[4] != 0) || \
		(data[5] != 0) || \
		(data[6] != 0) || \
		(data[7] != 0)
	) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		#ifdef CL_DEBUG
			printf("is not hot key.\n");
		#endif
		keyFlagSub = 0;
		keyFlagMain = 0;
		return 0;
	}

	if (data[2] == 0) {
		// release
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
		#ifdef CL_DEBUG
			printf("key released.\n");
			uint8_t origin = Active;
		#endif
		if (Active == 1 && keyFlagSub == 1) {
			Active = 0;
		} else if (Active != 1 && keyFlagMain == 1) {
			Active = 1;
		}

		keyFlagSub = 0;
		keyFlagMain = 0;

		#ifdef CL_DEBUG
			printf("\t hot check: origin: %d, curr: %d\n", origin, Active);
		#endif
	} else if (data[2] == 71) { // scroll lock
		// switch to sub
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

		#ifdef CL_DEBUG
			printf("sub key pressed.\n");
		#endif
		keyFlagSub = 1;
		keyFlagMain = 0;
	} else if (data[2] == 72) { // pause break
		// switch to main
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

		#ifdef CL_DEBUG
			printf("main key pressed.\n");
		#endif
		keyFlagSub = 0;
		keyFlagMain = 1;
	} else {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		#ifdef CL_DEBUG
			printf("single key is not hotkey.\n");
		#endif
		keyFlagSub = 0;
		keyFlagMain = 0;
	}
	return 0;
}

void ProcessData(uint8_t *data, Msg_SourceTypeDef source) {
#ifdef CL_DEBUG
	printf("is keyboard: %d\n", data[8]);
	for (uint8_t i = 0; i < 8; i++) {
		printf("%d, ", data[i]);
	}
	printf("\n");
#endif
	if (Active) {
		// 发送到USB
		mainLastActive = HAL_GetTick();
		USBA_AutoReport(data);
	} else {
		// 发送到从机
		SendToSlave(data);
	}
	CheckActive(data);
}


void MasterMain() {
	mainLastActive = HAL_GetTick();
	uint8_t mvBuff[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	Active = 1;
	keyFlagSub = 0;
	keyFlagMain = 0;

	while (1) {
		MX_USB_HOST_Process();
		// 主机适用于mac, 降低回报率的突发率，以获得良好的兼容性
		// 125Hz -> 8ms
		if ((!Active) || (HAL_GetTick() - mainLastActive >= 8)) {
			// 非激活的状态，直接处理消息，以快速发送到从机；
			// 激活状态需要8ms之后再处理消息，以降低突发传输
			Msg_Consume(&ProcessData);
		}

		if (HAL_GetTick() - mainLastActive > (1000 *60 *5)) { // 5分钟
			mainLastActive = HAL_GetTick();
			#ifdef CL_DEBUG
				printf("move mouse!\n");
			#endif

			// 随机移动鼠标
			mvBuff[1] = (mainLastActive >> 5) & 0xFF;
			mvBuff[2] = mainLastActive & 0xFF;
			USBA_AutoReport(mvBuff);
		}
	}
}
