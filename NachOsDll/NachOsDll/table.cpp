// table.cc
//    Routines to manipulate tables

#include "table.h"
#include "debug.h"

//--------------------------------------------------------------
// Table::Table
//   initialize a new table
//
//  "numitems" is the max number items allowed in the table
//--------------------------------------------------------------

Table::Table(int numitems)
{
  int i;
  ASSERT(numitems > 0);
  items = new void*[numitems];
  for(i=0;i<numitems;i++) items[i] = (void*)0;
  size = numitems;
}

//--------------------------------------------------------------
// Table::~Table
//   delete a table 
//--------------------------------------------------------------

Table::~Table()
{
  delete items;
}

//--------------------------------------------------------------
// Table::Insert
//   add a new item to the table and assign an id to it.
//   The id is returned, or -1 if there is no space
//
//   "item" is a pointer to an address space 
//--------------------------------------------------------------
int
Table::Insert(void* item)
{
  int id;
  
  for(id=0;id<size;id++) {
    if (items[id] == (void*)0) {
      items[id] = item;
      return(id);
    }
  }
  // if we get here, the table was full
  return(-1);
}

//--------------------------------------------------------------
// Table::Fetch
//   Return the item with the id "id" or a null pointer
//   if there is no such item
//   specified id, or return a null pointer if there
//--------------------------------------------------------------

void *
Table::Fetch(int id)
{
  ASSERT(id >= 0 && id < size);
  return(items[id]);
}

//--------------------------------------------------------------
// Table::Remove
//   Remove the item with id "id" from the table
//--------------------------------------------------------------
void
Table::Remove(int id)
{
  ASSERT(id >= 0 && id < size);
  items[id] = (void*)0;
}


