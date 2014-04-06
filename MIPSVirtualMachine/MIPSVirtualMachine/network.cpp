// network.cc 
//	Routines to simulate a network interface, using UNIX sockets
//	to deliver packets between multiple invocations of nachos.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "network.h"
#include "main.h"
#include "sysdep.h"
#include <Windows.h>		/*zhong_tran + */

//-----------------------------------------------------------------------
// NetworkInput::NetworkInput
// 	Initialize the simulation for the network input
//
//   	"toCall" is the interrupt handler to call when packet arrives
//-----------------------------------------------------------------------

NetworkInput::NetworkInput(CallBackObj *toCall)
{
    // set up the stuff to emulate asynchronous interrupts
    callWhenAvail = toCall;
    packetAvail = FALSE;
    inHdr.length = 0;
    
    //sock = OpenSocket();/*zhong_tran - */
    //sprintf(sockName, "SOCKET_%d", kernel->hostName);/*zhong_tran - */
    //AssignNameToSocket(sockName, sock);		 // Bind socket to a filename /*zhong_tran - */
						 // in the current directory.
	lpOverlapped = new OVERLAPPED;		/*zhong_tran + */
	
	sprintf(pipeName, "\\\\.\\pipe\\host_%d", hostName);		/*zhong_tran + */

	WCHAR    str3[32];				/* Liang DePeng + */
	MultiByteToWideChar( 0,0, pipeName, 33, str3, 32); /* Liang DePeng + */
	LPCWSTR cstr4 = str3;	/* Liang DePeng + */

	hPipe = (* fp_OpenPipe)(str3, lpOverlapped);		/*zhong_tran + */	/* Liang DePeng + */
	//hPipe = OpenPipe(str3, lpOverlapped);		/*zhong_tran + */	/* Liang DePeng - */
    // start polling for incoming packets
	//kernel->interrupt->Schedule(this, NetworkTime, NetworkRecvInt);		/* Liang DePeng - */
    interrupt->Schedule(this, NetworkTime, NetworkRecvInt);					/* Liang DePeng + */
}

//-----------------------------------------------------------------------
// NetworkInput::NetworkInput
// 	Deallocate the simulation for the network input
//		(basically, deallocate the input mailbox)
//-----------------------------------------------------------------------

NetworkInput::~NetworkInput()
{
    //CloseSocket(sock);/*zhong_tran - */
    //DeAssignNameToSocket(sockName);/*zhong_tran - */
	(* fp_ClosePipe)(hPipe, lpOverlapped);		/*zhong_tran + */	/* Liang DePeng + */
	//ClosePipe(hPipe, lpOverlapped);		/*zhong_tran + */	/* Liang DePeng - */
	delete lpOverlapped;
}

//-----------------------------------------------------------------------
// NetworkInput::CallBack
//	Simulator calls this when a packet may be available to
//	be read in from the simulated network.
//
//      First check to make sure packet is available & there's space to
//	pull it in.  Then invoke the "callBack" registered by whoever 
//	wants the packet.
//-----------------------------------------------------------------------

void
NetworkInput::CallBack()
{
    // schedule the next time to poll for a packet
    //kernel->interrupt->Schedule(this, NetworkTime, NetworkRecvInt);	/* Liang DePeng - */
	interrupt->Schedule(this, NetworkTime, NetworkRecvInt);			/* Liang DePeng + */

    if (inHdr.length != 0) 	// do nothing if packet is already buffered
		return;		
    //if (!PollSocket(sock)) 	// do nothing if no packet to be read		/*zhong_tran - */
	//if (!PollPipe(hPipe, lpOverlapped))		/*zhong_tran + */	/* Liang DePeng - */
	if (!(* fp_PollPipe)(hPipe, lpOverlapped))		/*zhong_tran + */	/* Liang DePeng + */
		return;

    // otherwise, read packet in
    char *buffer = new char[MaxWireSize];
    //ReadFromSocket(sock, buffer, MaxWireSize);		/*zhong_tran - */
	//ReadFromPipe(hPipe, buffer, MaxWireSize, lpOverlapped);		/*zhong_tran + */	/* Liang DePeng - */
	(* fp_ReadFromPipe)(hPipe, buffer, MaxWireSize, lpOverlapped);		/*zhong_tran + */	/* Liang DePeng + */

    // divide packet into header and data
    inHdr = *(PacketHeader *)buffer;
    ASSERT((inHdr.to == hostName) && (inHdr.length <= MaxPacketSize));
    //bcopy(buffer + sizeof(PacketHeader), inbox, inHdr.length);	/* Liang DePeng - */
	(* fp_bcopy)(buffer + sizeof(PacketHeader), inbox, inHdr.length);	/* Liang DePeng + */
    delete [] buffer ;

    DEBUG(dbgNet, "Network received packet from " << inHdr.from << ", length " << inHdr.length);
    //kernel->stats->numPacketsRecvd++; /* Liang DePeng - */
	stats->numPacketsRecvd++;			/* Liang DePeng + */

    // tell post office that the packet has arrived
    callWhenAvail->CallBack();
}

