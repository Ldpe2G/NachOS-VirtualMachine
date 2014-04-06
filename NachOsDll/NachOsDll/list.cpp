// list.cc 
//     	Routines to manage a singly linked list of "things".
//	Lists are implemented as templates so that we can store
//	anything on the list in a type-safe manner.
//
// 	A "ListElement" is allocated for each item to be put on the
//	list; it is de-allocated when the item is removed. This means
//      we don't need to keep a "next" pointer in every object we
//      want to put on a list.
// 
//     	NOTE: Mutual exclusion must be provided by the caller.
//  	If you want a synchronized list, you must use the routines 
//	in synchlist.cc.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "list.h"
#include <iostream>
using namespace std;


/**** add
 * Liang DePeng + */
class __declspec(dllexport) SortedList : public List {
  public:
    SortedList(int (*comp)(Item* x, Item* y)) : List() { 
		compare = comp;
		numInList = 0;
		first = NULL;
		last = NULL;
	};
    ~SortedList() {};		// base class destructor called automatically

    virtual void Prepend(Item* item){ Insert(item); };// Put item at the beginning of the list
    virtual void Append(Item* item){ Insert(item); }; // Put item at the end of the list

    virtual Item* Front(){ return first->item; };
    				// Return first item on list
				// without removing it
    virtual Item* RemoveFront(); 		// Take item off the front of the list
    virtual void Remove(Item* item); 	// Remove specific item from list

    virtual bool IsInList(Item* item) const;// is the item in the list?

    virtual unsigned int NumInList(){ return numInList;};
    				// how many items in the list?
    virtual bool IsEmpty(){ return (numInList == 0); };
    				// is the list empty? 

    virtual void Apply(void (*f)(Item*)) const; 
    				// apply function to all elements in list

	virtual void Insert(Item* item); 	// insert an item onto the list 

    virtual void SanityCheck() const;	
				// has this list been corrupted?
    virtual void SelfTest(Item** p, int numEntries);
				// verify module is working

  private:
    int (*compare)(Item* x, Item* y);	// function for sorting list elements
	
    //virtual void Prepend(Item* item) { Insert(item); }  // *pre*pending has no meaning 
				             //	in a sorted list
    //virtual void Append(Item* item) { Insert(item); }   // neither does *ap*pend 

};
/* Liang DePeng +
 ****/


//----------------------------------------------------------------------
// ListElement<T>::ListElement
// 	Initialize a list element, so it can be added somewhere on a list.
//
//	"itm" is the thing to be put on the list.
//----------------------------------------------------------------------

//template <class T>					/* Liang Depeng - */
//ListElement<T>::ListElement(T itm)	/* Liang Depeng - */
ListElement::ListElement(Item* itm){	/* Liang Depeng + */
     item = itm;
     next = NULL;	// always initialize to something!
}


//----------------------------------------------------------------------
// List<T>::List
//	Initialize a list, empty to start with.
//	Elements can now be added to the list.
//----------------------------------------------------------------------
/**** delete
 * Liang DePeng - */
/*
template <class T>	
List<T>::List()		
    first = last = NULL; 
    numInList = 0;
}
*/

//----------------------------------------------------------------------
// List<T>::~List
//	Prepare a list for deallocation.  
//      This does *NOT* free list elements, nor does it
//      free the data those elements point to.
//      Normally, the list should be empty when this is called.
//----------------------------------------------------------------------

//template <class T>	/* Liang Depeng - */
//List<T>::~List(){		/* Liang Depeng - */
//List::~List(){			
//}

//----------------------------------------------------------------------
// List<T>::Append
//      Append an "item" to the end of the list.
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, put it at the end.
//
//	"item" is the thing to put on the list.
//----------------------------------------------------------------------

