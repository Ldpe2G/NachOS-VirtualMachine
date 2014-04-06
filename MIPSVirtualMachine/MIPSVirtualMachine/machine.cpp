// machine.cc 
//	Routines for simulating the execution of user programs.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "machine.h"
#include "main.h"

using namespace std;


//move from translate.cpp

//----------------------------------------------------------------------
// Machine::ReadMem
//      Read "size" (1, 2, or 4) bytes of virtual memory at "addr" into 
//	the location pointed to by "value".
//
//   	Returns FALSE if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to read from
//	"size" -- the number of bytes to read (1, 2, or 4)
//	"value" -- the place to write the result
//----------------------------------------------------------------------

bool Machine::ReadMem(int addr, int size, int *value){
    int data;
    ExceptionType exception;
    int physicalAddress;
    
    DEBUG(dbgAddr, "Reading VA " << addr << ", size " << size);
    
    exception = Translate(addr, &physicalAddress, size, FALSE);
    if (exception != NoException) {
		RaiseException(exception, addr);
		return FALSE;
    }
    switch (size) {
      case 1:
		data = mainMemory[physicalAddress];
		*value = data;
		break;
	
      case 2:
		data = *(unsigned short *) &mainMemory[physicalAddress];
		//*value = ShortToHost(data);		/* Liang DePeng - */
		*value = (* fpShortToHost)(data);	/* Liang DePeng + */
		break;
	
      case 4:
		data = *(unsigned int *) &mainMemory[physicalAddress];
		//*value = WordToHost(data);		/* Liang DePeng - */
		*value = (* fpWordToHost)(data);	/* Liang DePeng + */
		break;

      default: ASSERT(FALSE);
    }
    
    DEBUG(dbgAddr, "\tvalue read = " << *value);
    return (TRUE);
}

//----------------------------------------------------------------------
// Machine::WriteMem
//      Write "size" (1, 2, or 4) bytes of the contents of "value" into
//	virtual memory at location "addr".
//
//   	Returns FALSE if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to write to
//	"size" -- the number of bytes to be written (1, 2, or 4)
//	"value" -- the data to be written
//----------------------------------------------------------------------

bool
Machine::WriteMem(int addr, int size, int value)
{
    ExceptionType exception;
    int physicalAddress;
     
    DEBUG(dbgAddr, "Writing VA " << addr << ", size " << size << ", value " << value);

    exception = Translate(addr, &physicalAddress, size, TRUE);
    if (exception != NoException) {
		RaiseException(exception, addr);
		return FALSE;
    }
    switch (size) {
      case 1:
		mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
		break;

      case 2:
		*(unsigned short *) &mainMemory[physicalAddress]
			= (* fpShortToMachine)((unsigned short) (value & 0xffff));	/* Liang DePeng + */
			//= ShortToMachine((unsigned short) (value & 0xffff));		/* Liang DePeng - */
		break;
      
      case 4:
		*(unsigned int *) &mainMemory[physicalAddress]
			= (* fpWordToMachine)((unsigned int) value);			/* Liang DePeng + */
			//= WordToMachine((unsigned int) value);				/* Liang DePeng - */
		break;
	
      default: ASSERT(FALSE);
    }
    
    return TRUE;
}

//----------------------------------------------------------------------
// Machine::Translate
// 	Translate a virtual address into a physical address, using 
//	either a page table or a TLB.  Check for alignment and all sorts 
//	of other errors, and if everything is ok, set the use/dirty bits in 
//	the translation table entry, and store the translated physical 
//	address in "physAddr".  If there was an error, returns the type
//	of the exception.
//
//	"virtAddr" -- the virtual address to translate
//	"physAddr" -- the place to store the physical address
//	"size" -- the amount of memory being read or written
// 	"writing" -- if TRUE, check the "read-only" bit in the TLB
//----------------------------------------------------------------------

