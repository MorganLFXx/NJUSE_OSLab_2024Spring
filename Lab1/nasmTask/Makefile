# Makefile for NASM projects

# 定义编译器和链接器
ASM=nasm
LD=ld

# 定义源文件和目标文件
SRC=convert.asm
OBJ=convert.o
OUT=convert

# 定义编译和链接标志
ASMFLAGS=-f elf32 -g
LDFLAGS= -m elf_i386

# 默认目标
all: $(OUT)

# 编译.asm文件到.o文件
$(OBJ): $(SRC)
	$(ASM) $(ASMFLAGS) $< -o $@

# 链接.o文件生成可执行文件
$(OUT): $(OBJ)
	$(LD) $(LDFLAGS) $< -o $@

# 运行可执行文件
run: $(OUT)
	./$(OUT)

# 使用gdb调试
debug: $(OUT)
	gdb ./$(OUT)

# 清理编译生成的文件
clean:
	rm -f $(OBJ) $(OUT)

# 声明伪目标
.PHONY: all run debug clean
