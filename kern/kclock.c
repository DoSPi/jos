/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <kern/kclock.h>
#include <inc/time.h>
static int gett()
{
	struct tm time;
	time.tm_sec = BCD2BIN(mc146818_read(RTC_SEC));
	time.tm_min = BCD2BIN(mc146818_read(RTC_MIN));
	time.tm_hour = BCD2BIN(mc146818_read(RTC_HOUR));
	time.tm_mday = BCD2BIN(mc146818_read(RTC_DAY));
	time.tm_mon = BCD2BIN(mc146818_read(RTC_MON)) - 1;
	time.tm_year = BCD2BIN(mc146818_read(RTC_YEAR));
	return timestamp(&time);
}

int gettime(void)
{
	nmi_disable();
	// LAB 12: your code here
	int time;
	do {
		while (mc146818_read(RTC_AREG) & RTC_UPDATE_IN_PROGRESS) {};
	} while ((time = gett()) != gett());

	nmi_enable();
	return time;
}

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

