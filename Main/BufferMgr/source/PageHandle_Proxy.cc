
#include "PageHandle_Proxy.h"
#include "PCB.h"

PageHandle_Proxy :: PageHandle_Proxy(string myFileName, long myIndex){
	next = nullptr;
	prev = nullptr;
	fileName = myFileName;
	index = myIndex;
	ref_count = 1;
}

void PageHandle_Proxy :: refInc(){
	++ ref_count;
}

void PageHandle_Proxy :: refDec(){
	if(!-- ref_count){
		// TODO free the buffer
		ref_count = 0;
	}
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
	pcb->setProxy(this);
}

// void PageHandle_Proxy :: updateLRU(){
// 	if(pcb) pcb->()
// }

PageHandle_Proxy :: ~PageHandle_Proxy(){

}
