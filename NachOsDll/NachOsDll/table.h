// table.h
//     A simple fixed-size table data structure
//     Each item in the table is a (void*), i.e., a pointer
//     to anything, plus a unique nonnegative id.
//     Items are assigned unused ids when they are inserted.
//     Items are retrieved and deleted by id number.
//
//     A table should be empty when it is deleted
//

#ifndef TABLE_H
#define TABLE_H

class Table {
 public:
  Table(int maxitems);                   // maxitems is
                                         // the maximum number of items
                                         // that can be in the table
					 // item ids will be in the range
                                         // zero to maxitems-1
                                         
  ~Table();

  int Insert(void* item);                // add a new item and return
                                         // an id for it, or -1 if the
					 // table is full

  void* Fetch(int id);                   // return the item
                                          // with the specified id
					  // the item remains on the list

  void Remove(int id);                    // remove the specified item
                                          // space from the table


 private:
  void** items;
  int size;
};
#endif  //TABLE_H
