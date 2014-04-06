// console.cc 
//	Routines to simulate a serial port to a console device.
//	A console has input (a keyboard) and output (a display).
//	These are each simulated by operations on UNIX files.
//	The simulated device is asynchronous, so we have to invoke 
//	the interrupt handler (after a simulated delay), to signal that 
//	a byte has arrived and/or that a written byte has departed.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "console.h"
#include "main.h"

#include <Windows.h>		/*zhong_tran + */

//----------------------------------------------------------------------
// ConsoleInput::ConsoleInput
// 	Initialize the simulation of the input for a hardware console device.
//
//	"readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
// 	"toCall" is the interrupt handler to call when a character arrives
//		from the keyboard
//----------------------------------------------------------------------

ConsoleInput::ConsoleInput(char *readFile, CallBackObj *toCall)
{
	if (readFile == NULL){
		//readFileNo = 0;					// keyboard = stdin
	
		readFileNo = 0;					// keyboard = stdin
		hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);		/*zhong_tran + */
		SetConsoleMode(hConsoleInput, ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);		/*zhong_tran + */
	}
	else
		readFileNo = (* fp_OpenForReadWrite)(readFile, TRUE); /* Liang DePeng + */
		//readFileNo = OpenForReadWrite(readFile, TRUE);	// should be read-only /* Liang DePeng - */

	// set up the stuff to emulate asynchronous interrupts
	callWhenAvail = toCall;
	incoming = EOF;

	// start polling for incoming keystrokes
	//kernel->interrupt->Schedule(this, ConsoleTime, ConsoleReadInt);	/* Liang DePeng - */
	interrupt->Schedule(this, ConsoleTime, ConsoleReadInt);				/* Liang DePeng + */
}

//----------------------------------------------------------------------
// ConsoleInput::~ConsoleInput
// 	Clean up console input emulation
//----------------------------------------------------------------------

ConsoleInput::~ConsoleInput()
{
    if (readFileNo != 0)
		(* fp_Close)(readFileNo);	/* Liang DePeng + */
	//Close(readFileNo);	WriteFkFile(writeFileNo, &ch, sizeof(char));	/* Liang DePeng - */
}


//----------------------------------------------------------------------
// ConsoleInput::CallBack()
// 	Simulator calls this when a character may be available to be
//	read in from the simulated keyboard (eg, the user typed something).
//
//	First check to make sure character is available.
//	Then invoke the "callBack" registered by whoever wants the character.
//----------------------------------------------------------------------

void
ConsoleInput::CallBack()
{
	char c;
	//int readCount;		/*zhong_tran - */
	DWORD readCount;		/*zhong_tran + */

	ASSERT(incoming == EOF);
	//if (!PollFile(readFileNo)) 		/*zhong_tran - */
	if (((readFileNo == 0) && !(* fp_PollConsole)(hConsoleInput)/*!PollConsole(hConsoleInput)*/) ||
		((readFileNo != 0) && !(* fp_PollFile)(readFileNo)/*!PollFile(readFileNo))*/)) 	/*zhong_tran + */
	{ // nothing to be read
		// schedule the next time to poll for a packet
		//kernel->interrupt->Schedule(this, ConsoleTime, ConsoleReadInt);	/* Liang DePeng - */
		interrupt->Schedule(this, ConsoleTime, ConsoleReadInt);				/* Liang DePeng + */
	} 
	else 
	{ 
		SetConsoleMode(hConsoleInput, ENABLE_PROCESSED_INPUT);		/*zhong_tran + */
		// otherwise, try to read a character
		//readCount = ReadPartial(readFileNo, &c, sizeof(char));		/*zhong_tran - */
		ReadFile(hConsoleInput, &c, sizeof(char), &readCount, NULL);		/*zhong_tran + */
		if (readCount == 0) 
		{
			// this seems to happen at end of file, when the
			// console input is a regular file
			// don't schedule an interrupt, since there will never
			// be any more input
			// just do nothing....
		}
		else 
		{
			// save the character and notify the OS that
			// it is available
			ASSERT(readCount == sizeof(char));
			if (c == '\r')		/*zhong_tran + */	//将'\r'换成'\n'
				c = '\n';		/*zhong_tran + */
			else if (c == '\4')	/*zhong_tran + */
				c = EOF;		/*zhong_tran + */	//将ctrl+D换成EOF
			if (c != EOF)		/*zhong_tran + */
				(* fp_WriteFkFile)(1, &c, 1);   /* Liang DePeng + */
				//WriteFkFile(1, &c, 1);		/*zhong_tran + */	//将读到的字符输出到控制台中 /* Liang DePeng - */
			incoming = c;
			//kernel->stats->numConsoleCharsRead++;    /* Liang DePeng - */
			stats->numConsoleCharsRead++;              /* Liang DePeng + */
		}
		SetConsoleMode(hConsoleInput, ENABLE_LINE_INPUT |
			ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);		/*zhong_tran + */
		callWhenAvail->CallBack();
	}
}

