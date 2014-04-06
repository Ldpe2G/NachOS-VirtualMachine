:BldAsm
:  将从参数传进来的源文件编译成汇编码
:  如：BldAsm halt, 对halt.c进行编译，得到汇编文件halt.s
:------------------------------------------------------

:将源文件%1编译为汇编码
decstation-ultrix-gcc.exe -I ../lib -I ../userprog -I ../threads -S %1.c



