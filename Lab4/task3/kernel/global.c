
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	PROCESS	proc_table[NR_TASKS + NR_PROCS];

PUBLIC	TASK	task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "tty"}};

PUBLIC  TASK    user_proc_table[NR_PROCS] = {
	{reporter_A, STACK_SIZE_TESTA, "reporter_A"},
	{p_one, STACK_SIZE_TESTB, "p_one"},
	{p_two, STACK_SIZE_TESTC, "p_two"},
	{c_one, STACK_SIZE_TESTD, "c_one"},
	{c_two_1, STACK_SIZE_TESTE, "c_two_1"},
	{c_two_2, STACK_SIZE_TESTF, "c_two_2"}};

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks, 
                                                      sys_print, sys_sleep, sys_p, sys_v};

PUBLIC int mode;

PUBLIC Semaphore empty_mutex;
PUBLIC Semaphore full_mutex;
PUBLIC Semaphore one_mutex;
PUBLIC Semaphore two_mutex;
PUBLIC Semaphore consume_two_mutex;
