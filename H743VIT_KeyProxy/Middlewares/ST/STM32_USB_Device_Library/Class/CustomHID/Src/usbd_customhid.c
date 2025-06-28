/**
  ******************************************************************************
  * @file    usbd_customhid.c
  * @author  MCD Application Team
  * @brief   This file provides the CUSTOM_HID core functions.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  * @verbatim
  *
  *          ===================================================================
  *                                CUSTOM_HID Class  Description
  *          ===================================================================
  *           This module manages the CUSTOM_HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (CUSTOM_HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - Usage Page : Generic Desktop
  *             - Usage : Vendor
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CUSTOM_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CUSTOM_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CUSTOM_HID_Private_Macros
  * @{
  */
/**
  * @}
  */
/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_CUSTOM_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CUSTOM_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CUSTOM_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t USBD_CUSTOM_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CUSTOM_HID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef  *pdev);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_CUSTOM_HID_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_CUSTOM_HID_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_CUSTOM_HID_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_CUSTOM_HID_GetDeviceQualifierDesc(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables
  * @{
  */


__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
		0x06, 0x00, 0xFF,
		0x09, 0x01,
		0xA1, 0x01,
		0x09, 0x01,
		0x15, 0x00,
		0x26, 0xFF, 0x00,
		0x75, 0x08,
		0x95, 0x40,
		0x81, 0x02,
		0x09, 0x01,
		0x15, 0x00,
		0x26, 0xFF, 0x00,
		0x75, 0x08,
		0x95, 0x40,
		0x91, 0x02,
		0xC0
};

__ALIGN_BEGIN static uint8_t CUSTOM_HID_MouseReportDesc_FS[USBD_CUSTOM_HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END = {
  /* USER CODE BEGIN 1 */
    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x02,                    // Usage (Mouse)
    0xA1, 0x01,                    // Collection (Application)
    0x09, 0x01,                    // Usage (Pointer)
    0xA1, 0x00,                    // Collection (Physical)

    0x05, 0x09,                    // Usage Page (Button)
    0x19, 0x01,                    // Usage Minimum (Button 1)
    0x29, 0x03,                    // Usage Maximum (Button 3)
    0x15, 0x00,                    // Logical Minimum (0)
    0x25, 0x01,                    // Logical Maximum (1)

    0x95, 0x03,                    // Report Count (3)
    0x75, 0x01,                    // Report Size (1)
    0x81, 0x02,                    // Input (Data, Variable, Absolute)
    0x95, 0x01,                    // Report Count (1)
    0x75, 0x05,                    // Report Size (5)

    0x81, 0x03,                    // Input (Constant, Var, abs)
    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x30,                    // Usage (X)
    0x09, 0x31,                    // Usage (Y)
    0x09, 0x38,                    // Usage (wheel)

    0x15, 0x81,                    // Logical Minimum (-127)
    0x25, 0x7F,                    // Logical Maximum (127)
    0x75, 0x08,                    // Report Size (8)
    0x95, 0x03,                    // Report Count (3)
    0x81, 0x06,                    // Input (Data, Variable, Relative)
    0xC0,                          // 需注意，此处的0xC0不是END_COLLECTION
    0xC0                           /*     END_COLLECTION             */
};

__ALIGN_BEGIN static uint8_t CUSTOM_HID_KeyboardReportDesc_FS[USBD_CUSTOM_HID_KEYBOARD_REPORT_DESC_SIZE] __ALIGN_END = {
    0x05, 0x01,
    0x09, 0x06,
    0xA1, 0x01,
    0x05, 0x07,
    0x19, 0xE0,

    0x29, 0xE7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,

    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,
    0x95, 0x05,

    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,

    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,
    0x95, 0x06,
    0x75, 0x08,

    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,

    0x81, 0x00,
    0xC0
};

USBD_ClassTypeDef  USBD_CUSTOM_HID =
{
  USBD_CUSTOM_HID_Init,
  USBD_CUSTOM_HID_DeInit,
  USBD_CUSTOM_HID_Setup,
  NULL, /*EP0_TxSent*/
  USBD_CUSTOM_HID_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
  USBD_CUSTOM_HID_DataIn, /*DataIn*/
  USBD_CUSTOM_HID_DataOut,
  NULL, /*SOF */
  NULL,
  NULL,
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_CUSTOM_HID_GetHSCfgDesc,
  USBD_CUSTOM_HID_GetFSCfgDesc,
  USBD_CUSTOM_HID_GetOtherSpeedCfgDesc,
  USBD_CUSTOM_HID_GetDeviceQualifierDesc,
#endif /* USE_USBD_COMPOSITE  */
};

#ifndef USE_USBD_COMPOSITE
/* USB CUSTOM_HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_CfgDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
		0x09,                                               /* bLength: Configuration Descriptor size */
		  USB_DESC_TYPE_CONFIGURATION,                        /* bDescriptorType: Configuration */
		  LOBYTE(USB_CUSTOM_HID_CONFIG_DESC_SIZ),             /* wTotalLength: Bytes returned */
		  HIBYTE(USB_CUSTOM_HID_CONFIG_DESC_SIZ),
		  0x03,                                               // 3个接口
		  0x01,                                               /* bConfigurationValue: Configuration value */
		  0x00,                                               /* iConfiguration: Index of string descriptor
		                                                         describing the configuration */
		  0xA0,                                               // 总线供电，支持唤醒
		  USBD_MAX_POWER,                                     /* MaxPower (mA) */
		  // ==================== 第0个接口 (默认)  ====================//
		  /************** Descriptor of CUSTOM HID interface ****************/
		  0x09, USB_DESC_TYPE_INTERFACE, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
		  /******************** Descriptor of CUSTOM_HID *************************/
		  0x09, CUSTOM_HID_DESCRIPTOR_TYPE, 0x11, 0x01, 0x00, 0x01, 0x22, LOBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE), HIBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),
		  /******************** Descriptor of Custom HID endpoints ********************/
		  0x07, USB_DESC_TYPE_ENDPOINT, CUSTOM_HID_EPIN_ADDR, 0x03, LOBYTE(CUSTOM_HID_EPIN_SIZE), HIBYTE(CUSTOM_HID_EPIN_SIZE), CUSTOM_HID_FS_BINTERVAL,
		  0x07, USB_DESC_TYPE_ENDPOINT, CUSTOM_HID_EPOUT_ADDR, 0x03, LOBYTE(CUSTOM_HID_EPOUT_SIZE), HIBYTE(CUSTOM_HID_EPOUT_SIZE), CUSTOM_HID_FS_BINTERVAL,
		  /* 9 + 9 + 9 + 7 + 7 = 41 */

		  // ==================== 第1个接口 (鼠标)  ====================//
		  /************** Descriptor of CUSTOM HID interface ****************/
		  0x09,
		  USB_DESC_TYPE_INTERFACE,
		  0x01,                                               // 接口编号，按顺序指定Interface */
		  0x00,                                               /* bAlternateSetting: Alternate setting */
		  0x01,                                               // 端点个数
		  0x03,                                               /* bInterfaceClass: CUSTOM_HID */
		  0x00,                                               /* bInterfaceSubClass : 1=BOOT, 0=no boot */
		  0x02,                                               /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
		  0x00,                                               /* iInterface: Index of string descriptor */
		  /* 50 */
		  /******************** Descriptor of CUSTOM_HID *************************/
		  0x09,
		  CUSTOM_HID_DESCRIPTOR_TYPE,
		  0x11,
		  0x01,
		  0x00,                                               // HID为键盘时设置国家， 0x21为美国。鼠标不用修改  0x00,
		  0x01,                                               /* bNumDescriptors: Number of CUSTOM_HID class descriptors
		                                                         to follow */
		  0x22,                                               // 描述符类型: 报告描述符
		  LOBYTE(USBD_CUSTOM_HID_MOUSE_REPORT_DESC_SIZE),     // 报告描述符的大小
		  HIBYTE(USBD_CUSTOM_HID_MOUSE_REPORT_DESC_SIZE),
		  /* 59 */
		  /******************** Descriptor of Custom HID endpoints ********************/
		  0x07,
		  USB_DESC_TYPE_ENDPOINT,
		  // CUSTOM_HID_EPIN_ADDR D0~D3端点编号，D4~D6保留。D7为传输方向，主机为out时为0, 主机为in时为1，即键盘鼠标类是以0b1xxx_xxxx开头。
		  // 这里使用2号 端点，MOUSE_EP 应为0x82。
		  MOUSE_EP,
		  0x03,                                               // 4种控制方式，11: 中断; 10: 批量; 01: 同步; 00: 控制。批量需要自己编写驱动
		  LOBYTE(MOUSE_EP_SIZE),
		  HIBYTE(MOUSE_EP_SIZE),
		  CUSTOM_HID_FS_BINTERVAL,
		  /* 66 */
		  // ==================== 第2个接口 (键盘)  ====================//
		  /************** Descriptor of CUSTOM HID interface ****************/
		  0x09,
		  USB_DESC_TYPE_INTERFACE,
		  0x02,                                               // modified: 接口编号，前面为1，这里用2
		  0x00,                                               /* bAlternateSetting: Alternate setting */
		  0x01,                                               // 端点个数，只有一个输入端点
		  0x03,                                               /* bInterfaceClass: CUSTOM_HID */
		  0x00,                                               /* bInterfaceSubClass : 1=BOOT, 0=no boot */
		  0x01,                                               /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
		  0x00,                                               /* iInterface: Index of string descriptor */
		  /* 75 */
		  /******************** Descriptor of CUSTOM_HID *************************/
		  0x09,
		  CUSTOM_HID_DESCRIPTOR_TYPE,
		  0x11,                                               /* bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number */
		  0x01,
		  0x21,                                               // 键盘设置国家， 0x21为美国
		  0x01,                                               /* bNumDescriptors: Number of CUSTOM_HID class descriptors
		                                                         to follow */
		  0x22,                                               // 0x22: 报告描述符
		  LOBYTE(USBD_CUSTOM_HID_KEYBOARD_REPORT_DESC_SIZE),  // 报告描述符的大小
		  HIBYTE(USBD_CUSTOM_HID_KEYBOARD_REPORT_DESC_SIZE),
		   /* 84 */
		   /******************** Descriptor of Custom HID endpoints ********************/
		   0x07,
		   USB_DESC_TYPE_ENDPOINT,
		   KEYBOARD_EP,                                       // CUSTOM_HID_EPIN_ADDR, 上述已使用了2号端点，KEYBOARD_EP 应该为0x83
		   0x03,                                              // modified: 4种控制方式，11: 中断; 10: 批量; 01: 同步; 00: 控制
		   LOBYTE(KEYBOARD_EP_SIZE),
		   HIBYTE(KEYBOARD_EP_SIZE),
		   CUSTOM_HID_FS_BINTERVAL
		   /* 91 */
};
#endif /* USE_USBD_COMPOSITE  */