//template <class T>				/* Liang Depeng - */
//void List<T>::Append(T item){		/* Liang Depeng - */
/*
void List::Append(Item* item){		
    
	//ListElement<T> *element = new ListElement<T>(item);	
	ListElement *element = new ListElement(item);			

    ASSERT(!IsInList(item));
    if (IsEmpty()) {		// list is empty
		first = element;
		last = element;
    } else {			// else put it after last
		last->next = element;
		last = element;
    }
    numInList++;
    ASSERT(IsInList(item));
}
*/

//----------------------------------------------------------------------
// List<T>::Prepend
//	Same as Append, only put "item" on the front.
//----------------------------------------------------------------------

//template <class T>						/* Liang Depeng - */
//void List<T>::Prepend(T item){			/* Liang Depeng - */
/*
void List::Prepend(Item* item){				

	//ListElement<T> *element = new ListElement<T>(item);	
	ListElement *element = new ListElement(item);	

    ASSERT(!IsInList(item));
    if (IsEmpty()) {		// list is empty
		first = element;
		last = element;
    } else {			// else put it before first
		element->next = first;
		first = element;
    }
    numInList++;
    ASSERT(IsInList(item));
}
*/
/* Liang DePeng -
 ****/

//----------------------------------------------------------------------
// List<T>::RemoveFront
//      Remove the first "item" from the front of the list.
//	List must not be empty.
// 
// Returns:
//	The removed item.
//----------------------------------------------------------------------

//template <class T>			/* Liang Depeng - */
//T List<T>::RemoveFront(){		/* Liang Depeng - */
Item* SortedList::RemoveFront(){		/* Liang Depeng + */    

	//ListElement<T> *element = first;	/* Liang Depeng - */
    //T thing;							/* Liang Depeng - */
	ListElement* element = first;		/* Liang Depeng + */
	Item* thing;						/* Liang Depeng + */

    ASSERT(!IsEmpty());

    thing = first->item;
    if (first == last) {	// list had one item, now has none 
        first = NULL;
		last = NULL;
    } else {
        first = element->next;
    }
    numInList--;
    delete element;
    return thing;
}

//----------------------------------------------------------------------
// List<T>::Remove
//      Remove a specific item from the list.  Must be in the list!
//----------------------------------------------------------------------

//template <class T>				/* Liang Depeng - */
//void List<T>::Remove(T item){		/* Liang Depeng - */
void SortedList::Remove(Item* item){

	//ListElement<T> *prev, *ptr;	/* Liang Depeng - */
    //T removed;					/* Liang Depeng - */
	ListElement *prev, *ptr;		/* Liang Depeng + */
	Item* removed;					/* Liang Depeng + */

    ASSERT(IsInList(item));

    // if first item on list is match, then remove from front
    if (item == first->item) {	
        removed = RemoveFront();
        ASSERT(item == removed);
		
    } else {
		prev = first;
        for (ptr = first->next; ptr != NULL; prev = ptr, ptr = ptr->next) {
            if (item == ptr->item) {
			
				prev->next = ptr->next;
				if (prev->next == NULL) {
					last = prev;
				}
				delete ptr;
				numInList--;
				break;
			}
        }
		ASSERT(ptr != NULL);	// should always find item!
    }
	ASSERT(!IsInList(item));
}

//----------------------------------------------------------------------
// List<T>::IsInList
//      Return TRUE if the item is in the list.
//----------------------------------------------------------------------

//template <class T>						/* Liang Depeng - */
//bool List<T>::IsInList(T item) const{		/* Liang Depeng - */
bool SortedList::IsInList(Item* item) const{

    //ListElement<T> *ptr;	/* Liang Depeng - */
	ListElement *ptr;		/* Liang Depeng - */

    for (ptr = first; ptr != NULL; ptr = ptr->next) {
        if (item == ptr->item) {
            return true;
        }
    }
    return false;
}


//----------------------------------------------------------------------
// SortedList::Apply
//      Apply function to every item on a list.
//
//	"func" -- the function to apply
//----------------------------------------------------------------------

