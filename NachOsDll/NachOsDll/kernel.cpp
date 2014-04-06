// kernel.cc 
//	Initialization and cleanup routines for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
//#include "main.h"		/* Liang DePeng - */
#include "kernel.h"
#include "sysdep.h"
#include "synch.h"
#include "synchlist.h"
#include "libtest.h"
#include "string.h"
#include "synchconsole.h"
#include "synchdisk.h"
//#include "post.h"		/* Liang DePeng - */
#include "synchbitmap.h"	/*zhong_expan + */
#include "proctable.h"	/*zhong_expan + */
using namespace std;







/**** add
 * Liang DePeng + */
class __declspec(dllexport) MyKernel: public Kernel {	
public:
    MyKernel(int argc, char **argv,
			char *mainMemoryy,
			int numTotalRegss,
			int memorySizee,
			int diskNumSectorss,
			int diskSectorSizee,
			DiskReadRequest diskReadRequestt,		
			DiskWriteRequest diskWriteRequestt,
			InitializeDisk initDiskk,
			MachineRun machineRunn,
			InterruptHalt interruptHaltt,
			RestoreMachineState restoreStatee,
			SetInterruptLevel setLevell, 
			GetInterruptLevel getLevell, 
			InterruptIdle idlee,
			ReadVMRegister readRegisterr, 
			WriteVMRegister writeRegisterr):Kernel(argc, argv,
													mainMemoryy,
													numTotalRegss,
													memorySizee,
													diskNumSectorss,
													diskSectorSizee,
													diskReadRequestt,
													diskWriteRequestt,
													initDiskk,
													machineRunn,
													interruptHaltt,
													restoreStatee,
													setLevell, 
													getLevell, 
													idlee,
													readRegisterr, 
													writeRegisterr){};

    					// Interpret command line arguments
	~MyKernel();		        // deallocate the kernel
   
    void Initialize(int numPhysPagess,  
				AlarmCallBack alarmCallBackk,
				void (* initConsoleInput)(char *inputFile, CallBackObj *callTo),
				void (* initConsoleOutput)(char *outputFile, CallBackObj *callTo),
				void (* initTimer)(bool doRandom, CallBackObj *toCall),
				ConGetChar getCharr,
				ConPutChar putCharr); 		
				// initialize the kernel -- separated
				// from constructor because 
				// refers to "kernel" as a global
	
    void ThreadSelfTest(); // self test of threads and synchronization

	void ConsoleTest() ;      // interactive console self test
};

Kernel *kernel; /* Liang DePeng + */

extern "C" {
	__declspec(dllexport) Kernel* GetKernelF(int argc, char **argv,
											char *mainMemoryy,
											int numTotalRegss,
											int memorySizee,
											int diskNumSectorss,
											int diskSectorSizee,
											DiskReadRequest diskReadRequestt,		
											DiskWriteRequest diskWriteRequestt,
											InitializeDisk initDiskk,
											MachineRun machineRunn,
											InterruptHalt interruptHaltt,
											RestoreMachineState restoreStatee,
											SetInterruptLevel setLevell, 
											GetInterruptLevel getLevell, InterruptIdle idlee,
											ReadVMRegister readRegisterr, WriteVMRegister writeRegisterr){
		kernel =  new MyKernel(argc, argv,
							mainMemoryy,
							numTotalRegss,
							memorySizee,
							diskNumSectorss,
							diskSectorSizee,
							diskReadRequestt,
							diskWriteRequestt,
							initDiskk,
							machineRunn,
							interruptHaltt,
							restoreStatee,
							setLevell, 
							getLevell, 
							idlee,
							readRegisterr, 
							writeRegisterr);

		return kernel;
	}
	
	__declspec(dllexport) void DeleteKernelF(Kernel* k){
		delete k;
	}
}
/* Liang DePeng +
 ****/





//----------------------------------------------------------------------
// Kernel::Kernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

