// interrupt.cc 
//	Routines to simulate hardware interrupts.
//
//	The hardware provides a routine (SetLevel) to enable or disable
//	interrupts.
//
//	In order to emulate the hardware, we need to keep track of all
//	interrupts the hardware devices would cause, and when they
//	are supposed to occur.  
//
//	This module also keeps track of simulated time.  Time advances
//	only when the following occur: 
//		interrupts are re-enabled
//		a user instruction is executed
//		there is nothing in the ready queue
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "interrupt.h"
#include "main.h"

//using namespace std;


// String definitions for debugging messages

static char *intLevelNames[] = { "off", "on"};
static char *intTypeNames[] = { "timer", "disk", "console write", 
			"console read", "network send", 
			"network recv"};

//----------------------------------------------------------------------
// PendingInterrupt::PendingInterrupt
// 	Initialize a hardware device interrupt that is to be scheduled 
//	to occur in the near future.
//
//	"callOnInt" is the object to call when the interrupt occurs
//	"time" is when (in simulated time) the interrupt is to occur
//	"kind" is the hardware device that generated the interrupt
//----------------------------------------------------------------------

PendingInterrupt::PendingInterrupt(CallBackObj *callOnInt, 
					int time, IntType kind){

    callOnInterrupt = callOnInt;
    when = time;
    type = kind;
}

//----------------------------------------------------------------------
// PendingCompare
//	Compare to interrupts based on which should occur first.
//----------------------------------------------------------------------

//static int PendingCompare (PendingInterrupt *x, PendingInterrupt *y){	/* Liang DePeng - */
static int PendingCompare (Item *x, Item *y){	/* Liang DePeng + */
	PendingInterrupt *xx = (PendingInterrupt*)x;
	PendingInterrupt *yy = (PendingInterrupt*)y;
    if (xx->when < yy->when) { return -1; }
    else if (xx->when > yy->when) { return 1; }
    else { return 0; }
}

//----------------------------------------------------------------------
// Interrupt::Interrupt
// 	Initialize the simulation of hardware device interrupts.
//	
//	Interrupts start disabled, with no interrupts pending, etc.
//----------------------------------------------------------------------

//Interrupt::Interrupt(){				/* Liang DePeng - */
Interrupt::Interrupt(HMODULE hDLL){		/* Liang DePeng + */

	int i;
    level = IntOff;

    //pending = new SortedList<PendingInterrupt *>(PendingCompare);	/* Liang DePeng - */
	
	pending = (* getList)(PendingCompare);

    inHandler = FALSE;
    yieldOnReturn = FALSE;
    status = SystemMode;
    idlecount = 0;
	for(i=0;i<6;i++)
		intCount[i] = 0;

}

//----------------------------------------------------------------------
// Interrupt::~Interrupt
// 	De-allocate the data structures needed by the interrupt simulation.
//----------------------------------------------------------------------

