// main.cpp
//	Driver code to initialize, selftest, and run the 
//	operating system kernel.  
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//              -s -x <nachos file> -ci <consoleIn> -co <consoleOut>
//              -f -cp <unix file> <nachos file>
//              -p <nachos file> -r <nachos file> -l -D
//              -n <network reliability> -m <machine id>
//              -z -K -C -N
//
//    -d causes certain debugging messages to be printed (see debug.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -ci specify file for console input (stdin is the default)
//    -co specify file for console output (stdout is the default)
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -K run a simple self test of kernel threads and synchronization
//    -C run an interactive console test
//    -N run a two-machine network test (see Kernel::NetworkTest)
//
//    Filesystem-related flags:
//    -f forces the Nachos disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system 
//
//  Note: the file system flags are not used if the stub filesystem
//        is being used
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "main.h"
//#include "kernel.h"  
//#include "filesys.h"		/* Liang DePeng - */
//#include "openfile.h"		/* Liang DePeng - */
#include "sysdep.h"
//#include "proctable.h"	/* Liang DePeng - */

#include "disk.h"		/* Liang DePeng + */				
#include "console.h"	/* Liang DePeng + */	
#include "timer.h"		/* Liang DePeng + */

#include<iostream>
using namespace std;

#ifdef TUT

#include "tut.h"
//implements callbacks for unit test
#include "tut_reporter.h"
namespace tut
{
    test_runner_singleton runner;
}

#endif TUT

// global variables
//Kernel *kernel;			/* Liang DePeng - */


/**** add
 * Liang DePeng + */
Debug *debug;		
HMODULE hDLL;
Interrupt *interrupt;     
Statistics *stats;			
Machine *machine;	
PostOfficeInput *postOfficeIn;
PostOfficeOutput *postOfficeOut;
double reliability;        
int hostName;	

Kernel* kernel;

//debug.h function pointers
CreateDebug createDebug;
DeleteDebug deleteDebug;


//kernel.h
GetKernel getKernel;
DeleteKernel deleteKernel;	

//addrspace.h
GetAddrSpace getAddrSpace;
DeleteAddrSpace deleteAddrSpace;	

//openfile.h
GetOpenNaFile getOpenNaFile;
DeleteOpenNaFile deleteOpenNaFile;	

//thread.h
GetThread getThread;
DeleteThread deleteThread;	

//list.h
GetSortedList getList;
DeleteSortedList deleteList;	

//syscall.h
FPExceptionHandler fpExceptionHandler;

//translate.h
GetTranslationEntry getTranslationEntry;
DeleteTranslationEntry deleteTranslationEntry;
FPWordToHost fpWordToHost;
FPShortToHost fpShortToHost;
FPWordToMachine fpWordToMachine;
FPShortToMachine fpShortToMachine;

//synchlist.h
GetSynchList getSynchList;
DeleteSynchList deleteSynchList;	

//synch.h
GetSemaphore getSemaphore;
DeleteSemaphore deleteSemaphore;	
GetLock getLock;
DeleteLock deleteLock;	

//sysdep.h  function pointers
FpAbort fp_Abort;
FpExit fp_Exit;
FpDelay fp_Delay;
FpUDelay fp_UDelay;
FpCallOnUserAbort fp_CallOnUserAbort;
FpRegisterSignalHandler fp_RegisterSignalHandler; 
FpRandomInit fp_RandomInit;
FpRandomNumber fp_RandomNumber;
FpAllocBoundedArray fp_AllocBoundedArray;
FpDeallocBoundedArray fp_DeallocBoundedArray;
FpPollFile fp_PollFile;
FpPollConsole fp_PollConsole;		
FpOpenForWrite fp_OpenForWrite;
FpOpenForReadWrite fp_OpenForReadWrite;
FpRead fp_Read;
FpReadPartial fp_ReadPartial;
FpWriteFkFile fp_WriteFkFile;
FpLseek fp_Lseek;
FpTell fp_Tell;
FpClose fp_Close;
FpUnlink fp_Unlink;
Fpbcopy fp_bcopy;
Fpbzero fp_bzero;
FpOpenPipe fp_OpenPipe;	 
FpClosePipe fp_ClosePipe;		
FpPollPipe fp_PollPipe;		
FpReadFromPipe fp_ReadFromPipe;		
FpSendToPipe fp_SendToPipe;

