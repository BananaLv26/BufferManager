#include "MyDB_BufferManager.h"
#include "PageHandle_Proxy.h"
#include "PCB.h"
#include "pu_debug.h"


PageHandle_Proxy :: PageHandle_Proxy(string my_fileName, long my_index, bool my_pinned, bool my_anonymous, MyDB_BufferManager* my_pBM){
	next = nullptr;
	prev = nullptr;

	pBM = my_pBM;
	ref_count = 1;
	fileName = my_fileName;
	index = my_index;
	pcb = nullptr;
	pinned = my_pinned;
	anonymous = my_anonymous;
}

void PageHandle_Proxy :: refInc(){
	++ ref_count;
}

void PageHandle_Proxy :: refDec(){
	// DBG1("called\n");
	if(!-- ref_count){
		// TODO free the buffer
		pBM->freePage(this);
		ref_count = 0;
	}
}

int PageHandle_Proxy :: getRef(){
	return ref_count;
}


string PageHandle_Proxy :: getFileName(){
	return fileName;
}

long PageHandle_Proxy :: getIndex(){
	return index;
}

PCB* PageHandle_Proxy :: getPCB(){
	return pcb;
}

void PageHandle_Proxy :: setPCB(PCB* myPCB){
	pcb = myPCB;
	if(myPCB) pcb->setProxy(this);
}

bool PageHandle_Proxy :: isPinned(){
	return pinned;
}

void PageHandle_Proxy :: setPinned(int my_pinned){
	pinned = my_pinned == PINNED ? true : false; 
	pBM->moveToList(pcb, my_pinned);
}

bool PageHandle_Proxy :: isAnonymous(){
	return anonymous;
}


PageHandle_Proxy :: ~PageHandle_Proxy(){

}
