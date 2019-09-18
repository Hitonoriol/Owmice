typedef struct {
	uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} task_regs_t;
 
typedef struct task_s {
	uint32_t id;
	volatile uint32_t sleeptime;
	task_regs_t regs;
	uint32_t entrypoint;
	struct task_s *next;
	struct task_s *prev;
} task_t;

static task_t *task_current;
static task_t *task_last;
static task_t task_main;
volatile uint32_t lid = 0;
extern uint32_t amalloc(uint32_t);
void task_yield();

void task_spawn(task_t *task, void (*main)(), uint32_t flags) {
	task->id = ++lid;
	task_last->next = task;
	task->prev = task_last;
	task_last = task;
	printf("New task created (0x%X | PID: %u)\n", (uint32_t)main, task->id);
	task->regs.eax = 0;
	task->regs.ebx = 0;
	task->regs.ecx = 0;
	task->regs.edx = 0;
	task->regs.esi = 0;
	task->regs.cr3 = task_main.regs.cr3;
	task->regs.edi = 0;
	task->regs.eflags = flags;
	task->regs.eip = (uint32_t) main;
	task->entrypoint = task->regs.eip;
	task->regs.esp = amalloc(0x1000);
	task->next = &task_main;
	task_main.prev = task;
	task_yield();
}

void task_sleep(task_t *task, uint32_t ticks) {
	task->sleeptime = ticks;
}

void task_self_sleep(uint32_t ticks) {
	task_sleep(task_current, ticks);
}

void task_kill(task_t *task) {
	if (task == task_last)
		task_last = task->prev;
	task->prev->next = task->next;
	task->next->prev = task->prev;
	task_current = &task_main;
	//free task stack and state
}

void task_self_kill() {
	printf("Task end request: PID %u\n", task_current->id);
	task_kill(task_current);
}

bool task_kill_pid(uint32_t pid) {
	task_t *tmp = &task_main;
	bool lst = true;
	while (lst || tmp != &task_main) {
		lst = false;
		if (tmp->id == pid) {
			task_kill(tmp);
			printf("Task #%u killed!\n", pid);
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}

void task_switch(task_regs_t *from, task_regs_t *to) {
asm volatile("\
	pusha;\
	pushf;\
	mov %cr3, %eax;\
	push %eax;\
	mov 44(%esp), %eax;\
	mov %ebx, 4(%eax);\
	mov %ecx, 8(%eax);\
	mov %edx, 12(%eax);\
	mov %esi, 16(%eax);\
	mov %edi, 20(%eax);\
	mov 36(%esp), %ebx;\
	mov 40(%esp), %ecx;\
	mov 20(%esp), %edx;\
	add $4, %edx;\
	mov 16(%esp), %esi;\
	mov 4(%esp), %edi;\
	mov %ebx, (%eax);\
	mov %edx, 24(%eax);\
	mov %esi, 28(%eax);\
	mov %ecx, 32(%eax);\
	mov %edi, 36(%eax);\
	pop %ebx;\
	mov %ebx, 40(%eax);\
	push %ebx;\
	mov 48(%esp), %eax;\
	mov 4(%eax), %ebx;\
	mov 8(%eax), %ecx;\
	mov 12(%eax), %edx;\
	mov 16(%eax), %esi;\
	mov 20(%eax), %edi;\
	mov 28(%eax), %ebp;\
	push %eax;\
	mov 36(%eax), %eax;\
	push %eax;\
	popf;\
	pop %eax;\
	mov 24(%eax), %esp;\
	push %eax;\
	mov 40(%eax), %eax;\
	mov %eax, %cr3;\
	pop %eax;\
	push %eax;\
	mov 32(%eax), %eax;\
	xchg (%esp), %eax;\
	mov (%eax), %eax;");
}
 
extern void kmain(unsigned long, unsigned long);
void tasking_init() {
	asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(task_main.regs.cr3)::"%eax");
	printf("cr3: 0x%X\n", task_main.regs.cr3);
	asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(task_main.regs.eflags)::"%eax");
	get_regs();
	task_main.entrypoint = (uint32_t)kmain;
	task_main.regs.esp = regdump.esp;
	task_main.regs.eip = regdump.eip;
	task_main.regs.eax = regdump.eax;
	task_main.regs.ebx = regdump.ebx;
	task_main.regs.ecx = regdump.ecx;
	task_main.regs.edx = regdump.edx;
	task_main.regs.esi = regdump.esi;
	task_main.regs.edi = regdump.edi;
	task_main.sleeptime = 0;
	task_last = &task_main;
	task_main.id = lid;
	task_main.next = &task_main;
	task_current = &task_main;
	printf ("Tasking initialized!\nKernel thread (0x%X | PID: %u)\n", task_main.regs.eip, task_main.id);
	
}

void task_list() {
	task_t *tmp = &task_main;
	bool lst = true;
	while (lst || tmp != &task_main) {
		lst = false;
		printf("PID: %u | Entrypoint: 0x%X | Sleeptime: %u\n", tmp->id, tmp->entrypoint, tmp->sleeptime);
		tmp = tmp->next;
	}
	term_undo_nl();
}

void task_yield() {
	if (task_current->next == task_current)
		return;
	task_t *last = task_current;
	task_current = task_current->next;
	if (task_current->sleeptime > 0) {
		task_current->sleeptime--;
		task_current = last;
	}
	task_switch(&last->regs, &task_current->regs);
}
