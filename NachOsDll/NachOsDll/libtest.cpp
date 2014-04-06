// libtest.cc 
//	Driver code to call self-test routines for standard library
//	classes -- bitmaps, lists, sorted lists, and hash tables.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "libtest.h"
#include "bitmap.h"
#include "hash.h"
#include "sysdep.h"



/**** add
 * Liang DePeng + */
class NInt: public Item{
public:
	NInt(int vall){this->val = vall;}
	int val;
};

static int NIntCompare(Item* x, Item* y){
	NInt* xx = (NInt* )x;
	NInt* yy = (NInt* )y;

	if (xx->val < yy->val) return -1;
    else if (xx->val == yy->val) return 0;
    else return 1;
}


static int fakeNIntComapre(Item* x, Item* y){
	return 0;
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// IntCompare
//	Compare two integers together.  Serves as the comparison
//	function for testing SortedLists
//----------------------------------------------------------------------
/**** delete
 * Liang DePeng - */
/*static int 
IntCompare(int x, int y) {
    if (x < y) return -1;
    else if (x == y) return 0;
    else return 1;
}*/
/* Liang DePeng -
 ****/


class MyKey: public Key{
public:
	MyKey(int keyy){this->key = keyy;}
	int key;
};

class MyStr: public Item{
public:
	MyStr(char* strr){this->str = strr;}
	char* str;
};



//----------------------------------------------------------------------
// HashInt, HashKey
//	Compute a hash function on an integer.  Serves as the
//	hashing function for testing HashTables.
//----------------------------------------------------------------------
/* Liang DePeng - */
/*static unsigned int  HashInt(int key) {	
    return (unsigned int) key;			
}*/										

/* Liang DePeng + */
static unsigned int  HashInt(Key* keyy) {	
	return (unsigned int) ((MyKey* )keyy)->key;			
}

//----------------------------------------------------------------------
// HashKey
//	Convert a string into an integer.  Serves as the function
//	to retrieve the key from the item in the hash table, for
//	testing HashTables.  Should be able to use "atoi" directly,
//	but some compilers complain about that.
//----------------------------------------------------------------------
/* Liang DePeng - */
/*static int HashKey(char *str) {
    return atoi(str);
}*/

/* Liang DePeng + */
static Key* HashKey(Item* strr) {
	return new MyKey(atoi(((MyStr* )strr)->str));
}

static bool KeyEqual(Key* k1, Key* k2){
	return ((MyKey* )k1)->key == ((MyKey* )k2)->key;
}

// Array of values to be inserted into a List or SortedList. 
//static int listTestVector[] = { 9, 5, 7 };

// Array of values to be inserted into the HashTable
// There are enough here to force a ReHash().
//static char *hashTestVector[] = { "0", "1", "2", "3", "4", "5", "6",
//	 "7", "8", "9", "10", "11", "12", "13", "14"};


//----------------------------------------------------------------------
// LibSelfTest
//	Run self tests on bitmaps, lists, sorted lists, and 
//	hash tables.
//----------------------------------------------------------------------

void LibSelfTest () {
    Bitmap *map = new Bitmap(200);
    //List<int> *list = new List<int>;		/* Liang DePeng - */
    //SortedList<int> *sortList = new SortedList<int>(IntCompare);	/* Liang DePeng - */
	List* list = getSortedList(fakeNIntComapre);	/* Liang DePeng + */
	List* sortList = getSortedList(NIntCompare);	/* Liang DePeng + */

    //HashTable<int, char *> *hashTable =							/* Liang DePeng - */
	//new HashTable<int, char *>(HashKey, HashInt);					/* Liang DePeng - */
	MyHashTable* hashTable = new MyHashTable(HashKey, HashInt, KeyEqual);		/* Liang DePeng - */
	
	/**** add
	 * Liang DePeng + */
	Item** hashTestVector = new Item*[15];
	hashTestVector[0] = new MyStr("0");
	hashTestVector[1] = new MyStr("1");
	hashTestVector[2] = new MyStr("2");
	hashTestVector[3] = new MyStr("3");
	hashTestVector[4] = new MyStr("4");
	hashTestVector[5] = new MyStr("5");
	hashTestVector[6] = new MyStr("6");
	hashTestVector[7] = new MyStr("7");
	hashTestVector[8] = new MyStr("8");
	hashTestVector[9] = new MyStr("9");
	hashTestVector[10] = new MyStr("10");
	hashTestVector[11] = new MyStr("11");
	hashTestVector[12] = new MyStr("12");
	hashTestVector[13] = new MyStr("13");
	hashTestVector[14] = new MyStr("14");
	/* Liang DePeng +
	 ****/


	Item** listTestVector = new Item*[3];	/* Liang DePeng + */
	listTestVector[0] = new NInt(9);		/* Liang DePeng + */
	listTestVector[1] = new NInt(5);		/* Liang DePeng + */
	listTestVector[2] = new NInt(7);		/* Liang DePeng + */

    map->SelfTest();
    
	//list->SelfTest(listTestVector, sizeof(listTestVector)/sizeof(int));			/* Liang DePeng - */
    //sortList->SelfTest(listTestVector, sizeof(listTestVector)/sizeof(int));		/* Liang DePeng - */
    //hashTable->SelfTest(hashTestVector, sizeof(hashTestVector)/sizeof(char *));	/* Liang DePeng - */

	list->SelfTest(listTestVector, 3);			/* Liang DePeng + */
    sortList->SelfTest(listTestVector, 3);		/* Liang DePeng + */
    
	hashTable->SelfTest(hashTestVector, 15);		/* Liang DePeng + */


    delete map;
	deleteSortedList(list);		/* Liang DePeng + */
	deleteSortedList(sortList);		/* Liang DePeng + */
    //delete list;		/* Liang DePeng - */
    //delete sortList;	/* Liang DePeng - */
    delete hashTable;
}
