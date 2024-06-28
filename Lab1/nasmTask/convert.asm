section .data
    ErrorMsg db "Error",0h
    input db 0      ; 输入缓存
    B db '0b',0      ; 二进制
    O db '0o',0      ; 八进制
    H db '0x',0      ; 十六进制
    base db 0         
    res: times 255 db 0  
    resB: times 255 db 0 
    reverseTemp: times 255 db 0 ; 用于逆序输出
    tempQ: times 255 db 0  ; 存储每次循环的被除数
    temp: times 255 db 0   ; 存储每次循环的商，用于更新tempQ
    num: times 255 db 0     
    ; 规定：字符串只存ACSII数字
section .text
    global _start
_start:
; input
    ; 逐个读入字符，存放至num中，直到读入空格
    ; 读入进制，检验是否是合法进制
    ; 如果读到q，直接退出
    mov eax,0
readNum_loop:
    call read
    ; 判断是否读到q，退出
    mov esi,input
    cmp byte [esi],'q'
    je quit
    ; 判断是否读到空格，开始读进制
    mov esi,input
    cmp byte [esi],32
    je readBase

    ; 将ASCII字符转换为数字
    mov esi,input
    mov al,byte [esi]
    call strToNum
    ; 如果出现了超过9或者小于0的数字，输出错误消息
    cmp al,9
    jg .callNumErr
    cmp al,0
    jl .callNumErr
    ; 将数字存入num
    mov bl,byte [esi]
    mov eax,num
    call slen
    mov esi,num
    add esi,eax
    mov byte [esi],bl
    jmp readNum_loop
.callNumErr:
    call Error
    jmp freshVar

readBase:
    ; 读入进制
    call read
    mov esi,input
    mov ecx,base
    mov bl,byte [esi]
    mov byte [ecx],bl
    ; 判断是否读到q，退出
    cmp byte [esi],'q'
    je quit

    ; 把num放进tempQ
    mov eax,num
    call slen 
    mov ecx,eax
    mov edx,0
.next:
    mov esi,num
    add esi,edx
    mov bl,byte [esi]
    mov esi,tempQ
    add esi,edx
    mov byte [esi],bl
    inc edx
    cmp edx,ecx
    jne .next

    mov esi,base
    cmp byte [esi],'b' ; 判断是否是二进制
    je process
    cmp byte [esi],'o' ; 判断是否是八进制
    je process
    cmp byte [esi],'h' ; 判断是否是十六进制
    je process
    call Error ; 出现异常
    jmp freshVar
process:
    ; 继续读入,直到读到回车才开始执行，如果中途读到其他奇怪的东西则报错
.readLoop:
    call read
    mov esi,input
    cmp byte [esi],0x0A
    je dToB
    cmp byte [esi],32
    je .readLoop
    call Error
    jmp freshVar
dToB:
    mov ecx,0
checktempQ:
    ; 检查tempQ是否达到退出的条件
    mov esi,tempQ
    add esi,ecx
    cmp byte [esi],'0'
    jne transLoop
    inc ecx
    cmp ecx,64
    jne checktempQ
    mov eax,base
    jmp further; 八进制和十六进制跳转到进一步处理
transLoop:
    mov ecx,0 ; 用于统计已经遍历的字符数
    mov edi,0 ; 用于存余数的零寄存器
    jmp .charLoop
.next:
    inc ecx
    mov eax,tempQ
    call slen
    cmp ecx,eax
    jne .charLoop; 循环直到遍历完整个字符串
    ; 将该轮遍历的余数放入resB
    mov eax,edi
    call numToStr
    mov edi,eax
    mov eax,resB
    call slen
    mov esi,resB
    add esi,eax
    mov eax,edi
    mov byte [esi],al

    ; 将temp更新给tempQ
    ; 先清空tempQ
    mov edx,0
.freshTempQ:
    mov esi,tempQ
    add esi,edx
    mov byte [esi],0
    inc edx
    cmp edx,64
    jne .freshTempQ
    ; 将temp的值赋给tempQ
    mov eax,temp
    call slen
    mov ecx,eax
    mov edx,0
.tempTotempQ:
    mov esi,temp
    add esi,edx
    mov al,byte [esi]
    mov esi,tempQ
    add esi,edx
    mov byte [esi],al
    inc edx
    cmp edx,ecx
    jne .tempTotempQ
    mov edx,0
.freshtemp:
    mov esi,temp
    add esi,edx
    mov byte [esi],0
    inc edx
    cmp edx,64
    jne .freshtemp
    jmp dToB
.allZero:
    call allZero
    jmp dToB

