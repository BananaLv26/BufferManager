
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include <string>
// #include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
// #include <stdio.h>
#include <fcntl.h>

#include "MyDB_BufferManager.h"
#include "MyDB_PageHandle.h"
#include "pu_debug.h"


using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr table_ptr, long index) {
	return helper_getPage(table_ptr, index);
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	MyDB_TablePtr table_ptr = make_shared <MyDB_Table> ("tempTable", "tmpFile");
	return helper_getPage(table_ptr, tmpPageCount ++);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr, long) {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	return nullptr;		
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
}

MyDB_BufferManager :: MyDB_BufferManager (size_t my_pageSize, size_t numPages, string tempFile){
	DBG("Constructor called\n");
	pageSize = my_pageSize;
	// allocate all at once all pages needed from the memory
	// put them all in the free poll
	listFree = nullptr;
	for(int i = 0; i < numPages; ++ i){
		PCB* tmp = new PCB(malloc(pageSize));
		DBG("pcb=%#X\n", tmp);
		tmp->next = listFree;
		listFree = tmp;
	}
	// unpin/pin pool is empty for now
	listUnpin = nullptr;
	listPin = nullptr;
	listPhandleProxy = nullptr;
	tmpPageCount = 0;
	LRUCount = 0;
	DBG("done with buffer allocation\n");
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
	DBG("Desctructor called\n");

	while(listFree){
		PCB* tmp = listFree->next;
		free(listFree);
		listFree = tmp;
	}

	while(listUnpin){
		PCB* tmp = listUnpin->next;
		free(listUnpin);
		listUnpin = tmp;
	}

	while(listPin){
		PCB* tmp = listPin->next;
		free(listPin);
		listPin = tmp;
	}

	while(listPhandleProxy){
		PageHandle_Proxy* tmp = listPhandleProxy->next;
		free(listPhandleProxy);
		listPhandleProxy = tmp;
	}
}


PCB* MyDB_BufferManager :: getPCB(){
	// when there is still free pages
	if(listFree){
		PCB* tmp = listFree;
		listFree = listFree->next;
		DBG("listUnpin=%#X\n", listUnpin);
		// move tmp to listUnpin // TODO differentiate between unpin and pin
		if(!listUnpin) {
			listUnpin = tmp;
			tmp->next = nullptr;
		}
		else{
			tmp->next = listUnpin;
			listUnpin = tmp;
		}
		return tmp;
	}

	// when no unpin page exists
	if(!listUnpin) return nullptr;

	PCB* tmp = LRU();
	return tmp;
}

void* MyDB_BufferManager :: getBytes(PageHandle_Proxy* my_pHandleProxy){
	DBG("called\n");
	PCB* tmp = my_pHandleProxy->getPCB();
	// cout << "my_pHandleProxy=" << my_pHandleProxy;
	DBG("my_pHandleProxy=%#X\n", my_pHandleProxy);
	if(tmp == nullptr){
		DBG("called\n");
		tmp = getPCB();
		if(tmp) my_pHandleProxy->setPCB(tmp); 
		else{
			ERR("No page currently available, please free pinned pages\n");	
			return nullptr;
		}
	}
	DBG("called\n");
	readFromDisk(tmp);
	DBG("called\n");
	tmp->setLRU(++ LRUCount); // update the LRU
	DBG("called\n");
	return tmp->getAddr();
}

void MyDB_BufferManager :: wroteBytes(PageHandle_Proxy* my_pHandleProxy){
	PCB* tmp = my_pHandleProxy->getPCB();
	if(tmp == nullptr){
		tmp = getPCB();
		if(tmp) my_pHandleProxy->setPCB(tmp); 
		else{
			ERR("No page currently available, please free pinned pages\n");	
		}
	}
	tmp->setLRU(++ LRUCount); // update the LRU
	tmp->setDirty();
}

//*************************** private function **************************//

MyDB_PageHandle MyDB_BufferManager :: helper_getPage(MyDB_TablePtr table_ptr, long index){
	DBG("called\n");
	if(!table_ptr){
		ERR("error: MyDB_TablePtr null pointer!!! return nullptr");
		return nullptr;
	} 
	if(index < 0){
		ERR("error: MyDB_TablePtr null pointer!!! return nullptr");	
		return nullptr;
	} 

	// iterate listPhandleProxy list
	PageHandle_Proxy* curr = listPhandleProxy;

	DBG("called\n");
	while(curr != nullptr){
		DBG("called\n");
		if(curr->getFileName() == table_ptr->getStorageLoc() && curr->getIndex() == index) break;
		curr = curr->next;
	}

	DBG("called\n");
	// printf("%#x\n", curr);
	if(curr != nullptr) curr->refInc();
	else{
		curr = new PageHandle_Proxy(table_ptr->getStorageLoc(), index);
		if(listPhandleProxy == nullptr) listPhandleProxy = curr;
		else{
			curr->next = listPhandleProxy;
			listPhandleProxy->prev = curr;
			listPhandleProxy = curr;
		}
	}

	
	MyDB_PageHandle pPH(new MyDB_PageHandleBase(this, curr));
	// std::cout << "curr=" << curr << ",curr->next=" << curr->next << "\n";
	DBG("called\n");
	return pPH;

}


PCB* MyDB_BufferManager :: LRU(){
	DBG("called\n");
	// pool is listUnpin
	if(!listUnpin) return nullptr;

	PCB* curr = listUnpin;
	PCB* evict = curr;
	while(curr){
		DBG("called dirty=%d, LRU=%d, curr=%#X\n", curr->isDirty(), curr->getLRU(), curr);		
		if(curr->getLRU() < evict->getLRU()) evict = curr;
		curr = curr->next;
	}

	DBG("called %d\n", evict->isDirty());

	if(evict->isDirty()) writeToDisk(evict);
	DBG("called\n");
	return evict;
}

void MyDB_BufferManager :: writeToDisk(PCB* pcb){
	PageHandle_Proxy* tmp = pcb->getProxy();
	// open the file the proxy is pointing to
	int fd = open((tmp->getFileName()).c_str(), O_WRONLY | O_FSYNC | O_CREAT);
	if(fd == -1 || lseek(fd, tmp->getIndex() * pageSize, SEEK_SET) == -1 ||
		write(fd, pcb->getAddr(), pageSize) == -1){
		ERR("Cannot open the file to write back!!! Should not happen!!!\n");
		return;
	}
	if(close(fd) == -1) ERR("Cannot close the file to write back!!! Should not happen!!!\n");
		
	return;
}

void MyDB_BufferManager :: readFromDisk(PCB* pcb){
	PageHandle_Proxy* tmp = pcb->getProxy();
	// open the file the proxy is pointing to
	DBG("fd = %s\n", (tmp->getFileName()).c_str());
	int fd = open((tmp->getFileName()).c_str(), O_RDONLY | O_FSYNC | O_CREAT);
	DBG("fd = %d\n", fd);
	if(fd == -1 || lseek(fd, tmp->getIndex() * pageSize, SEEK_SET) == -1 ||
		read(fd, pcb->getAddr(), pageSize) == -1){
		ERR("Cannot open the file to write back!!! Should not happen!!!\n");
		return;
	}
	if(close(fd) == -1) ERR("Cannot close the file to write back!!! Should not happen!!!\n");
		
	return;
}

#endif
