void initPointers(HMODULE hDLL){
	//sysdep.h
	fp_Abort = FpAbort(GetProcAddress(hDLL, "Abort"));
	fp_Exit = FpExit(GetProcAddress(hDLL, "MyExit"));
	fp_Delay = FpDelay(GetProcAddress(hDLL, "Delay"));
	fp_UDelay = FpUDelay(GetProcAddress(hDLL, "UDelay"));
	fp_CallOnUserAbort = FpCallOnUserAbort(GetProcAddress(hDLL, "CallOnUserAbort"));
	fp_RegisterSignalHandler = FpRegisterSignalHandler(GetProcAddress(hDLL, "RegisterSignalHandler")); 
	fp_RandomInit = FpRandomInit(GetProcAddress(hDLL, "RandomInit"));
	fp_RandomNumber = FpRandomNumber(GetProcAddress(hDLL, "RandomNumber"));
	fp_AllocBoundedArray = FpAllocBoundedArray(GetProcAddress(hDLL, "AllocBoundedArray"));
	fp_DeallocBoundedArray = FpDeallocBoundedArray(GetProcAddress(hDLL, "DeallocBoundedArray"));
	fp_PollFile = FpPollFile(GetProcAddress(hDLL, "PollFile"));
	fp_PollConsole = FpPollConsole(GetProcAddress(hDLL, "PollConsole"));		
	fp_OpenForWrite = FpOpenForWrite(GetProcAddress(hDLL, "OpenForWrite"));
	fp_OpenForReadWrite = FpOpenForReadWrite(GetProcAddress(hDLL, "OpenForReadWrite"));
	fp_Read = FpRead(GetProcAddress(hDLL, "Read"));
	fp_ReadPartial = FpReadPartial(GetProcAddress(hDLL, "ReadPartial"));
	fp_WriteFkFile = FpWriteFkFile(GetProcAddress(hDLL, "WriteFkFile"));
	fp_Lseek = FpLseek(GetProcAddress(hDLL, "Lseek"));
	fp_Tell = FpTell(GetProcAddress(hDLL, "Tell"));
	fp_Close = FpClose(GetProcAddress(hDLL, "MyClose"));
	fp_Unlink = FpUnlink(GetProcAddress(hDLL, "Unlink"));
	fp_bcopy = Fpbcopy(GetProcAddress(hDLL, "bcopy"));
	fp_bzero = Fpbzero(GetProcAddress(hDLL, "bzero"));
	fp_OpenPipe = FpOpenPipe(GetProcAddress(hDLL, "OpenPipe")); 
	fp_ClosePipe = FpClosePipe(GetProcAddress(hDLL, "ClosePipe"));	
	fp_PollPipe = FpPollPipe(GetProcAddress(hDLL, "PollPipe"));		
	fp_ReadFromPipe = FpReadFromPipe(GetProcAddress(hDLL, "ReadFromPipe"));		
	fp_SendToPipe = FpSendToPipe(GetProcAddress(hDLL, "SendToPipe"));

	//synchlist.h
	getSynchList = GetSynchList(GetProcAddress(hDLL, "GetSynchListF"));	
	deleteSynchList = DeleteSynchList(GetProcAddress(hDLL, "DeleteSynchListF"));	

	//synch.h
	getSemaphore = GetSemaphore(GetProcAddress(hDLL, "GetSemaphoreF"));
	deleteSemaphore = DeleteSemaphore(GetProcAddress(hDLL, "DeleteSemaphoreF"));	
	getLock = GetLock(GetProcAddress(hDLL, "GetLockF"));
	deleteLock = DeleteLock(GetProcAddress(hDLL, "DeleteLockF"));	

	//translate.h
	getTranslationEntry = GetTranslationEntry(GetProcAddress(hDLL, "GetTranslationEntryF"));
	deleteTranslationEntry = DeleteTranslationEntry(GetProcAddress(hDLL, "DeleteTranslationEntryF"));
	fpWordToHost = FPWordToHost(GetProcAddress(hDLL, "WordToHost"));
	fpShortToHost = FPShortToHost(GetProcAddress(hDLL, "ShortToHost"));
	fpWordToMachine = FPWordToMachine(GetProcAddress(hDLL, "WordToMachine"));
	fpShortToMachine = FPShortToMachine(GetProcAddress(hDLL, "ShortToMachine"));

	//syscall.h
	fpExceptionHandler = FPExceptionHandler(GetProcAddress(hDLL, "ExceptionHandler"));

	//thread.h
	getThread = GetThread(GetProcAddress(hDLL, "GetThreadF"));
	deleteThread = DeleteThread(GetProcAddress(hDLL, "DeleteThreadF"));

	//list.h
	getList = GetSortedList(GetProcAddress(hDLL, "getSortedList"));
	deleteList = DeleteSortedList(GetProcAddress(hDLL, "deleteSortedList"));	

	//openfile.h
	getOpenNaFile = GetOpenNaFile(GetProcAddress(hDLL, "GetOpenNaFileF"));
	deleteOpenNaFile = DeleteOpenNaFile(GetProcAddress(hDLL, "DeleteOpenNaFileF"));

	//addrspace.h
	getAddrSpace = GetAddrSpace(GetProcAddress(hDLL, "GetAddrSpaceF"));
	deleteAddrSpace = DeleteAddrSpace(GetProcAddress(hDLL, "DeleteAddrSpaceF"));

	//kernel.h
	getKernel = GetKernel(GetProcAddress(hDLL, "GetKernelF"));
	deleteKernel = DeleteKernel(GetProcAddress(hDLL, "DeleteKernelF"));
}