Kernel::Kernel(int argc, char **argv, 
				char *mainMemoryy,
				int numTotalRegss,
				int memorySizee,
				int diskNumSectorss,
				int diskSectorSizee,
				DiskReadRequest diskReadRequestt,		
				DiskWriteRequest diskWriteRequestt,
				InitializeDisk initDiskk,
				MachineRun machineRunn,
				InterruptHalt interruptHaltt,
				RestoreMachineState restoreStatee,
				SetInterruptLevel setLevell, 
				GetInterruptLevel getLevell, InterruptIdle idlee,
				ReadVMRegister readRegisterr, WriteVMRegister writeRegisterr)
{			
	this->diskReadRequest = diskReadRequestt;	/* Liang DePeng + */
	this->diskWriteRequest = diskWriteRequestt;	/* Liang DePeng + */
	this->initDisk = initDiskk;	/* Liang DePeng + */

	this->diskSectorSize = diskSectorSizee;	/* Liang DePeng + */
	this->diskNumSectors = diskNumSectorss;	/* Liang DePeng + */
	this->machineRun = machineRunn;			/* Liang DePeng + */
	this->interruptHalt = interruptHaltt;	/* Liang DePeng + */
	this->memorySize = memorySizee;		/* Liang DePeng + */
	this->restoreState = restoreStatee;		/* Liang DePeng + */
	this->numTotalRegs = numTotalRegss;		/* Liang DePeng + */
	this->mainMemory = mainMemoryy;		/* Liang DePeng + */
	this->setLevel = setLevell;	/* Liang DePeng + */
	this->getLevel = getLevell;	/* Liang DePeng + */
	this->idle = idlee;			/* Liang DePeng + */
	this->readRegister = readRegisterr;		/* Liang DePeng + */
	this->writeRegister = writeRegisterr;	/* Liang DePeng + */

    randomSlice = FALSE; 
    debugUserProg = FALSE;
    consoleIn = NULL;          // default is stdin
    consoleOut = NULL;         // default is stdout
//#ifndef FILESYS_STUB
    formatFlag = FALSE;
//#endif
    //reliability = 1;  // network reliability, default is 1.0	/* Liang DePeng - */
    //hostName = 0;    // machine id, also UNIX socket name		/* Liang DePeng - */
                      // 0 is the default machine id
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-rs") == 0) {
 			ASSERT(i + 1 < argc);
			RandomInit(atoi(argv[i + 1]));// initialize pseudo-random
						// number generator
			randomSlice = TRUE;
			i++;
        } else if (strcmp(argv[i], "-s") == 0) {
            debugUserProg = TRUE;
		} else if (strcmp(argv[i], "-ci") == 0) {
			ASSERT(i + 1 < argc);
			consoleIn = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-co") == 0) {
			ASSERT(i + 1 < argc);
			consoleOut = argv[i + 1];
			i++;
//#ifndef FILESYS_STUB
		} else if (strcmp(argv[i], "-f") == 0) {
			formatFlag = TRUE;


//#endif
        }/* else if (strcmp(argv[i], "-n") == 0) {		
            ASSERT(i + 1 < argc);   // next argument is float
            reliability = atof(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-m") == 0) {
            ASSERT(i + 1 < argc);   // next argument is int
            hostName = atoi(argv[i + 1]);
            i++;
        } */else if (strcmp(argv[i], "-u") == 0) {
            cout << "Partial usage: nachos [-rs randomSeed]\n";
			cout << "Partial usage: nachos [-s]\n";
            cout << "Partial usage: nachos [-ci consoleIn] [-co consoleOut]\n";

//#ifndef FILESYS_STUB											/* Liang DePeng - */	
//			cout << "Partial usage: nachos [-nf]\n";			/* Liang DePeng - */	
//#endif														/* Liang DePeng - */	
//            cout << "Partial usage: nachos [-n #] [-m #]\n";	/* Liang DePeng - */	
		}
    }
}

//----------------------------------------------------------------------
// Kernel::Initialize
// 	Initialize Nachos global data structures.  Separate from the 
//	constructor because some of these refer to earlier initialized
//	data via the "kernel" global variable.
//----------------------------------------------------------------------

//void Kernel::Initialize(int numPhysPagess){	/* Liang DePeng - */
void MyKernel::Initialize(int numPhysPagess,
						AlarmCallBack alarmCallBackk,
						void (* initConsoleInput)(char *inputFile, CallBackObj *callTo),
						void (* initConsoleOutput)(char *outputFile, CallBackObj *callTo),
						void (* initTimer)(bool doRandom, CallBackObj *toCall),
						ConGetChar getCharr,
						ConPutChar putCharr){	/* Liang DePeng + */

	this->numPhysPages = numPhysPagess;
    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    //currentThread = new Thread("main");		/* Liang DePeng - */		
	currentThread = GetThreadF("main");		/* Liang DePeng + */		
    currentThread->setStatus(RUNNING);

	//stats = new Statistics();		// collect statistics
    //interrupt = new Interrupt;		// start up interrupt handling
    scheduler = new Scheduler();	// initialize the ready queue

    alarm = new Alarm(randomSlice, alarmCallBackk);	// start up time slicing
	(* initTimer)(randomSlice, alarm);	/* Liang DePeng + */


    //machine = new Machine(debugUserProg);
    synchConsoleIn = new SynchConsoleInput(consoleIn, initConsoleInput,getCharr); // input from stdin
    synchConsoleOut = new SynchConsoleOutput(consoleOut, initConsoleOutput, putCharr); // output to stdout
    
	
	frameMap = new SynchBitmap(numPhysPagess);	/*zhong_expan + */
    
	//procTable = new ProcTable();	/*zhong_expan + */	/* Liang DePeng - */
	procTable = GetProcTableF();	/* Liang DePeng + */

    synchDisk = new SynchDisk();    //
	(* initDisk)(synchDisk);		/* Liang DePeng + */

//#ifdef FILESYS_STUB					/* Liang DePeng - */	
//    fileSystem = new FileSystem();	/* Liang DePeng - */	
//#else									/* Liang DePeng - */	
    //fileSystem = new FileSystem(formatFlag);		/* Liang DePeng - */	
	fileSystem = GetFileSystemF(formatFlag);		/* Liang DePeng + */	

//#endif // FILESYS_STUB								/* Liang DePeng - */	
    //postOfficeIn = new PostOfficeInput(10);			 /* Liang DePeng - */			
    //postOfficeOut = new PostOfficeOutput(reliability); /* Liang DePeng - */
    //interrupt->Enable();								 /* Liang DePeng - */
}

