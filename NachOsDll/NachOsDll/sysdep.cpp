// sysdep.cc
//	Implementation of system-dependent interface.  Nachos uses the 
//	routines defined here, rather than directly calling the UNIX library,
//	to simplify porting between versions of UNIX, and even to
//	other systems, such as MSDOS.
//
//	On UNIX, almost all of these routines are simple wrappers
//	for the underlying UNIX system calls.
//
//	NOTE: all of these routines refer to operations on the underlying
//	host machine (e.g., the DECstation, SPARC, etc.), supporting the 
//	Nachos simulation code.  Nachos implements similar operations,
//	(such as opening a file), but those are implemented in terms
//	of hardware devices, which are simulated by calls to the underlying
//	routines in the host workstation OS.
//
//	This file includes lots of calls to C routines.  C++ requires
//	us to wrap all C definitions with a "extern "C" block".
// 	This prevents the internal forms of the names from being
// 	changed by the C++ compiler.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include <tchar.h>
#include "copyright.h"
#include "debug.h"
#include "sysdep.h"
#include "stdlib.h"
//#include "unistd.h"		/*zhong_tran - */
//#include "sys/time.h"		/*zhong_tran - */
//#include "sys/file.h"		/*zhong_tran - */
//#include <sys/socket.h>		/*zhong_tran - */
//#include <sys/un.h>		/*zhong_tran - */

//#ifdef SOLARIS		/*zhong_tran - */
// KMS
// for open()
#include <fcntl.h>
//#endif		/*zhong_tran - */

#ifdef LINUX	 // at this point, linux doesn't support mprotect 
#define NO_MPROT     
#endif
#ifdef DOS	// neither does DOS
#define NO_MPROT
#endif

//extern "C" {		/*zhong_tran - */
#include <signal.h>
//#include <sys/types.h>		/*zhong_tran - */

#include <windows.h>		/*zhong_tran + */
#include <io.h>		/*zhong_tran + */

#ifndef NO_MPROT 
#include <sys/mman.h>
#endif

// UNIX routines called by procedures in this file 

//#if defined CYGWIN		/*zhong_tran - */
//  size_t getpagesize(void);		/*zhong_tran - */
//#else		/*zhong_tran - */
// int getpagesize(void);		/*zhong_tran - */
//#endif		/*zhong_tran - */
//unsigned sleep(unsigned);		/*zhong_tran - */
//#ifdef SOLARIS
//int usleep(useconds_t);
//#else
//void usleep(unsigned int);  // rcgood - to avoid spinning processes.
//#endif


#ifndef NO_MPROT	

#ifdef OSF
#define OSF_OR_AIX
#endif
#ifdef AIX
#define OSF_OR_AIX
#endif

#ifdef OSF_OR_AIX
int mprotect(const void *, long unsigned int, int);
#else
int mprotect(char *, unsigned int, int);
#endif
#endif

#if defined(BSD) || defined(SOLARIS) || defined(LINUX)
//KMS
// added Solaris and LINUX
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
             struct timeval *timeout);
#else
int select(int numBits, void *readFds, void *writeFds, void *exceptFds, 
	struct timeval *timeout);
#endif

//int socket(int, int, int);		/*zhong_tran - */

#if defined(SUNOS) || defined(ULTRIX)
long tell(int);
int bind (int, const void*, int);
int recvfrom (int, void*, int, int, void*, int *);
int sendto (int, const void*, int, int, void*, int);
#endif

//}		/*zhong_tran - */

