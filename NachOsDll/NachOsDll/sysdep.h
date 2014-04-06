// sysdep.h 
//	System-dependent interface.  Nachos uses the routines defined
//	here, rather than directly calling the UNIX library functions, to
//	simplify porting between versions of UNIX, and even to
//	other systems, such as MSDOS and the Macintosh.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSDEP_H
#define SYSDEP_H

#include "copyright.h"
#include <tchar.h>			/* Liang Depeng + */
//#include "IOSTREAM.H"		/* Liang Depeng - */
#include <iostream>         /* Liang Depeng + */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>		/* Liang Depeng + */
#include <io.h>				/* Liang Depeng + */

#define DOS				/*zhong_tran + */
struct _OVERLAPPED;			/*zhong_tran + */
typedef struct _OVERLAPPED OVERLAPPED, *LPOVERLAPPED;			/*zhong_tran + */
typedef void *HANDLE;			/*zhong_tran + */

/**** delete
 * Liang DePeng - */
/*
// Process control: abort, exit, and sleep
extern void Abort();
extern void Exit(int exitCode);
extern void Delay(int seconds);
extern void UDelay(unsigned int usec);// rcgood - to avoid spinners.

// Initialize system so that cleanUp routine is called when user hits ctl-C
extern void CallOnUserAbort(void (*cleanup)(int));

// Initialize the pseudo random number generator
extern void RandomInit(unsigned seed);
extern unsigned int RandomNumber();

// Allocate, de-allocate an array, such that de-referencing
// just beyond either end of the array will cause an error
extern char *AllocBoundedArray(int size);
extern void DeallocBoundedArray(char *p, int size);

// Check file to see if there are any characters to be read.
// If no characters in the file, return without waiting.
extern bool PollFile(int fd);*/
//extern bool PollConsole(HANDLE hConsoleInput);		/*zhong_tran + */

/*
// File operations: open/read/write/lseek/close, and check for error
// For simulating the disk and the console devices.
extern int OpenForWrite(char *name);
extern int OpenForReadWrite(char *name, bool crashOnError);
extern void Read(int fd, char *buffer, int nBytes);
extern int ReadPartial(int fd, char *buffer, int nBytes);
extern void WriteFile(int fd, char *buffer, int nBytes);
extern void Lseek(int fd, int offset, int whence);
extern int Tell(int fd);
extern int Close(int fd);
extern bool Unlink(char *name);
*/
// Other C library routines that are used by Nachos.
// These are assumed to be portable, so we don't include a wrapper.
//extern "C" {			/*zhong_tran - */

int atoi(const char *str);
double atof(const char *str);
int abs(int i);
//void bcopy(const void *s1, void *s2, size_t n);
//void bzero(void *s, size_t n);*/
//}			/*zhong_tran - */
/*
// Interprocess communication operations, for simulating the network
//extern int OpenSocket();			/*zhong_tran - */
//extern void CloseSocket(int sockID);			/*zhong_tran - */
//extern void AssignNameToSocket(char *socketName, int sockID);			/*zhong_tran - */
//extern void DeAssignNameToSocket(char *socketName);			/*zhong_tran - */
//extern bool PollSocket(int sockID);			/*zhong_tran - */
//extern void ReadFromSocket(int sockID, char *buffer, int packetSize);			/*zhong_tran - */
//extern void SendToSocket(int sockID, char *buffer, int packetSize,char *toName);			/*zhong_tran - */

//extern HANDLE OpenPipe(char* name, LPOVERLAPPED lpOverlapped);	 /* Liang Depeng - */
//extern HANDLE OpenPipe(LPCWSTR name, LPOVERLAPPED lpOverlapped);	 /* Liang Depeng + */

//extern void ClosePipe(HANDLE hPipe, LPOVERLAPPED lpOverlapped);		/*zhong_tran + */
//extern bool PollPipe(HANDLE hPipe, LPOVERLAPPED lpOverlapped);		/*zhong_tran + */
//extern void ReadFromPipe(HANDLE hPipe, char *buffer, int packetSize, LPOVERLAPPED lpOverlapped);		/*zhong_tran + */

