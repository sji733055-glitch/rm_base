/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "osal_def.h"
#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "cdc_acm.h"
#include <stdio.h>
#include <string.h>

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     50
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

// 单个CDC设备实例
static USB_CDC_Device g_cdc_device = {0};
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[CDC_MAX_MPS][2];

static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01)
};

static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02)
};

static const uint8_t device_quality_descriptor[] = {
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
};

static const char *string_descriptors[] = {
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "CherryUSB",                  /* Manufacturer */
    "CherryUSB CDC DEMO",         /* Product */
    "2022123456",                 /* Serial Number */
};

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    return device_quality_descriptor;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    if (index > 3) {
        return NULL;
    }
    return string_descriptors[index];
}

const struct usb_descriptor cdc_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback
};

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            osal_sem_post(&g_cdc_device.ep_tx_sem);
            /* setup first out ep read transfer */
            usbd_ep_start_read(g_cdc_device.busid, CDC_OUT_EP, g_cdc_device.rx_buf[g_cdc_device.rx_active_buf], 
                              g_cdc_device.expected_rx_len ? g_cdc_device.expected_rx_len : g_cdc_device.rx_buf_size);
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)ep;
    (void)busid;
    
    // 保存当前缓冲区接收到的数据长度
    g_cdc_device.real_rx_len = nbytes;
    // 通知应用层数据已接收完成
    osal_event_set(&g_cdc_device.usb_cdc_event, USB_CDC_RX_DONE_EVENT);
    // 切换活动缓冲区
    g_cdc_device.rx_active_buf = !g_cdc_device.rx_active_buf;
    /* setup next out ep read transfer */
    usbd_ep_start_read(g_cdc_device.busid, CDC_OUT_EP, g_cdc_device.rx_buf[g_cdc_device.rx_active_buf], 
                      g_cdc_device.expected_rx_len ? g_cdc_device.expected_rx_len : g_cdc_device.rx_buf_size);
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    
    if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(g_cdc_device.busid, CDC_IN_EP, NULL, 0);
    } else {
        osal_sem_post(&g_cdc_device.ep_tx_sem);
    }
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

static struct usbd_interface intf0;
static struct usbd_interface intf1;

USB_CDC_Device* cdc_acm_init(uint8_t busid, uintptr_t reg_base, uint16_t expected_rx_len)
{
    // 检查设备是否已初始化
    if (g_cdc_device.is_initialized) {
        return NULL;
    }
    
    // 初始化参数
    memset(&g_cdc_device, 0, sizeof(USB_CDC_Device));
    g_cdc_device.busid = busid;
    g_cdc_device.reg_base = reg_base;
    g_cdc_device.rx_buf = (uint8_t (*)[2])read_buffer;
    g_cdc_device.rx_buf_size = CDC_MAX_MPS;
    g_cdc_device.expected_rx_len = expected_rx_len;
    if (g_cdc_device.expected_rx_len > g_cdc_device.rx_buf_size) {
        g_cdc_device.expected_rx_len = g_cdc_device.rx_buf_size;
    }
    
    // 创建信号量
    osal_sem_create(&g_cdc_device.ep_tx_sem, "ep_tx_sem", 1);
    
    // 创建事件组
    osal_event_create(&g_cdc_device.usb_cdc_event, "usb_cdc_event");
    
    // 标记设备已初始化
    g_cdc_device.is_initialized = 1;
    
#ifdef CONFIG_USBDEV_ADVANCE_DESC
    usbd_desc_register(g_cdc_device.busid, &cdc_descriptor);
#else
    usbd_desc_register(g_cdc_device.busid, cdc_descriptor);
#endif
    usbd_add_interface(g_cdc_device.busid, usbd_cdc_acm_init_intf(g_cdc_device.busid, &intf0));
    usbd_add_interface(g_cdc_device.busid, usbd_cdc_acm_init_intf(g_cdc_device.busid, &intf1));
    usbd_add_endpoint(g_cdc_device.busid, &cdc_out_ep);
    usbd_add_endpoint(g_cdc_device.busid, &cdc_in_ep);
    usbd_initialize(g_cdc_device.busid, g_cdc_device.reg_base, usbd_event_handler);
    
    return &g_cdc_device;
}

void cdc_acm_send(USB_CDC_Device *device, uint8_t* buffer, uint8_t size)
{
    if (device && buffer && size > 0) {
        if (osal_sem_wait(&device->ep_tx_sem, OSAL_WAIT_FOREVER) == OSAL_SUCCESS) {
            usbd_ep_start_write(device->busid, CDC_IN_EP, buffer, size);
        }
    }
}

uint8_t* cdc_acm_read(USB_CDC_Device *device)
{
    if (!device || !device->is_initialized) {
        return NULL;
    }
    
    unsigned int actual_flags;
    osal_status_t status;
    
    // 等待接收事件
    status = osal_event_wait(&device->usb_cdc_event, USB_CDC_RX_DONE_EVENT,
                           OSAL_EVENT_WAIT_FLAG_OR | OSAL_EVENT_WAIT_FLAG_CLEAR, 
                           OSAL_WAIT_FOREVER, &actual_flags);
    
    if (status == OSAL_SUCCESS) {
        // 返回非活动缓冲区的数据指针
        return device->rx_buf[!device->rx_active_buf];
    }
    
    return NULL;
}

void cdc_acm_deinit(USB_CDC_Device *device) 
{
    if (device == NULL || !device->is_initialized) {
        return;
    }
    
    usbd_deinitialize(device->busid);
    osal_sem_delete(&device->ep_tx_sem);
    osal_event_delete(&device->usb_cdc_event);
    memset(device, 0, sizeof(USB_CDC_Device));
}