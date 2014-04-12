// thread.cc 
//	Routines to manage threads.  These are the main operations:
//
//	Fork -- create a thread to run a procedure concurrently
//		with the caller (this is done in two steps -- first
//		allocate the Thread object, then call Fork on it)
//	Begin -- called when the forked procedure starts up, to turn
//		interrupts on and clean up after last thread
//	Finish -- called when the forked procedure finishes, to clean up
//	Yield -- relinquish control over the CPU to another ready thread
//	Sleep -- relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly 
//		put back on the ready queue.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "sysdep.h"
#include "kernel.h"


extern "C" {

	//由于在调用ThreadRoot时,ecx会被修改,
	//用全局变量StartUp暂存线程启动函数ThreadBegin的地址
	void* StartUp = (void*)0;
	void* IniArg = (void*)0;
	void* Func = (void*)0;
	//void* WhenDone = (void*)0;


	void ThreadRoot()
	{
		kernel->currentThread->Begin();
		__asm{		
			push   IniArg       /* 线程函数func的参数入栈 */
			call   Func          /* call线程函数func */
			add    esp,4        /* 释放参数传递的栈空间 */
		}
		kernel->currentThread->Finish();
	}

	/* void SWITCH( thread *t1, thread *t2 )
	**
	** on entry, stack looks like this:
	**      8(esp)  ->              thread *t2
	**      4(esp)  ->              thread *t1
	**       (esp)  ->              return address
	**
	** we push the current eax on the stack so that we can use it as
	** a pointer to t1, this decrements esp by 4, so when we use it
	** to reference stuff on the stack, we add 4 to the offset.
	*/
	unsigned long _eax_save = 0;                          //全局中间变量

	void SWITCH(Thread *oldThread, Thread *newThread)
	{



		__asm{
			//align  2
			pop         edi        /* 恢复edi */
			pop         esi        /* 恢复esi */
			pop         ebx        /* 恢复ebx */
			mov         esp,ebp    /* 释放要函数局部变量空间 */
			pop         ebp        /* 恢复ebp */

	/* 注意:现在除PC外主要寄存器的值都是执行SWITCH前的值,也是执行SWITCH后的值。*/

			mov    _eax_save,eax   /* 暂存eax, 注意:_eax_save为全局变量 */

			mov    eax, [esp+4]    /* eax 指向oldThread */
			mov    [_EBX+eax + 4],ebx  /* 保存相关寄存器值, 到oldThread的空间中 */
			mov    [_ECX+eax + 4],ecx
			mov    [_EDX+eax + 4],edx
			mov    [_ESI+eax + 4],esi
			mov    [_EDI+eax + 4],edi
			mov    [_EBP+eax + 4],ebp
			mov    [_ESP+eax + 4],esp  /* 保存栈指针 */

			mov     ebx,_eax_save  /* 取暂存的eax，从全局变量 _eax_save中 */
			mov    [_EAX+eax + 4],ebx  /* 保存初始eax的值 */
			mov    ebx,[esp+0]     /* 取返回地址 */
			mov    [_PC +eax + 4],ebx  /* 保存返回地址 */

			mov    eax,[esp+8]     /* eax指向newThread */
			mov    ebx,[_EAX+eax + 4]  /* 取newThread保存的eax值*/
			mov    _eax_save,ebx   /* 暂存到 _eax_save */

			mov    ebx,[_EBX+eax + 4]  /* 恢复newThread保存的寄存器值 */
			mov    ecx,[_ECX+eax + 4]
			mov    StartUp,ecx     //****
			mov    edx,[_EDX+eax + 4]
			mov    IniArg,edx      //****
			mov    esi,[_ESI+eax + 4]
			mov    Func,esi        //****
			mov    edi,[_EDI+eax + 4]
			mov    ebp,[_EBP+eax + 4]
			mov    esp,[_ESP+eax + 4]  /*restore stack pointer*/

			mov    eax,[_PC +eax + 4]  /*restore return address into eax*/
			mov    [esp+0],eax     /*copy over the ret address on the stack*/

			mov    eax,[_eax_save]

			ret                    /*直接返回*/
		}
	}

}


class  __declspec(dllexport) MyThread: public Thread{
public:
	MyThread(char* debugName):Thread(debugName){}		// initialize a Thread 
	~MyThread();				// deallocate a Thread
					// NOTE -- thread being deleted
					// must not be running when delete 
					// is called

    // basic thread operations

