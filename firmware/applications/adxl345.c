#include <stdint.h>

#include "lpc134x.h"

#include "usb/usb.h"
#include "usbdesc.h"
#include "rom_drivers.h"

#include <sysinit.h>
#include "basic/basic.h"
#include <string.h>
#include "core/i2c/i2c.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "lcd/allfonts.h"

USB_DEV_INFO DeviceInfo;
HID_DEVICE_INFO HidDevInfo;

typedef struct hiddata_out_s {
	int16_t sensordata[3];
	uint32_t status;
	uint32_t systicks;
	uint32_t rollovers;
	uint32_t skipped;
} hiddata_out_t;

hiddata_out_t buf[2];
uint8_t activebuf;
uint32_t rot;

ROM ** myrom = (ROM **)0x1fff1ff8;
#ifndef CFG_USBMSC
USB_IRQHandler(void)
{
	(*myrom)->pUSBD->isr();
}
#endif

const uint8_t USB_StringDescriptor[] = {
  /* Index 0x00: LANGID Codes */
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
  /* Index 0x04: Manufacturer */
  0x1C,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'h',0, 'i',0, 'l',0, 's',0, 'e',0, '@',0, 'w',0, 'e',0, 'b',0, '.',0, 'd',0, 'e',0, ' ',0,
  /* Index 0x20: Product */
  0x28,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'r',0, '0',0, 'k',0, 'e',0, 't',0, ' ',0, 'H',0, 'I',0, 'D',0, ' ',0, ' ',0, ' ',0, ' ',0, ' ',0, ' ',0, ' ',0, ' ',0, ' ',0, ' ',0,
  /* Index 0x48: Serial Number */
  0x1A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0,
  /* Index 0x62: Interface 0, Alternate Setting 0 */
  0x0E,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'H',0, 'I',0, 'D',0, ' ',0, ' ',0, ' ',0,
};

#define USB_VENDOR_ID 0x1337
#define USB_PROD_ID   0xbabe
#define USB_DEVICE    0x0100

#define ADXL345_I2C_WRITE	0xA6
#define ADXL345_I2C_READ	0xA7

#define ADXL345_I2C_R_DEVID	0x0
#define ADXL345_I2C_R_BW_RATE	0x2C
#define ADXL345_I2C_R_POWER	0x2D
#define ADXL345_I2C_R_DATA_FORMAT	0x31
#define ADXL345_I2C_R_DATAX0	0x32
#define ADXL345_I2C_R_DATAX1	0x33
#define ADXL345_I2C_R_DATAY0	0x34
#define ADXL345_I2C_R_DATAY1	0x35
#define ADXL345_I2C_R_DATAZ0	0x36
#define ADXL345_I2C_R_DATAZ1	0x37

uint32_t adxl345SetByte(uint8_t cr, uint8_t value) {
	I2CMasterBuffer[0] = ADXL345_I2C_WRITE;
	I2CMasterBuffer[1] = cr;
	I2CMasterBuffer[2] = value;
	I2CWriteLength = 3;
	I2CReadLength = 0;
	return i2cEngine();
}

uint32_t adxl345GetBytes(uint8_t cr, uint8_t count) {
	I2CMasterBuffer[0] = ADXL345_I2C_WRITE;
	I2CMasterBuffer[1] = cr;
	I2CWriteLength = 2;
	I2CReadLength = 0;
	i2cEngine();

	I2CMasterBuffer[0] = ADXL345_I2C_READ;
	I2CWriteLength = 1;
	I2CReadLength = count;

	return i2cEngine();
}

uint32_t adxl345GetByte(uint8_t cr) {
	return adxl345GetBytes(cr, 1);
}

void GetInReport (uint8_t src[], uint32_t length) {
	memcpy(src, (const void*) &(buf[activebuf]), sizeof(hiddata_out_t));
	rot=0;
	return;
}