//----------------------------------------------------------------------
// ConsoleInput::GetChar()
// 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//----------------------------------------------------------------------

char
ConsoleInput::GetChar()
{
   char ch = incoming;

   if (incoming != EOF) {	// schedule when next char will arrive
	   //kernel->interrupt->Schedule(this, ConsoleTime, ConsoleReadInt);	/* Liang DePeng - */
       interrupt->Schedule(this, ConsoleTime, ConsoleReadInt);				/* Liang DePeng + */
   }
   incoming = EOF;
   return ch;
}



//----------------------------------------------------------------------
// ConsoleOutput::ConsoleOutput
// 	Initialize the simulation of the output for a hardware console device.
//
//	"writeFile" -- UNIX file simulating the display (NULL -> use stdout)
// 	"toCall" is the interrupt handler to call when a write to 
//	the display completes.
//----------------------------------------------------------------------

ConsoleOutput::ConsoleOutput(char *writeFile, CallBackObj *toCall)
{
    if (writeFile == NULL)
		writeFileNo = 1;				// display = stdout
    else
		writeFileNo = (* fp_OpenForWrite)(writeFile); /* Liang DePeng + */
    	//writeFileNo = OpenForWrite(writeFile);      /* Liang DePeng - */

    callWhenDone = toCall;
    putBusy = FALSE;
}

//----------------------------------------------------------------------
// ConsoleOutput::~ConsoleOutput
// 	Clean up console output emulation
//----------------------------------------------------------------------

ConsoleOutput::~ConsoleOutput()
{
    if (writeFileNo != 1)
		(* fp_Close)(writeFileNo);	/* Liang DePeng + */
		//Close(writeFileNo);	/* Liang DePeng - */
}

//----------------------------------------------------------------------
// ConsoleOutput::CallBack()
// 	Simulator calls this when the next character can be output to the
//	display.
//----------------------------------------------------------------------

void
ConsoleOutput::CallBack()
{
    putBusy = FALSE;
    //kernel->stats->numConsoleCharsWritten++;  /* Liang DePeng - */
	stats->numConsoleCharsWritten++;            /* Liang DePeng + */
    callWhenDone->CallBack();
}

//----------------------------------------------------------------------
// ConsoleOutput::PutChar()
// 	Write a character to the simulated display, schedule an interrupt 
//	to occur in the future, and return.
//----------------------------------------------------------------------

void
ConsoleOutput::PutChar(char ch)
{
    ASSERT(putBusy == FALSE);
	(* fp_WriteFkFile)(writeFileNo, &ch, sizeof(char));	/* Liang DePeng + */
    //WriteFkFile(writeFileNo, &ch, sizeof(char));	/* Liang DePeng - */
    putBusy = TRUE;
	//kernel->interrupt->Schedule(this, ConsoleTime, ConsoleWriteInt);	/* Liang DePeng - */
    interrupt->Schedule(this, ConsoleTime, ConsoleWriteInt);			/* Liang DePeng + */
}
