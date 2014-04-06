# 1 "start.s"
 








# 1 "../syscall.h" 1
 














# 1 "../copyright.h" 1
 























# 16 "../syscall.h" 2






 

enum ExceptionType { NoException,            
		     SyscallException,       
		     PageFaultException,     
		     ReadOnlyException,      
					     
		     BusErrorException,      
					     
		     AddressErrorException,  
					     
					     
		     OverflowException,      
		     IllegalInstrException,  
		     
		     NumExceptionTypes
};

extern "C" __declspec(dllexport) void ExceptionHandler(ExceptionType which);
				 
				 
				 
typedef void (* FPExceptionHandler)(ExceptionType );
extern FPExceptionHandler fpExceptionHandler;
 




 


























# 250 "../syscall.h"




# 10 "start.s" 2


        .text   
        .align  2

 








	.globl __start
	.ent	__start
__start:
	jal	main
	move	$4,$0  	
	jal	Exit	   
	.end __start

 












	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,0 
	syscall
	j	$31
	.end Halt

	.globl Add
	.ent	Add
Add:
	addiu $2,$0,42 
	syscall
	j 	$31
	.end Add

	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,1 
	syscall
	j	$31
	.end Exit

	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,2 
	syscall
	j	$31
	.end Exec

	.globl ExecV
	.ent	ExecV
ExecV:
	addiu $2,$0,14 
	syscall
	j	$31
	.end ExecV

	.globl Join
	.ent	Join
Join:
	addiu $2,$0,3 
	syscall
	j	$31
	.end Join

	.globl Create
	.ent	Create
Create:
	addiu $2,$0,4 
	syscall
	j	$31
	.end Create

	.globl Remove
	.ent	Remove
Remove:
	addiu $2,$0,5 
	syscall
	j	$31
	.end Remove

	.globl Open
	.ent	Open
Open:
	addiu $2,$0,6 
	syscall
	j	$31
	.end Open

	.globl Read
	.ent	Read
Read:
	addiu $2,$0,7 
	syscall
	j	$31
	.end Read

	.globl Write
	.ent	Write
Write:
	addiu $2,$0,8 
	syscall
	j	$31
	.end Write

 
    .globl Putw
	.ent	Putw
Putw:
    addiu $2,$0,SC_Putw
	syscall
	j	$31
	.end Putw
	
	.globl Close
	.ent	Close
Close:
	addiu $2,$0,10 
	syscall
	j	$31
	.end Close

	.globl Seek
	.ent	Seek
Seek:
	addiu $2,$0,9 
	syscall
	j	$31
	.end Seek

	.globl Delete
	.ent	Delete
Delete:
        addiu $2,$0,11 
	syscall
	j	$31
	.end Delete

        .globl ThreadFork
        .ent    ThreadFork
ThreadFork:
        addiu $2,$0,12 
        syscall
        j       $31
        .end ThreadFork

        .globl ThreadYield
        .ent    ThreadYield
ThreadYield:
        addiu $2,$0,13 
        syscall
        j       $31
        .end ThreadYield

	.globl ThreadExit
	.ent    ThreadExit
ThreadExit:
	addiu $2, $0, 15 
	syscall
	j 	$31
	.end ThreadExit

	.globl ThreadJoin
	.ent    ThreadJoin
ThreadJoin:
	addiu $2, $0, 16 
	syscall
	j 	$31
	.end ThreadJoin
	
	.globl getSpaceID
	.ent	getSpaceID
getSpaceID:
	addiu $2,$0,17 
	syscall
	j	$31
	.end getSpaceID

	.globl getThreadID
	.ent	getThreadID
getThreadID:
	addiu $2,$0,18 
	syscall
	j	$31
	.end getThreadID

	.globl Ipc
	.ent   Ipc
Ipc:
    	addiu $2,$0,19 
	syscall
	j       $31
	.end Ipc
	
	.globl Clock
	.ent   Clock
Clock:
	addiu $2,$0,20 
	syscall
	j       $31
	.end Clock

 
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main

