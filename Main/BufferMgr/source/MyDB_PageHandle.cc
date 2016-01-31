
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"
#include "MyDB_BufferManager.h"
#include "PCB.h"
#include "pu_debug.h"

MyDB_PageHandleBase :: MyDB_PageHandleBase(MyDB_BufferManager* my_pBM, PageHandle_Proxy* my_phandle_proxy){
	pBM = my_pBM;
	pHandleProxy = my_phandle_proxy;
}

void *MyDB_PageHandleBase :: getBytes () {
	return pBM->getBytes(pHandleProxy);
}

void MyDB_PageHandleBase :: wroteBytes () {
	// change the dirty bit
	pBM->wroteBytes(pHandleProxy);
}

PageHandle_Proxy* MyDB_PageHandleBase :: getProxy(){
	return pHandleProxy;
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
	// DBG2("page hanlde is called\n");
	pHandleProxy->refDec();
}

#endif

