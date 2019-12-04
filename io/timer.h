#ifndef _IO_TIMER_H
#define _IO_TIMER_H

#define PIT_BASE_FQ 1193180
#define PIT_10MSEC 100
extern void pit_handler(void);
unsigned long long kticks = 0;
volatile int32_t sleep_amt = 0;
extern void die(uint32_t stat);

unsigned long long *get_ticks(){
	return &kticks;
}

void sleep (uint32_t msec) {	//global sleep(stops scheduler because he can)
	sleep_amt = msec;
	while (sleep_amt > 0) {
		wait_int();
	}
}

void pit_handler_main() {
	//task_yield();
	kticks++;
	if (sleep_amt > 0)
		sleep_amt -= 10;
	EOI();
}

void timer_init(uint32_t frequency) {
	irq_map_handler(IRQ_TIMER, (unsigned long)pit_handler);
	uint32_t divisor = PIT_BASE_FQ / frequency;
	write_port(0x43, 0x36);
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor>>8) & 0xFF);
	write_port(0x40, l);
	write_port(0x40, h);
	write_port(0x21, 0xFC);
}

#endif