ExceptionType
Machine::Translate(int virtAddr, int* physAddr, int size, bool writing)
{
    int i;
    unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG(dbgAddr, "\tTranslate " << virtAddr << (writing ? " , write" : " , read"));

// check for alignment errors
    if (((size == 4) && (virtAddr & 0x3)) || ((size == 2) && (virtAddr & 0x1))){
		DEBUG(dbgAddr, "Alignment problem at " << virtAddr << ", size " << size);
		return AddressErrorException;
    }
    
    // we must have either a TLB or a page table, but not both!
    ASSERT(tlb == NULL || pageTable == NULL);	
    ASSERT(tlb != NULL || pageTable != NULL);	

// calculate the virtual page number, and offset within the page,
// from the virtual address
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    
    if (tlb == NULL) {		// => page table => vpn is index into table
		if (vpn >= pageTableSize) {
			DEBUG(dbgAddr, "Illegal virtual page # " << virtAddr);
			return AddressErrorException;
		} else if (!pageTable[vpn]->valid) {
			DEBUG(dbgAddr, "Invalid virtual page # " << virtAddr);
			return PageFaultException;
		}
		entry = pageTable[vpn];
    } else {
        for (entry = NULL, i = 0; i < TLBSize; i++)
    	    if (tlb[i]->valid && (tlb[i]->virtualPage == ((int)vpn))) {
				entry = tlb[i];			// FOUND!
				break;
			}
		if (entry == NULL) {				// not found
    	    DEBUG(dbgAddr, "Invalid TLB entry for this virtual page!");
    	    return PageFaultException;		// really, this is a TLB fault,
						// the page may be in memory,
						// but not in the TLB
		}
    }

    if (entry->readOnly && writing) {	// trying to write to a read-only page
		DEBUG(dbgAddr, "Write to read-only page at " << virtAddr);
		return ReadOnlyException;
    }
    pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong! 
    // An invalid translation was loaded into the page table or TLB. 
    if (pageFrame >= NumPhysPages) { 
		DEBUG(dbgAddr, "Illegal pageframe " << pageFrame);
		return BusErrorException;
    }
    entry->use = TRUE;		// set the use, dirty bits
    if (writing)
		entry->dirty = TRUE;
    *physAddr = pageFrame * PageSize + offset;
    ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
    DEBUG(dbgAddr, "phys addr = " << *physAddr);
    return NoException;
}


// Textual names of the exceptions that can be generated by user program
// execution, for debugging.
static char* exceptionNames[] = { "no exception", "syscall", 
				"page fault/no TLB entry", "page read only",
				"bus error", "address error", "overflow",
				"illegal instruction" };

//----------------------------------------------------------------------
// CheckEndian
// 	Check to be sure that the host really uses the format it says it 
//	does, for storing the bytes of an integer.  Stop on error.
//----------------------------------------------------------------------

static
void CheckEndian()
{
    union checkit {
        char charword[4];
        unsigned int intword;
    } check;

    check.charword[0] = 1;
    check.charword[1] = 2;
    check.charword[2] = 3;
    check.charword[3] = 4;

#ifdef HOST_IS_BIG_ENDIAN
    ASSERT (check.intword == 0x01020304);
#else
    ASSERT (check.intword == 0x04030201);
#endif
}

//----------------------------------------------------------------------
// Machine::Machine
// 	Initialize the simulation of user program execution.
//
//	"debug" -- if TRUE, drop into the debugger after each user instruction
//		is executed.
//----------------------------------------------------------------------

Machine::Machine(bool debug){

    int i;

    for (i = 0; i < NumTotalRegs; i++)
        registers[i] = 0;

    mainMemory = new char[MemorySize];
    for (i = 0; i < MemorySize; i++)
      	mainMemory[i] = 0;
#ifdef USE_TLB
    //tlb = new TranslationEntry[TLBSize];	/* Liang DePeng - */
	tlb = (* getTranslationEntry)();		/* Liang DePeng + */
    for (i = 0; i < TLBSize; i++)
	tlb[i].valid = FALSE;
    pageTable = NULL;
#else	// use linear page table
    tlb = NULL;
    pageTable = NULL;
#endif

    singleStep = debug;
    CheckEndian();
}

//----------------------------------------------------------------------
// Machine::~Machine
// 	De-allocate the data structures used to simulate user program execution.
//----------------------------------------------------------------------

Machine::~Machine(){
	if(mainMemory != NULL)
		delete [] mainMemory;
    if (tlb != NULL)
        delete [] tlb;
}

//----------------------------------------------------------------------
// Machine::RaiseException
// 	Transfer control to the Nachos kernel from user mode, because
//	the user program either invoked a system call, or some exception
//	occured (such as the address translation failed).
//
//	"which" -- the cause of the kernel trap
//	"badVaddr" -- the virtual address causing the trap, if appropriate
//----------------------------------------------------------------------

