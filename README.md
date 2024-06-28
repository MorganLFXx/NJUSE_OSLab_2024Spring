# NJUSE_OSLab_2024Spring
南大软院2024年春的操作系统课程实验
## Lab1
今年的Lab1做的是大数的进制转换，好像每年都是不一样的大数操作，而且我的代码写的挺屎山（）
## Lab2
Lab2里面好像有个bug
如果输入的指令是 ls NJU/ABOUT.TXT/.. 之类的指令不会报错，即把文件当做文件夹使用不会报错，偷懒了请自行修复，在某个函数里加判断就行（）
Lab2好像是参考的这位学长：https://github.com/Darel777/3-1-OS
## Lab3
Lab3中有以下一段
```
	# sudo mount -o loop a.img /mnt/floppy/
	# sudo cp -fv boot/loader.bin /mnt/floppy/
	# sudo cp -fv kernel.bin /mnt/floppy
	# sudo umount /mnt/floppy
	sudo mount -o loop a.img ./mounted
	sudo cp -fv boot/loader.bin ./mounted
	sudo cp -fv kernel.bin ./mounted
	sudo umount ./mounted
```
这是我自己的小改动，记得在该文件夹中创建mounted文件夹就行
Lab3好像是参考的这位学长，这是他的主页：https://eaglebear2002.github.io/
## Lab4
Lab4中和Lab3同理，记得创建mounted文件夹
这是Lab4参考代码：https://github.com/SEBugMaker/NJUSE_OS_Lab/tree/main/lab4