.charLoop:
    imul edi,10 ; 将余数乘以10
    mov esi,tempQ
    add esi,ecx
    mov al,byte [esi] ; 将当前要计算的位存入eax
    call strToNum ; 将字符转换为数字
    add edi,eax ; 将数字加到余数上

    xor edx,edx
    mov eax,edi
    mov ebx,2
    idiv ebx ; 计算余数/2的商
    call numToStr ; 将商转换为字符
    mov ebx,eax
    push eax
    ; 将个位数的tempQ单独处理
    mov eax,tempQ
    call slen
    cmp eax,1
    je .addOnlyOne
    ; 如果被除数不止有一位，且为前导零0，直接跳过不加入tempQ
    pop eax
    cmp eax,30h
    je .furtherJugde
    jne .add
    ; 判断是否为前导0
.furtherJugde:
    push eax
    mov eax,temp
    call slen
    cmp eax,0
    pop eax
    jg .add 
    je .freshReaminder 
.add:
    mov eax,temp
    call slen
    mov esi,temp ; temp为暂存这一轮的商
    add esi,eax
    mov byte [esi],bl
    jmp .freshReaminder
.isZero:
    call allZero
    jmp .freshReaminder
.addOnlyOne:
    pop eax
    cmp eax,30h
    je .isZero
    jne .add
.freshReaminder: ; 更新余数
    mov eax,edi
    xor edx,edx
    mov ebx,2
    idiv ebx
    mov edi,edx
    jmp .next

    ;转换结束之后，若是八进制和十六进制，进一步处理，否则跳转到输出
further:
    call reverse
    mov esi,base
    cmp byte [esi],'b'
    je output
    cmp byte [esi],'o'
    je dToO
    cmp byte [esi],'h'
    je dToX

dToO:
    mov eax,resB ; 二进制转换为八进制
    call slen
    xor edx,edx
    mov ebx,3
    idiv ebx
    cmp edx,0
    mov ecx,0 ; 用于统计每3个转换一次
    mov edi,0 ; 用于统计是否遍历完整个resB
    mov ebx,0 ; 用来存储每3个转换一次的和
    cmp edx,0
    je .trans
    mov ecx,3
    sub ecx,edx
.trans:
    mov esi,resB
    add esi,edi
    mov al,byte [esi]
    call strToNum
    call transO
    add bl,al ; bl用来统计每3个转换一次的和
    inc ecx  
    inc edi
    mov eax,ecx
    xor edx,edx
    push ebx
    mov ebx,3
    idiv ebx
    pop ebx
    cmp edx,0
    jne .trans ; 如果不是3的倍数，继续转换
    ; 是3的倍数则转为八进制
    mov ecx,0
    mov al,bl
    call numToStr ; 将bl中的和转为字符
    mov bl,al
    mov eax,res
    call slen 
    mov esi,res
    add esi,eax
    mov byte [esi],bl ; 将转换后的字符放入res
    mov eax,resB
    call slen
    cmp eax,edi ; 检测是否遍历了整个resB
    je output
    mov ebx,0
    jne .trans

dToX:
    mov eax,resB ; 二进制转换为十六进制
    call slen
    xor edx,edx
    mov ebx,4
    idiv ebx
    mov ecx,0 ; 用于统计每4个转换一次
    mov edi,0 ; 用于统计是否遍历完整个resB
    mov ebx,0
    cmp edx,0
    je .trans
    mov ecx,4
    sub ecx,edx
.trans:
    mov esi,resB
    add esi,edi
    mov al,byte [esi]
    call strToNum
    call transX
    add bl,al
    inc ecx
    inc edi
    mov eax,ecx
    xor edx,edx
    push ebx
    mov ebx,4
    idiv ebx
    pop ebx
    cmp edx,0
    jne .trans
    mov ecx,0
    mov al,bl
    call xNumtoStr
    mov bl,al
    mov eax,res
    call slen
    mov esi,res
    add esi,eax
    mov byte [esi],bl
    mov eax,resB
    call slen
    cmp eax,edi
    je output
    mov ebx,0
    jne .trans

;-----------
; output
output:
    mov esi,base
    cmp byte [esi],'b'
    je .outputB
    cmp byte [esi],'o'
    je .outputO
    cmp byte [esi],'h'
    je .outputX
.outputB:
    mov eax,B
    call sprint
    mov eax,resB
    jmp .continue
.outputO:
    mov eax,O
    call sprint
    mov eax,res
    jmp .continue
.outputX:
    mov eax,H
    call sprint
    mov eax,res
    jmp .continue
.continue:
    call sprintLF
freshVar:
    ; 清理变量
    mov ecx,base ; 清理base
    mov byte [ecx],0
    mov ecx,input ; 清理input
    mov byte [ecx],0
    mov eax,res
    call fresh
    mov eax,resB
    call fresh
    mov eax,tempQ
    call fresh
    mov eax,temp
    call fresh
    mov eax,num
    call fresh
    mov eax,reverseTemp
    call fresh
freshReg:
    ; 清理寄存器
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    xor esi,esi
    xor edi,edi
    jmp _start
;-----------
; function to use
;------
; int slen(string msg)
; string length 
slen:
    push ebx
    mov ebx,eax
