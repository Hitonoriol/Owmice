#ifndef _OWSHELL_EXECUTE_H
#define _OWSHELL_EXECUTE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../standard/stdio.h"
#include "../standard/strings.h"
#include "../standard/owapi.h"

char* bin_ext_str = ".owb";
char* tbuf;
char* cmd[]={", ", "help", "title <title string>"};

void whelp() {
	int CMDS = (sizeof(cmd) / sizeof(cmd[0])) - 1;
	int i = 1;
	owmice_writestring("Built-in commands:\n");
	while(i <= CMDS) {
		owmice_writestring(cmd[i]);
		if (i != CMDS) owmice_writestring(cmd[0]);
		i++;
	}
	owmice_writestring("\nFor more commands type \"lsrd\".");
}

#define BIN_EXT 4

void bin_file_exec(char* com, int arg1, int arg2) {
	uint32_t clen = strlen(com);
	char *bin_name = (char*)owmice_malloc(clen + BIN_EXT + 1);
	strcpy(bin_name, com);
	owmice_exec_initrd(strcat(bin_name, bin_ext_str), arg1, arg2);
	owmice_free(bin_name);
}

void execute(char* com) {
		clearchar(tbuf);
		if (streq(com, "help"))
			whelp();
		else if (strchr(com, (int)' ') && strtok(tbuf, com, " ") != NULL){
			bin_file_exec(tbuf, (int)com, 0);
			/*if (streq(tbuf, "catrd"))
				owmice_cat_initrd(com);
			else if (streq(tbuf, "title"))
				owmice_set_title(com);
			else if (streq(tbuf, "execrd"))
				owmice_exec_initrd(com);*/
		} else {
			bin_file_exec(com, 0, 0);
		}
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