//template <class T>								/* Liang Depeng - */
//void List<T>::Apply(void (*func)(T)) const {		/* Liang Depeng - */
void SortedList::Apply(void (*func)(Item*)) const{

	//ListElement<T> *ptr;							/* Liang Depeng - */
	ListElement *ptr;								/* Liang Depeng + */

    for (ptr = first; ptr != NULL; ptr = ptr->next) {
        (*func)(ptr->item);
    }
}


//----------------------------------------------------------------------
// SortedList::Insert
//      Insert an "item" into a list, so that the list elements are
//	sorted in increasing order.
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, walk through the list, one element at a time,
//	to find where the new item should be placed.
//
//	"item" is the thing to put on the list. 
//----------------------------------------------------------------------

//template <class T>						/* Liang Depeng - */
//void SortedList<T>::Insert(T item){		/* Liang Depeng - */
void SortedList::Insert(Item* item){

    //ListElement<T> *element = new ListElement<T>(item);	/* Liang Depeng - */
    //ListElement<T> *ptr;		// keep track				/* Liang Depeng - */
	ListElement *element = new ListElement(item);			/* Liang Depeng + */
	ListElement *ptr;										/* Liang Depeng + */

    if(!IsInList(item)){
		if (IsEmpty()) {			// if list is empty, put at front
			first = element;
			last = element;
		} else if ((* compare)(item, first->item) < 0) {  // item goes at front 
			element->next = first;
			first = element;
		} else {		// look for first elt in list bigger than item
			for (ptr = first; ptr->next != NULL; ptr = ptr->next) {
				if ((* compare)(item, ptr->next->item) < 0) {
					element->next = ptr->next;
					ptr->next = element;
					numInList++;
					return;
				}
			}
			last->next = element;		// item goes at end of list
			last = element;
		}
		numInList++;

		ASSERT(IsInList(item));
	}
}

//----------------------------------------------------------------------
// SortedList::SanityCheck
//      Test whether this is still a legal list.
//
//	Tests: do I get to last starting from first?
//	       does the list have the right # of elements?
//----------------------------------------------------------------------

//template <class T>					/* Liang Depeng - */
//void  List<T>::SanityCheck() const{	/* Liang Depeng - */
/*
void List::SanityCheck() const{	

    //ListElement<T> *ptr;				
	ListElement *ptr;					
    int numFound;

    if (first == NULL) {
		ASSERT((numInList == 0) && (last == NULL));
    } else if (first == last) {
		ASSERT((numInList == 1) && (last->next == NULL));
    } else {
        for (numFound = 1, ptr = first; ptr != last; ptr = ptr->next) {
			numFound++;
            ASSERT(numFound <= numInList);	// prevent infinite loop
        }
        ASSERT(numFound == numInList);
        ASSERT(last->next == NULL);
    }
}
*/

//----------------------------------------------------------------------
// SortedList::SelfTest
//      Test whether this module is working.
//----------------------------------------------------------------------

//template <class T>								/* Liang Depeng - */
//void  List<T>::SelfTest(T *p, int numEntries){	/* Liang Depeng - */
/*
void List::SelfTest(Item* p, int numEntries){	
    
	int i;
    //ListIterator<T> *iterator = new ListIterator<T>(this);	
	ListIterator *iterator = new ListIterator(this);	

    SanityCheck();									
    // check various ways that list is empty
    ASSERT(IsEmpty() && (first == NULL));
    for (; !iterator->IsDone(); iterator->Next()) {
		ASSERTNOTREACHED();	// nothing on list
    }

    for (i = 0; i < numEntries; i++) {
		Append(&p[i]);
		ASSERT(IsInList(&p[i]));
		ASSERT(!IsEmpty());
     }
     SanityCheck();

     // should be able to get out everything we put in
     for (i = 0; i < numEntries; i++) {
		Remove(&p[i]);
        ASSERT(!IsInList(&p[i]));
     }
     ASSERT(IsEmpty());
     SanityCheck();
     delete iterator;
}
*/

