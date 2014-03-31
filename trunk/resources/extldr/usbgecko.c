#include "exi.h"
#include "usbgecko.h"
#include "types.h"
#define USBGECKO_CMD_IDENTIFY		0x90000000
#define USBGECKO_IDENTIFY_RESPONSE	0x04700000
#define USBGECKO_CMD_TX_STATUS		0xc0000000
#define USBGECKO_TX_FIFO_READY		0x04000000
#define USBGECKO_CMD_RX_STATUS		0xd0000000
#define USBGECKO_RX_FIFO_READY		0x04000000
#define USBGECKO_CMD_RECEIVE		0xa0000000 
#define USBGECKO_CMD_TRANSMIT(ch)	(0xb0000000 | ((ch) << 20))

static int usb_gecko_channel = -1;

static u32 usbgecko_transaction(int ch, u32 data)
{
	volatile void* exi_base = (volatile void*)EXI_ADDR(ch);

	/* 5.9.1.2: Selecting a specific EXI device: select the USB Gecko device */
	*(vu32*)(exi_base + EXI_CSR) = EXI_CSR_CLK_32MHZ | EXI_CSR_CS(1);

	/* 5.9.1.4: Perform IMM operation: setup data */
	*(vu32*)(exi_base + EXI_DATA) = data;

	/* 5.9.1.4: Perform IMM operation: schedule read/write; 2 bytes; start now */
	*(vu32*)(exi_base + EXI_CR) = EXI_CR_TLEN_2 | EXI_CR_RW_RW | EXI_CR_TSTART;

	/* 5.9.1.4: Perform IMM operation: Wait until transfer is completed */
	while (1) {
		if ((*(vu32*)(exi_base + EXI_CR) & EXI_CR_TSTART) == 0)
			break;
		/* XXX barrier */
	}

	/* 5.9.1.4: Fetch the data */
	data = *(vu32*)(exi_base + EXI_DATA);

	/* 5.9.1.3: Deselecting EXI devices */
	*(vu32*)(exi_base + EXI_CSR) = 0;

	return data;
}

static void usbgecko_putch(char ch)
{
	if (usb_gecko_channel < 0)
		return;

	int timeout = 16;
	while (1) {
		if (usbgecko_transaction(usb_gecko_channel, USBGECKO_CMD_TX_STATUS) & USBGECKO_TX_FIFO_READY)
			break;
		timeout--;
		if (timeout < 0) {
			break;
		}
	}
	usbgecko_transaction(usb_gecko_channel, USBGECKO_CMD_TRANSMIT(ch));
	/* XXX hack to get newlines right */
	if (ch == '\n')
		usbgecko_putch('\r');
}

void gprintf(const char* string)
{
	char* temp = (char*)string;
	while (*temp != '\x0')
	{
		usbgecko_putch(*temp);
		temp++;
	}
}

void usbgecko_init()
{
	int i;
	for (i = 0; i < 2; i++) {
		u32 d = usbgecko_transaction(i, USBGECKO_CMD_IDENTIFY);
		if (d != USBGECKO_IDENTIFY_RESPONSE)
			continue;
		usb_gecko_channel = i;
	}
}

/* vim:set ts=2 sw=2: */