.nextchar:
    cmp byte [eax],0
    jz .finished
    inc eax
    jmp .nextchar
.finished:
    sub eax,ebx
    pop ebx
    ret
sprint:
    push edx
    push ecx
    push ebx
    push eax
    call slen 
    mov edx,eax
    pop eax
    mov ecx,eax
    mov ebx,1
    mov eax,4
    int 0x80
    pop ebx
    pop ecx
    pop edx
    ret
sprintLF:
    call sprint 
    push eax
    mov eax,0Ah
    push eax
    mov eax,esp
    call sprint
    pop eax
    pop eax
    ret
read:
    push edx
    push ecx
    push ebx
    push eax
    mov edx,1
    mov ecx,input
    mov ebx,0
    mov eax,3
    int 0x80
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
numToStr:
    push ebx
    mov bl,al
    add bl,30h
    mov al,bl
    pop ebx
    ret
strToNum:
    push ebx
    mov bl,al
    sub bl,30h
    mov al,bl
    pop ebx
    ret 
xNumtoStr:
    push ebx
    mov bl,al
    cmp bl,10
    jl .numtoStr
    cmp bl,10
    je .ten
    cmp bl,11
    je .eleven
    cmp bl,12
    je .twelve
    cmp bl,13
    je .thirteen
    cmp bl,14
    je .fourteen
    cmp bl,15
    je .fifteen
.numtoStr:
    call numToStr
    pop ebx
    ret
.ten:   
    mov al,'a'
    pop ebx
    ret
.eleven:    
    mov al,'b'
    pop ebx
    ret
.twelve:
    mov al,'c'
    pop ebx
    ret
.thirteen:  
    mov al,'d'
    pop ebx
    ret
.fourteen:  
    mov al,'e'
    pop ebx
    ret
.fifteen:   
    mov al,'f'
    pop ebx
    ret
transO:
    cmp al,1
    je .plus
    jne .noAdd
.plus:
    cmp ecx,0
    je .plus4
    cmp ecx,1
    je .plus2
    cmp ecx,2
    je .plus1
.plus4:
    mov al,4    
    ret
.plus2:
    mov al,2
    ret
.plus1:
    mov al,1
    ret
.noAdd:
    ret
transX:
    cmp al,1
    je .plus
    jne .noAdd
.plus:
    cmp ecx,0
    je .plus8
    cmp ecx,1
    je .plus4
    cmp ecx,2
    je .plus2
    cmp ecx,3
    je .plus1
.plus8:
    mov al,8
    ret
.plus4:
    mov al,4
    ret
.plus2:
    mov al,2
    ret
.plus1:
    mov al,1
    ret
.noAdd:
    ret
reverse:
    ; 先把resB逆序放入reverseTemp
    push eax
    push ecx
    push edi
    push esi
    mov eax,resB
    call slen
    mov ecx,eax
    mov edi,0
.next:
    mov esi,resB
    add esi,edi
    mov al,byte [esi]
    mov esi,reverseTemp
    add esi,ecx
    sub esi,1
    mov byte [esi],al
    inc edi
    dec ecx
    cmp ecx,0
    jne .next
    ; 再把reverseTemp放入resB
    mov ecx,0
.next2:
    mov esi,reverseTemp
    add esi,ecx
    mov al,byte [esi]
    mov esi,resB
    add esi,ecx
    mov byte [esi],al
    inc ecx
    cmp ecx,255
    jne .next2
    pop esi
    pop edi
    pop ecx
    pop eax
    ret
; 将temp填充满'0'
allZero:
    push eax
    push ecx
    push edx
    mov ecx,0
.next:
    mov eax,temp
    add eax,ecx
    mov byte [eax],'0'
    inc ecx
    cmp ecx,64
    jne .next
    pop eax
    pop ecx
    pop edx
    ret
;------
fresh:
    push edx
    push ecx
    mov edx,0
.finished:
    mov ecx,eax
    add ecx,edx
    mov byte [ecx],0
    inc edx
    cmp edx,255
    jne .finished
    pop ecx
    pop edx
    ret
;------ 
Error:
    mov eax,ErrorMsg
    call sprintLF
    ret
;------
quit:
    mov ebx,0
    mov eax,1
    int 0x80
    ret
; 调用约定
; eax 为返回值
; eax,ebx,ecx,edx 视情况依次为参数

; process
    ; 准备一个零寄存器，从左到右遍历十进制字符串的每个字符
        ; 将余数乘以10
        ; 将当前字符转换为对应数字，加到余数上
        ; 计算余数/2的商，转换为字符，添加到tempQ字符串，该字符串存本次循环遍历的商
        ; 更新余数为余数%2
        ; 重复循环直到遍历完整个字符串
    ; 将余数转换为字符，添加到结果字符串(insert到最前面，或者最后逆序)
    ; 若temp字符串不为0，将temp字符串作为被除数继续计算，直到temp字符为0