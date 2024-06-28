section .data
	;27 means \033;[1;31m表示红色高亮;[31m表示红色
	red:	 	db  27, '[31m'
    	.len:   equ $ - red
	def: 		db 27, '[0m'
    	.len:   equ $ - def

section .text
	global my_print
	global back_to_default
	global change_to_red

	my_print:
		mov	eax,4
		mov	ebx,1
		mov	ecx,[esp+4];要打印的字符串地址
		mov	edx,[esp+8];字符串长度
		int	80h
		ret
		
	
	back_to_default:
		mov eax, 4
		mov ebx, 1
		mov ecx, def
		mov edx, def.len
		int 80h
		ret

	change_to_red:
		mov eax, 4
		mov ebx, 1
		mov ecx, red
		mov edx, red.len
		int 80h
		ret