extern "C" {  /* Liang Depeng + */
	//由于Windows中没有bcopy和bzero函数，这里需要自己定义			/*zhong_tran + */
	__declspec(dllexport) void bcopy(const void *s1, void *s2, size_t n)
	{
		memcpy(s2,s1,n);
	}

	__declspec(dllexport) void bzero(void *s, size_t n)
	{
		memset(s,0,n);
	}


	//----------------------------------------------------------------------
	// CallOnUserAbort
	// 	Arrange that "func" will be called when the user aborts (e.g., by
	//	hitting ctl-C.
	//----------------------------------------------------------------------

	__declspec(dllexport) void CallOnUserAbort(void (*func)(int))
	{
		(void)signal(SIGINT, func);
	}

	//----------------------------------------------------------------------
	// RegisterSignal
	// 	Arrange that "func" will be called when NachOS get a signal from
	// 	the host operating system.
	//----------------------------------------------------------------------

	__declspec(dllexport) void RegisterSignalHandler(void (*func)(int), int sig)
	{
		(void)signal(sig, func);
	}

	//----------------------------------------------------------------------
	// Delay
	// 	Put the UNIX process running Nachos to sleep for x seconds,
	//	to give the user time to start up another invocation of Nachos
	//	in a different UNIX shell.
	//----------------------------------------------------------------------

	__declspec(dllexport) void Delay(int seconds)
	{
		//(void) sleep((unsigned) seconds);			/*zhong_tran - */
		Sleep(seconds * 1000);			/*zhong_tran + */
	}

	//----------------------------------------------------------------------
	// UDelay
	// 	Put the UNIX process running Nachos to sleep for x microseconds,
	//	to prevent an idle Nachos process from spinning...
	//----------------------------------------------------------------------

	__declspec(dllexport) void UDelay(unsigned int useconds)
	{
	//#ifdef SOLARIS
	//   usleep(useconds_t useconds);
	//#else
	//   usleep(useconds);
	//#endif /* SOLARIS */

	}

	//----------------------------------------------------------------------
	// Abort
	// 	Quit and drop core.
	//----------------------------------------------------------------------

	__declspec(dllexport) void Abort()
	{
		abort();
	}

	//----------------------------------------------------------------------
	// Exit
	// 	Quit without dropping core.
	//----------------------------------------------------------------------

	__declspec(dllexport) void MyExit(int exitCode)
	{
		exit(exitCode);
	}

	//----------------------------------------------------------------------
	// RandomInit
	// 	Initialize the pseudo-random number generator.  We use the
	//	now obsolete "srand" and "rand" because they are more portable!
	//----------------------------------------------------------------------

	__declspec(dllexport) void RandomInit(unsigned seed)
	{
		srand(seed);
	}

	//----------------------------------------------------------------------
	// RandomNumber
	// 	Return a pseudo-random number.
	//----------------------------------------------------------------------

	__declspec(dllexport) unsigned int RandomNumber()
	{
		return rand();
	}

	//----------------------------------------------------------------------
	// AllocBoundedArray
	// 	Return an array, with the two pages just before 
	//	and after the array unmapped, to catch illegal references off
	//	the end of the array.  Particularly useful for catching overflow
	//	beyond fixed-size thread execution stacks.
	//
	//	Note: Just return the useful part!
	//
	//	"size" -- amount of useful space needed (in bytes)
	//----------------------------------------------------------------------

	__declspec(dllexport) char * AllocBoundedArray(int size)
	{
	#ifdef NO_MPROT
		return new char[size];
	#else
		int pgSize = getpagesize();
		char *ptr = new char[pgSize * 2 + size];

		mprotect(ptr, pgSize, 0);
		mprotect(ptr + pgSize + size, pgSize, 0);
		return ptr + pgSize;
	#endif
	}

	//----------------------------------------------------------------------
	// DeallocBoundedArray
	// 	Deallocate an array of integers, unprotecting its two boundary pages.
	//
	//	"ptr" -- the array to be deallocated
	//	"size" -- amount of useful space in the array (in bytes)
	//----------------------------------------------------------------------

	#ifdef NO_MPROT
	__declspec(dllexport) void DeallocBoundedArray(char *ptr, int /* size */)
	{
		delete [] ptr;
	}
	#else
	__declspec(dllexport) void DeallocBoundedArray(char *ptr, int size)
	{
		int pgSize = getpagesize();

		mprotect(ptr - pgSize, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
		mprotect(ptr + size, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
		delete [] (ptr - pgSize);
	}
	#endif

	//----------------------------------------------------------------------
	// PollFile
	// 	Check open file or open socket to see if there are any 
	//	characters that can be read immediately.  If so, read them
	//	in, and return TRUE.
	//
	//	"fd" -- the file descriptor of the file to be polled
	//----------------------------------------------------------------------

	__declspec(dllexport) bool PollFile(int fd)
	{
	//#if defined(SOLARIS) || defined(LINUX)
	//// KMS
	//    fd_set rfd,wfd,xfd;
	//#else
	//    int rfd = (1 << fd), wfd = 0, xfd = 0;
	//#endif
	//    int retVal;
	//    struct timeval pollTime;
	//
	//#if defined(SOLARIS) || defined(LINUX)
	//// KMS
	//    FD_ZERO(&rfd);
	//    FD_ZERO(&wfd);
	//    FD_ZERO(&xfd);
	//    FD_SET(fd,&rfd);
	//#endif
	//
	//// don't wait if there are no characters on the file
	//    pollTime.tv_sec = 0;
	//    pollTime.tv_usec = 0;
	//
	//// poll file or socket
	//#if defined(BSD)
	//    retVal = select(32, (fd_set*)&rfd, (fd_set*)&wfd, (fd_set*)&xfd, &pollTime);
	//#elif defined(SOLARIS) || defined(LINUX)
	//    // KMS
	//    retVal = select(32, &rfd, &wfd, &xfd, &pollTime);
	//#else
	//    retVal = select(32, &rfd, &wfd, &xfd, &pollTime);
	//#endif
	//
	//    ASSERT((retVal == 0) || (retVal == 1));
	//    if (retVal == 0)
	//	return FALSE;                 		// no char waiting to be read
	//    return TRUE;			/*zhong_tran - */
		if (filelength(fd) == tell(fd))		/*zhong_tran + */
			return false;
		else
			return true;
	}

	__declspec(dllexport) bool PollConsole(HANDLE hConsoleInput)/*zhong_tran + */
	{
		DWORD cNumRead, i;
		INPUT_RECORD irInBuf[128];
		PeekConsoleInput(hConsoleInput, irInBuf, 128, &cNumRead);
		for (i=0; i<cNumRead; i++)
		{
			if (irInBuf[i].EventType == KEY_EVENT)
			{
	//			Sleep(1);
				return true;
			}
		}
	//	Sleep(1);
		return false;
	}

	//----------------------------------------------------------------------
	// OpenForWrite
	// 	Open a file for writing.  Create it if it doesn't exist; truncate it 
	//	if it does already exist.  Return the file descriptor.
	//
	//	"name" -- file name
	//----------------------------------------------------------------------

	__declspec(dllexport) int OpenForWrite(char *name)
	{
		//int fd = open(name, O_RDWR|O_CREAT|O_TRUNC, 0666);		/*zhong_tran - */
		int fd = open(name, O_RDWR|O_CREAT|O_TRUNC|_O_BINARY, 0666);		/*zhong_tran + */
		ASSERT(fd >= 0); 
		return fd;
	}

	//----------------------------------------------------------------------
	// OpenForReadWrite
	// 	Open a file for reading or writing.
	//	Return the file descriptor, or error if it doesn't exist.
	//
	//	"name" -- file name
	//----------------------------------------------------------------------

	__declspec(dllexport) int OpenForReadWrite(char *name, bool crashOnError)
	{
		//int fd = open(name, O_RDWR, 0);		/*zhong_tran - */
		int fd = open(name, O_RDWR|_O_BINARY, 0);/*zhong_tran + */
		ASSERT(!crashOnError || fd >= 0);
		return fd;
	}

	//----------------------------------------------------------------------
	// Read
	// 	Read characters from an open file.  Abort if read fails.
	//----------------------------------------------------------------------

	__declspec(dllexport) void Read(int fd, char *buffer, int nBytes)
	{
		int retVal = read(fd, buffer, nBytes);
		ASSERT(retVal == nBytes);
	}

	//----------------------------------------------------------------------
	// ReadPartial
	// 	Read characters from an open file, returning as many as are
	//	available.
	//----------------------------------------------------------------------

	__declspec(dllexport) int ReadPartial(int fd, char *buffer, int nBytes)
	{
		return read(fd, buffer, nBytes);
	}


	//----------------------------------------------------------------------
	// WriteFile
	// 	Write characters to an open file.  Abort if write fails.
	//----------------------------------------------------------------------

	__declspec(dllexport) void WriteFkFile(int fd, char *buffer, int nBytes)
	{
		int retVal = write(fd, buffer, nBytes);
		ASSERT(retVal == nBytes);
	}

	//----------------------------------------------------------------------
	// Lseek
	// 	Change the location within an open file.  Abort on error.
	//----------------------------------------------------------------------

	__declspec(dllexport) void Lseek(int fd, int offset, int whence)
	{
		int retVal = lseek(fd, offset, whence);
		ASSERT(retVal >= 0);
	}

	//----------------------------------------------------------------------
	// Tell
	// 	Report the current location within an open file.
	//----------------------------------------------------------------------

	__declspec(dllexport) int Tell(int fd)
	{
	#if defined(BSD) || defined(SOLARIS) || defined(LINUX)
		return lseek(fd,0,SEEK_CUR); // 386BSD doesn't have the tell() system call
									 // neither do Solaris and Linux  -KMS
	#else
		return tell(fd);
	#endif
	}


	//----------------------------------------------------------------------
	// Close
	// 	Close a file.  Abort on error.
	//----------------------------------------------------------------------

	__declspec(dllexport) int MyClose(int fd)
	{
		int retVal = close(fd);
		ASSERT(retVal >= 0); 
		return retVal;
	}

	//----------------------------------------------------------------------
	// Unlink
	// 	Delete a file.
	//----------------------------------------------------------------------

	__declspec(dllexport) bool Unlink(char *name)
	{
		//return unlink(name);		/*zhong_tran - */
		if (unlink(name) == 0)		/*zhong_tran + */
			return true;
		else
			return false;
	}

	////----------------------------------------------------------------------
	//// OpenSocket
	//// 	Open an interprocess communication (IPC) connection.  For now, 
	////	just open a datagram port where other Nachos (simulating 
	////	workstations on a network) can send messages to this Nachos.
	////----------------------------------------------------------------------
	//
	//int
	//OpenSocket()
	//{
	//    int sockID;
	//    
	//    sockID = socket(AF_UNIX, SOCK_DGRAM, 0);
	//    ASSERT(sockID >= 0);
	//
	//    return sockID;
	//}
	//
	////----------------------------------------------------------------------
	//// CloseSocket
	//// 	Close the IPC connection. 
	////----------------------------------------------------------------------
	//
	//void
	//CloseSocket(int sockID)
	//{
	//    (void) close(sockID);
	//}
	//
	////----------------------------------------------------------------------
	//// InitSocketName
	//// 	Initialize a UNIX socket address -- magical!
	////----------------------------------------------------------------------
	//
	//static void 
	//InitSocketName(struct sockaddr_un *uname, char *name)
	//{
	//    uname->sun_family = AF_UNIX;
	//    strcpy(uname->sun_path, name);
	//}
	//
	////----------------------------------------------------------------------
	//// AssignNameToSocket
	//// 	Give a UNIX file name to the IPC port, so other instances of Nachos
	////	can locate the port. 
	////----------------------------------------------------------------------
	//
	//void
	//AssignNameToSocket(char *socketName, int sockID)
	//{
	//    struct sockaddr_un uName;
	//    int retVal;
	//
	//    (void) unlink(socketName);    // in case it's still around from last time
	//
	//    InitSocketName(&uName, socketName);
	//    retVal = bind(sockID, (struct sockaddr *) &uName, sizeof(uName));
	//    ASSERT(retVal >= 0);
	//    DEBUG(dbgNet, "Created socket " << socketName);
	//}
	//
	////----------------------------------------------------------------------
	//// DeAssignNameToSocket
	//// 	Delete the UNIX file name we assigned to our IPC port, on cleanup.
	////----------------------------------------------------------------------
	//void
	//DeAssignNameToSocket(char *socketName)
	//{
	//    (void) unlink(socketName);
	//}
	//
	////----------------------------------------------------------------------
	//// PollSocket
	//// 	Return TRUE if there are any messages waiting to arrive on the
	////	IPC port.
	////----------------------------------------------------------------------
	//bool
	//PollSocket(int sockID)
	//{
	//    return PollFile(sockID);	// on UNIX, socket ID's are just file ID's
	//}
	//
	////----------------------------------------------------------------------
	//// ReadFromSocket
	//// 	Read a fixed size packet off the IPC port.  Abort on error.
	////----------------------------------------------------------------------
	//void
	//ReadFromSocket(int sockID, char *buffer, int packetSize)
	//{
	//    int retVal;
	//#if defined(__GNUC__) && __GNUC__ < 3
	//    extern int errno;
	//#endif
	//    struct sockaddr_un uName;
	//#ifdef LINUX
	//    socklen_t size = sizeof(uName);
	//#else
	//    int size = sizeof(uName);
	//#endif
	//   
	//    retVal = recvfrom(sockID, buffer, packetSize, 0,
	//				   (struct sockaddr *) &uName, &size);
	//
	//    if (retVal != packetSize) {
	//        perror("in recvfrom");
	//#if defined (CYGWIN) || (__GNUC__ >= 3)
	//	cerr << "called with " << packetSize << ", got back " << retVal 
	//						<< ", and " << "\n";
	//#else 	
	//        cerr << "called with " << packetSize << ", got back " << retVal 
	//						<< ", and " << errno << "\n";
	//#endif 
	//    }
	//    ASSERT(retVal == packetSize);
	//}
	//
	////----------------------------------------------------------------------
	////    modified by KMS to add retry...
	//// SendToSocket
	//// 	Transmit a fixed size packet to another Nachos' IPC port.
	////	Try 10 times with a one second delay between attempts.
	////      This is useful, e.g., to give the other socket a chance
	////      to get set up.
	////      Terminate if we still fail after 10 tries.
	////----------------------------------------------------------------------
	//void
	//SendToSocket(int sockID, char *buffer, int packetSize, char *toName)
	//{
	//    struct sockaddr_un uName;
	//    int retVal;
	//    int retryCount;
	//
	//    InitSocketName(&uName, toName);
	//
	//    for(retryCount=0;retryCount < 10;retryCount++) {
	//      retVal = sendto(sockID, buffer, packetSize, 0, 
	//			(struct sockaddr *) &uName, sizeof(uName));
	//      if (retVal == packetSize) return;
	//      // if we did not succeed, we should see a negative
	//      // return value indicating complete failure.  If we
	//      // don't, something fishy is going on...
	//      ASSERT(retVal < 0);
	//      // wait a second before trying again
	//      Delay(1);
	//    }
	//    // At this point, we have failed many times
	//    // The most common reason for this is that the target machine
	//    // has halted and its socket no longer exists.
	//    // We simply do nothing (drop the packet).
	//    // This may mask other kinds of failures, but it is the
	//    // right thing to do in the common case.
	//}

	#define STRINGSIZE 256		/*zhong_tran + */

	__declspec(dllexport) HANDLE OpenPipe(LPCWSTR name, LPOVERLAPPED lpOverlapped)		/*zhong_tran + */
	{
		HANDLE hPipe;
		hPipe = CreateNamedPipe(name, PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			1, STRINGSIZE, STRINGSIZE, 50, NULL);
		ASSERT(hPipe != INVALID_HANDLE_VALUE);
		memset(lpOverlapped, 0, sizeof(OVERLAPPED));
		lpOverlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		ConnectNamedPipe(hPipe, lpOverlapped);
		return hPipe;
	}

	__declspec(dllexport) void ClosePipe(HANDLE hPipe, LPOVERLAPPED lpOverlapped)		/*zhong_tran + */
	{
		CloseHandle(lpOverlapped->hEvent);
		CloseHandle(hPipe);
	}

	__declspec(dllexport) bool PollPipe(HANDLE hPipe, LPOVERLAPPED lpOverlapped)		/*zhong_tran + */
	{
		DWORD NumberOfBytesTransferred;
		if (GetOverlappedResult(hPipe, lpOverlapped, &NumberOfBytesTransferred, FALSE))
			return true;
		else
			return false;
	}

	__declspec(dllexport) void ReadFromPipe(HANDLE hPipe, char *buffer, int packetSize, LPOVERLAPPED lpOverlapped)		/*zhong_tran + */
	{
		DWORD retVal;
		ResetEvent(lpOverlapped->hEvent);
		ReadFile(hPipe, buffer, packetSize, &retVal, lpOverlapped);
		DisconnectNamedPipe(hPipe);
		ResetEvent(lpOverlapped->hEvent);
		ConnectNamedPipe(hPipe, lpOverlapped);
		ASSERT(retVal == packetSize);
	}

	__declspec(dllexport) void SendToPipe(char *buffer, int packetSize, LPCWSTR toName)		/*zhong_tran + */
	{
		int retryCount;
		HANDLE hPipe;
		DWORD cbWritten;
		for(retryCount=0; retryCount<10; retryCount++)
		{
			hPipe = CreateFile(toName, GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, //为了与命名管道连接，此参数应一直为OPEN_EXISTING
				0, NULL);
			if (hPipe == INVALID_HANDLE_VALUE)
			{
				//std::cout <<GetLastError()<<std::endl;
				Delay(1);
			}
			else
			{
				WriteFile(hPipe, buffer, packetSize, &cbWritten, NULL);
				CloseHandle(hPipe);
				break;
			}
		}
	}

} //extern "C"
