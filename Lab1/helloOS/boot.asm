	org 07c00h ; 告诉编译器程序加载到07c00h处
	mov ax,cs ; 将代码段寄存器cs加载到ax
	mov ds,ax ; 加载到数据段寄存器ds
	mov es,ax ; 加载到附加段寄存器es
	call DispStr ; 调用显示字符串例程
	jmp $ ; 无条件跳转到当前位置
DispStr:
	mov ax,BootMessage
	mov bp,ax ; ES:BP = 串地址  指向了要显示的字符串
	mov cx,10 ; CX = 串长度
	mov ax,01301h ; AH = 13h（功能号），AL = 01h（光标跟随显示）
	mov bx,000ch ; 页号为0(BH = 0) 黑底红字(BL = 0Ch)
	mov dl,0 ; 将dl置零，表示在当前显示器上执行显示操作
	int 10h ; 中断,执行显示字符串的操作
	ret
BootMessage:db "Hello, OS!"
times 510-($-$$) db 0  ; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 0xaa55 ; 引导扇区的结束标识