/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_Desc[USB_CUSTOM_HID_DESC_SIZ] __ALIGN_END =
{
  /* 18 */
  0x09,                                               /* bLength: CUSTOM_HID Descriptor size */
  CUSTOM_HID_DESCRIPTOR_TYPE,                         /* bDescriptorType: CUSTOM_HID */
  0x11,                                               /* bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number */
  0x01,
  0x00,                                               /* bCountryCode: Hardware target country */
  0x01,                                               /* bNumDescriptors: Number of CUSTOM_HID class descriptors
                                                         to follow */
  0x22,                                               /* bDescriptorType */
  LOBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),                   /* wItemLength: Total length of Report descriptor */
  HIBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),
};

#ifndef USE_USBD_COMPOSITE
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};
#endif /* USE_USBD_COMPOSITE  */

static uint8_t CUSTOMHIDInEpAdd = CUSTOM_HID_EPIN_ADDR;
static uint8_t CUSTOMHIDOutEpAdd = CUSTOM_HID_EPOUT_ADDR;
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         Initialize the CUSTOM_HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)USBD_malloc(sizeof(USBD_CUSTOM_HID_HandleTypeDef));

  if (hhid == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)hhid;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CUSTOMHIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
  CUSTOMHIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].bInterval = CUSTOM_HID_HS_BINTERVAL;
    pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].bInterval = CUSTOM_HID_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].bInterval = CUSTOM_HID_FS_BINTERVAL;
    pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].bInterval = CUSTOM_HID_FS_BINTERVAL;
  }

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, CUSTOMHIDInEpAdd, USBD_EP_TYPE_INTR,
                       CUSTOM_HID_EPIN_SIZE);

  pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].is_used = 1U;

	// =========================================
	//  以下是添加自定义的EP
	(void)USBD_LL_OpenEP(pdev, MOUSE_EP, USBD_EP_TYPE_INTR, MOUSE_EP_SIZE);
	pdev->ep_in[MOUSE_EP & 0xFU].is_used = 1U;

	(void)USBD_LL_OpenEP(pdev, KEYBOARD_EP, USBD_EP_TYPE_INTR, KEYBOARD_EP_SIZE);
	pdev->ep_in[KEYBOARD_EP & 0xFU].is_used = 1U;
	// =========================================

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, CUSTOMHIDOutEpAdd, USBD_EP_TYPE_INTR,
                       CUSTOM_HID_EPOUT_SIZE);

  pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].is_used = 1U;

  hhid->state = CUSTOM_HID_IDLE;

  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init();

