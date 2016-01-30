#include "PCB.h"
#include <stdlib.h>
#include "PageHandle_Proxy.h"

PCB :: PCB(void* my_addr){
	next = nullptr;
	prev = nullptr;

	// ref_count = 0;
	addr = my_addr;
	LRUCount = 0;
	bool dirty = false;
	phandle_proxy = nullptr;
	type = FREE;
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

int PCB :: getType(){
	return type;
}

void PCB :: setType(int my_type){
	type = my_type;
}

PCB :: ~PCB(){
	free(addr);
}
