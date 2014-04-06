// main.h 
//	This file defines the Nachos global variables
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef MAIN_H
#define MAIN_H

#include "copyright.h"
#include "debug.h"

#include "utility.h"        /* Liang DePeng + */
#include "interrupt.h"      /* Liang DePeng + */
#include "stats.h"          /* Liang DePeng + */
#include "machine.h"        /* Liang DePeng + */
#include "post.h"			/* Liang DePeng + */
#include "translate.h"		/* Liang DePeng + */
//#include "timer.h"
#include "openfile.h"		/* Liang DePeng + */
#include "proctable.h"		/* Liang DePeng + */   

#include "kernel.h"  

//extern Kernel *kernel;   /* Liang DePeng - */

/**** add
 * Liang DePeng + */		
extern HMODULE hDLL;		

extern  Statistics *stats;	  // performance metrics   
extern  Interrupt *interrupt;  // interrupt status     
extern  Machine *machine;    // the simulated CPU      
extern PostOfficeInput *postOfficeIn;
extern PostOfficeOutput *postOfficeOut;
extern double reliability;         // likelihood messages are dropped
extern int hostName;       // machine identifier       

/* Liang DePeng +
 ****/


#endif // MAIN_H