void SetOutReport (uint8_t dst[], uint32_t length) {
	lcdPrintln("got output"); lcdRefresh(); 
	return;
}
void usbHIDInit (void)
{
	// Setup USB clock
	SCB_PDRUNCFG &= ~(SCB_PDSLEEPCFG_USBPAD_PD);        // Power-up USB PHY
	SCB_PDRUNCFG &= ~(SCB_PDSLEEPCFG_USBPLL_PD);        // Power-up USB PLL

	SCB_USBPLLCLKSEL = SCB_USBPLLCLKSEL_SOURCE_MAINOSC; // Select PLL Input
	SCB_USBPLLCLKUEN = SCB_USBPLLCLKUEN_UPDATE;         // Update Clock Source
	SCB_USBPLLCLKUEN = SCB_USBPLLCLKUEN_DISABLE;        // Toggle Update Register
	SCB_USBPLLCLKUEN = SCB_USBPLLCLKUEN_UPDATE;

	// Wait until the USB clock is updated
	while (!(SCB_USBPLLCLKUEN & SCB_USBPLLCLKUEN_UPDATE));

	// Set USB clock to 48MHz (12MHz x 4)
	SCB_USBPLLCTRL = (SCB_USBPLLCTRL_MULT_4);
	while (!(SCB_USBPLLSTAT & SCB_USBPLLSTAT_LOCK));    // Wait Until PLL Locked
	SCB_USBCLKSEL = SCB_USBCLKSEL_SOURCE_USBPLLOUT;

	// Set USB pin functions
	IOCON_PIO0_1 &= ~IOCON_PIO0_1_FUNC_MASK;
	IOCON_PIO0_1 |= IOCON_PIO0_1_FUNC_CLKOUT;           // CLK OUT
	IOCON_PIO0_3 &= ~IOCON_PIO0_3_FUNC_MASK;
	IOCON_PIO0_3 |= IOCON_PIO0_3_FUNC_USB_VBUS;         // VBus
	IOCON_PIO0_6 &= ~IOCON_PIO0_6_FUNC_MASK;
	IOCON_PIO0_6 |= IOCON_PIO0_6_FUNC_USB_CONNECT;      // Soft Connect

	// Disable internal resistor on VBUS (0.3)
	gpioSetPullup(&IOCON_PIO0_3, gpioPullupMode_Inactive);

	// HID Device Info
	volatile int n;
	HidDevInfo.idVendor = USB_VENDOR_ID;
	HidDevInfo.idProduct = USB_PROD_ID;
	HidDevInfo.bcdDevice = USB_DEVICE;
	HidDevInfo.StrDescPtr = (uint32_t)&USB_StringDescriptor[0];
	HidDevInfo.InReportCount = sizeof(hiddata_out_t);
	HidDevInfo.OutReportCount = 1;
	HidDevInfo.SampleInterval = 0x02;
	HidDevInfo.InReport = GetInReport;
	HidDevInfo.OutReport = SetOutReport;

	DeviceInfo.DevType = USB_DEVICE_CLASS_HUMAN_INTERFACE;
	DeviceInfo.DevDetailPtr = (uint32_t)&HidDevInfo;

	/* Enable Timer32_1, IOCON, and USB blocks (for USB ROM driver) */
	SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT32B1 | SCB_SYSAHBCLKCTRL_IOCON | SCB_SYSAHBCLKCTRL_USB_REG);

	/* Use pll and pin init function in rom */
	/* Warning: This will also set the system clock to 48MHz! */
	// (*rom)->pUSBD->init_clk_pins();

	/* insert a delay between clk init and usb init */
	for (n = 0; n < 75; n++) {__asm("nop");}

	(*myrom)->pUSBD->init(&DeviceInfo); /* USB Initialization */
	(*myrom)->pUSBD->connect(true);     /* USB Connect */
}

void main_adxl345(void)
{
	uint8_t b;
	memset(buf, 0, 2*sizeof(hiddata_out_t));
	activebuf=0;
	rot=0;

	lcdPrintln("start"); lcdRefresh(); 
	i2cInit(I2CMASTER); // Init I2C
	lcdPrintln("i2c"); lcdRefresh();
	adxl345SetByte(ADXL345_I2C_R_DATA_FORMAT, 1<<3 | 1<<1 | 1<<0); // full res at +-16g
	adxl345SetByte(ADXL345_I2C_R_BW_RATE, 0x0D); // 800 Hz sampling rate
	adxl345SetByte(ADXL345_I2C_R_POWER, 1<<3); // start measuring
	lcdPrintln("adxl345"); lcdRefresh();
	usbHIDInit();
	lcdPrintln("usb"); lcdRefresh(); 
	while(1){
		b = activebuf ? 0 : 1;
		buf[b].status = adxl345GetBytes(ADXL345_I2C_R_DATAX0, 6);
		memcpy(buf[b].sensordata, (const void*) I2CSlaveBuffer, 6);
		buf[b].systicks = systickGetTicks();
		buf[b].rollovers = systickGetRollovers();
		buf[b].skipped = rot;
		activebuf = b;
		rot++;
	}
}
