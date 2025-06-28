#include "stm32h7xx_hal.h"
#include "message.h"
#include "spia.h"
#include "slave.h"


// #define CL_DEBUG

uint8_t stxBuff[32];

void SendToMaster(uint8_t *data) {
	// 前8字节为HID 报文，第9字节为键鼠区分，第10字节为CRC
	uint8_t crc = 0;
	for (uint8_t i = 0; i < 9; i++) {
		stxBuff[i] = data[i];
		crc += data[i];
	}

	stxBuff[9] = crc;
	SPIA_TxData((uint8_t*)stxBuff, 10);
}

void SlaveProcessData(uint8_t *data, Msg_SourceTypeDef source) {
	#ifdef CL_DEBUG
		printf("slave rx data!\n");
		for (uint8_t i = 0; i < 9; i++) {
			printf("%u, ", data[i]);
		}
		printf("\n");
	#endif

	if (source == MSG_FROM_USB) {
		// slave 接收到的USB消息，一律发送到主机
		SendToMaster(data);
	} else {
		// slave 接收到的SPI消息，一律发送到USB设备
		USBA_AutoReport(data);
	}
}


void SlaveMain() {
	while (1) {
		MX_USB_HOST_Process();
		Msg_Consume(&SlaveProcessData);
	}
}
