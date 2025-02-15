
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				  console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
							Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_

/* CONSOLE */
typedef struct s_console {
	unsigned int current_start_addr; /* 当前显示到了什么位置 */
	unsigned int original_addr; /* 当前控制台对应显存位置 */
	unsigned int v_mem_limit; /* 当前控制台占的显存大小 */
	unsigned int cursor; /* 当前光标位置 */
	unsigned int searchMode; /* 是否处于查找模式 */     
	unsigned int blocked; /* 是否屏蔽按键 */
	unsigned int rolling; // 正在回滚
	unsigned int cleaning; // 正在清除
} CONSOLE;

#define SCR_UP 1  /* scroll forward */
#define SCR_DN -1 /* scroll backward */

#define SCREEN_SIZE (80 * 25)
#define SCREEN_WIDTH 80

#define TAB_LEN 4

#define DEFAULT_CHAR_COLOR 0x07 /* 0000 0111 黑底白字 */
#define RED_CHAR_COLOR 0x04 /* 0000 0100 黑底红字 */  
#define TAB_CHAR 0x00 /* '\0' */ // 

#endif /* _ORANGES_CONSOLE_H_ */
