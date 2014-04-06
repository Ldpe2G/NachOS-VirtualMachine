:BldStart
:  编译start.s为start.o
:----------------------------------------------------

:如果已存在，先删除
IF EXIST strt.s del strt.s
IF EXIST start.o del start.o

:对start.s进行预处理,输出到中间文件strt.s
cpp0.exe -I../userprog -I../lib start.s > strt.s

:对中间文件strt.s进行汇编,得到目标文件start.o
decstation-ultrix-as.exe -mips2 -o start.o strt.s

:删除中间文件
del strt.s