//----------------------------------------------------------------------
// Kernel::~Kernel
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------

MyKernel::~MyKernel(){
    //delete stats;			/* Liang DePeng - */
	//delete interrupt;		/* Liang DePeng - */
    delete scheduler;
    delete alarm;
    //delete machine;		/* Liang DePeng - */
    delete synchConsoleIn;
    delete synchConsoleOut;
    delete frameMap;	/*zhong_expan + */
    
	//delete procTable;	/*zhong_expan + */	/* Liang DePeng - */
	DeleteProcTableF(procTable);		/* Liang DePeng + */

    delete synchDisk;
    //delete fileSystem;	/* Liang DePeng - */
	DeleteFileSystemF(fileSystem);		/* Liang DePeng + */
    //delete postOfficeIn;	/* Liang DePeng - */
    //delete postOfficeOut;	/* Liang DePeng - */
    
    MyExit(0);
}

//----------------------------------------------------------------------
// Kernel::ThreadSelfTest
//      Test threads, semaphores, synchlists
//----------------------------------------------------------------------
class MyInt: public Item{
public:
	MyInt(int val){
		this->value = val;
	}
	int value;
};


int fakeCompare (Item *x, Item *y){	
	return 0;
}

//void Kernel::ThreadSelfTest() {	/* Liang DePeng - */
void MyKernel::ThreadSelfTest() {	/* Liang DePeng + */
   FSemaphore *semaphore;
   //SynchList<int> *synchList;	/* Liang DePeng - */
   SynchList* synchList;		/* Liang DePeng + */

   LibSelfTest();		// test library routines
   
   currentThread->SelfTest();	// test thread switching
   				// test semaphore operation
   //semaphore = new Semaphore("test", 0);		/* Liang DePeng - */
   semaphore = GetSemaphoreF("test", 0);		/* Liang DePeng + */

   semaphore->SelfTest();
   DeleteSemaphoreF(semaphore);		/* Liang DePeng + */
   //delete semaphore;					/* Liang DePeng - */
   
   				// test locks, condition variables
				// using synchronized lists
   //synchList = new SynchList<int>;	/* Liang DePeng - */
   //synchList->SelfTest(9);			/* Liang DePeng - */
   //delete synchList;					/* Liang DePeng - */

   synchList = GetSynchListF(fakeCompare);	/* Liang DePeng + */
   MyInt myint(9);								/* Liang DePeng + */
   synchList->SelfTest(&myint);					/* Liang DePeng + */
   DeleteSynchListF(synchList);				/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Kernel::ConsoleTest
//      Test the synchconsole
//----------------------------------------------------------------------

//void Kernel::ConsoleTest() {	/* Liang DePeng - */
void MyKernel::ConsoleTest() {	/* Liang DePeng + */
    char ch;

    cout << "Testing the console device.\n" 
        << "Typed characters will be echoed, until ^D is typed.\n"
        << "Note newlines are needed to flush input through UNIX.\n";
    cout.flush();

    do {
        ch = synchConsoleIn->GetChar();
        if(ch != EOF) synchConsoleOut->PutChar(ch);   // echo it!
    } while (ch != EOF);

    cout << "\n";

}

//----------------------------------------------------------------------
// Kernel::NetworkTest
//      Test whether the post office is working. On machines #0 and #1, do:
//
//      1. send a message to the other machine at mail box #0
//      2. wait for the other machine's message to arrive (in our mailbox #0)
//      3. send an acknowledgment for the other machine's message
//      4. wait for an acknowledgement from the other machine to our 
//          original message
//
//  This test works best if each Nachos machine has its own window
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/* move to MIPSVirtualMachine project post.h
void Kernel::NetworkTest() {

    if (hostName == 0 || hostName == 1) {
        // if we're machine 1, send to 0 and vice versa
        int farHost = (hostName == 0 ? 1 : 0); 
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "Hello there!";
        char *ack = "Got it!";
        char buffer[MaxMailSize];

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = farHost;         
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;

        // Send the first message
        postOfficeOut->Send(outPktHdr, outMailHdr, data); 

        // Wait for the first message from the other machine
        postOfficeIn->Receive(0, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box " 
                                                << inMailHdr.from << "\n";
        cout.flush();

        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.length = strlen(ack) + 1;
        postOfficeOut->Send(outPktHdr, outMailHdr, ack); 

        // Wait for the ack from the other machine to the first message we sent
		postOfficeIn->Receive(1, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box " 
                                                << inMailHdr.from << "\n";
        cout.flush();
    }

    // Then we're done!
}*/
/* Liang DePeng -
 ****/
