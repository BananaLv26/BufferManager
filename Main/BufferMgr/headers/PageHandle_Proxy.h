#ifndef __PAGE_HANDLE_PROXY__H__
#define __PAGE_HANDLE_PROXY__H__

// #include "PCB.h"
using namespace std;
#include <string>

class PCB;

class PageHandle_Proxy
{
public:
	PageHandle_Proxy* next;
	PageHandle_Proxy* prev;
	
	PageHandle_Proxy(string myFileName, long myIndex);
	void refInc();
	void refDec();
	string getFileName();
	long getIndex();
	PCB* getPCB();
	void setPCB(PCB* myPCB);
	void updateLRU();
	~PageHandle_Proxy();

private:
	int ref_count;
	string fileName;
	long index;
	PCB* pcb;
};

#endif