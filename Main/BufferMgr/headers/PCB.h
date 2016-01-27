#ifndef __PU_PCB__
#define __PU_PCB__

// using namespace std;
#include "PageHandle_Proxy.h"

class PCB{
public:
	PCB* next;

	PCB(void* my_addr);
	void setLRU(int num);
	long getLRU();
	bool isDirty();
	void setDirty();
	PageHandle_Proxy* getProxy();
	void setProxy(PageHandle_Proxy* newProxy);
	void* getAddr();
	~PCB();

private:
	void* addr;
	// int ref_count;
	long LRUCount;
	bool dirty;
	PageHandle_Proxy* phandle_proxy;

};

#endif