// debug.cc 
//	Debugging routines.  Allows users to control whether to 
//	print DEBUG statements, based on a command line argument.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "debug.h" 
#include "string.h"

/**** delete
 * Liang DePeng -
 */
/*
Debug::Debug(char *flagList)
{
    enableFlags = flagList;
}

bool
Debug::IsEnabled(char flag)
{
    if (enableFlags != NULL) {
	return ((strchr(enableFlags, flag) != 0) 
		|| (strchr(enableFlags, '+') != 0));
    } else {
    	return FALSE;
    }
}
*/
/*
 * Liang DePeng -
 ****/


/**** add
 * Liang DePeng +
 */
class __declspec(dllexport) MyDebug : public Debug
{
public: 
	MyDebug(char *flagList);

	~MyDebug(){};

    bool IsEnabled(char flag);
 
};

//----------------------------------------------------------------------
// Debug::Debug
//      Initialize so that only DEBUG messages with a flag in flagList 
//	will be printed.
//
//	If the flag is "+", we enable all DEBUG messages.
//
// 	"flagList" is a string of characters for whose DEBUG messages are 
//		to be enabled.
//----------------------------------------------------------------------

MyDebug::MyDebug(char *flagList):Debug(flagList)
{
    this->enableFlags = flagList;
}


//----------------------------------------------------------------------
// Debug::IsEnabled
//      Return TRUE if DEBUG messages with "flag" are to be printed.
//----------------------------------------------------------------------

bool MyDebug::IsEnabled(char flag)
{
    if (enableFlags != NULL) {
	return ((strchr(enableFlags, flag) != 0) 
		|| (strchr(enableFlags, '+') != 0));
    } else {
    	return FALSE;
    }
}

Debug *debug; 

//定义导出函数
extern "C" {
	__declspec(dllexport) Debug* getDebugInstance(char* flagList){
		debug =  new MyDebug(flagList);
		return debug;
	}
	__declspec(dllexport) void deleteDebugInstance(Debug* debug){
		delete debug;
	}
}

/*
 * Liang DePeng +
 ****/
