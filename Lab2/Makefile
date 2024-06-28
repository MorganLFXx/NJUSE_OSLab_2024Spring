main: main.cpp my_print.asm
	nasm -f elf32 my_print.asm
	g++ -g -m32 main.cpp my_print.o -o main -std=c++11
	rm -rf *.o
	./main
.PHONY:clean,gdb
clean:
	rm -rf main
	rm -rf *.o
gdb:
	gdb ./main
