#define KEY_SHIFT 15
#define KEY_UP 11
#define KEY_LEFT 12
#define KEY_RIGHT 13
#define KEY_DOWN 14

extern void owmice_die(uint32_t);
extern void owmice_putchar(uint32_t);
extern void owmice_writestring(char*);
extern void owmice_cls(void);
extern void owmice_kbd_get_string(char*);
extern void owmice_print_date(void);
extern void owmice_print_meminfo(void);
extern void* owmice_malloc(uint32_t);
extern void owmice_free(void*);
extern void owmice_ls_initrd(void);
extern void owmice_cat_initrd(char*);
extern void owmice_prompt(void);
extern void owmice_set_title(char*);
extern int owmice_exec_initrd(char*, int, int);
extern char owmice_get_char(void);
extern unsigned long long owmice_get_ticks(void);
extern char* owmice_read_initrd(char* fname);
extern void owmice_draw(uint32_t, uint32_t, uint8_t);
extern void owmice_disable_scroll(void);
extern void owmice_enable_scroll(void);
extern uint32_t owmice_now(void);
extern void owmice_status_cls(void);
extern void owmice_sleep(uint32_t);
extern char owmice_kbd_current_char(void);