#ifndef USBD_CUSTOMHID_OUT_PREPARE_RECEIVE_DISABLED
  /* Prepare Out endpoint to receive 1st packet */
  (void)USBD_LL_PrepareReceive(pdev, CUSTOMHIDOutEpAdd, hhid->Report_buf,
                               USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
#endif /* USBD_CUSTOMHID_OUT_PREPARE_RECEIVE_DISABLED */

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CUSTOMHIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
  CUSTOMHIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close CUSTOM_HID EP IN */
  (void)USBD_LL_CloseEP(pdev, CUSTOMHIDInEpAdd);
  pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].is_used = 0U;
  pdev->ep_in[CUSTOMHIDInEpAdd & 0xFU].bInterval = 0U;

	// ======================================
	// 以下关闭自定义的端点
	(void)USBD_LL_CloseEP(pdev, MOUSE_EP);
	pdev->ep_in[MOUSE_EP & 0xFU].is_used = 0U;
	pdev->ep_in[MOUSE_EP & 0xFU].bInterval = 0U;

	(void)USBD_LL_CloseEP(pdev, KEYBOARD_EP);
	pdev->ep_in[KEYBOARD_EP & 0xFU].is_used = 0U;
	pdev->ep_in[KEYBOARD_EP & 0xFU].bInterval = 0U;
	// ======================================

  /* Close CUSTOM_HID EP OUT */
  (void)USBD_LL_CloseEP(pdev, CUSTOMHIDOutEpAdd);
  pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].is_used = 0U;
  pdev->ep_out[CUSTOMHIDOutEpAdd & 0xFU].bInterval = 0U;

  /* Free allocated memory */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit();
    USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Setup
  *         Handle the CUSTOM_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_Setup(USBD_HandleTypeDef *pdev,
                                     USBD_SetupReqTypedef *req)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t len = 0U;
