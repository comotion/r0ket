#include <stdint.h>

#include "lpc134x.h"

#include "usb/usb.h"
#include "usbdesc.h"
#include "rom_drivers.h"

#include <sysinit.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/print.h"
#include "lcd/allfonts.h"

USB_DEV_INFO DeviceInfo;
HID_DEVICE_INFO HidDevInfo;

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

void GetInReport (uint8_t src[], uint32_t length) {
	*((uint32_t*)src) = GetLight();
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
	HidDevInfo.InReportCount = 4;
	HidDevInfo.OutReportCount = 1;
	HidDevInfo.SampleInterval = 0x20;
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

void main_usbinput(void)
{
	usbHIDInit();

	lcdPrintln("Done"); lcdRefresh(); 
	while(1){
	}
}
