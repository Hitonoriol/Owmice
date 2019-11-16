#include <stddef.h>
#include <stdint.h>

#include "../standard/owapi.inc"

char* cmd[]={", ", "help - view this menu", "lsrd - view ramdisk file list"};

void main() {
	int CMDS = (sizeof(cmd) / sizeof(cmd[0])) - 1;
	int i = 1;
	while(i <= CMDS) {
		owmice_writestring(cmd[i]);
		if (i != CMDS) owmice_writestring(cmd[0]);
		i++;
	}
}

#include "../standard/owapi.h"
