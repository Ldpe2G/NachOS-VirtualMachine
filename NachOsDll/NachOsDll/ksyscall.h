/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#include "kernel.h"
#include "synchconsole.h"	/*zhong_expan + */
#include "proctable.h"	/*zhong_expan + */

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 



void SysHalt(){
  //kernel->interrupt->Halt();	/* Liang DePeng - */
	(* kernel->interruptHalt)();	/* Liang DePeng + */
}


int SysAdd(int op1, int op2){
  return op1 + op2;
}

//--------------------------------------------------------------------
// ProcessStartup
//
//   This is the function run by each new thread when it is Forked.
//   By the time this function is invoked, the new thread should
//   already have an address space and a process identifier.
//   All we need to do is initialize the machine and begin
//   simulation of the user program that is already loaded
//   into the address space
//--------------------------------------------------------------------
static void ProcessStartup(int dummy){
	kernel->currentThread->space->InitRegisters();
	kernel->currentThread->space->RestoreState();
	(* kernel->machineRun)();	/* Liang DePeng + */
	//kernel->machine->Run();	/* Liang DePeng - */
	ASSERT(FALSE);
}

//----------------------------------------------------------------------
// Terminator
//    A helper function to terminate execution of the process that
//    calls it.
//
//    "status" is the desired exit status for the terminating process
//
//    Control never returns from the function
//----------------------------------------------------------------------
void Terminator(int status)	/*zhong_expan + */
{
	int id;
	id = kernel->currentThread->space->id;

	// tell the process table that this process is done,
	// and record its status value
	kernel->procTable->ProcIsDone(id,status);

	// delete the address space - this closes any files opened
	// by this process and releases the physical memory it used
	delete kernel->currentThread->space;
	kernel->currentThread->Finish();
	ASSERT(FALSE);
}

//----------------------------------------------------------------------
// SysCreate
//    A helper function to implement the Create system call
//
//    "vaddr" is the address of the filename argument
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysCreate(int vaddr)	/*zhong_expan + */
{
	char *buf;
	int rval;

	buf = new char[MaxStringArgLength];
	// read user-provided file name into kernel buffer
	if (kernel->currentThread->space->UserStringToKernel(vaddr,buf) < 0)
	{
		// problem gettting the filename from user space
		DEBUG(dbgSys,"Error reading file name from user address space");
		rval = -1;
	}
	else
	{
		// create the file
#ifdef FILESYS_STUB
		if ( kernel->fileSystem->Create(buf) )
		{ 
			rval = 0;
		}
		else
		{
			DEBUG(dbgSysCall,"Error creating the file");
			rval = -1;
		}
#else
		if ( kernel->fileSystem->Create(buf,0) )
		{ 
			rval = 0;
		}
		else
		{
			DEBUG(dbgSys,"Error creating the file");
			rval = -1;
		}
#endif
	}
	delete buf;
	return(rval);
}

//----------------------------------------------------------------------
// SysOpen
//    A helper function to implement the Open system call
//
//    "vaddr" is the address of the filename argument
//    "modeaddr" is the address of the string indicating the
//        mode that the file should be opened with
//        e.g., read, read/write, write, append
//        NOTE: this is currently ignored.
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysOpen(int vaddr, int modeaddr)	/*zhong_expan + */
{
	OpenNaFile* file;
	int fileID;
	char* buf;

	buf = new char[MaxStringArgLength];
	// read user-provided file name into kernel buffer
	if (kernel->currentThread->space->UserStringToKernel(vaddr,buf) < 0)
	{
		// problem reading file name from user space
		DEBUG(dbgSys,"Error reading file name from user address space");
		fileID = -1;
	}
	else
	{
		// open the file
		file = kernel->fileSystem->Open(buf);
		if (file != (OpenNaFile*)0)
		{
			// put the open file into the open file
			// table for this address space
			fileID = kernel->currentThread->space->AddOpenFile(file);
			if (fileID < 0)
			{
				// there was no room in the open file table
				delete file;
				DEBUG(dbgSys,"Error obtaining file ID - process file table full?");
				fileID = -1;
			}
		}
		else
		{
			// problem opening the specified file
			DEBUG(dbgSys,"Error opening the file");
			fileID = -1;
		}
	}
	delete buf;
	return(fileID);
}

