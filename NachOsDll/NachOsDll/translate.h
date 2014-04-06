// translate.h 
//	Data structures for managing the translation from 
//	virtual page # -> physical page #, used for managing
//	physical memory on behalf of user programs.
//
//	The data structures in this file are "dual-use" - they
//	serve both as a page table entry, and as an entry in
//	a software-managed translation lookaside buffer (TLB).
//	Either way, each entry is of the form:
//	<virtual page #, physical page #>.
//
// DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef TLB_H
#define TLB_H

#include "copyright.h"
#include "utility.h"

// The following class defines an entry in a translation table -- either
// in a page table or a TLB.  Each entry defines a mapping from one 
// virtual page to one physical page.
// In addition, there are some extra bits for access control (valid and 
// read-only) and some bits for usage information (use and dirty).

/**** delete
 * Liang DePeng - */
/*
class TranslationEntry {
public:
    int virtualPage;  	// The page number in virtual memory.
    int physicalPage;  	// The page number in real memory (relative to the
			//  start of "mainMemory"
    bool valid;         // If this bit is set, the translation is ignored.
			// (In other words, the entry hasn't been initialized.)
    bool readOnly;	// If this bit is set, the user program is not allowed
			// to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
			// page is referenced or modified.
    bool dirty;         // This bit is set by the hardware every time the
			// page is modified.
};*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
class  __declspec(dllexport) TranslationEntry {
public:
	TranslationEntry(){}
	virtual ~TranslationEntry() = 0{}
    int virtualPage;  	// The page number in virtual memory.
    int physicalPage;  	// The page number in real memory (relative to the
			//  start of "mainMemory"
    bool valid;         // If this bit is set, the translation is ignored.
			// (In other words, the entry hasn't been initialized.)
    bool readOnly;	// If this bit is set, the user program is not allowed
			// to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
			// page is referenced or modified.
    bool dirty;         // This bit is set by the hardware every time the
			// page is modified.
};

//宏定义函数指针
typedef TranslationEntry* (*GetTranslationEntry)();  
typedef void (*DeleteTranslationEntry)(TranslationEntry*);

extern "C" {
	__declspec(dllexport) TranslationEntry* GetTranslationEntryF();
	__declspec(dllexport) void DeleteTranslationEntryF(TranslationEntry*);
}

extern GetTranslationEntry getTranslationEntry;
extern DeleteTranslationEntry deleteTranslationEntry;

/* Liang DePeng +
 ****/



// Routines for converting Words and Short Words to and from the
// simulated machine's format of little endian.  If the host machine
// is little endian (DEC and Intel), these end up being NOPs.
//
// What is stored in each format:
//	host byte ordering:
//	   kernel data structures
//	   user registers
//	simulated machine byte ordering:
//	   contents of main memory

/**** add
 * Liang DePeng + */
// move from machine.h in MIPSVirtualMachine project
extern "C" {
	__declspec(dllexport) unsigned int WordToHost(unsigned int word);
	__declspec(dllexport) unsigned short ShortToHost(unsigned short shortword);
	__declspec(dllexport) unsigned int WordToMachine(unsigned int word);
	__declspec(dllexport) unsigned short ShortToMachine(unsigned short shortword);
}
//declare function pointer
typedef unsigned int (* FPWordToHost)(unsigned int/* word*/);
typedef unsigned short (* FPShortToHost)(unsigned short/* shortword*/);
typedef unsigned int (* FPWordToMachine)(unsigned int/* word*/);
typedef unsigned short (* FPShortToMachine)(unsigned short/* shortword*/);

extern FPWordToHost fpWordToHost;
extern FPShortToHost fpShortToHost;
extern FPWordToMachine fpWordToMachine;
extern FPShortToMachine fpShortToMachine;
/* Liang DePeng +
 ****/

#endif
