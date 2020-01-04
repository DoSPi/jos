/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <kern/kclock.h>

void
rtc_init(void)
{
	nmi_disable();
	// LAB 4: your code here
	outb(IO_RTC_CMND, RTC_BREG);
	uint8_t b_reg = inb(IO_RTC_DATA);
	b_reg |= RTC_PIE;
	outb(IO_RTC_DATA, b_reg);
	outb(IO_RTC_CMND, RTC_AREG);
	uint8_t a_reg = inb(IO_RTC_DATA);
	a_reg |= 0xF;
	outb(IO_RTC_DATA, a_reg);	
	nmi_enable();
}

uint8_t
rtc_check_status(void)
{
	uint8_t status = 0;
	// LAB 4: your code here
	outb(IO_RTC_CMND, RTC_CREG);
	status = inb(IO_RTC_DATA);
	return status;
}

unsigned
mc146818_read(unsigned reg)
{
	outb(IO_RTC_CMND, reg);
	return inb(IO_RTC_DATA);
}

void
mc146818_write(unsigned reg, unsigned datum)
{
	outb(IO_RTC_CMND, reg);
	outb(IO_RTC_DATA, datum);
}