//----------------------------------------------------------------------
// SysRead
//    A helper function to implement the Read system call
//
//    "fileID" identifies the file to read from
//    "vaddr" is the address of the caller's buffer
//    "length" is the length of the caller's buffer
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysRead(int fileID, int vaddr, int length)		/*zhong_expan + */
{
	int rval;
	OpenNaFile* file;
	char* buf;

	if( fileID == ConsoleOutId) 
	{
		// attempt to read from the output console
		// This is an error! 
		DEBUG(dbgSys,"Attempt to read from output console.");
		rval = -1;
	} 
	else if (length < 0) 
	{
		DEBUG(dbgSys,"Negative read length.");
		rval = -1;
	} 
	else if (length == 0 ) 
	{
		// Nothing to do, but not a device or OS error
		DEBUG(dbgSys,"Zero read length.");
		rval = 0;
	} 
	else if (fileID == ConsoleInId) 
	{
		// Read from the input console.
		// Would be better (and more realistic) to read
		// the data in chunks if length is large.
		// However, we don't do that here...
		buf = new char[length];
		// read data from console into kernel buffer
		DEBUG(dbgSys,"Input console read.");
		rval = kernel->synchConsoleIn->ConsoleToKernel(length,buf);
		// move data from kernel buffer to user virtual address space
		if (kernel->currentThread->space->KernelToUserBuf(vaddr,rval,buf) != rval) 
		{
			DEBUG(dbgSys,"Error copying data to user address space");
			rval = -1;
		} 
		delete buf;
	} 
	else 
	{
		// Read from a file...
		// find the open file that corresponds to the file ID
		// passed to the kernel by the user process
		DEBUG(dbgSys,"File read.");
		file = kernel->currentThread->space->GetOpenFile(fileID);
		if (file != (OpenNaFile*)0) 
		{
			// if we get here, we were passed a legit file ID
			// everything looks good - time to read the data
			buf = new char[length];
			// read data from the file into the user buffer
			// rval is the number of bytes actually read
			// Would be better (and more realistic) to read
			// the data in chunks if length is large.
			// However, we don't do that here...
			rval = file->Read(buf,length);
			ASSERT(rval >= 0 && rval <= length);
			// copy data from kernel buffer to user buffer
			if (kernel->currentThread->space->KernelToUserBuf(vaddr,rval,buf) != rval) 
			{
				DEBUG(dbgSys,"Error copying data to user address space");
				rval = -1;
			}
			delete buf;
		} 
		else 
		{
			// problem with fileID supplied by caller
			DEBUG(dbgSys,"Invalid file ID.");
			rval = -1;
		}
	}
	return(rval);
}

//----------------------------------------------------------------------
// SysWrite
//    A helper function to implement the Write system call
//
//    "fileID" identifies the file to write from
//    "vaddr" is the address of the caller's buffer
//    "length" is the length of the caller's buffer
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysWrite(int fileID, int vaddr, int length)		/*zhong_expan + */
{
	int rval;
	OpenNaFile* file;
	char* buf;

	if (fileID == ConsoleInId)
	{
		// Write to the console input
		// this is an error
		DEBUG(dbgSys,"Attempt to write to input console.");
		rval = -1;
	}
	else if (length < 0)
	{
		DEBUG(dbgSys,"Negative write length.");
		rval = -1;
	}
	else if (length == 0)
	{
		// Nothing to do, but not a system or OS error
		DEBUG(dbgSys,"Zero write length.");
		rval = 0;
	}
	else if (fileID == ConsoleOutId)
	{
		// Write to the console...
		buf = new char[length];
		// get the user string into a buffer
		// Would be better (and more realistic) to copy
		// the data in chunks if length is large.
		// However, we don't do that here...
		DEBUG(dbgSys,"Output console write.");
		if (kernel->currentThread->space->UserBufToKernel(vaddr,length,buf)
			!= length)
		{
			// problem reading in user's buffer
			DEBUG(dbgSys,"Error copying data from user address space");
			rval = -1;
		}
		else
		{
			// write the buffer contents to the console
			kernel->synchConsoleOut->KernelToConsole(length,buf);
			rval = length;
		}
		delete buf;
	}
	else
	{
		// Write to a file...
		// find the open file that corresponds to the file ID
		// passed to the kernel by the user process
		DEBUG(dbgSys,"File write.");
		file = kernel->currentThread->space->GetOpenFile(fileID);
		if (file != (OpenNaFile*)0)
		{
			// if we get here, we were passed a legit file ID
			// everything looks good - time to write the data
			buf = new char[length];
			// get the user data into a buffer
			// Would be better (and more realistic) to copy
			// the data in chunks if length is large.
			// However, we don't do that here...
			if (kernel->currentThread->space->UserBufToKernel(vaddr,length,buf) != length){
				// problem reading in the user's buffer
				DEBUG(dbgSys,"Error copying data from user address space");
				rval = -1;
			}
			else
			{
				// write the data to the file
				rval = file->Write(buf,length);
				ASSERT(rval >= 0 && rval <= length);
			}
			delete buf;
		}
		else
		{
			// problem with fileID passed by caller
			DEBUG(dbgSys,"Invalid file ID.");
			rval = -1;
		}
	}
	return(rval);
}