//-----------------------------------------------------------------------
// NetworkInput::Receive
// 	Read a packet, if one is buffered
//-----------------------------------------------------------------------

PacketHeader
NetworkInput::Receive(char* data)
{
    PacketHeader hdr = inHdr;

    inHdr.length = 0;
    if (hdr.length != 0) {
		(* fp_bcopy)(inbox, data, hdr.length);	/* Liang DePeng + */	
    	//bcopy(inbox, data, hdr.length);	/* Liang DePeng - */	
    }
    return hdr;
}

//-----------------------------------------------------------------------
// NetworkOutput::NetworkOutput
// 	Initialize the simulation for sending network packets
//
//   	"reliability" says whether we drop packets to emulate unreliable links
//   	"toCall" is the interrupt handler to call when next packet can be sent
//-----------------------------------------------------------------------

NetworkOutput::NetworkOutput(double reliability, CallBackObj *toCall)
{
    if (reliability < 0) chanceToWork = 0;
    else if (reliability > 1) chanceToWork = 1;
    else chanceToWork = reliability;

    // set up the stuff to emulate asynchronous interrupts
    callWhenDone = toCall;
    sendBusy = FALSE;
    //sock = OpenSocket();		/*zhong_tran - */
}

//-----------------------------------------------------------------------
// NetworkOutput::~NetworkOutput
// 	Deallocate the simulation for sending network packets
//-----------------------------------------------------------------------

NetworkOutput::~NetworkOutput()
{
    //CloseSocket(sock);		/*zhong_tran - */
}

//-----------------------------------------------------------------------
// NetworkOutput::CallBack
// 	Called by simulator when another packet can be sent.
//-----------------------------------------------------------------------

void
NetworkOutput::CallBack()
{
    sendBusy = FALSE;
    //kernel->stats->numPacketsSent++;	/* Liang DePeng - */
	stats->numPacketsSent++;			/* Liang DePeng + */
    callWhenDone->CallBack();
}

//-----------------------------------------------------------------------
// NetworkOutput::Send
// 	Send a packet into the simulated network, to the destination in hdr.
// 	Concatenate hdr and data, and schedule an interrupt to tell the user 
// 	when the next packet can be sent 
//
// 	Note we always pad out a packet to MaxWireSize before putting it into
// 	the socket, because it's simpler at the receive end.
//-----------------------------------------------------------------------

void
NetworkOutput::Send(PacketHeader hdr, char* data)
{
    char toName[32];
	//LPCWSTR toName;

    //sprintf(toName, "SOCKET_%d", (int)hdr.to);		/*zhong_tran - */
	sprintf(toName, "\\\\.\\pipe\\host_%d", (int)hdr.to);		/*zhong_tran + */
    
    ASSERT((sendBusy == FALSE) && (hdr.length > 0) && 
	(hdr.length <= MaxPacketSize) && (hdr.from == hostName));
    DEBUG(dbgNet, "Sending to addr " << hdr.to << ", length " << hdr.length);

    //kernel->interrupt->Schedule(this, NetworkTime, NetworkSendInt);	/* Liang DePeng - */
	interrupt->Schedule(this, NetworkTime, NetworkSendInt);				/* Liang DePeng + */

    //if (RandomNumber() % 100 >= chanceToWork * 100) { // emulate a lost packet	/* Liang DePeng - */
	if ((* fp_RandomNumber)() % 100 >= chanceToWork * 100) { // emulate a lost packet	/* Liang DePeng + */
		DEBUG(dbgNet, "oops, lost it!");
		return;
    }

    // concatenate hdr and data into a single buffer, and send it out
    char *buffer = new char[MaxWireSize];
    *(PacketHeader *)buffer = hdr;
	(* fp_bcopy)(data, buffer + sizeof(PacketHeader), hdr.length);	/* Liang DePeng + */
    //bcopy(data, buffer + sizeof(PacketHeader), hdr.length);	/* Liang DePeng - */
    //SendToSocket(sock, buffer, MaxWireSize, toName);		/*zhong_tran - */

	WCHAR  str3[32];								/* Liang DePeng + */
	MultiByteToWideChar( 0,0, toName, 33, str3, 32);  /* Liang DePeng + */

	/*int size=MultiByteToWideChar(CP_ACP,0,toName,-1,NULL,0);
	if(!MultiByteToWideChar(CP_ACP,0,toName,-1,str3,size))
		std::cout <<"ke\n";*/
	
	LPCWSTR cstr4 = str3;  /* Liang DePeng + */
	//SendToPipe(buffer, MaxWireSize, str3);		/*zhong_tran + */	/* Liang DePeng - */

	(* fp_SendToPipe)(buffer, MaxWireSize, str3);		/*zhong_tran + */	/* Liang DePeng + */
    delete [] buffer;
}
