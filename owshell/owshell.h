#ifndef _OWSHELL_EXECUTE_H
#define _OWSHELL_EXECUTE_H
#include <stdbool.h>
#include "../screen/terminal.h"
#include "../standard/strings.h"
char* tbuf;
static char* cmd[]={", ", "die","cls","help", "ticks", "lsrd", "catrd <file>", "mem", "now"};

void whelp() {
	int CMDS = (sizeof(cmd) / sizeof(cmd[0])) - 1;
	int i = 1;
	while(i <= CMDS) {
		term_writestring(cmd[i]);
		if (i != CMDS) term_writestring(cmd[0]);
		i++;
	}
}

extern void ls_initrd();
extern uint32_t malloc(uint32_t sz);
extern char* kbd_get_string(char* buf);
extern void cat_initrd(char* fname);

extern volatile uint32_t mem_free;
extern volatile uint32_t mem_unused;
void execute(char* com) {
		clearchar(tbuf);
		if (streq(com, "exit"))
			return;
		else if (streq(com, "help"))
			whelp();
		else if (streq(com, "die"))
			die(STATUS_GENERAL);
		else if (streq(com, "cls"))
			term_cls();
		else if (streq(com, "ticks"))
			printf("Ticks: %d\nUptime: %d seconds", kticks, kticks/100);
		else if (streq(com, "lsrd"))
			ls_initrd();
		else if (streq(com, "mem"))
			printf("Available memory: %uB\nLast unused addr: 0x%X", mem_free, mem_unused);
		else if (streq(com, "now")) {
			today();
		}
		else if (strtok(tbuf, com, " ") != NULL){
			if (streq(tbuf, "catrd"))
				cat_initrd(strtok(tbuf, com, " "));
			else
				printf("No such command.");
		}
}

void owshell_main() {
	char* cmd = (char*)malloc(129);
	tbuf = (char*)malloc(129);
	printf("\nOwshell is alive!");
	while (!streq(cmd, "exit")) {
		prompt();
		kbd_get_string(cmd);
		execute(cmd);
	}
	cprint("End of session.", VGA_COLOR_MAGENTA);
}
#endif
