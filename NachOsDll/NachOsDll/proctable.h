// proctable.h
//   Data structure to keep track of all of the processes running
//   in the system
//
//   Every process except the first has a parent (its creator).  Every
//   process has zero or more children.  The proctable structure maintains
//   these parent/child relationships
//
//   Every process may return an exit status, which is recorded in this
//   data structure.
//
//   Every process has a unique identifier assigned to it, which is
//   its index into the proctable.  Identifiers are eventually
//   re-used.
//
//   Information about a process "A" is maintained in this table until
//   "A" has exited AND either "A"'s parent has exited, or "A"'s parent
//   has retrieved "A"'s status value.
//   This guarantees that a process'
//   exit status will be available to its parent even if the process
//   has already exited.  The system makes no other guarantees about
//   the availability of exit status values.  In particular, a process
//   may not Join() a process other than one of its
//   children, and a process should not attempt to Join() the same
//   process more than one time.
//


#ifndef PROCTABLE_H
#define PROCTABLE_H

class FLock;
class Condition;
class Table;

#define  MaxNumProcesses       128 

/**** delete
 * Liang DePeng - */
/*class ProcTable {
public:
    ProcTable();                       // constructor.
    ~ProcTable();                      // destructor

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
private:
    Table* table;                       // the process table
    Lock* lock;                         // the lock that provides mutual
                                        // exclusion for this table
};*/
/* Liang DePeng -
 ****/


/**** add
 * Liang DePeng + */
class __declspec(dllexport) ProcTable {
public:
    ProcTable();                       // constructor.
	virtual ~ProcTable() = 0{};                      // destructor

    virtual int GetNewProcId(int parentId) = 0;     // allocate an entry in the table
                                        // for a new process.  "parentId"
                                        // is the identifier of the parent,
                                        // or a negative number if there
                                        // is no parent.  return an
                                        // identifier for the new process

    virtual void ProcIsDone(int id, int status) = 0; // the specified process has exited

    virtual bool GetProcStatus(int childId,int parentId,int* status) = 0;
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
protected:
    Table* table;                       // the process table
    FLock* lock;                         // the lock that provides mutual
                                        // exclusion for this table
};

typedef ProcTable* (*GetProcTable)();  
typedef void (*DeleteProcTable)(ProcTable*);

extern "C" {
	__declspec(dllexport) ProcTable* GetProcTableF();
	__declspec(dllexport) void DeleteProcTableF(ProcTable*);
}

extern GetProcTable getProcTable;
extern DeleteProcTable deleteProcTable;

/* Liang DePeng +
 ****/

// Each entry in the process table looks like this:

class ProcTableEntry{
  public:
    ProcTableEntry();         // constructor
    ~ProcTableEntry();        // destructor

    bool hasExited;           // has this process exited?
    int  exitStatus;          // if so, what was its exit status?
    int  parentId;            // ID of parent process, or -1 if parent has
                              //   exited
    int  firstChildId;        // ID of one child, or -1 if no children
    int  siblingId;           // ID of a sibling - this and the previous
                              // field are used to maintain a list of the
                              // children of each process
    Condition* waitingRoom;   // used to make processes wait for this
                              // process to finish
};
    
#endif //PROCTABLE_H

