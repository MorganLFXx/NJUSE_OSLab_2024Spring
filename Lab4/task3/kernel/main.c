
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

char sign[3] = {'X', 'O', 'Z'};
char color[3] = {'\01', '\02', '\03'};

PRIVATE void init_tasks()
{

    // 表驱动，对应进程0, 1, 2, 3, 4, 5, 6
    int prior[7] = {1, 1, 1, 1, 1, 1, 1};
    for (int i = 0; i < 7; ++i)
    {
        proc_table[i].ticks = prior[i];
        proc_table[i].priority = prior[i];
    }

    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;
}

PUBLIC int clean_screen()
{
    disp_pos = 0;
    for (int i = 0; i < SCREEN_SIZE; ++i)
    {
        disp_str(" ");
    }
    disp_pos = 0;
}

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
    disp_str("-----\"kernel_main\" begins-----\n");


    clean_screen();

    TASK *p_task = task_table;
    PROCESS *p_proc = proc_table;
    char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16 selector_ldt = SELECTOR_LDT_FIRST;
    u8 privilege;
    u8 rpl;
    int eflags;
    for (int i = 0; i < NR_TASKS + NR_PROCS; i++)
    {
        if (i < NR_TASKS)
        {     /* 任务 */
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
        } else
        {                  /* 用户进程 */
            p_task = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202; /* IF=1, bit 2 is always 1 */
        }

        strcpy(p_proc->p_name, p_task->name);    // name of the process
        p_proc->pid = i;            // pid
        p_proc->wakeup = 0; // 初始化结构体新增成员
        p_proc->block = 0;
        p_proc->status = RELAXING;        // pid
        p_proc->total = 0;
        p_proc->ldt_sel = selector_ldt;

        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | privilege << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
        p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl; // 初始化寄存器
        p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
        p_proc->regs.eip = (u32) p_task->initial_eip;
        p_proc->regs.esp = (u32) p_task_stack;
        p_proc->regs.eflags = eflags;
        p_proc->nr_tty = 0;

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1 << 3;
    }


    empty_mutex.value = 2; // 仓库容量
    full_mutex.value = 0; // 已经生产的产品数量
    one_mutex.value = 0;
    two_mutex.value = 0;
    consume_two_mutex.value = 1;
    init_tasks();  

    init_clock(); // 在这里实现了时钟的中断
    restart();

    while (1)
    {}
}

PUBLIC void reporter_A()
{
    mysleep(TIMESLICE);
    while (1)
    {
        for (int i = 1; i <= 20; i++)
        {
            printf("No%c%c%c: ", '\06', i / 10 + '0', i % 10 + '0');
            for (int j = NR_TASKS + 1; j < NR_PROCS + NR_TASKS; j++)
            {
                int num = (proc_table + j)->total;


                printf("%c%c%c ", '\06', num / 10 + '0', num % 10 + '0');
            }
            printf("\n");
            mysleep(TIMESLICE);
        }
        while (1);
    }
}

PUBLIC void p_one()
{
    while (1)
    {
        P(&empty_mutex);
        produce_proc();
        V(&one_mutex);
        V(&full_mutex);
    }
}

PUBLIC void p_two()
{
    while (1)
    {
        P(&empty_mutex);
        produce_proc();
        V(&two_mutex);
        V(&full_mutex);
    }
}

PUBLIC void c_one()
{
    while (1)
    {
        P(&full_mutex);
        P(&one_mutex);
        consume_proc();
        V(&empty_mutex);
    }
}

PUBLIC void c_two_1()
{
    while (1)
    {
        P(&consume_two_mutex);
        P(&full_mutex);
        P(&two_mutex);
        consume_proc();
        V(&empty_mutex);
        V(&consume_two_mutex);
    }
}

PUBLIC void c_two_2()
{
    while (1)
    {
        P(&consume_two_mutex);
        P(&full_mutex);
        P(&two_mutex);
        consume_proc();
        V(&empty_mutex);
        V(&consume_two_mutex);
    }
}

PUBLIC void produce_proc()
{
    p_proc_ready->total++;
    mysleep(1000);
}

PUBLIC void consume_proc()
{
    p_proc_ready->total++;
    mysleep(1000);
}