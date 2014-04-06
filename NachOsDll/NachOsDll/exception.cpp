// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
//#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "utility.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

extern "C" __declspec(dllexport) void ExceptionHandler(ExceptionType which){
	
	//int type = kernel->machine->ReadRegister(2);	/* Liang DePeng - */
	int type = (* kernel->readRegister)(2);			/* Liang DePeng + */

	int returnval;
	int vaddr;
	int modeaddr;
	int fileID;
	int length;
	int childId;
	int status;

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch(type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
			ASSERTNOTREACHED();
			break;
		case SC_Add:
			//returnval = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),	/* Liang DePeng - */
			//	/* int op2 */(int)kernel->machine->ReadRegister(5));					/* Liang DePeng - */
			returnval = SysAdd(/* int op1 */(int)(* kernel->readRegister)(4),	/* Liang DePeng + */
				/* int op2 */(int)(* kernel->readRegister)(5));					/* Liang DePeng + */

			break;
		case SC_Create:		/*zhong_expan + */
			//vaddr = kernel->machine->ReadRegister(4);	/* Liang DePeng - */
			vaddr = (* kernel->readRegister)(4);		/* Liang DePeng + */
			DEBUG(dbgSys,"System Call: Create vaddr=" << vaddr);
			returnval = SysCreate(vaddr);
			break;
		case SC_Open:		/*zhong_expan + */
			//vaddr = kernel->machine->ReadRegister(4);		/* Liang DePeng - */
			//modeaddr = kernel->machine->ReadRegister(5);	/* Liang DePeng - */
			vaddr = (* kernel->readRegister)(4);		/* Liang DePeng + */
			modeaddr = (* kernel->readRegister)(5);	/* Liang DePeng + */

			DEBUG(dbgSys,"System Call: Open vaddr=" << vaddr);
			DEBUG(dbgSys,"System Call: Open modeaddr=" << modeaddr);
			returnval = SysOpen(vaddr, modeaddr);
			break;
		case SC_Read:		/*zhong_expan + */
			//fileID = kernel->machine->ReadRegister(6);		/* Liang DePeng - */
			//vaddr = kernel->machine->ReadRegister(4);		/* Liang DePeng - */
			//length = kernel->machine->ReadRegister(5);		/* Liang DePeng - */

			fileID = (* kernel->readRegister)(6);		/* Liang DePeng + */
			vaddr = (* kernel->readRegister)(4);		/* Liang DePeng + */
			length = (* kernel->readRegister)(5);		/* Liang DePeng + */

			DEBUG(dbgSys,"System Call: Read vaddr=" << vaddr << " length=" << length << " fileID=" << fileID);
			returnval = SysRead(fileID,vaddr,length);
			break;
		case SC_Write:		/*zhong_expan + */
			//fileID = kernel->machine->ReadRegister(6);	/* Liang DePeng - */	
			//vaddr = kernel->machine->ReadRegister(4);		/* Liang DePeng - */
			//length = kernel->machine->ReadRegister(5);	/* Liang DePeng - */
			
			fileID = (* kernel->readRegister)(6);	/* Liang DePeng + */	
			vaddr = (* kernel->readRegister)(4);		/* Liang DePeng + */
			length = (* kernel->readRegister)(5);	/* Liang DePeng + */

			DEBUG(dbgSys,"System Call: Write vaddr=" << vaddr << " length=" << length << " fileID=" << fileID);
			returnval = SysWrite(fileID,vaddr,length);
			break;
		case SC_Close:		/*zhong_expan + */
			//fileID = kernel->machine->ReadRegister(4);	/* Liang DePeng - */
			fileID = (* kernel->readRegister)(4);	/* Liang DePeng + */
			DEBUG(dbgSys,"System Call: Close fileID=" << fileID);
			returnval = SysClose(fileID);
			break;
		case SC_Exec:		/*zhong_expan + */
			//vaddr = kernel->machine->ReadRegister(4);	/* Liang DePeng - */
			vaddr = (* kernel->readRegister)(4);	/* Liang DePeng + */
			DEBUG(dbgSys,"System Call: Exec vaddr=" << vaddr);
			returnval = SysExec(vaddr);
			break;
		case SC_Join:
			//childId = kernel->machine->ReadRegister(4);	/* Liang DePeng - */	
			childId = (* kernel->readRegister)(4);	/* Liang DePeng + */	
			DEBUG(dbgSys,"System Call: Join pid=" << childId);
			returnval = SysJoin(childId);
			break;
		case SC_Exit:
			//status = kernel->machine->ReadRegister(4);	/* Liang DePeng - */	
			status = (* kernel->readRegister)(4);	/* Liang DePeng + */
			// we require the status value to be non-negative, since
			// negative values are used to indicate errors during Join
			// If the status is negative, make it positive.
			DEBUG(dbgSys,"System Call: Exit status=" << status);
			if (status < 0) status *= (-1);
			ASSERT(status >= 0);
			Terminator(status);
			//SysHalt();
			break;
		default:
			std::cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;

	default:
		std::cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}

	kernel->currentThread->space->RestoreState();
	kernel->currentThread->RestoreUserState();

	// if this was a system call, 
	//  place the return value in r2, and
	// increment the PC before returning
	if (which == SyscallException)
	{
		/* set previous programm counter (debugging only)*/
		//kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));	/* Liang DePeng - */
		(* kernel->writeRegister)(prevPCReg, (* kernel->readRegister)(pCReg));	/* Liang DePeng + */
		
		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/	
		//kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);	/* Liang DePeng - */
		(* kernel->writeRegister)(pCReg, (* kernel->readRegister)(pCReg) + 4);	/* Liang DePeng + */

		/* set next programm counter for brach execution */
		//kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);	/* Liang DePeng - */
		(* kernel->writeRegister)(nextPCReg, (* kernel->readRegister)(pCReg)+4);	/* Liang DePeng + */

		// we always place a return value into the register,
		// even if the system call does not require one.
		// this just simplifies the code....
		//kernel->machine->WriteRegister(2,returnval);	/* Liang DePeng - */
		(* kernel->writeRegister)(2, returnval);	/* Liang DePeng + */
	}
}
