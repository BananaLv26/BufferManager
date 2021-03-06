
#ifndef CATALOG_UNIT_H
#define CATALOG_UNIT_H

#include "MyDB_BufferManager.h"
#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "QUnit.h"
#include <iostream>
#include <unistd.h>
#include <vector>

#include "pu_debug.h"

using namespace std;

// these functions write a bunch of characters to a string... used to produce data
void writeNums (char *bytes, size_t len, int i) {
	for (size_t j = 0; j < len - 1; j++) {
		bytes[j] = '0' + (i % 10);
	}
	bytes[len - 1] = 0;
}

void writeLetters (char *bytes, size_t len, int i) {
	for (size_t j = 0; j < len - 1; j++) {
		bytes[j] = 'i' + (i % 10);
	}
	bytes[len - 1] = 0;
}

void writeSymbols (char *bytes, size_t len, int i) {
	for (size_t j = 0; j < len - 1; j++) {
		bytes[j] = '!' + (i % 10);
		// bytes[j] = '0' + (j % 8);
	}
	bytes[len - 1] = 0;
}

int main () {

	QUnit::UnitTest qunit(cerr, QUnit::verbose);
	
	// UNIT TEST 1: A BIG ONE!!
	{

		// cout << "main[0]\n";
		// create a buffer manager 
		MyDB_BufferManager myMgr (64, 16, "tempDSFSD");
		MyDB_TablePtr table1 = make_shared <MyDB_Table> ("tempTable", "foobar");
		// MyDB_TablePtr table1 = make_shared <MyDB_Table> ("tempTable", "/storage-home/p/pd22/comp530/A1/foobar");

		myMgr.showBufferPool();
		// cout << "main[1]\n";
		// allocate a pinned page
		cout << "allocating pinned page\n";
		MyDB_PageHandle pinnedPage = myMgr.getPinnedPage (table1, 0);
		char *bytes = (char *) pinnedPage->getBytes ();
		writeNums (bytes, 64, 0);
		pinnedPage->wroteBytes ();

		myMgr.showBufferPool();
		// cout << "main[2]\n";
		
		// create a bunch of pinned pages and remember them
		vector <MyDB_PageHandle> myHandles;
		for (int i = 1; i < 10; i++) {
			cout << "allocating pinned page, " << i << endl;
			MyDB_PageHandle temp = myMgr.getPinnedPage (table1, i);
			char *bytes = (char *) temp->getBytes ();
			writeNums (bytes, 64, i);
			temp->wroteBytes ();
			myHandles.push_back (temp);
		}

		myMgr.showBufferPool();
		// now forget the pages we created
		vector <MyDB_PageHandle> temp;
		myHandles = temp;
		myMgr.showBufferPool();

		// now remember 8 more pages
		for (int i = 0; i < 8; i++) {
			cout << "allocating pinned page, " << i << endl;
			MyDB_PageHandle temp = myMgr.getPinnedPage (table1, i);
			char *bytes = (char *) temp->getBytes ();

			// write numbers at the 0th position
			if (i == 0)
				writeNums (bytes, 64, i);
			else
				writeSymbols (bytes, 64, i);
			temp->wroteBytes ();
			myHandles.push_back (temp);
		}

		myMgr.showBufferPool();
		// cout << "main[3]\n";
		// myMgr.unpin(pinnedPage);
		// cout << "main[3.1]\n";
		myMgr.showBufferPool();


		// now correctly write nums at the 0th position
		cout << "allocating unpinned page\n";
		MyDB_PageHandle anotherDude = myMgr.getPage (table1, 0);
		bytes = (char *) anotherDude->getBytes ();
		writeSymbols (bytes, 64, 0);
		anotherDude->wroteBytes ();

		myMgr.showBufferPool();
		// cout << "main[4]\n";
		
		// now do 90 more pages, for which we forget the handle immediately
		for (int i = 10; i < 100; i++) {
			cout << "allocating unpinned page\n";
			MyDB_PageHandle temp = myMgr.getPage (table1, i);
			char *bytes = (char *) temp->getBytes ();
			// myMgr.showBufferPool();
			writeNums (bytes, 64, i);
			temp->wroteBytes ();
		}

		myMgr.showBufferPool();
		// cout << "main[5]\n";

		// now forget all of the pinned pages we were remembering
		vector <MyDB_PageHandle> temp2;
		myHandles = temp2;

		myMgr.showBufferPool();
		// cout << "main[5.1]\n";

		// now get a pair of pages and write them
		for (int i = 0; i < 100; i++) {
			cout << "allocating pinned page\n";
			MyDB_PageHandle oneHandle = myMgr.getPinnedPage ();
			char *bytes = (char *) oneHandle->getBytes ();
			writeNums (bytes, 64, i);
			oneHandle->wroteBytes ();
			cout << "allocating pinned page\n";
			MyDB_PageHandle twoHandle = myMgr.getPinnedPage ();
			writeNums (bytes, 64, i);
			twoHandle->wroteBytes ();
		}

		myMgr.showBufferPool();
		// cout << "main[6]\n";

		// make a second table
		MyDB_TablePtr table2 = make_shared <MyDB_Table> ("tempTable2", "barfoo");
		for (int i = 0; i < 100; i++) {
			cout << "allocating unpinned page\n";
			MyDB_PageHandle temp = myMgr.getPage (table2, i);
			char *bytes = (char *) temp->getBytes ();
			writeLetters (bytes, 64, i);
			temp->wroteBytes ();
		}

		myMgr.showBufferPool();
		// cout << "main[6.1]\n";
		
	}




	// UNIT TEST 2
	{
		MyDB_BufferManager myMgr (64, 16, "tempDSFSD");
		// MyDB_TablePtr table1 = make_shared <MyDB_Table> ("tempTable", "/storage-home/p/pd22/comp530/A1/foobar");
		MyDB_TablePtr table1 = make_shared <MyDB_Table> ("tempTable", "foobar");
		DBG("called\n");
		// look up all of the pages, and make sure they have the correct numbers
		for (int i = 0; i < 100; i++) {
			DBG("~~~~~~~~i=%d~~~~~~~~\n", i);
			MyDB_PageHandle temp = myMgr.getPage (table1, i);
			char answer[64];
			if (i < 8)
				writeSymbols (answer, 64, i);
			else
				writeNums (answer, 64, i);
			char *bytes = (char *) temp->getBytes ();
			DBG("called\n");
			QUNIT_IS_EQUAL (string (answer), string (bytes));
		}
	}
}

#endif
