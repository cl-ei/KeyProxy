#include "message.h"

#define MESSAGE_SIZE 12   // 每条消息固定字节 (4字节对齐)
#define MESSAGE_SIZE_VALID 10 // 每条消息中，前10字节是有效的
#define BUFFER_SIZE  200  // 缓冲区可存储的消息数量

// 消息结构体
typedef struct {
    uint8_t data[MESSAGE_SIZE];
    Msg_SourceTypeDef source;
} Message;

// 缓冲区结构体
typedef struct {
    Message messages[BUFFER_SIZE];
    volatile uint16_t head;  // 写入位置(由中断修改)
    volatile uint16_t tail;  // 读取位置(由主循环修改)
} MessageBuffer;

__attribute__((section(".dtcm_ram"))) MessageBuffer msg_buff;
__attribute__((section(".dtcm_ram"))) MessageBuffer* buffer;


void Msg_Init() {
	buffer = &msg_buff;
	buffer->head = 0;
	buffer->tail = 0;
}

// 添加消息到缓冲区
// 这里会添加11字节的数据: 前8字节报文，第9字节键鼠标志，第10字节crc
uint8_t Msg_Add(const uint8_t* data, Msg_SourceTypeDef source) {
    uint16_t next_head = (buffer->head + 1) % BUFFER_SIZE;

    // 检查缓冲区是否已满
    if (next_head == buffer->tail) {
        return 1;  // 缓冲区已满
    }

    // 复制数据到缓冲区
    memcpy(buffer->messages[buffer->head].data, data, MESSAGE_SIZE_VALID);
    buffer->messages[buffer->head].source = source;
    // 更新head指针(确保编译器不会优化这个操作)
    buffer->head = next_head;

    return 0;
}

// 检查是否有消息
uint8_t Msg_IsEmpty() {
    return (buffer->head == buffer->tail) ? 1 : 0;
}

// 处理一条消息
uint8_t Msg_Consume(void (*cb)(uint8_t *data, Msg_SourceTypeDef source)) {
	if (buffer->head == buffer->tail) {
		return 0;
	}

	uint8_t *data_ptr = buffer->messages[buffer->tail].data;
	if (buffer->messages[buffer->tail].source == MSG_FROM_SPI) {
		// 检查CRC值
		uint8_t count = 0;
		for (uint8_t i = 0; i < 9; i++) {
			count += data_ptr[i];
		}
		if (count != data_ptr[9]) {
			printf("received data crc error: %ud, last: %u\n", count, data_ptr[9]);
			Error_Handler();
		}
	}

	cb(data_ptr, buffer->messages[buffer->tail].source);

    // 更新tail指针
    buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
    return 1;
}
