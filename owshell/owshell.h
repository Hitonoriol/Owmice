#ifndef _OWSHELL_EXECUTE_H
#define _OWSHELL_EXECUTE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../standard/strings.h"
#include "../standard/owapi.h"

char* tbuf;
char* cmd[]={", ", "die","cls","help", "lsrd", "catrd <file>", "mem", "now", "title <title string>"};

void whelp() {
	int CMDS = (sizeof(cmd) / sizeof(cmd[0])) - 1;
	int i = 1;
	while(i <= CMDS) {
		owmice_writestring(cmd[i]);
		if (i != CMDS) owmice_writestring(cmd[0]);
		i++;
	}
}

void execute(char* com) {
		clearchar(tbuf);
		if (streq(com, "exit"))
			return;
		else if (streq(com, "help"))
			whelp();
		else if (streq(com, "die"))
			owmice_die(0xDEAD);
		else if (streq(com, "cls"))
			owmice_cls();
		//else if (streq(com, "ticks"))
		//	printf("Ticks: %d\nUptime: %d seconds", kticks, kticks/100);
		else if (streq(com, "lsrd"))
			owmice_ls_initrd();
		else if (streq(com, "mem")) {
			owmice_print_meminfo();
			return;
		}
		else if (streq(com, "now"))
			owmice_print_date();
		//else if (streq(com, "lstasks"))
		//	task_list();
		else if (strchr(com, (int)' ') && strtok(tbuf, com, " ") != NULL){
			if (streq(tbuf, "catrd"))
				owmice_cat_initrd(strtok(tbuf, com, " "));
			else if (streq(tbuf, "title"))
				owmice_set_title(com);
		} else
			owmice_writestring("No such command.");
}

void owshell_main() {
	char* cmd = (char*)owmice_malloc(129);
	tbuf = (char*)owmice_malloc(129);

	owmice_writestring("\nOwshell is alive!\n");
	owmice_print_date();
	owmice_set_title("Owshell");

	while (true) {		
		owmice_prompt();
		owmice_kbd_get_string(cmd);
		execute(cmd);
	}
	owmice_die(0);
}
#endif