#ifdef USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
  uint16_t ReportLength = 0U;
#endif /* USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED */
  uint8_t  *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case CUSTOM_HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case CUSTOM_HID_REQ_GET_PROTOCOL:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->Protocol, 1U);
          break;

        case CUSTOM_HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case CUSTOM_HID_REQ_GET_IDLE:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->IdleState, 1U);
          break;

        case CUSTOM_HID_REQ_SET_REPORT:
#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
          if (((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete != NULL)
          {
            /* Let the application decide when to enable EP0 to receive the next report */
            ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete(req->bRequest,
                                                                                            req->wLength);
          }
#endif /* USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED */
#ifndef USBD_CUSTOMHID_EP0_OUT_PREPARE_RECEIVE_DISABLED
          hhid->IsReportAvailable = 1U;
          (void)USBD_CtlPrepareRx(pdev, hhid->Report_buf,
                                  MIN(req->wLength, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE));
#endif /* USBD_CUSTOMHID_EP0_OUT_PREPARE_RECEIVE_DISABLED */
          break;
#ifdef USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
        case CUSTOM_HID_REQ_GET_REPORT:
          if (((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->GetReport != NULL)
          {
            ReportLength = req->wLength;

            /* Get report data buffer */
            pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->GetReport(&ReportLength);
          }

          if ((pbuf != NULL) && (ReportLength != 0U))
          {
            len = MIN(ReportLength, req->wLength);

            /* Send the report data over EP0 */
            (void)USBD_CtlSendData(pdev, pbuf, len);
          }
          else
          {
#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
            if (((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete != NULL)
            {
              /* Let the application decide what to do, keep EP0 data phase in NAK state and
                 use USBD_CtlSendData() when data become available or stall the EP0 data phase */
              ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->CtrlReqComplete(req->bRequest,
                                                                                              req->wLength);
            }
            else
            {
              /* Stall EP0 if no data available */
              USBD_CtlError(pdev, req);
            }
#else
            /* Stall EP0 if no data available */
            USBD_CtlError(pdev, req);
#endif /* USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED */
          }
          break;
#endif /* USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED */

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == CUSTOM_HID_REPORT_DESC)
          {
            /* len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
               pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->pReport;
               */

        	  // 以下判断自定义的端点，进行setup
        	  if (req->wIndex == 0) {
        	    len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
        	    pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->pReport;
        	  } else if (req->wIndex == 1) {
        	    len = MIN(USBD_CUSTOM_HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
        	    pbuf = CUSTOM_HID_MouseReportDesc_FS;
        	  } else if (req->wIndex == 2) {
        	    len = MIN(USBD_CUSTOM_HID_KEYBOARD_REPORT_DESC_SIZE, req->wLength);
        	    pbuf = CUSTOM_HID_KeyboardReportDesc_FS;
        	  }
          }
          else
          {
            if ((req->wValue >> 8) == CUSTOM_HID_DESCRIPTOR_TYPE)
            {
              pbuf = USBD_CUSTOM_HID_Desc;
              len = MIN(USB_CUSTOM_HID_DESC_SIZ, req->wLength);
            }
          }

          if (pbuf != NULL)
          {
            (void)USBD_CtlSendData(pdev, pbuf, len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
  return (uint8_t)ret;
}

/**
  * @brief  USBD_CUSTOM_HID_SendReport
  *         Send CUSTOM_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @param  ClassId: The Class ID
  * @retval status
  */
#ifdef USE_USBD_COMPOSITE
uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef *pdev,
                                   uint8_t *report, uint16_t len, uint8_t ClassId)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];
#else
uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef *pdev,
                                   uint8_t *report, uint16_t len)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#endif /* USE_USBD_COMPOSITE */

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

#ifdef USE_USBD_COMPOSITE
  /* Get Endpoint IN address allocated for this class instance */
  CUSTOMHIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, ClassId);
#endif /* USE_USBD_COMPOSITE */

  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == CUSTOM_HID_IDLE)
    {
      hhid->state = CUSTOM_HID_BUSY;
      (void)USBD_LL_Transmit(pdev, CUSTOMHIDInEpAdd, report, len);
    }
    else
    {
      return (uint8_t)USBD_BUSY;
    }
  }
  return (uint8_t)USBD_OK;
}
#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_CUSTOM_HID_GetFSCfgDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CUSTOM_HID_GetFSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_CUSTOM_HID_CfgDesc, CUSTOM_HID_EPIN_ADDR);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_CUSTOM_HID_CfgDesc, CUSTOM_HID_EPOUT_ADDR);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = CUSTOM_HID_EPIN_SIZE;
    pEpInDesc->bInterval = CUSTOM_HID_FS_BINTERVAL;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = CUSTOM_HID_EPOUT_SIZE;
    pEpOutDesc->bInterval = CUSTOM_HID_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_CUSTOM_HID_CfgDesc);
  return USBD_CUSTOM_HID_CfgDesc;
}