//corresponding to the Kernel function pointers
void machineRun(){
	machine->Run();
}

void interruptHalt(){
	interrupt->Halt();
}

void restoreMachineState(TranslationEntry** pageTable, int numPages){
	machine->pageTable = pageTable;		
    machine->pageTableSize = numPages;	
}

void writeVMRegister(int num, int value){
	machine->WriteRegister(num, value);
}

int readVMRegister(int num){
	return machine->ReadRegister(num);
}

void interruptIdle(){
	interrupt->Idle();
}

int setInterruptLevel(int status){
	return interrupt->SetLevel(status);
}

int getInterruptLevel(){
	return interrupt->getLevel();
}


Disk* disk;

void initDisk(CallBackObj *toCall){
	disk = new Disk(toCall);
}

void diskReadRequest(int sectorNumber, char* data){
	disk->ReadRequest(sectorNumber, data);
}

void diskWriteRequest(int sectorNumber, char* data){
	disk->WriteRequest(sectorNumber, data);
}

void InterruptSchedule(CallBackObj *callTo, int when){
	interrupt->Schedule(callTo, when, TimerInt);
}  

Timer* timer;

void initTimer(bool doRandom, CallBackObj *toCall){
	timer = new Timer(doRandom, toCall);
}



void alarmCallBack(){
	MachineStatus status = interrupt->getStatus();
    if (status != IdleMode) {
		interrupt->YieldOnReturn();
    }
}

ConsoleInput *consoleInput;
ConsoleOutput *consoleOutput;

void initConsoleInput(char *inputFile, CallBackObj *callTo){
	consoleInput = new ConsoleInput(inputFile, callTo);
}

void initConsoleOutput(char *outputFile, CallBackObj *callTo){
	consoleOutput = new ConsoleOutput(outputFile, callTo);	
}

char conGetChar(){
	return consoleInput->GetChar();
}

void conPutChar(char ch){
	consoleOutput->PutChar(ch);
}
/* Liang DePeng +
 ****/



//----------------------------------------------------------------------
// Cleanup
//	Delete kernel data structures; called when user hits "ctl-C".
//----------------------------------------------------------------------

static void 
Cleanup(int x) 
{     
    std::cerr << "\nCleaning up after signal " << x << "\n";
    //delete kernel;	/* Liang DePeng - */
	delete interrupt;	/* Liang DePeng + */
	delete stats;		/* Liang DePeng + */
	delete machine;		/* Liang DePeng + */
}

//-------------------------------------------------------------------
// Constant used by "Copy" and "Print"
//   It is the number of bytes read from the Unix file (for Copy)
//   or the Nachos file (for Print) by each read operation
//-------------------------------------------------------------------
static const int TransferSize = 128;


//#ifndef FILESYS_STUB	/* Liang DePeng - */
//----------------------------------------------------------------------
// Copy
//      Copy the contents of the UNIX file "from" to the Nachos file "to"
//----------------------------------------------------------------------

