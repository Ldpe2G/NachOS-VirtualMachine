// synchconsole.cc 
//	Routines providing synchronized access to the keyboard 
//	and console display hardware devices.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// SynchConsoleInput::SynchConsoleInput
//      Initialize synchronized access to the keyboard
//
//      "inputFile" -- if NULL, use stdin as console device
//              otherwise, read from this file
//----------------------------------------------------------------------

SynchConsoleInput::SynchConsoleInput(char *inputFile, 
				void (* initConsoleInput)(char *inputFile, CallBackObj *callTo),
				ConGetChar getCharr)
{
    //consoleInput = new ConsoleInput(inputFile, this);	/* Liang DePeng - */
    //lock = new Lock("console in");	/* Liang DePeng - */
    //waitFor = new Semaphore("console in", 0);	/* Liang DePeng - */

	(* initConsoleInput)(inputFile, this);	/* Liang DePeng + */
	lock = GetLockF("console in"); /* Liang DePeng + */
	waitFor = GetSemaphoreF("console in", 0); /* Liang DePeng + */
	this->getChar = getCharr;	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// SynchConsoleInput::~SynchConsoleInput
//      Deallocate data structures for synchronized access to the keyboard
//----------------------------------------------------------------------

SynchConsoleInput::~SynchConsoleInput()
{ 
    //delete consoleInput;	/* Liang DePeng - */
    //delete lock;			/* Liang DePeng - */
    //delete waitFor;		/* Liang DePeng - */

	DeleteLockF(lock);			/* Liang DePeng + */
    DeleteSemaphoreF(waitFor);		/* Liang DePeng + */
}

//----------------------------------------------------------------------
// SynchConsoleInput::GetChar
//      Read a character typed at the keyboard, waiting if necessary.
//----------------------------------------------------------------------

char
SynchConsoleInput::GetChar()
{
    char ch;

    lock->Acquire();
    waitFor->P();	// wait for EOF or a char to be available.
    
	//ch = consoleInput->GetChar();	/* Liang DePeng - */
    ch = (* getChar)();		/* Liang DePeng + */

	lock->Release();
    return ch;
}

//----------------------------------------------------------------------
// SynchConsoleInput::ConsoleToKernel
//    Read characters from the console into a kernel buffer,
//    one at a time
//
//    "buf" is the buffer
//    "length" the number of characters to be read
//----------------------------------------------------------------------
int SynchConsoleInput::ConsoleToKernel(int length, char* buf)		/*zhong_expan + */
{
	int retv = 0;
	while(length > 0)
	{
		*buf = GetChar();
		retv++;
		if (*buf == '\n')
			break;
		length--;
		buf++;
	}
	return retv;
}

//----------------------------------------------------------------------
// SynchConsoleInput::CallBack
//      Interrupt handler called when keystroke is hit; wake up
//	anyone waiting.
//----------------------------------------------------------------------

void
SynchConsoleInput::CallBack()
{
    waitFor->V();
}

//----------------------------------------------------------------------
// SynchConsoleOutput::SynchConsoleOutput
//      Initialize synchronized access to the console display
//
//      "outputFile" -- if NULL, use stdout as console device
//              otherwise, read from this file
//----------------------------------------------------------------------

SynchConsoleOutput::SynchConsoleOutput(char *outputFile,  
	void (* initConsoleOutput)(char *outputFile, CallBackObj *callTo),
	ConPutChar putCharr)
{
    //consoleOutput = new ConsoleOutput(outputFile, this);	/* Liang DePeng - */
    //lock = new Lock("console out");		/* Liang DePeng - */
    //waitFor = new Semaphore("console out", 0);	/* Liang DePeng - */

	(* initConsoleOutput)(outputFile, this);	/* Liang DePeng + */
	lock = GetLockF("console out");	/* Liang DePeng + */
	waitFor = GetSemaphoreF("console out", 0);	/* Liang DePeng + */
	this->putChar = putCharr;	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// SynchConsoleOutput::~SynchConsoleOutput
//      Deallocate data structures for synchronized access to the keyboard
//----------------------------------------------------------------------

SynchConsoleOutput::~SynchConsoleOutput()
{ 
    //delete consoleOutput;		/* Liang DePeng - */
    //delete lock;				/* Liang DePeng - */
    //delete waitFor;			/* Liang DePeng - */

	DeleteLockF(lock);			/* Liang DePeng + */
    DeleteSemaphoreF(waitFor);		/* Liang DePeng + */
}

//----------------------------------------------------------------------
// SynchConsoleOutput::PutChar
//      Write a character to the console display, waiting if necessary.
//----------------------------------------------------------------------

void
SynchConsoleOutput::PutChar(char ch)
{
    lock->Acquire();

    //consoleOutput->PutChar(ch);	/* Liang DePeng - */
	(* putChar)(ch);	/* Liang DePeng + */

    waitFor->P();
    lock->Release();
}

//----------------------------------------------------------------------
// SynchConsoleOutput::KernelToConsole
//    write the contents of a kernel buffer to
//    the console one character at a time
//
//    "buf" is the buffer
//    "length" the number of characters in the buffer
//----------------------------------------------------------------------
void SynchConsoleOutput::KernelToConsole(int length, char* buf)		/*zhong_expan + */
{
	while(length > 0)
	{
		PutChar(*buf);
		length--;
		buf++;
	}
	return;
}

//----------------------------------------------------------------------
// SynchConsoleOutput::CallBack
//      Interrupt handler called when it's safe to send the next 
//	character can be sent to the display.
//----------------------------------------------------------------------

void
SynchConsoleOutput::CallBack()
{
    waitFor->V();
}