/**
  * @brief  USBD_CUSTOM_HID_GetHSCfgDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CUSTOM_HID_GetHSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_CUSTOM_HID_CfgDesc, CUSTOM_HID_EPIN_ADDR);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_CUSTOM_HID_CfgDesc, CUSTOM_HID_EPOUT_ADDR);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = CUSTOM_HID_EPIN_SIZE;
    pEpInDesc->bInterval = CUSTOM_HID_HS_BINTERVAL;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = CUSTOM_HID_EPOUT_SIZE;
    pEpOutDesc->bInterval = CUSTOM_HID_HS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_CUSTOM_HID_CfgDesc);
  return USBD_CUSTOM_HID_CfgDesc;
}

/**
  * @brief  USBD_CUSTOM_HID_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CUSTOM_HID_GetOtherSpeedCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_CUSTOM_HID_CfgDesc, CUSTOM_HID_EPIN_ADDR);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_CUSTOM_HID_CfgDesc, CUSTOM_HID_EPOUT_ADDR);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = CUSTOM_HID_EPIN_SIZE;
    pEpInDesc->bInterval = CUSTOM_HID_FS_BINTERVAL;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = CUSTOM_HID_EPOUT_SIZE;
    pEpOutDesc->bInterval = CUSTOM_HID_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_CUSTOM_HID_CfgDesc);
  return USBD_CUSTOM_HID_CfgDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_CUSTOM_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId])->state = CUSTOM_HID_IDLE;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application processing */
  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->OutEvent(hhid->Report_buf[0],
                                                                           hhid->Report_buf[1]);

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CUSTOM_HID_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

