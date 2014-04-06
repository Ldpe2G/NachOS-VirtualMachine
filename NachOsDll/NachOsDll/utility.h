// utility.h 
//	Miscellaneous useful definitions.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef UTILITY_H
#define UTILITY_H

#include "copyright.h"



/**** add
 * Liang DePeng + */
enum ExceptionType { NoException,           // Everything ok!
		     SyscallException,      // A program executed a system call.
		     PageFaultException,    // No valid translation found
		     ReadOnlyException,     // Write attempted to page marked 
					    // "read-only"
		     BusErrorException,     // Translation resulted in an 
					    // invalid physical address
		     AddressErrorException, // Unaligned reference or one that
					    // was beyond the end of the
					    // address space
		     OverflowException,     // Integer overflow in add or sub.
		     IllegalInstrException, // Unimplemented or reserved instr.
		     
		     NumExceptionTypes
};

extern "C" __declspec(dllexport) void ExceptionHandler(ExceptionType which);
				// Entry point into Nachos for handling
				// user system calls and exceptions
				// Defined in exception.cpp
typedef void (* FPExceptionHandler)(ExceptionType/* which*/);
extern FPExceptionHandler fpExceptionHandler;
/* Liang DePeng +
 ****/

// Miscellaneous useful routines

#define NULL 0
#define TRUE  true
#define FALSE  false
// #define bool int		// necessary on the Mac?

#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define max(a,b)  (((a) > (b)) ? (a) : (b))

// Divide and either round up or down 
#define divRoundDown(n,s)  ((n) / (s))
#define divRoundUp(n,s)    (((n) / (s)) + ((((n) % (s)) > 0) ? 1 : 0))

// This declares the type "VoidFunctionPtr" to be a "pointer to a
// function taking an arbitrary pointer argument and returning nothing".  With
// such a function pointer (say it is "func"), we can call it like this:
//
//	(*func) ("help!");
//
// This is used by Thread::Fork as well as a couple of other places.

typedef void (*VoidFunctionPtr)(void *arg); 
typedef void (*VoidNoArgFunctionPtr)(); 

#endif // UTILITY_H