static void Copy(char *from, char *to)
{
    int fd;
    OpenNaFile* openFile;
    int amountRead, fileLength;
    char *buffer;

// Open UNIX file
    if ((fd =  (* fp_OpenForReadWrite)(from,FALSE)) < 0) {       
        printf("Copy: couldn't open input file %s\n", from);
        return;
    }

// Figure out length of UNIX file
    (* fp_Lseek)(fd, 0, 2);            
    fileLength = (* fp_Tell)(fd);
    (* fp_Lseek)(fd, 0, 0);

// Create a Nachos file of the same length
    DEBUG('f', "Copying file " << from << " of size " << fileLength <<  " to file " << to);
	
    if (!kernel->fileSystem->Create(to, fileLength)) {   // Create Nachos file
        std::cout<<"Copy: couldn't create output file "<<to<<std::endl;
        (* fp_Close)(fd);
        return;
    }
   
    openFile = kernel->fileSystem->Open(to);
    ASSERT(openFile != NULL);
    
// Copy the data in TransferSize chunks
    buffer = new char[TransferSize];
    while ((amountRead = (* fp_ReadPartial)(fd, buffer, sizeof(char)*TransferSize)) > 0){
        openFile->Write(buffer, amountRead);    
	}
    delete [] buffer;
// Close the UNIX and the Nachos files
    //delete openFile;
	(* deleteOpenNaFile)(openFile);
    (* fp_Close)(fd);
}

//#endif // FILESYS_STUB */	/* Liang DePeng - */

//----------------------------------------------------------------------
// Print
//      Print the contents of the Nachos file "name".
//----------------------------------------------------------------------

void
Print(char *name)
{
    OpenNaFile *openFile;    
    int i, amountRead;
    char *buffer;

    if ((openFile = kernel->fileSystem->Open(name)) == NULL) {
        printf("Print: unable to open file %s\n", name);
        return;
    }
    
    buffer = new char[TransferSize];
    while ((amountRead = openFile->Read(buffer, TransferSize)) > 0)
        for (i = 0; i < amountRead; i++)
            printf("%c", buffer[i]);
    delete [] buffer;

    delete openFile;            // close the Nachos file
    return;
}



//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.  
//	
//	Initialize kernel data structures
//	Call some test routines
//	Call "Run" to start an initial user program running
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------


void MyTimerTest(CallBackObj *callTo, int when){
	interrupt->Schedule(callTo, when, TimerInt);
}


int main(int argc, char **argv)
{
    int i;
    char *debugArg = "";
    char *userProgName = NULL;        // default is not to execute a user prog
    bool threadTestFlag = false;
	int threadNum = 0;
    bool consoleTestFlag = false;
    bool networkTestFlag = false;
//#ifndef FILESYS_STUB	/* Liang DePeng - */
    char *copyUnixFileName = NULL;    // UNIX file to be copied into Nachos
    char *copyNachosFileName = NULL;  // name of copied file in Nachos
    char *printFileName = NULL; 
    char *removeFileName = NULL;
    bool dirListFlag = false;
    bool dumpFlag = false;
//#endif //FILESYS_STUB	/* Liang DePeng - */

	//argc = 5;
	//argv[0] = "NachOS";
	//argv[1] = "-f";
	//argv[2] = "-cp";
	//argv[3] = "sort.noff";
	//argv[4] = "sort";

	//argc = 4;
	//argv[0] = "NachOS";
	//argv[1] = "-cp";
	//argv[2] = "console.noff";
	//argv[3] = "console";

	//argc = 4;
	//argv[0] = "NachOS";
	//argv[1] = "-cp";
	//argv[2] = "shell.noff";
	//argv[3] = "shell";

	//argc = 3;
	//argv[0] = "NachOS";
	//argv[1] = "-x";
	//argv[2] = "shell";

	//argc = 2;
	//argv[0] = "NachOS";
	//argv[1] = "-C";

	//argc = 4;
	//argv[0] = "NachOS";
	//argv[1] = "-x";
	//argv[2] = "console";
	//argv[3] = "-s";
	//argv[4] = "-d";
	//argv[5] = "m";

	//argc = 5;
	//argv[0] = "NachOS";
	//argv[1] = "-x";
	//argv[2] = "console1";
	//argv[3] = "-d";
	//argv[4] = "u";

	//argc = 3;
	//argv[0] = "NachOS";
	//argv[1] = "-d";
	//argv[2] = "i";

	//argc = 2;
	//argv[0] = "NachOS";
	//argv[1] = "-K";

	//argc = 4;
	//argv[0] = "NachOS";
	//argv[1] = "-N";
	//argv[2] = "-m";
	//argv[3] = "1";


    // some command line arguments are handled here.
    // those that set kernel parameters are handled in
    // the Kernel constructor
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
			ASSERT(i + 1 < argc);   // next argument is debug string
				debugArg = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-z") == 0) {
				std::cout << copyright << "\n";
		}
		else if (strcmp(argv[i], "-x") == 0) {
			ASSERT(i + 1 < argc);
			userProgName = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-K") == 0) {
			ASSERT(i + 1 < argc);   // next argument is thread number	/* Liang DePeng + */
				threadNum = atoi(argv[i + 1]);							/* Liang DePeng + */
			i++;														/* Liang DePeng + */
			threadTestFlag = TRUE;
		}
		else if (strcmp(argv[i], "-C") == 0) {
			consoleTestFlag = TRUE;
		}
		else if (strcmp(argv[i], "-N") == 0) {
			networkTestFlag = TRUE;
		}
