#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>




List newList(void){
	List l = (List)malloc(sizeof(struct _list));
	l->key = NULL;
	l->next = NULL;
	return l;
}

void add_list(void* newItem,List l,int id){

	if (l->key == NULL){
		l->key = newItem;
		l->id = id;
	}else{
		List curr = l;
		while (curr -> next != NULL){
			curr = curr->next;
		}
		List newLink = (List)malloc(sizeof(struct _list));
		newLink->key = newItem;
		newLink->next = NULL;
		newLink->id = id;
		curr->next = newLink;
	}
}

List delete_list(void* del,List l,void (*foo)(void*)){
	List curr = l;
	List last = curr;
	int found = 0;
	if (l->key == del){
		curr = l;
		l = l->next;
		free(curr);
	}else{
		while (curr != NULL && !found){
			if (curr->key == del){
				found = 1;
			}else{
				last = curr;
				curr = curr->next;
			}
		}
		last->next = curr->next;
		foo(curr->key);
		free(curr);
	}
	return l;
}

void destroyList(List l,void (*foo)(void*)){
	List curr = l;
	List last = curr;
	while (curr != NULL){
		last = curr;
		curr = curr->next;
		if (last->key != NULL && foo) foo(last->key);
		free(last);
	}
}

void printList(List l,void (*foo)(void*)){
	List curr = l;
	while (curr != NULL){
		foo(curr->key);
		curr = curr->next;
	}
	printf("end\n");
}

void* list_getKey(List l){
	return l->key;
}

List list_pop(List l){
	return l->next;
}

List list_push (List l,void* newItem, int id){
	
	List newLink = (List)malloc(sizeof (struct _list));
	newLink->key = newItem;
	newLink->id  = id;
	newLink->next = l;
	return newLink;
}

