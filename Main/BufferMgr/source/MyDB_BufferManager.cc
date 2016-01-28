
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
	return helper_getPage(table_ptr, index, false, false);
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	MyDB_TablePtr table_ptr = make_shared <MyDB_Table> ("tempTable", "tmpFile");
	return helper_getPage(table_ptr, tmpPageCount ++, false, true);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr table_ptr, long index) {
	return helper_getPage(table_ptr, index, true, false);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	MyDB_TablePtr table_ptr = make_shared <MyDB_Table> ("tempTable", "tmpFile");
	return helper_getPage(table_ptr, tmpPageCount ++, true, true);
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
		if(listFree == nullptr) listFree = tmp;
		else{
			tmp->next = listFree;
			listFree->prev = tmp;
			listFree = tmp;
		}
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


PCB* MyDB_BufferManager :: getPCB(bool pinned){
	// when there is still free pages
	// DBG1("listFree=%#X,", listFree);
	if(listFree){
		PCB* tmp = listFree;
		// DBG1("listFree->next%#X\n", listFree->next);
		// listFree = listFree->next;
		// DBG("listUnpin=%#X\n", listUnpin);
		// move tmp to listUnpin // TODO differentiate between unpin and pin
		// PCB** head = pinned ? &listPin : &listUnpin;
		// // DBG2("head=%#X\n", head);
		// if(!*head) {
		// 	tmp->next = nullptr;
		// 	tmp->prev = nullptr;
		// 	*head = tmp;
		// }
		// else{
		// 	tmp->next = *head;
		// 	(*head)->prev = tmp;
		// 	*head = tmp;
		// }
		moveToList(tmp, (pinned? PINNED : UNPINNED));
		return tmp;
	}

	// when no unpin page exists
	if(!listUnpin) return nullptr;

	PCB* tmp = LRU(pinned);
	return tmp;
}


void* MyDB_BufferManager :: getBytes(PageHandle_Proxy* my_pHandleProxy){
	DBG("called\n");
	PCB* tmp = my_pHandleProxy->getPCB();
	// cout << "my_pHandleProxy=" << my_pHandleProxy;
	DBG("my_pHandleProxy=%#X\n", my_pHandleProxy);
	DBG1("pcb=%#X\n", tmp);
	if(tmp == nullptr){
		DBG("called\n");
		tmp = getPCB(my_pHandleProxy->isPinned());
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
		ERR("Write to a buffer that does not exist, should not happen\n");	
		return;
	}
	
	tmp->setDirty();
}


void MyDB_BufferManager :: freePage(PageHandle_Proxy* my_pHandleProxy){
	PCB* tmp = my_pHandleProxy->getPCB();
	if(tmp == nullptr){
		ERR("Free a buffer that does not exist, should not happen\n");
		return;
	}

	// free the page from its pool
	// TODO
	bool pu_pinned = my_pHandleProxy->isPinned();

	// if pinned non-ananymous, downgrad it from pinned to unpinned. put in the
	// unpin pool. don't free the proxy
	if(!my_pHandleProxy->isAnonymous() && my_pHandleProxy->isPinned()){
		if(tmp->next && tmp->prev)
			DBG1("pcb->prev->LRU=%d, pcb->LRU=%d, pcb->next->LRU=%d\n", tmp->prev->getLRU(), tmp->getLRU(), tmp->next->getLRU());

		moveToList(my_pHandleProxy->getPCB(), UNPINNED);
		// setProxy type
		my_pHandleProxy->setPinned(UNPINNED);
		showBufferPool();
	}
	// if anonymous page, put it back to the free pool, free the proxy	
	else{
		moveToList(my_pHandleProxy->getPCB(), FREE);
		// remove the proxy from the list
		freeProxy(my_pHandleProxy); // placeholder
	}

}


void MyDB_BufferManager :: moveToList(PCB* pcb, int destination){
	PCB* tmp = pcb;
	if(tmp == nullptr){
		ERR("Move a buffer that does not exist, should not happen\n");
		return;
	}

	if(destination == tmp->getType()) return;

	PCB** destination_list;
	if(destination == FREE) destination_list = &listFree;
	else if(destination == UNPINNED) destination_list = &listUnpin;
	else destination_list = &listPin;

	PCB** head;
	if(tmp->getType() == FREE) head = &listFree;
	else if(tmp->getType() == UNPINNED) head = &listUnpin;
	else head = &listPin;

	if(!*destination_list) { // destination == nullptr
		if(!tmp->prev){ // tmp is the head in its own list
			if(tmp->next) tmp->next->prev = nullptr;
			*head = tmp->next;
		}
		else{ 
			if(!tmp->next){ // tmp is the tail
				tmp->prev->next = nullptr;
			}
			else{
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;
			}
		}
		tmp->next = nullptr;
		tmp->prev = nullptr;
		*destination_list = tmp;
	}
	else{
		if(!tmp->prev){ // tmp is the head in its own list
			if(tmp->next) tmp->next->prev = nullptr;
			*head = tmp->next;
		}
		else{ 
			if(!tmp->next){ // tmp is the tail
				tmp->prev->next = nullptr;
			}
			else{
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;
			}
		}
		tmp->next = *destination_list;
		tmp->prev = nullptr;
		(*destination_list)->prev = tmp;
		*destination_list = tmp;
	}

	tmp->setType(destination);
}


void MyDB_BufferManager :: freeProxy(PageHandle_Proxy* my_pHandleProxy){
	if(!my_pHandleProxy->prev){
		listPhandleProxy = my_pHandleProxy->next;
		if(listPhandleProxy) listPhandleProxy->prev = nullptr;
	}
	else{
		if(!my_pHandleProxy->next){
			my_pHandleProxy->prev->next = nullptr;
		}
		else{
			my_pHandleProxy->prev->next = my_pHandleProxy->next;
			my_pHandleProxy->next->prev = my_pHandleProxy->prev;
		}
	}
	free(my_pHandleProxy);
}


void MyDB_BufferManager :: showBufferPool(){
	{
		printf("\t\t\t");
		PCB* tmp = listFree;
		int count = 0;
		printf("free pool(%d):", count);
		tmp = listFree;

		while(tmp){printf("%d(%d)->", tmp->getLRU(), tmp->getType()); tmp = tmp->next;}
		printf("null\n");
	}
	
	{
		printf("\t\t\t");
		PCB* tmp = listUnpin;
		int count = 0;
		PCB* tail = tmp;
		while(tmp){tail = tmp; tmp = tmp->next; ++ count;}
		while(tail){printf("%d->", tail->getLRU()); tail=tail->prev;}
		printf("unpin pool(%d):", count);
		
		tmp = listUnpin;
		while(tmp){printf("%d(%d)->", tmp->getLRU(), tmp->getType()); tmp = tmp->next;}
		printf("null\n");
	}

	{
		printf("\t\t\t");
		PCB* tmp = listPin;
		int count = 0;
		PCB* tail = tmp;
		while(tmp){tail = tmp; tmp = tmp->next; ++ count;}
		while(tail){printf("%d->", tail->getLRU()); tail=tail->prev;}
		printf("pin pool(%d):", count);
		
		tmp = listPin;
		while(tmp){printf("%d(%d)->", tmp->getLRU(), tmp->getType()); tmp = tmp->next;}
		printf("null\n");
	}
}


//*************************** private function **************************//

MyDB_PageHandle MyDB_BufferManager :: helper_getPage(MyDB_TablePtr table_ptr, long index, bool my_pinned, bool my_anonymous){
	DBG("called\n");
	if(!table_ptr){
		ERR("error: MyDB_TablePtr null pointer!!! return nullptr\n");
		return nullptr;
	} 
	if(index < 0){
		ERR("error: MyDB_TablePtr null pointer!!! return nullptr\n");	
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
	
	if(curr != nullptr){
		curr->refInc();
		if(my_pinned && !(curr->isPinned())) curr->setPinned(PINNED); // if it is not in the pinned pool, need to move it to the pinned pool, immediately
																// it is done in moveToList();
	}
	else{
		curr = new PageHandle_Proxy(table_ptr->getStorageLoc(), index, my_pinned, my_anonymous, this);
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


PCB* MyDB_BufferManager :: LRU(bool my_pinned){ // get a available page from unpinned pool 
	DBG("called\n");
	// pool is listUnpin
	if(!listUnpin) return nullptr;

	PCB* curr = listUnpin; PCB* prev = listUnpin; 
	PCB* evict = curr;
	PCB* before_evict = curr;
	while(curr){
		DBG("called dirty=%d, LRU=%d, curr=%#X\n", curr->isDirty(), curr->getLRU(), curr);		
		if(curr->getLRU() < evict->getLRU()){
			before_evict = prev;
			evict = curr;
		}
		prev = curr;
		curr = curr->next;
	}

	// if(my_pinned){
	// 	if(before_evict == evict) listUnpin = evict->next;
	// 	else before_evict->next = evict->next;
	// 	listUnpin->prev = nullptr;

	// 	evict->next = listPin;
	// 	listPin->prev = evict;
	// 	listPin = evict;
	// }
	if(my_pinned) moveToList(evict, PINNED);
	if(evict->isDirty()) writeToDisk(evict);
	// remove proxy info
	PageHandle_Proxy* tmp = evict->getProxy();
	tmp->setPCB(nullptr);

	return evict;
}


void MyDB_BufferManager :: writeToDisk(PCB* pcb){
	PageHandle_Proxy* tmp = pcb->getProxy();
	// open the file the proxy is pointing to
	int fd = open((tmp->getFileName()).c_str(), O_RDWR | O_FSYNC | O_CREAT);
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
	int fd = open((tmp->getFileName()).c_str(), O_RDWR | O_FSYNC | O_CREAT, S_IRWXU);
	// DBG1("fd= %d\n",fd);
	// DBG("fd = %d\n", fd);
	if(fd == -1 || lseek(fd, tmp->getIndex() * pageSize, SEEK_SET) == -1 ||
		read(fd, pcb->getAddr(), pageSize) == -1){
		ERR("Cannot open the file to write back!!! Should not happen!!!\n");
		return;
	}
	
	if(close(fd) == -1) ERR("Cannot close the file to write back!!! Should not happen!!!\n");
		
	return;
}

#endif
