Interrupt::~Interrupt(){

    while (!pending->IsEmpty()) {
		delete pending->RemoveFront();
    }
    //delete pending;			/* Liang DePeng - */
	(* deleteList)(pending);	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Interrupt::ChangeLevel
// 	Change interrupts to be enabled or disabled, without advancing 
//	the simulated time (normally, enabling interrupts advances the time).
//
//	Used internally.
//
//	"old" -- the old interrupt status
//	"now" -- the new interrupt status
//----------------------------------------------------------------------

void
Interrupt::ChangeLevel(int old, int now){

    level = now;
    DEBUG(dbgInt, "\tinterrupts: " << intLevelNames[old] << " -> " << intLevelNames[now]);
}

//----------------------------------------------------------------------
// Interrupt::SetLevel
// 	Change interrupts to be enabled or disabled, and if interrupts
//	are being enabled, advance simulated time by calling OneTick().
//
// Returns:
//	The old interrupt status.
// Parameters:
//	"now" -- the new interrupt status
//----------------------------------------------------------------------

int Interrupt::SetLevel(int now){

    int old = level;
    
    // interrupt handlers are prohibited from enabling interrupts
    ASSERT((now == IntOff) || (inHandler == FALSE));

    ChangeLevel(old, now);			// change to new state
    if ((now == IntOn) && (old == IntOff)) {
		OneTick();				// advance simulated time
    }
    return old;
}

//----------------------------------------------------------------------
// Interrupt::OneTick
// 	Advance simulated time and check if there are any pending 
//	interrupts to be called. 
//
//	Two things can cause OneTick to be called:
//		interrupts are re-enabled
//		a user instruction is executed
//----------------------------------------------------------------------
void Interrupt::OneTick(){

    MachineStatus oldStatus = status;
    //Statistics *stats = kernel->stats;	/* Liang DePeng - */
	Statistics *statss = stats;				/* Liang DePeng + */

// advance simulated time
    if (status == SystemMode) {
        statss->totalTicks += SystemTick;
		statss->systemTicks += SystemTick;
    } else {
		statss->totalTicks += UserTick;
		statss->userTicks += UserTick;
    }
    DEBUG(dbgInt, "== Tick " << statss->totalTicks << " ==");

// check any pending interrupts are now ready to fire
    ChangeLevel(IntOn, IntOff);	// first, turn off interrupts
				// (interrupt handlers run with
				// interrupts disabled)
    CheckIfDue(FALSE);		// check for pending interrupts
    ChangeLevel(IntOff, IntOn);	// re-enable interrupts
    if (yieldOnReturn) {	// if the timer device handler asked 
    				// for a context switch, ok to do it now
		yieldOnReturn = FALSE;
 		status = SystemMode;		// yield is a kernel routine
		kernel->currentThread->yield();   

		status = oldStatus;
    }
}

//----------------------------------------------------------------------
// Interrupt::YieldOnReturn
// 	Called from within an interrupt handler, to cause a context switch
//	(for example, on a time slice) in the interrupted thread,
//	when the handler returns.
//
//	We can't do the context switch here, because that would switch
//	out the interrupt handler, and we want to switch out the 
//	interrupted thread.
//----------------------------------------------------------------------

void
Interrupt::YieldOnReturn()
{ 
    ASSERT(inHandler == TRUE);  
    yieldOnReturn = TRUE; 
}

//----------------------------------------------------------------------
// Interrupt::Idle
// 	Routine called when there is nothing in the ready queue.
//
//	Since something has to be running in order to put a thread
//	on the ready queue, the only thing to do is to advance 
//	simulated time until the next scheduled hardware interrupt.
//
//	If there are no pending interrupts, stop.  There's nothing
//	more for us to do.
//----------------------------------------------------------------------
void Interrupt::Idle(){

    DEBUG(dbgInt, "Machine idling; checking for interrupts.");
    status = IdleMode;

	//if ((intCount[DiskInt]==0)&&(intCount[ConsoleWriteInt]==0)&&
	//	(intCount[NetworkSendInt]==0))
	//	idlecount++;

	//if (idlecount > 10000)
	//	Halt();

    if (CheckIfDue(TRUE)) {	// check for any pending interrupts
		status = SystemMode;
		return;			// return in case there's now
					// a runnable thread
    }

    // if there are no pending interrupts, and nothing is on the ready
    // queue, it is time to stop.   If the console or the network is 
    // operating, there are *always* pending interrupts, so this code
    // is not reached.  Instead, the halt must be invoked by the user program.

    DEBUG(dbgInt, "Machine idle.  No interrupts to do.");
    std::cout << "No threads ready or runnable, and no pending interrupts.\n";
    std::cout << "Assuming the program completed.\n";
    Halt();
}

//----------------------------------------------------------------------
// Interrupt::Halt
// 	Shut down Machine cleanly, printing out performance statistics.
//----------------------------------------------------------------------
void
Interrupt::Halt()
{
    std::cout << "Machine halting!\n\n";
    //kernel->stats->Print();	/* Liang DePeng - */
	stats->Print();				/* Liang DePeng + */
    //delete kernel;	// Never returns.
	delete machine;		/* Liang DePeng + */
	delete interrupt;	/* Liang DePeng + */
	delete stats;		/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Interrupt::Schedule
// 	Arrange for the CPU to be interrupted when simulated time
//	reaches "now + when".
//
//	Implementation: just put it on a sorted list.
//
//	NOTE: the Nachos kernel should not call this routine directly.
//	Instead, it is only called by the hardware device simulators.
//
//	"toCall" is the object to call when the interrupt occurs
//	"fromNow" is how far in the future (in simulated time) the 
//		 interrupt is to occur
//	"type" is the hardware device that generated the interrupt
//----------------------------------------------------------------------
void
Interrupt::Schedule(CallBackObj *toCall, int fromNow, IntType type)
{
    //int when = kernel->stats->totalTicks + fromNow;	/* Liang DePeng - */
	int when = stats->totalTicks + fromNow;				/* Liang DePeng + */

    PendingInterrupt *toOccur = new PendingInterrupt(toCall, when, type);

    DEBUG(dbgInt, "Scheduling interrupt handler the " << intTypeNames[type] << " at time = " << when);
    ASSERT(fromNow > 0);

    pending->Insert(toOccur);
    intCount[type] += 1;
}

//----------------------------------------------------------------------
// Interrupt::CheckIfDue
// 	Check if any interrupts are scheduled to occur, and if so, 
//	fire them off.
//
// Returns:
//	TRUE, if we fired off any interrupt handlers
// Params:
//	"advanceClock" -- if TRUE, there is nothing in the ready queue,
//		so we should simply advance the clock to when the next 
//		pending interrupt would occur (if any).
//----------------------------------------------------------------------
bool Interrupt::CheckIfDue(bool advanceClock){

    PendingInterrupt *next;
    //Statistics *stats = kernel->stats;	/* Liang DePeng - */
	Statistics *statss = stats;				/* Liang DePeng + */

    ASSERT(level == IntOff);		// interrupts need to be disabled,
					// to invoke an interrupt handler
    if (debug->IsEnabled(dbgInt)) {
		DumpState();
    }
    if (pending->IsEmpty()) {   	// no pending interrupts
		return FALSE;	
    }		
    Item *temp = pending->Front();	/* Liang DePeng + */
	next = (PendingInterrupt*)temp;	/* Liang DePeng + */

    if (next->when > stats->totalTicks) {
        if (!advanceClock) {		// not time yet
            return FALSE;
        }
        else {      		// advance the clock to next interrupt
			statss->idleTicks += (next->when - stats->totalTicks);
			statss->totalTicks = next->when;
			// UDelay(1000L); // rcgood - to stop nachos from spinning.
		}
    }

    DEBUG(dbgInt, "Invoking interrupt handler for the ");
    DEBUG(dbgInt, intTypeNames[next->type] << " at time " << next->when);
	
    //if (kernel->machine != NULL) {            /* Liang DePeng - */
    //	kernel->machine->DelayedLoad(0, 0);     /* Liang DePeng - */
	if (machine != NULL) {						/* Liang DePeng + */
    	machine->DelayedLoad(0, 0);				/* Liang DePeng + */
    }
	


    inHandler = TRUE;
    do {
		temp = pending->RemoveFront();		/* Liang DePeng + */
		next = (PendingInterrupt*)temp;		/* Liang DePeng + */
        //next = pending->RemoveFront();    // pull interrupt off list	/* Liang DePeng - */

        next->callOnInterrupt->CallBack();// call the interrupt handler
		ASSERT(intCount[next->type] > 0);
		intCount[next->type] -= 1;
		delete next;

    } while (!pending->IsEmpty() && (((PendingInterrupt*)pending->Front())->when <= stats->totalTicks));
    inHandler = FALSE;
    return TRUE;
}

//----------------------------------------------------------------------
// PrintPending
// 	Print information about an interrupt that is scheduled to occur.
//	When, where, why, etc.
//----------------------------------------------------------------------

//static void PrintPending (PendingInterrupt *pending){	/* Liang DePeng - */
static void PrintPending (Item *pending){   /* Liang DePeng + */
	PendingInterrupt *pendingg = (PendingInterrupt*)pending;
    std::cout << "Interrupt handler "<< intTypeNames[pendingg->type];
    std::cout << ", scheduled at " << pendingg->when;
}

//----------------------------------------------------------------------
// DumpState
// 	Print the complete interrupt state - the status, and all interrupts
//	that are scheduled to occur in the future.
//----------------------------------------------------------------------

void Interrupt::DumpState() {
    //cout << "Time: " << kernel->stats->totalTicks;	/* Liang DePeng - */
	std::cout << "Time: " << stats->totalTicks;				/* Liang DePeng + */
    std::cout << ", interrupts " << intLevelNames[level] << "\n";
    std::cout << "Pending interrupts:\n";
    pending->Apply(PrintPending);
    std::cout << "\nEnd of pending interrupts\n";
}