    void Fork(VoidFunctionPtr func, void *arg); 
    				// Make thread run (*func)(arg)
    void yield();  		// Relinquish the CPU if any 
									// other thread is runnable
    void Ssleep(bool finishing); // Put the thread to sleep and 
											// relinquish the processor
    void Begin();			// Startup code for the thread	
    void Finish();  		// The thread is done executing
    
    void CheckOverflow();   	// Check if thread stack has overflowed
    void setStatus(ThreadStatus st){ status = st; }
    char* getName(){ return (name); }
    void Print(){ std::cout << name; }
    void SelfTest(int threadNum);		// test whether thread impl is working
public:
    void SaveUserState();			// save user-level register state
    void RestoreUserState();		// restore user-level register state

    AddrSpace *space;							// User code this thread is running.
};

extern "C" {
	__declspec(dllexport) Thread* GetThreadF(char* debugName){
		return new MyThread(debugName);
	}

	__declspec(dllexport) void DeleteThreadF(Thread* th){
		delete th;
	}
}

/* Liang DePeng +
 ****/


// this is put at the top of the execution stack, for detecting stack overflows
const int STACK_FENCEPOST = 0xdedbeef;

//----------------------------------------------------------------------
// Thread::Thread
// 	Initialize a thread control block, so that we can then call
//	Thread::Fork.
//
//	"threadName" is an arbitrary string, useful for debugging.
//----------------------------------------------------------------------
void Testmain(void* t){
	std::cout <<"success"<<std::endl;
}

Thread::Thread(char* threadName){
    
	name = threadName;
    
    /*stack = (int* )(new char[StackSize * sizeof(int)]);
	stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
	std::cout <<"testmain"<<Testmain<<std::endl;*/
	stackTop = NULL;
    stack = NULL;


    status = JUST_CREATED;
    for (int i = 0; i < MachineStateSize; i++) {
		machineState[i] = NULL;//(void*)1;		// not strictly necessary, since
					// new thread ignores contents 
					// of machine registers
    }
    space = NULL;

	//StackAllocate(Testmain, this);
}

//----------------------------------------------------------------------
// Thread::~Thread
// 	De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      NOTE: if this is the main thread, we can't delete the stack
//      because we didn't allocate it -- we got it automatically
//      as part of starting up Nachos.
//----------------------------------------------------------------------

MyThread::~MyThread()
{
    DEBUG(dbgThread, "Deleting thread: " << name);

    ASSERT(this != kernel->currentThread);
    if (stack != NULL)
		DeallocBoundedArray((char *) stack, StackSize * sizeof(int));
}

//----------------------------------------------------------------------
// Thread::Fork
// 	Invoke (*func)(arg), allowing caller and callee to execute 
//	concurrently.
//
//	NOTE: although our definition allows only a single argument
//	to be passed to the procedure, it is possible to pass multiple
//	arguments by making them fields of a structure, and passing a pointer
//	to the structure as "arg".
//
// 	Implemented as the following steps:
//		1. Allocate a stack
//		2. Initialize the stack so that a call to SWITCH will
//		cause it to run the procedure
//		3. Put the thread on the ready queue
// 	
//	"func" is the procedure to run concurrently.
//	"arg" is a single argument to be passed to the procedure.
//----------------------------------------------------------------------

void MyThread::Fork(VoidFunctionPtr func, void *arg)
{
    //Interrupt *interrupt = kernel->interrupt;	
    Scheduler *scheduler = kernel->scheduler;
    int oldLevel;
    
    DEBUG(dbgThread, "Forking thread: " << name << " f(a): " << (int) func << " " << arg);
    
    StackAllocate(func, arg);

	oldLevel = (* kernel->setLevel)(IntOff);	/* Liang DePeng + */
	//interrupt->SetLevel(IntOff);				/* Liang DePeng - */

    scheduler->ReadyToRun(this);	// ReadyToRun assumes that interrupts 
					// are disabled!

	(void)(* kernel->setLevel)(oldLevel);		/* Liang DePeng + */
	//interrupt->SetLevel(oldLevel);			/* Liang DePeng - */
}    

//----------------------------------------------------------------------
// Thread::CheckOverflow
// 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
//----------------------------------------------------------------------

void MyThread::CheckOverflow()
{
    if (stack != NULL) {
#ifdef HPUX			// Stacks grow upward on the Snakes
	ASSERT(stack[StackSize - 1] == STACK_FENCEPOST);
#else
	ASSERT(*stack == STACK_FENCEPOST);
#endif
   }
}

