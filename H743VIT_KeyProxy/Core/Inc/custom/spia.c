#include "spia.h"
#include "message.h"


extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

__attribute__((section(".dtcm_ram"))) uint8_t txBusy = 0;
uint8_t spiaTxBuff[20];

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi == &hspi1) {
		txBusy = 0;
	}
}

void SPIA_TxData(uint8_t *buff, uint8_t size) {
	// size == 9
	while (txBusy) {
		// delay?
		__NOP();
		__NOP();
	}
	HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&hspi1, buff, size);
	if (status != HAL_OK) {
		printf("tx error! status: %d, size: %d\n\tdata:", status, size);
		for (uint32_t i = 0; i < size; i++) {
			printf("%d, ", buff[i]);
		}
		printf("\n");

		Error_Handler();
	}
}

// 这里虽然接收10个字节，但前9位可用
#define RX_BATCH (10U)
uint8_t rxBuff[32];

void SPIA_Init() {
	HAL_StatusTypeDef status;
	txBusy = 0;

	status = HAL_SPI_Receive_DMA(&hspi2, rxBuff, RX_BATCH);
	if (status != HAL_OK) {
		printf("\n set Received ERROR! status: %d\n", status);
		Error_Handler();
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	HAL_StatusTypeDef status;

	if (hspi == &hspi2) {
		// 添加到消息队列中
		if (Msg_Add(rxBuff, MSG_FROM_SPI) != 0) {
			printf("add data to msg_buf error!\n");
			Error_Handler();
		}

		// 继续接收
		status = HAL_SPI_Receive_DMA(&hspi2, rxBuff, RX_BATCH);
		if (status != HAL_OK) {
			printf("\n set Received ERROR! status: %d\n", status);
			Error_Handler();
		}
	}
}