//----------------------------------------------------------------------
// SysClose
//    A helper function to implement the Close system call
//
//    "fileID" identifies the file to close
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysClose(int fileID)		/*zhong_expan + */
{
	int rval;
	OpenNaFile* file;

	// first, find the open file on the open file list
	// and remove it from the list
	file = kernel->currentThread->space->DeleteOpenFile(fileID);
	// if we found the open file, close it
	if (file != (OpenNaFile*)0)
	{
		delete file;
		rval = 0;
	}
	else
	{
		DEBUG(dbgSys,"Invalid file ID.");
		rval = -1;
	}
	return(rval);
}

//----------------------------------------------------------------------
// SysExec
//    A helper function to implement the Exec system call
//
//    "vaddr" is the address of the filename argument
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysExec(int vaddr)		/*zhong_expan + */
{
	int rval;
	int childId,myId;
	OpenNaFile *file;
	char *buf;
	AddrSpace* space;
	Thread* newthread;
	char threadname[MAX_THREAD_NAME_LEN+1];

	myId = kernel->currentThread->space->id;
	buf = new char[MaxStringArgLength];
	if (kernel->currentThread->space->UserStringToKernel(vaddr,buf) < 0)
	{
		// problem getting the file name from user space
		DEBUG(dbgSys,"Error reading file name from user address space");
		rval = -1;
	}
	else
	{
		DEBUG(dbgSys,"Exec file name is: " << buf );
		// open the file specified by the user
		file = kernel->fileSystem->Open(buf);
		if (file != (OpenNaFile*)0)
		{
			// assign an identifier to the new process
			// and get a space for it in the process table
			childId = kernel->procTable->GetNewProcId(myId);
			if (childId >= 0)
			{
				// create a new address space for the new process
				DEBUG(dbgSys,"Exec new process ID is: " << childId);
				
				//space = new AddrSpace(file,kernel->frameMap,childId);						/* Liang DePeng - */
				space = GetAddrSpaceF(file,kernel->frameMap,childId,kernel->numPhysPages);	/* Liang DePeng + */

				ASSERT(space != (AddrSpace*)0);
				// check that the address space is OK
				if (space->id == childId)
				{
					// build a name for the new thread - for debugging purposes
					// for now, combine name of executable file and process Id
					// to make a thread name (which may not be unique)
					// feel free to change this
					_snprintf(threadname,MAX_THREAD_NAME_LEN+1,"%d(%s)",childId,buf);
					// create a new thread and assign the address space to it

					//newthread = new Thread(threadname);	/* Liang DePeng - */
					newthread = GetThreadF(threadname);	/* Liang DePeng + */

					ASSERT(newthread != (Thread*)0);
					newthread->space = space;
					// put the new thread on the ready queue
					newthread->Fork((VoidFunctionPtr) ProcessStartup, (void *) 0);
					// we have been successful 
					// the return value should be the Space ID
					rval = childId;
				}
				else
				{
					// address space not initialized properly - probably
					// because of lack of physical memory.  
					delete space;
					DEBUG(dbgSys,"Error initializing address space - memory full?");
					rval = -1;
				}
			}
			else
			{
				// could not get new process id
				DEBUG(dbgSys,"Error obtaining new process ID");
				rval = -1;
			}
			delete file;  // close the executable file
		}
		else
		{
			// could not open program file
			DEBUG(dbgSys,"Error opening program file");
			rval = -1;
		}
	}
	delete buf;
	return(rval);
}

//----------------------------------------------------------------------
// SysJoin
//    A helper function to implement the Join system call
//
//    "childId" is the id of the process to join
//
//    returns system call exit status
//----------------------------------------------------------------------
int SysJoin(int childId)
{
	int rval;
	int myId,status;

	myId = kernel->currentThread->space->id;
	// Join will return the exit status of the child process. If
	// If the child is not finished, the caller will wait until it is.
	// If GetProcStatus returns FALSE, the specified process is
	//  not actually a child of the calling process.
	if (kernel->procTable->GetProcStatus(childId,myId,&status) == TRUE)
	{
		ASSERT(status >= 0);
		rval = status;
	}
	else
	{
		// invalid process id
		DEBUG(dbgSys,"Invalid process ID");
		rval = -1;
	}
	return(rval);
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
