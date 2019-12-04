#ifndef _IO_INITRD_H
#define _IO_INITRD_H

#define INITRD_BUFFER_SIZE 1048576

char *read_buffer;

typedef struct {
   uint32_t nfiles; 
} initrd_header_t;

typedef struct {
   uint8_t magic;     
   uint8_t name[64];  
   uint32_t offset;   
   uint32_t length;   
} initrd_file_header_t;

initrd_header_t *initrd_header;     
initrd_file_header_t *file_headers; 
fs_node_t *initrd_root;             
fs_node_t *initrd_mtp;              
fs_node_t *root_nodes;              
int nroot_nodes;                    

struct dirent dirent;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	initrd_file_header_t header = file_headers[node->inode];
	if (offset > header.length)
		return 0;
	if (offset+size > header.length)
		size = header.length-offset;
	if (size > INITRD_BUFFER_SIZE)
		return 0;
	memcpy(buffer, (uint8_t*) (header.offset+offset), size);
	return size;
}

struct dirent *initrd_readdir(fs_node_t *node, uint32_t index) {
	if (node == initrd_root && index == 0) {
		strcpy(dirent.name, "mtp");
		dirent.name[3] = 0;
		dirent.ino = 0;
		return &dirent;
	}
	if (index-1 >= (unsigned)nroot_nodes)
		return 0;
	strcpy(dirent.name, root_nodes[index-1].name);
	dirent.name[strlen(root_nodes[index-1].name)] = 0;
	dirent.ino = root_nodes[index-1].inode;
	return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name) {
	if (node == initrd_root && streq(name, "mtp"))
		return initrd_mtp;
	int i;
	for (i = 0; i < nroot_nodes; i++)
		if (streq(name, root_nodes[i].name))
			return &root_nodes[i];
	return 0;
}

fs_node_t *initrd_init(uint32_t location) {
	printf("Initializing ramdisk (0x%X)... ", location);
	initrd_header = (initrd_header_t *)location;
	file_headers = (initrd_file_header_t *) (location+sizeof(initrd_header_t));
	initrd_root = (fs_node_t*)malloc(sizeof(fs_node_t));
	strcpy(initrd_root->name, "initrd");
	initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
	initrd_root->flags = FS_DIRECTORY;
	initrd_root->read = 0;
	initrd_root->write = 0;
	initrd_root->open = 0;
	initrd_root->close = 0;
	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;
	initrd_root->ptr = 0;
	initrd_root->impl = 0;
	initrd_mtp = (fs_node_t*)malloc(sizeof(fs_node_t));
	strcpy(initrd_mtp->name, "mtp");
	initrd_mtp->mask = initrd_mtp->uid = initrd_mtp->gid = initrd_mtp->inode = initrd_mtp->length = 0;
	initrd_mtp->flags = FS_DIRECTORY;
	initrd_mtp->read = 0;
	initrd_mtp->write = 0;
	initrd_mtp->open = 0;
	initrd_mtp->close = 0;
	initrd_mtp->readdir = &initrd_readdir;
	initrd_mtp->finddir = &initrd_finddir;
	initrd_mtp->ptr = 0;
	initrd_mtp->impl = 0;
	root_nodes = (fs_node_t*)malloc(sizeof(fs_node_t) * initrd_header->nfiles);
	nroot_nodes = initrd_header->nfiles;
	printf("%d files found on ramdisk\n", initrd_header->nfiles);
	printf("Max ramdisk read buffer size: %uB\n", INITRD_BUFFER_SIZE);
	uint32_t i;
	for (i = 0; i < initrd_header->nfiles; i++) {
		file_headers[i].offset += location;
		strcpy((char*)root_nodes[i].name, (char*)file_headers[i].name);
		root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
		root_nodes[i].length = file_headers[i].length;
		root_nodes[i].inode = i;
		root_nodes[i].flags = FS_FILE;
		root_nodes[i].read = &initrd_read;
		root_nodes[i].write = 0;
		root_nodes[i].readdir = 0;
		root_nodes[i].finddir = 0;
		root_nodes[i].open = 0;
		root_nodes[i].close = 0;
		root_nodes[i].impl = 0;
	}
	printf(" Done!\n");
	return initrd_root;
}

char *read_initrd(char* fname) {
	fs_node_t *fsnode = finddir_fs(fs_root, fname);
	if (fsnode == NULL)
		return NULL;
	char* buf = (char*)malloc(INITRD_BUFFER_SIZE);
	uint32_t sz = read_fs(fsnode, 0, INITRD_BUFFER_SIZE, (uint8_t*)buf);
	if (sz == 0) {
		free((uint32_t)buf);
		return NULL;
	}
	char* ret = (char*)malloc(sz);
	memmove(ret, buf, sz);
	free((uint32_t)buf);
	return ret;
}

#define EXEC_ADDR 0x100060
void cat_initrd(char* fname) {
	printf("Contents of %s:\n", fname);
	fs_node_t *fsnode = finddir_fs(fs_root, fname);
	if (fsnode == NULL) {
		printf("No such file\n");
		return;
	}
	char* buf = (char*)malloc(INITRD_BUFFER_SIZE);
	uint32_t sz = read_fs(fsnode, 0, INITRD_BUFFER_SIZE, (uint8_t*)buf);
	if (sz == 0) {
		printf("Invalid file entry or the file is too large.\n");
		free((uint32_t)buf);
	}
	uint32_t j;
	for (j = 1; j <= sz; j++) {
		printf("0x%X ", buf[j-1]);
		if (j % 5 == 0)
			printf("\n");
	}
	free((uint32_t)buf);
}

void ls_initrd() {
	int i = 0;
	struct dirent *node = 0;
	enum vga_color color_out;
	while ((node = readdir_fs(fs_root, i)) != 0) {
		fs_node_t *fsnode = finddir_fs(fs_root, node->name);
		if ((fsnode->flags) == FS_DIRECTORY)
			color_out = VGA_COLOR_LIGHT_BLUE;
		else 
			color_out = VGA_COLOR_WHITE;
  		cprint(node->name, color_out);
  		i++;
	}
	term_undo_nl();
}

extern uint32_t shell_buf;
extern uint32_t shell_size;
extern char* shell_fname;
typedef int bin_main(int, int);
int exec_initrd(char* fname, int arg1, int arg2) {
	
	if (!streq(fname, shell_fname))
		memmove((void*)shell_buf, (void*)EXEC_ADDR, shell_size);

	fs_node_t *fsnode = finddir_fs(fs_root, fname);
	if (fsnode == NULL)
		return (int)0xDEADF113;
	uint32_t sz = read_fs(fsnode, 0, INITRD_BUFFER_SIZE, (uint8_t*)EXEC_ADDR);

	if (sz == 0) {
		printf("Invalid file entry or the file is too large.\n");
		return (int)0xDEADF113;
	}

	int ret = ((bin_main*)EXEC_ADDR)(arg1, 0);
	//printf("\n* %s returned: 0x%X", fname, ret);

	if (!streq(fname, shell_fname))
		memmove((void*)EXEC_ADDR, (void*)shell_buf, shell_size);

	return ret;
}
#endif
