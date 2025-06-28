#include "usba.h"
#include "usbh_def.h"
#include "usbh_hid_mouse.h"
#include "usb_device.h"
#include "message.h"

// ===================== cb =========================
__attribute__((section(".dtcm_ram"))) HID_HandleTypeDef *HID_Handle;
__attribute__((section(".dtcm_ram"))) uint16_t usbRxLen;
__attribute__((section(".dtcm_ram"))) uint8_t usbRxBuff[64];

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost) {
	HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;
	if ((HID_Handle->length == 0U) || (HID_Handle->fifo.buf == NULL)) {
		return;
	}

	// 读取并复制数据到缓冲区
	usbRxLen = USBH_HID_FifoRead(&HID_Handle->fifo, usbRxBuff, HID_Handle->length);
	if (usbRxLen != HID_Handle->length) {
		printf("usb rx len != dist len! rx: %d, hid handler len: %d\n", usbRxLen, HID_Handle->length);
		Error_Handler();
	}

	if ((phost->device.CfgDesc.bNumInterfaces > 0) && (phost->device.CfgDesc.Itf_Desc[0].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE)) {
		usbRxBuff[8] = 1;
	} else {
		usbRxBuff[8] = 0;
	}
	if (Msg_Add(usbRxBuff, MSG_FROM_USB)) {
		printf("add usb received data to msg queue err! status not eq 0\n");
		Error_Handler();
	}
}

uint8_t usbDevBuff[8];
extern USBD_HandleTypeDef hUsbDeviceFS;

void USBA_AutoReport(uint8_t *data) {
	uint8_t retry = 10;
	memcpy(usbDevBuff, data, 8);
	while (retry--) {
		if (data[8] == 1) {
			if (USBD_OK == USBD_KEYBOADR_HID_SendReport(&hUsbDeviceFS, usbDevBuff, 8)) {
				return;
			}
		} else {
			if (USBD_OK == USBD_MOUSE_HID_SendReport(&hUsbDeviceFS, usbDevBuff, 4)) {
				return;
			}
		}
		__NOP();
	}
}
