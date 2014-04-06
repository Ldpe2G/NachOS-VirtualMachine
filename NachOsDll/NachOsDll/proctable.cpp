// proctable.cc
//   Routines to implement the process table
//

#include "proctable.h"
#include "table.h"
#include "synch.h"


/**** add
 * Liang DePeng + */
class __declspec(dllexport) MyProcTable: public ProcTable {
public:
	MyProcTable():ProcTable(){}                       // constructor.
	~MyProcTable();                     // destructor

      int GetNewProcId(int parentId);     // allocate an entry in the table
                                        // for a new process.  "parentId"
                                        // is the identifier of the parent,
                                        // or a negative number if there
                                        // is no parent.  return an
                                        // identifier for the new process

      void ProcIsDone(int id, int status); // the specified process has exited

      bool GetProcStatus(int childId,int parentId,int* status);
                                        // get the exit status of the
                                        // specified process.  if that
                                        // process is not done, caller
                                        // will sleep until it is done.
                                        // childId is the id of the process
                                        // whose status is wanted, parentId
                                        // is the id of the process that
                                        // wants it.
                                        // returns FALSE if the specified
                                        // process is not a child of the
                                        // caller, else TRUE
};

extern "C" {
	__declspec(dllexport) ProcTable* GetProcTableF(){
		return new MyProcTable;
	}
	
	__declspec(dllexport) void DeleteProcTableF(ProcTable* pt){
		delete pt;
	}
}
/* Liang DePeng +
 ****/

//-------------------------------------------------------------
// ProcTableEntry::ProcTableEntry
//     Create a new entry in the process table;
//
//-------------------------------------------------------------

ProcTableEntry::ProcTableEntry()
{
  waitingRoom = new Condition("");
  ASSERT(waitingRoom != (Condition*)0);
}

//-------------------------------------------------------------
// ProcTableEntry::~ProcTableEntry
//     Destroy an entry in the process table;
//
//-------------------------------------------------------------

ProcTableEntry::~ProcTableEntry()
{
  delete waitingRoom;
}

//-------------------------------------------------------------
// ProcTable::ProcTable
//     Create a new process table;
//
//-------------------------------------------------------------

ProcTable::ProcTable()
{
  table = new Table(MaxNumProcesses);
  ASSERT(table != (Table*)0);
  //lock = new Lock("proc table lock");	/* Liang DePeng - */
  lock = GetLockF("proc table lock");	/* Liang DePeng + */
  ASSERT(lock != (FLock*)0);
}

//-------------------------------------------------------------
// ProcTable::~ProcTable
//     Destroy a process table;
//
//-------------------------------------------------------------

MyProcTable::~MyProcTable()
{
  delete table;
  //delete lock;	/* Liang DePeng - */
  DeleteLockF(lock);	/* Liang DePeng + */	
}

//-------------------------------------------------------------
// ProcTable::GetNewProcId()
//     create a new process table entry and assign it an unused
//     identifier.
//
//     "parentId" is the identifier of the parent, or a negative number
//       if there is no parent
//
//     returns the new identifier or -1 if the process table is full
//
//-------------------------------------------------------------
//int ProcTable::GetNewProcId(int parentId){	/* Liang DePeng - */
int MyProcTable::GetNewProcId(int parentId){	/* Liang DePeng + */
  
  ProcTableEntry* entry = new ProcTableEntry();
  ProcTableEntry* parentEntry;
  int id;

  lock->Acquire();
  // first, make sure there is room in the table for a new entry
  id = table->Insert((void*)entry);
  if (id < 0) {
    // if not, return an invalid id
    delete entry;
    id = -1;
  } else {
    // there was room in the table
    // first, find the parent's entry in the process table
    if (parentId >= 0) {
      parentEntry = (ProcTableEntry*)(table->Fetch(parentId));
      ASSERT(parentEntry != (ProcTableEntry*)0);
    }
    // initialize the entry for the new child process
    entry->hasExited = FALSE;
    entry->parentId = parentId;
    entry->firstChildId = -1;   // no grandchildren yet...
    // fix the parent's entry so that the new child is
    // on the parent's list of children
    if (parentId >= 0) {
      entry->siblingId = parentEntry->firstChildId;
      parentEntry->firstChildId = id;
    } else {
      entry->siblingId = -1;
    }
  }
  lock->Release();
  return(id);
}

