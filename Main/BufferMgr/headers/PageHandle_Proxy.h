#ifndef __PAGE_HANDLE_PROXY__H__
#define __PAGE_HANDLE_PROXY__H__

// #include "PCB.h"
using namespace std;
#include <string>

class MyDB_BufferManager;
class PCB;

class PageHandle_Proxy
{
public:
	PageHandle_Proxy* next;
	PageHandle_Proxy* prev;
	
	PageHandle_Proxy(string my_fileName, long my_index, bool my_pinned, bool my_anonymous, MyDB_BufferManager* my_pBM);
	void refInc();
	void refDec();
	string getFileName();
	long getIndex();
	PCB* getPCB();
	void setPCB(PCB* myPCB);
	bool isPinned();
	void setPinned(int my_pinned);
	bool isAnonymous();
	// void updateLRU();

	~PageHandle_Proxy();

private:
	MyDB_BufferManager* pBM;
	int ref_count;
	string fileName;
	long index;
	PCB* pcb;
	bool pinned;
	bool anonymous;
};

#endif