//----------------------------------------------------------------------
// SortedList::SanityCheck
//      Test whether this is still a legal sorted list.
//
//	Test: is the list sorted?
//----------------------------------------------------------------------

//template <class T>							/* Liang Depeng - */
//void SortedList<T>::SanityCheck() const{		/* Liang Depeng - */
void SortedList::SanityCheck() const{			/* Liang Depeng + */

    //ListElement<T> *prev, *ptr;				/* Liang Depeng - */
	ListElement *prev, *ptr;					/* Liang Depeng + */

    //List<T>::SanityCheck();					/* Liang Depeng - */					
    int numFound;

    if (first == NULL) {
		ASSERT((numInList == 0) && (last == NULL));
    } else if (first == last) {
		ASSERT((numInList == 1) && (last->next == NULL));
    } else {
        for (numFound = 1, ptr = first; ptr != last; ptr = ptr->next) {
			numFound++;
            ASSERT(numFound <= numInList);	// prevent infinite loop
        }
        ASSERT(numFound == numInList);
        ASSERT(last->next == NULL);
    }

    if (first != last) {
        for (prev = first, ptr = first->next; ptr != NULL; 
						prev = ptr, ptr = ptr->next) {
            ASSERT((* compare)(prev->item, ptr->item) <= 0);
        }
    }
}

//----------------------------------------------------------------------
// SortedList::SelfTest
//      Test whether this module is working.
//----------------------------------------------------------------------

//template <class T>									/* Liang Depeng - */
//void SortedList<T>::SelfTest(T *p, int numEntries){	/* Liang Depeng - */
void SortedList::SelfTest(Item** p, int numEntries){		/* Liang Depeng + */

	int i;
    //T *q = new T[numEntries]	;				/* Liang Depeng - */
	Item** q = new Item*[numEntries];			/* Liang Depeng + */

    //List<T>::SelfTest(p, numEntries);			/* Liang Depeng - */
	//List::SelfTest(p, numEntries);				/* Liang Depeng + */

    //ListIterator<T> *iterator = new ListIterator<T>(this);	
	ListIterator *iterator = new ListIterator(this);	


    SanityCheck();									
    // check various ways that list is empty
    ASSERT(IsEmpty() && (first == NULL));
    for (; !iterator->IsDone(); iterator->Next()) {
		ASSERTNOTREACHED();	// nothing on list
    }


    for (i = 0; i < numEntries; i++) {
		Append(p[i]);
		ASSERT(IsInList(p[i]));
		ASSERT(!IsEmpty());
     }

     SanityCheck();

     // should be able to get out everything we put in
     for (i = 0; i < numEntries; i++) {
		Remove(p[i]);
        ASSERT(!IsInList(p[i]));
     }
     ASSERT(IsEmpty());

     SanityCheck();
     delete iterator;

    for (i = 0; i < numEntries; i++) {
		 Insert(p[i]);
		 ASSERT(IsInList(p[i]));
     }
     SanityCheck();
     // should be able to get out everything we put in
     for (i = 0; i < numEntries; i++) {
		q[i] = RemoveFront();
         ASSERT(!IsInList(q[i]));
     }
     ASSERT(IsEmpty());

     // make sure everything came out in the right order
     for (i = 0; i < (numEntries - 1); i++) {
		ASSERT((* compare)(q[i], q[i + 1]) <= 0);
     }
     SanityCheck();

     delete q;
}

/**** add
 * Liang DePeng + */
extern "C" {
	__declspec(dllexport) List* getSortedList(int (*comp)(Item* x, Item* y)){
		return new SortedList(comp);
	}

	__declspec(dllexport) void deleteSortedList(List* l){
		delete l;
	}
}
/* Liang DePeng +
 ****/