//-------------------------------------------------------------
// ProcTable::ProcIsDone(int id, int status)
//     update the process table to reflect a process Exit
//
//     "id" is the identifier of the exited process
//         this is assumed to be valid
//
//     "status" is the exit status of the exited process
//
//-------------------------------------------------------------
//void ProcTable::ProcIsDone(int id, int status){	/* Liang DePeng - */
void MyProcTable::ProcIsDone(int id, int status){	/* Liang DePeng + */

  ProcTableEntry* entry;
  ProcTableEntry* childEntry;
  int childId;

  lock->Acquire();
  entry = (ProcTableEntry*)(table->Fetch(id));
  ASSERT(entry != (ProcTableEntry*)0);

  // do the following for each child of the process that
  // is done
  childId = entry->firstChildId;
  while(childId >= 0) {
    childEntry = (ProcTableEntry*)(table->Fetch(childId));
    ASSERT(childEntry != (ProcTableEntry*)0);
    if (childEntry->hasExited == TRUE) {
      // if the child is also done, then we can delete its entry
      table->Remove(childId);
      childId = childEntry->siblingId;
      delete childEntry;
    } else {
      // otherwise, the child is still running.  in this case, we
      // do not delete its entry, but we do record that its parent
      // is no more...
      childEntry->parentId = -1;
      childId = childEntry->siblingId;
    }
  }

  if (entry->parentId < 0) {
    // if the parent of the finished process is also finished,
    // then we can delete the finished process from the process table
    table->Remove(id);
    delete entry;
  } else {
    // otherwise, we do not delete the finished process, because its
    // parent might ask us about the status value.
    // record the status value and the fact that it is finshed
    entry->hasExited = TRUE;
    entry->exitStatus = status;
    // if anyone was waiting for this process to finish, wake them up
    entry->waitingRoom->Broadcast(lock);
  }
  lock->Release();
}

//-------------------------------------------------------------
// ProcTable::GetProcStatus(int childId, int parentId, int* status)
//     return the exit status of a child process.  if necessary, wait
//     for the specified process to Exit.  The specified process
//     must be a child of the calling process
//     
//     "childId" is the process whose status is needed
//          this ID is not assumed to be valid
//
//     "parentId" is the process requesting the status
//          this ID is assumed to be valid
//
//     "status" is an integer buffer into which its status will be placed
//
//     returns FALSE if the id is not the id of a child of the caller,
//     TRUE otherwise
//-------------------------------------------------------------
//bool ProcTable::GetProcStatus(int childId, int parentId, int* status){	/* Liang DePeng - */
bool MyProcTable::GetProcStatus(int childId, int parentId, int* status){	/* Liang DePeng + */
  ProcTableEntry* parentEntry;
  ProcTableEntry* childEntry;
  ProcTableEntry* previousChildEntry;
  int id;
  
  lock->Acquire();

  ASSERT(status != (int*)0);
  parentEntry = (ProcTableEntry*)(table->Fetch(parentId));
  ASSERT(parentEntry != (ProcTableEntry*)0);

  // we need to make sure that a process only asks for the
  // status of its children.  So, we check the list of the
  // children of the parent to make sure
  // for each child, do the following...
  previousChildEntry = (ProcTableEntry*)0;
  id = parentEntry->firstChildId;
  while(id >= 0) {
    childEntry = (ProcTableEntry*)(table->Fetch(id));
    ASSERT(childEntry != (ProcTableEntry*)0);
    ASSERT(childEntry->parentId == parentId);
    if (id == childId) {
      // found what we were looking for - childId really is a child
      // of parentId
      if (childEntry->hasExited == FALSE) {
	// if the child has not exited, we have to wait before
	// we can get its status value - this will cause us to block
	childEntry->waitingRoom->Wait(lock);
      }
      ASSERT(childEntry->hasExited == TRUE);
      // we can now delete this child process because it has exited
      // and its parent is about to retrieve its status value
      // first, take the child out of the list of children of the
      // parent
      if (id == parentEntry->firstChildId) {
	ASSERT(previousChildEntry == (ProcTableEntry*)0);
	parentEntry->firstChildId = childEntry->siblingId;
      } else {
	ASSERT(previousChildEntry != (ProcTableEntry*)0);
	previousChildEntry->siblingId = childEntry->siblingId;
      }
      // save the child's status value before we destroy it
      *status = childEntry->exitStatus;
      // destroy the child
      table->Remove(id);
      delete childEntry;
      lock->Release();
      return(TRUE);
    }
    id = childEntry->siblingId;
    previousChildEntry = childEntry;
  }
  // if we get here, we did not find childId on the list of
  // parentId's children
  lock->Release();
  return(FALSE);
}