//----------------------------------------------------------------------
// Thread::Begin
// 	Called by ThreadRoot when a thread is about to begin
//	executing the forked procedure.
//
// 	It's main responsibilities are:
//	1. deallocate the previously running thread if it finished 
//		(see Thread::Finish())
//	2. enable interrupts (so we can get time-sliced)
//----------------------------------------------------------------------

void MyThread::Begin (){
    ASSERT(this == kernel->currentThread);
    DEBUG(dbgThread, "Beginning thread: " << name);
    kernel->scheduler->CheckToBeDestroyed();
    //kernel->interrupt->Enable();	// Enable = setLevel(IntOn);	/* Liang DePeng - */
	(* kernel->setLevel)(IntOn);	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Thread::Finish
// 	Called by ThreadRoot when a thread is done executing the 
//	forked procedure.
//
// 	NOTE: we can't immediately de-allocate the thread data structure 
//	or the execution stack, because we're still running in the thread 
//	and we're still on the stack!  Instead, we tell the scheduler
//	to call the destructor, once it is running in the context of a different thread.
//
// 	NOTE: we disable interrupts, because Sleep() assumes interrupts
//	are disabled.
//----------------------------------------------------------------------

//
void MyThread::Finish (){
    //(void) kernel->interrupt->SetLevel(IntOff);	/* Liang DePeng - */
	(void)(* kernel->setLevel)(IntOff);				/* Liang DePeng + */
    ASSERT(this == kernel->currentThread);
    
    DEBUG(dbgThread, "Finishing thread: " << name);
    
    Ssleep(TRUE);				// invokes SWITCH
    // not reached
}

//----------------------------------------------------------------------
// Thread::Yield
// 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled. 
//
// 	Similar to Thread::Sleep(), but a little different.
//----------------------------------------------------------------------

void MyThread::yield (){
    Thread *nextThread;
	int oldLevel = (* kernel->setLevel)(IntOff);	/* Liang DePeng + */
	//kernel->interrupt->SetLevel(IntOff);				/* Liang DePeng - */
    
    ASSERT(this == kernel->currentThread);
    
    DEBUG(dbgThread, "Yielding thread: " << name);

    nextThread = kernel->scheduler->FindNextToRun();


    if (nextThread != NULL) {
		kernel->scheduler->ReadyToRun(this);
		kernel->scheduler->Run(nextThread, FALSE);
		
    }
    (void)(* kernel->setLevel)(oldLevel);		/* Liang DePeng + */
	//kernel->interrupt->SetLevel(oldLevel);	/* Liang DePeng - */
}

//----------------------------------------------------------------------
// Thread::Sleep
// 	Relinquish the CPU, because the current thread has either
//	finished or is blocked waiting on a synchronization 
//	variable (Semaphore, Lock, or Condition).  In the latter case,
//	eventually some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off 
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
//----------------------------------------------------------------------
void MyThread::Ssleep (bool finishing){

    Thread *nextThread;
    
    //ASSERT(this == kernel->currentThread);	
    //ASSERT(kernel->interrupt->getLevel() == IntOff);	/* Liang DePeng - */
	ASSERT((* kernel->getLevel)() == IntOff);			/* Liang DePeng + */
    
    DEBUG(dbgThread, "Sleeping thread: " << name);

	
    status = BLOCKED;
    while ((nextThread = kernel->scheduler->FindNextToRun()) == NULL)
	//kernel->interrupt->Idle();	// no one to run, wait for an interrupt	/* Liang DePeng - */
		(* kernel->idle)();		/* Liang DePeng + */

    // returns when it's time for us to run
    kernel->scheduler->Run(nextThread, finishing); 
}

//----------------------------------------------------------------------
// ThreadBegin, ThreadFinish,  ThreadPrint
//	Dummy functions because C++ does not (easily) allow pointers to member
//	functions.  So we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the 
//	member function.
//----------------------------------------------------------------------

static void ThreadFinish()    { kernel->currentThread->Finish(); }
static void ThreadBegin() { kernel->currentThread->Begin(); }
extern "C" void ThreadPrint(Item *t) { ((Thread* )t)->Print(); }

#ifdef PARISC

//----------------------------------------------------------------------
// PLabelToAddr
//	On HPUX, function pointers don't always directly point to code,
//	so we need to do the conversion.
//----------------------------------------------------------------------

static void *PLabelToAddr(void *plabel)
{
    int funcPtr = (int) plabel;

    if (funcPtr & 0x02) {
        // L-Field is set.  This is a PLT pointer
        funcPtr -= 2;	// Get rid of the L bit
        return (*(void **)funcPtr);
    } else {
        // L-field not set.
        return plabel;
    }
}
#endif

//----------------------------------------------------------------------
// Thread::StackAllocate
//	Allocate and initialize an execution stack.  The stack is
//	initialized with an initial stack frame for ThreadRoot, which:
//		enables interrupts
//		calls (*func)(arg)
//		calls Thread::Finish
//
//	"func" is the procedure to be forked
//	"arg" is the parameter to be passed to the procedure
//----------------------------------------------------------------------

void
Thread::StackAllocate(VoidFunctionPtr func, void *arg)
{
    stack = (int *) AllocBoundedArray(StackSize * sizeof(int));

/**** delete
 * Liang DePeng - */
/*#ifdef PARISC
    // HP stack works from low addresses to high addresses
    // everyone else works the other way: from high addresses to low addresses
    stackTop = stack + 16;	// HP requires 64-byte frame marker
    stack[StackSize - 1] = STACK_FENCEPOST;
#endif

#ifdef SPARC
    stackTop = stack + StackSize - 96; 	// SPARC stack must contains at 
					// least 1 activation record 
					// to start with.
    *stack = STACK_FENCEPOST;
#endif 

#ifdef PowerPC // RS6000
    stackTop = stack + StackSize - 16; 	// RS6000 requires 64-byte frame marker
    *stack = STACK_FENCEPOST;
#endif 

#ifdef DECMIPS
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif

#ifdef ALPHA
    stackTop = stack + StackSize - 8;	// -8 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif


#ifdef x86*/
    // the x86 passes the return address on the stack.  In order for SWITCH() 
    // to go to ThreadRoot when we switch to this thread, the return addres 
    // used in SWITCH() must be the starting address of ThreadRoot.
/* Liang DePeng -
 ****/	

    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *(--stackTop) = (int) ThreadRoot;
    *stack = STACK_FENCEPOST;


/**** delete
 * Liang DePeng - */
/*#endif
    
#ifdef PARISC
    machineState[PCState] = PLabelToAddr(ThreadRoot);
    machineState[StartupPCState] = PLabelToAddr(ThreadBegin);
    machineState[InitialPCState] = PLabelToAddr(func);
    machineState[InitialArgState] = arg;
    machineState[WhenDonePCState] = PLabelToAddr(ThreadFinish);
#else*/
/* Liang DePeng -
 ****/

    machineState[PCState] = (void*)ThreadRoot;		//7 pc
    machineState[StartupPCState] = (void*)ThreadBegin;	//2 ecx
    machineState[InitialPCState] = (void*)func;		//5	esi
    machineState[InitialArgState] = (void*)arg;		//3 edx
    machineState[WhenDonePCState] = (void*)ThreadFinish;	//6	edi

//#endif
}

//#include "machine.h"

//----------------------------------------------------------------------
// Thread::SaveUserState
//	Save the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine saves the former.
//----------------------------------------------------------------------

void MyThread::SaveUserState(){
    for (int i = 0; i < TotalRegisters; i++)
		userRegisters[i] = (* kernel->readRegister)(i);			/* Liang DePeng + */
		//userRegisters[i] = kernel->machine->ReadRegister(i);	/* Liang DePeng - */
}

//----------------------------------------------------------------------
// Thread::RestoreUserState
//	Restore the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine restores the former.
//----------------------------------------------------------------------

void MyThread::RestoreUserState()
{
    for (int i = 0; i < TotalRegisters; i++)
		(* kernel->writeRegister)(i, userRegisters[i]);			/* Liang DePeng + */
		//kernel->machine->WriteRegister(i, userRegisters[i]);	/* Liang DePeng - */
}


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

static void SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
		std::cout << "*** thread " << which << " looped " << num << " times\n";
		kernel->currentThread->yield();
    }
}

//----------------------------------------------------------------------
// Thread::SelfTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void MyThread::SelfTest(int threadNum)
{
    DEBUG(dbgThread, "Entering Thread::SelfTest");

    //Thread *t = GetThreadF("forked thread")	;//new Thread("forked thread");

	for(int i=0; i<threadNum; i++){
		Thread* t  = GetThreadF("forked thread");
		t->Fork((VoidFunctionPtr) SimpleThread, (void *) (i + 1));
	}
    //t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);

    kernel->currentThread->yield();

    SimpleThread(0);
}

