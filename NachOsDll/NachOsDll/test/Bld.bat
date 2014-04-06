:Bld:
:  将从参数传进来的源文件编译为coff文件
:  如：Bld halt，将halt.c编译为halt.coff
:--------------------------------------------------------

:重新编译start.s
IF EXIST strt.s del strt.s
IF EXIST start.o del start.o
cpp0.exe -I../userprog -I../lib start.s > strt.s
decstation-ultrix-as.exe -mips2 -o start.o strt.s

:编译成目标文件
decstation-ultrix-gcc.exe -G 0 -O3 -ggdb -c -I ../lib -I ../userprog -I ../threads %1.c -o %1.o

decstation-ultrix-ld.exe -T script -N start.o %1.o -o %1.coff

:删除中间文件
del strt.s %1.o

