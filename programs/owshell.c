#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define INPUT_BUFFER_SIZE 129

char* bin_ext_str = ".owb";
char* tbuf;

#define BIN_EXT 4

#include "../standard/stdio.inc"
#include "../standard/strings.inc"
#include "../standard/owapi.inc"

extern void bin_file_exec(char*, int, int);
extern void execute(char*);

void owshell_main() {
	char* cmd = (char*)owmice_malloc(INPUT_BUFFER_SIZE);
	tbuf = (char*)owmice_malloc(INPUT_BUFFER_SIZE);
	owmice_set_title("Owshell");
	printf("\nOwshell is alive!\n");
	owmice_print_date();
	while (true) {		
		owmice_prompt();
		owmice_kbd_get_string(cmd);
		execute(cmd);
	}
	owmice_die(0);
}

void bin_file_exec(char* com, int arg1, int arg2) {
	uint32_t clen = strlen(com);
	char *bin_name = (char*)owmice_malloc(clen + BIN_EXT + 1);
	strcpy(bin_name, com);
	owmice_exec_initrd(strcat(bin_name, bin_ext_str), arg1, arg2);
	owmice_free(bin_name);
}

void execute(char* com) {
		clearchar(tbuf);
		if (strchr(com, (int)' ') && strtok(tbuf, com, " ") != NULL)
			bin_file_exec(tbuf, (int)com, 0);
		else
			bin_file_exec(com, 0, 0);
}

#include "../standard/stdio.h"
#include "../standard/strings.h"
#include "../standard/owapi.h"

