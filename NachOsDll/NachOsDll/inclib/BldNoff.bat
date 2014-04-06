:BldNoff
:  将从参数传进来的源文件直接编译为noff格式文件
:  如：BldNoff halt，将源程序halt.c编译成文件halt.noff
:-----------------------------------------------------

:重新编译start.s
IF EXIST strt.s del strt.s
IF EXIST start.o del start.o
cpp0.exe -I../  start.s > strt.s
decstation-ultrix-as.exe -mips2 -o start.o strt.s

:编译成目标文件
decstation-ultrix-gcc.exe -G 0 -O3 -ggdb -c -I ../  MergeSort.c -o MergeSort.o

:编译成coff文件
decstation-ultrix-ld.exe -T script -N start.o MergeSort.o -o MergeSort.coff


:将coff文件转换为noff文件
decstation-ultrix-strip.exe MergeSort.coff
coff2noff.exe MergeSort.coff MergeSort.noff


:删除中间文件
del strt.s MergeSort.o MergeSort.coff