void Machine::RaiseException(ExceptionType which, int badVAddr){
    DEBUG(dbgMach, "Exception: " << exceptionNames[which]);
    
    registers[BadVAddrReg] = badVAddr;
    DelayedLoad(0, 0);			// finish anything in progress
    //kernel->interrupt->setStatus(SystemMode);		/* Liang DePeng - */
	interrupt->setStatus(SystemMode);				/* Liang DePeng + */
    
	//ExceptionHandler(which);		// interrupts are enabled at this point	/* Liang DePeng - */
	(* fpExceptionHandler)(which);		/* Liang DePeng + */		

    //kernel->interrupt->setStatus(UserMode);		/* Liang DePeng - */
	interrupt->setStatus(UserMode);					/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Machine::Debugger
// 	Primitive debugger for user programs.  Note that we can't use
//	gdb to debug user programs, since gdb doesn't run on top of Nachos.
//	It could, but you'd have to implement *a lot* more system calls
//	to get it to work!
//
//	So just allow single-stepping, and printing the contents of memory.
//----------------------------------------------------------------------

void Machine::Debugger()
{
    char *buf = new char[80];
    int num;
    bool done = FALSE;

    //kernel->interrupt->DumpState();	/* Liang DePeng - */
	interrupt->DumpState();				/* Liang DePeng + */
    DumpState();
    while (!done) {
      // read commands until we should proceed with more execution
      // prompt for input, giving current simulation time in the prompt
      //cout << kernel->stats->totalTicks << ">";		/* Liang DePeng - */
	  cout << stats->totalTicks << ">";				/* Liang DePeng + */
      // read one line of input (80 chars max)
     
	  // cin.get(buf, 80);		/* Liang DePeng - */
	  cin >> buf;				/* Liang DePeng + */

      if (sscanf(buf, "%d", &num) == 1) {
		runUntilTime = num;
		done = TRUE;
      }
      else {
		runUntilTime = 0;
		switch (*buf) {
			case 'n':
			  done = TRUE;
			  break;
			case 'c':
			  singleStep = FALSE;
			  done = TRUE;
			  break;
			case '?':
			  cout << "Machine commands:\n";
			  cout << "    n  execute one instruction\n";
			  cout << "    <number>  run until the given timer tick\n";
			  cout << "    c         run until completion\n";
			  cout << "    ?         print help message\n";
			  break;
			default:
			  cout << "Unknown command: " << buf << "\n";
			  cout << "Type ? for help.\n";
		}
      }
      // consume the newline delimiter, which does not get
      // eaten by cin.get(buf,80) above.
      buf[0] = cin.get();
    }
    delete [] buf;
}
 
//----------------------------------------------------------------------
// Machine::DumpState
// 	Print the user program's CPU state.  We might print the contents
//	of memory, but that seemed like overkill.
//----------------------------------------------------------------------

void
Machine::DumpState()
{
    int i;
    
    cout << "Machine registers:\n";
    for (i = 0; i < NumGPRegs; i++) {
		switch (i) {
		  case StackReg:
			cout << "\tSP(" << i << "):\t" << registers[i];
			break;
	    
		  case RetAddrReg:
			cout << "\tRA(" << i << "):\t" << registers[i];
			break;
	  
		  default:
			cout << "\t" << i << ":\t" << registers[i];
			break;
		}
		if ((i % 4) == 3) { 
			cout << "\n"; 
		}
    }
    
    cout << "\tHi:\t" << registers[HiReg];
    cout << "\tLo:\t" << registers[LoReg];
    cout << "\tPC:\t" << registers[PCReg];
    cout << "\tNextPC:\t" << registers[NextPCReg];
    cout << "\tPrevPC:\t" << registers[PrevPCReg];
    cout << "\tLoad:\t" << registers[LoadReg];
    cout << "\tLoadV:\t" << registers[LoadValueReg] << "\n";
}

//----------------------------------------------------------------------
// Machine::ReadRegister/WriteRegister
//   	Fetch or write the contents of a user program register.
//----------------------------------------------------------------------

int 
Machine::ReadRegister(int num)
{
    ASSERT((num >= 0) && (num < NumTotalRegs));
    return registers[num];
}

void 
Machine::WriteRegister(int num, int value)
{
    ASSERT((num >= 0) && (num < NumTotalRegs));
    registers[num] = value;
}

