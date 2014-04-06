:Coff2Noff
:  将从参数传进来的coff文件转换为noff文件
:  如：Coff2Noff halt,将文件halt.coff转换为文件halt.noff
:---------------------------------------------------

:清除调试信息
decstation-ultrix-strip %1.coff

:转换
coff2noff %1.coff %1.noff