//extern void SendToPipe(char *buffer, int packetSize, char *toName);	/* Liang Depeng - */
//extern void SendToPipe(char *buffer, int packetSize, LPCWSTR toName);	/* Liang Depeng + */

/* Liang DePeng -
 ****/

/**** modify
 * Liang DePeng + */
extern "C" { 
	// Process control: abort, exit, and sleep
	__declspec(dllexport) void Abort();
	__declspec(dllexport) void MyExit(int exitCode);
	__declspec(dllexport) void Delay(int seconds);
	__declspec(dllexport) void UDelay(unsigned int usec);// rcgood - to avoid spinners.

	// Initialize system so that cleanUp routine is called when user hits ctl-C
	__declspec(dllexport) void CallOnUserAbort(void (*cleanup)(int));
	__declspec(dllexport) void RegisterSignalHandler(void (*func)(int), int sig);

	// Initialize the pseudo random number generator
	__declspec(dllexport) void RandomInit(unsigned seed);
	__declspec(dllexport) unsigned int RandomNumber();

	// Allocate, de-allocate an array, such that de-referencing
	// just beyond either end of the array will cause an error
	__declspec(dllexport) char *AllocBoundedArray(int size);
	__declspec(dllexport) void DeallocBoundedArray(char *p, int size);

	// Check file to see if there are any characters to be read.
	// If no characters in the file, return without waiting.
	__declspec(dllexport) bool PollFile(int fd);
	__declspec(dllexport) bool PollConsole(HANDLE hConsoleInput);		/*zhong_tran + */

	// File operations: open/read/write/lseek/close, and check for error
	// For simulating the disk and the console devices.
	__declspec(dllexport) int OpenForWrite(char *name);
	__declspec(dllexport) int OpenForReadWrite(char *name, bool crashOnError);
	__declspec(dllexport) void Read(int fd, char *buffer, int nBytes);
	__declspec(dllexport) int ReadPartial(int fd, char *buffer, int nBytes);
	__declspec(dllexport) void WriteFkFile(int fd, char *buffer, int nBytes);
	__declspec(dllexport) void Lseek(int fd, int offset, int whence);
	__declspec(dllexport) int Tell(int fd);
	__declspec(dllexport) int MyClose(int fd);
	__declspec(dllexport) bool Unlink(char *name);

	
	__declspec(dllexport) void bcopy(const void *s1, void *s2, size_t n);
	__declspec(dllexport) void bzero(void *s, size_t n);

	//extern HANDLE OpenPipe(char* name, LPOVERLAPPED lpOverlapped);	 /* Liang Depeng - */
	__declspec(dllexport)  HANDLE OpenPipe(LPCWSTR name, LPOVERLAPPED lpOverlapped);	 /* Liang Depeng + */

	__declspec(dllexport)  void ClosePipe(HANDLE hPipe, LPOVERLAPPED lpOverlapped);		/*zhong_tran + */
	__declspec(dllexport)  bool PollPipe(HANDLE hPipe, LPOVERLAPPED lpOverlapped);		/*zhong_tran + */
	__declspec(dllexport)  void ReadFromPipe(HANDLE hPipe, char *buffer, int packetSize, LPOVERLAPPED lpOverlapped);		/*zhong_tran + */

	//extern void SendToPipe(char *buffer, int packetSize, char *toName);	/* Liang Depeng - */
	__declspec(dllexport)  void SendToPipe(char *buffer, int packetSize, LPCWSTR toName);	/* Liang Depeng + */

}
/* Liang DePeng +
 ****/


/**** add
 * Liang DePeng + */ 
// declare function pointer to the corresponding export function above
typedef void (* FpAbort)();
typedef void (* FpExit)(int/*exitCode*/);
typedef void (* FpDelay)(int/* seconds*/);
typedef void (* FpUDelay)(unsigned int/* usec*/);

