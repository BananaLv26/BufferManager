#ifndef __PU_PCB__
#define __PU_PCB__

// using namespace std;
#include "PageHandle_Proxy.h"

#define FREE 		0
#define UNPINNED 	1
#define PINNED 		2

class PCB{
public:
	PCB* next;
	PCB* prev;

	PCB(void* my_addr);
	void setLRU(int num);
	long getLRU();
	bool isDirty();
	void setDirty();
	PageHandle_Proxy* getProxy();
	void setProxy(PageHandle_Proxy* newProxy);
	void* getAddr();
	int getType();
	void setType(int my_type);
	~PCB();

private:
	void* addr;
	long LRUCount;
	bool dirty;
	PageHandle_Proxy* phandle_proxy;
	int type; // FREE, UNPINNED, PINNED
};

#endif