//#ifndef FILESYS_STUB	/* Liang DePeng - */
		else if (strcmp(argv[i], "-cp") == 0) {
			ASSERT(i + 2 < argc);
			copyUnixFileName = argv[i + 1];
			copyNachosFileName = argv[i + 2];
			i += 2;
		}
		else if (strcmp(argv[i], "-p") == 0) {
			ASSERT(i + 1 < argc);
			printFileName = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-r") == 0) {
			ASSERT(i + 1 < argc);
			removeFileName = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-l") == 0) {
			dirListFlag = true;
		}
		else if (strcmp(argv[i], "-D") == 0) {
			dumpFlag = true;
		}
//#endif //FILESYS_STUB	/* Liang DePeng - */
		else if (strcmp(argv[i], "-u") == 0) {
            std::cout << "Partial usage: nachos [-z -d debugFlags]\n";
            std::cout << "Partial usage: nachos [-x programName]\n";
			std::cout << "Partial usage: nachos [-K] [-C] [-N]\n";
//#ifndef FILESYS_STUB	/* Liang DePeng - */
            std::cout << "Partial usage: nachos [-cp UnixFile NachosFile]\n";
            std::cout << "Partial usage: nachos [-p fileName] [-r fileName]\n";
            std::cout << "Partial usage: nachos [-l] [-D]\n";
//#endif //FILESYS_STUB	/* Liang DePeng - */
		}

    }

	/**** add 
	 * Liang DePeng + */
	//load the NachOs Dll, to simulate loading the operating system
	hDLL = LoadLibrary(L"E:\\Github\\Custom--NachOS\\NachOsDll\\Debug\\NachOsDll.dll");

	if(hDLL == NULL){
		std::cout << "can't find the NachOs" <<std::endl<<"please provide the correct path of the NachOS DLL" << std::endl;
		ASSERTNOTREACHED();
	}else{
		createDebug = CreateDebug(GetProcAddress(hDLL,"getDebugInstance"));
		deleteDebug = DeleteDebug(GetProcAddress(hDLL,"deleteDebugInstance"));

		debug = (* createDebug)(debugArg);
		initPointers(hDLL);
	/* Liang DePeng +
	 ****/

		DEBUG(dbgThread, "Entering main");
	
	#ifdef TUT
		::tut::callback * clbk = new tut::reporter(cout);
		::tut::runner.get().set_callback(clbk);  
		::tut::runner.get().run_tests(); //run all unit tests
	#endif 
    
		//kernel = new Kernel(argc, argv);	/* Liang DePeng - */
		//kernel->Initialize();				/* Liang DePeng - */

		/**** add
		 * Liang DePeng + */
		//initialize the machine
		stats = new Statistics();		// collect statistics
		interrupt = new Interrupt(hDLL);// start up interrupt handling
		bool debugUserProg = FALSE;


		reliability = 1;
		hostName = 0;
		 for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-n") == 0) {
				ASSERT(i + 1 < argc);   // next argument is float
				reliability = atof(argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "-m") == 0) {
				ASSERT(i + 1 < argc);   // next argument is int
				hostName = atoi(argv[i + 1]);
				i++;
			}else if (strcmp(argv[i], "-s") == 0){
				debugUserProg = TRUE;
			}
		}
		 std::cout <<debugUserProg<<std::endl;
		machine = new Machine(debugUserProg);

		//initialize the NachOS kernel
		kernel = (* getKernel)(argc, argv,
							   machine->mainMemory,
							   NumTotalRegs,
							   MemorySize,
							   NumSectors,
							   SectorSize,
							   diskReadRequest,
							   diskWriteRequest,
							   initDisk,
							   machineRun,
							   interruptHalt,
							   restoreMachineState,
							   setInterruptLevel,
							   getInterruptLevel,
							   interruptIdle,
							   readVMRegister,
							   writeVMRegister);

		kernel->Initialize(NumPhysPages, alarmCallBack,
							initConsoleInput, initConsoleOutput,initTimer, 
							conGetChar, conPutChar);


		postOfficeIn = new PostOfficeInput(10);
		postOfficeOut = new PostOfficeOutput(reliability);


		interrupt->Enable();
		
		(* fp_CallOnUserAbort)(Cleanup);	/* Liang DePeng + */
		//CallOnUserAbort(Cleanup);		// if user hits ctl-C /* Liang DePeng - */
		
		if (networkTestFlag) {
			// two-machine test of the network
			NetworkTest(postOfficeIn, postOfficeOut, hostName);   
		}

		/* Liang DePeng +
		 ****/
		
		
		// at this point, the kernel is ready to do something
		// run some tests, if requested
		
		if (threadTestFlag) {
			
			kernel->ThreadSelfTest(threadNum);  // test threads and synchronization
		}
		if (consoleTestFlag) {
			kernel->ConsoleTest();   // interactive test of the synchronized console
		}
		//if (networkTestFlag) {	/* Liang DePeng - */
		//  kernel->NetworkTest();   // two-machine test of the network	
		//}							/* Liang DePeng - */

	//#ifndef FILESYS_STUB
		if (removeFileName != NULL) {
			kernel->fileSystem->Remove(removeFileName);
		}
		if (copyUnixFileName != NULL && copyNachosFileName != NULL) {
		  
		  Copy(copyUnixFileName,copyNachosFileName);
		}
		if (dumpFlag) {
		  kernel->fileSystem->Print();
		}
		if (dirListFlag) {
		  kernel->fileSystem->List();
		}
		if (printFileName != NULL) {
		  Print(printFileName);
		}
	

	//#endif // FILESYS_STUB

		// finally, run an initial user program if requested to do so
	 //   if (userProgName != NULL) {		
	 //     AddrSpace *space = new AddrSpace;
	 //     ASSERT(space != (AddrSpace *)NULL);
	 //     if (space->Load(userProgName)) {  // load the program into the space
		//space->Execute();              // run the program
		//ASSERTNOTREACHED();            // Execute never returns
	 //     }
	 //   }
	
	
		

		if (userProgName != NULL){		/*zhong_expan + */
			
			OpenNaFile *executable;
			AddrSpace *space;
			int myId;

			executable = kernel->fileSystem->Open(userProgName);
			if (executable == NULL) {
				cout << "Unable to open program file \n";
			}else{
				// get a slot in the process table for this first process
				// the -1 indicates that this process has no parent
				myId = kernel->procTable->GetNewProcId(-1);
				ASSERT(myId >= 0);
				// make an address space for this first process

				//space = new AddrSpace(executable,kernel->frameMap,myId);				/* Liang DePeng - */
				space = (* getAddrSpace)(executable, kernel->frameMap, myId, PageSize);	/* Liang DePeng + */

				//delete executable;       // close file	/* Liang DePeng - */
				(* deleteOpenNaFile)(executable);			/* Liang DePeng + */			

				// check that the space was actually initialized
				if (space->id < 0){
					// space was not initialized, probably because of lack
					// of memory space.  Nothing to do here but
					// shut down the machine
					(* deleteAddrSpace)(space);
					cout << "Unable to create address space for initial process.\n";
				}else{
					kernel->currentThread->space = space;
					space->InitRegisters();        // set the initial register values
					space->RestoreState();         // load page table register

					machine->Run();        // jump to the user progam
					ASSERT(FALSE);                 // machine->Run never returns;
				}
			}
		}

		// If we don't run a user program, we may get here.
		// Calling "return" would terminate the program.
		// Instead, call Halt, which will first clean up, then
		//  terminate.
		//kernel->interrupt->Halt();  /* Liang DePeng - */
		interrupt->Halt();  /* Liang DePeng + */

		//free the NachOs 

		delete disk;

		(* deleteKernel)(kernel);
		(* deleteDebug)(debug);

		FreeLibrary(hDLL);

		//ASSERTNOTREACHED();
	}
   
}

