// debug.h 
//	Data structures for debugging routines.
//
//	The debugging routines allow the user to turn on selected
//	debugging messages, controllable from the command line arguments
//	passed to Nachos (-d).  You are encouraged to add your own
//	debugging flags.  Please.... 
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.




#ifndef DEBUG_H
#define DEBUG_H

#include "copyright.h"		
#include "utility.h"
#include "sysdep.h"

// The pre-defined debugging flags are:

const char dbgAll = '+';		// turn on all debug messages
const char dbgThread = 't';		// threads
const char dbgSynch = 's';		// locks, semaphores, condition vars
const char dbgInt = 'i'; 		// interrupt emulation
const char dbgMach = 'm'; 		// machine emulation
const char dbgDisk = 'd'; 		// disk emulation
const char dbgFile = 'f'; 		// file system
const char dbgAddr = 'a'; 		// address spaces
const char dbgNet = 'n'; 		// network emulation
const char dbgSys = 'u';                // systemcall

/**** delete
 * Liang DePeng -
 */
/*
class Debug {
  public:
    Debug(char *flagList);

    bool IsEnabled(char flag);

  private:
    char *enableFlags;		// controls which DEBUG messages are printed
};

extern Debug *debug;
*/
/*
 * Liang DePeng -
 ****/



/**** modify
 * Liang DePeng +
 */
class __declspec(dllexport) Debug {
  public:
	Debug(char *flagList){}
	virtual ~Debug() = 0{};
    virtual bool IsEnabled(char flag) = 0;

  protected:
    char *enableFlags;		// controls which DEBUG messages are printed
};
/*
 * Liang DePeng +
 ****/

/**** add
 * Liang DePeng +
 */
//宏定义函数指针
typedef Debug* (*CreateDebug)(char*);  
typedef void (*DeleteDebug)(Debug*);

extern "C" {
	__declspec(dllexport) Debug* getDebugInstance(char* flagList);
	__declspec(dllexport) void deleteDebugInstance(Debug*);

}

extern Debug *debug;    
extern CreateDebug createDebug;
extern DeleteDebug deleteDebug;	

/*
 * Liang DePeng +
 ****/

//----------------------------------------------------------------------
// DEBUG
//      If flag is enabled, print a message.
//----------------------------------------------------------------------

#define DEBUG(flag,expr)                                                     \
    if (!debug->IsEnabled(flag)) {} else { 				\
        std::cerr << expr << "\n";   				        \
    }
	


//----------------------------------------------------------------------
// ASSERT
//      If condition is false,  print a message and dump core.
//	Useful for documenting assumptions in the code.
//
//	NOTE: needs to be a #define, to be able to print the location 
//	where the error occurred.
//----------------------------------------------------------------------
#define ASSERT(condition)                                               \
    if (condition) {} else { 						\
	std::cerr << "Assertion failed: line " << __LINE__ << " file " << __FILE__ << "\n";      \
        abort();                                                              \
    }

//----------------------------------------------------------------------
// ASSERTNOTREACHED
//      Print a message and dump core (equivalent to ASSERT(FALSE) without
//	making the compiler whine).  Useful for documenting when
//	code should not be reached.
//
//	NOTE: needs to be a #define, to be able to print the location 
//	where the error occurred.
//----------------------------------------------------------------------

#define ASSERTNOTREACHED()                                             \
    { 						\
	std::cerr << "Assertion failed: line " << __LINE__ << " file " << __FILE__ << "\n";      \
        abort();                                                              \
    }

//----------------------------------------------------------------------
// ASSERTUNIMPLEMENTED
//     Print a message that unimplemented code is executed and dump core
//----------------------------------------------------------------------
#define UNIMPLEMENTED()                                                      \
{                                                                            \
  std::cerr << "Reached UNIMPLEMENTED function " << __FUNCTION__ << " in file: "  \
       << __FILE__ << " line: " << __LINE__ << ".\n";                        \
}

#endif // DEBUG_H
