#include "PCB.h"
#include <stdlib.h>


PCB :: PCB(void* my_addr){
	// ref_count = 0;
	addr = my_addr;
}

void PCB :: setLRU(int num){
	LRUCount = num;
}

long PCB :: getLRU(){
	return LRUCount;
}

bool PCB :: isDirty(){
	return dirty;
}
void PCB :: setDirty(){
	dirty = true;
}

PageHandle_Proxy* PCB :: getProxy(){
	return phandle_proxy;
}

void PCB :: setProxy(PageHandle_Proxy* newProxy){
	phandle_proxy = newProxy;
}


void* PCB :: getAddr(){
	return addr;
}

PCB :: ~PCB(){
	free(addr);
}