typedef void (* FpCallOnUserAbort)(void (*cleanup)(int));
typedef void (* FpRegisterSignalHandler)(void (*func)(int), int/* sig*/);

typedef void (* FpRandomInit)(unsigned/* seed*/);
typedef unsigned int (* FpRandomNumber)();

typedef char* (* FpAllocBoundedArray)(int/* size*/);
typedef void (* FpDeallocBoundedArray)(char* /*p*/, int/* size*/);

typedef bool (* FpPollFile)(int/* fd*/);
typedef bool (* FpPollConsole)(HANDLE/* hConsoleInput*/);		

typedef int (* FpOpenForWrite)(char* /*name*/);
typedef int (* FpOpenForReadWrite)(char* /*name*/, bool/* crashOnError*/);
typedef void (* FpRead)(int/* fd*/, char* /*buffer*/, int/* nBytes*/);
typedef int (* FpReadPartial)(int/* fd*/, char* /*buffer*/, int/* nBytes*/);
typedef void (* FpWriteFkFile)(int/* fd*/, char* /*buffer*/, int/* nBytes*/);
typedef void (* FpLseek)(int/* fd*/, int/* offset*/, int/* whence*/);
typedef int (* FpTell)(int/* fd*/);
typedef int (* FpClose)(int/* fd*/);
typedef bool (* FpUnlink)(char* /*name*/);
	
typedef void (* Fpbcopy)(const void* /*s1*/, void* /*s2*/, size_t/* n*/);
typedef void (* Fpbzero)(void* /*s*/, size_t/* n*/);

typedef  HANDLE (* FpOpenPipe)(LPCWSTR/* name*/, LPOVERLAPPED/* lpOverlapped*/);	 

typedef  void (* FpClosePipe)(HANDLE/* hPipe*/, LPOVERLAPPED/* lpOverlapped*/);		
typedef  bool (* FpPollPipe)(HANDLE/* hPipe*/, LPOVERLAPPED/* lpOverlapped*/);		
typedef  void (* FpReadFromPipe)(HANDLE/* hPipe*/, char* /*buffer*/, int/* packetSize*/, LPOVERLAPPED/* lpOverlapped*/);		

typedef  void (* FpSendToPipe)(char* /*buffer*/, int/* packetSize*/, LPCWSTR/* toName*/);	

//declare global function pointer variable to the corresponding function pointer above
extern FpAbort fp_Abort;
extern FpExit fp_Exit;
extern FpDelay fp_Delay;
extern FpUDelay fp_UDelay;

extern FpCallOnUserAbort fp_CallOnUserAbort;
extern FpRegisterSignalHandler fp_RegisterSignalHandler; 

extern FpRandomInit fp_RandomInit;
extern FpRandomNumber fp_RandomNumber;

extern FpAllocBoundedArray fp_AllocBoundedArray;
extern FpDeallocBoundedArray fp_DeallocBoundedArray;

extern FpPollFile fp_PollFile;
extern FpPollConsole fp_PollConsole;		

extern FpOpenForWrite fp_OpenForWrite;
extern FpOpenForReadWrite fp_OpenForReadWrite;
extern FpRead fp_Read;
extern FpReadPartial fp_ReadPartial;
extern FpWriteFkFile fp_WriteFkFile;
extern FpLseek fp_Lseek;
extern FpTell fp_Tell;
extern FpClose fp_Close;
extern FpUnlink fp_Unlink;

extern Fpbcopy fp_bcopy;
extern Fpbzero fp_bzero;

extern FpOpenPipe fp_OpenPipe;	 

extern  FpClosePipe fp_ClosePipe;		
extern  FpPollPipe fp_PollPipe;		
extern  FpReadFromPipe fp_ReadFromPipe;		

extern  FpSendToPipe fp_SendToPipe;

/* Liang DePeng +
 ****/

#endif // SYSDEP_H