#ifdef USE_USBD_COMPOSITE
  /* Get OUT Endpoint address allocated for this class instance */
  CUSTOMHIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  /* Resume USB Out process */
  (void)USBD_LL_PrepareReceive(pdev, CUSTOMHIDOutEpAdd, hhid->Report_buf,
                               USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (hhid->IsReportAvailable == 1U)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData[pdev->classId])->OutEvent(hhid->Report_buf[0],
                                                                             hhid->Report_buf[1]);
    hhid->IsReportAvailable = 0U;
  }

  return (uint8_t)USBD_OK;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CUSTOM_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_CUSTOM_HID_DeviceQualifierDesc);

  return USBD_CUSTOM_HID_DeviceQualifierDesc;
}
#endif /* USE_USBD_COMPOSITE  */
/**
  * @brief  USBD_CUSTOM_HID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CUSTOMHID Interface callback
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_RegisterInterface(USBD_HandleTypeDef *pdev,
                                          USBD_CUSTOM_HID_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}
/**
  * @}
  */

// ========================= Send report ===================================
uint8_t USBD_KEYBOADR_HID_SendReport(USBD_HandleTypeDef  *pdev, uint8_t *report, uint16_t len) {

    USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;

    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        if (hhid->state == CUSTOM_HID_IDLE) {
            hhid->state = CUSTOM_HID_BUSY;
            USBD_LL_Transmit(pdev, KEYBOARD_EP, report, len);
        } else {
            return USBD_BUSY;
        }
    }
    return USBD_OK;
}

// ========================= Send report ===================================
uint8_t USBD_MOUSE_HID_SendReport(USBD_HandleTypeDef  *pdev, uint8_t *report, uint16_t len) {

    USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;

    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        if (hhid->state == CUSTOM_HID_IDLE) {
            hhid->state = CUSTOM_HID_BUSY;
            USBD_LL_Transmit(pdev, MOUSE_EP, report, len);
        } else {
            return USBD_BUSY;
        }
    }
    return USBD_OK;
}

/**
  * @}
  */


/**
  * @}
  */

