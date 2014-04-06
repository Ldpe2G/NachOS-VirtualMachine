// synchconsole.h 
//	Data structures for synchronized access to the keyboard
//	and console display devices.
//
//	NOTE: this abstraction is not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "utility.h"
#include "callback.h"
//#include "console.h"
#include "synch.h"



#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

// The following two classes define synchronized input and output to
// a console device

typedef void (* ConPutChar)(char ch);
typedef char (* ConGetChar)();


class SynchConsoleInput : public CallBackObj {
public:
    SynchConsoleInput(char *inputFile, 
					void (* initConsoleInput)(char *inputFile, CallBackObj *callTo),
					ConGetChar getCharr); // Initialize the console device
    ~SynchConsoleInput();		// Deallocate console device

    char GetChar();		// Read a character, waiting if necessary

    // Read "length" characters from input console into kernel buffer
    int ConsoleToKernel(int length, char* buf);		/*zhong_expan + */
    
private:
	ConGetChar getChar;
    //ConsoleInput *consoleInput;	/* Liang DePeng - */	// the hardware keyboard
    FLock* lock;			// only one reader at a time
    FSemaphore* waitFor;		// wait for callBack

    void CallBack();		// called when a keystroke is available
};

class SynchConsoleOutput : public CallBackObj {
public:
    SynchConsoleOutput(char *outputFile,  
					void (* initConsoleOutput)(char *outputFile, CallBackObj *callTo),
					ConPutChar putCharr); // Initialize the console device
    ~SynchConsoleOutput();

    void PutChar(char ch);	// Write a character, waiting if necessary

    // Write "length" characters from kernel buffer to output console
    void KernelToConsole(int length, char* buf);		/*zhong_expan + */
    
private:
	ConPutChar putChar;
    //ConsoleOutput *consoleOutput;	/* Liang DePeng - */	// the hardware display
    FLock *lock;			// only one writer at a time
    FSemaphore *waitFor;		// wait for callBack

    void CallBack();		// called when more data can be written
};

#endif // SYNCHCONSOLE_H
