
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "PCB.h"
#include "PageHandle_Proxy.h"

using namespace std;

class MyDB_BufferManager {

public:

	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS 
	// get a free page (pcb)
	PCB* getPCB(bool pinned);

	// server function for client's getBytes
	void* getBytes(PageHandle_Proxy* my_pHandleProxy);

	// server function for client's wroteBytes;
	void wroteBytes(PageHandle_Proxy* my_pHandleProxy);

	// free the page corresponding to the proxy
	void freePage(PageHandle_Proxy* my_pHandleProxy);
	
	// move the page to pin pool
	void moveToList(PCB* pcb, int destination);

	// remove a proxy if handle is gone
	void freeProxy(PageHandle_Proxy* my_pHandleProxy);
	
	// for debugging purpose only 
	void showBufferPool();

private:
	size_t pageSize;
	// PCB is one way linked-list
	PCB* listFree; // free pool
	PCB* listUnpin; // unpin pool
	PCB* listPin; // pinned pool
	
	// YOUR STUFF HERE
	PageHandle_Proxy* listPhandleProxy; // phandle proxy is two way linked-list
	long tmpPageCount; // for anonymous usage
	long LRUCount; // for LRU usage

	// helper function for getting a free page
	MyDB_PageHandle helper_getPage(MyDB_TablePtr table_ptr, long index, bool my_pinned, bool my_anonymous);

	// LRU algorithm to return a free page from the unpin pool
	PCB* LRU(bool my_pinned);

	// write func
	void writeToDisk(PCB* pcb);

	// read func
	void readFromDisk(PCB* pcb);
};

#endif


