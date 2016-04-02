#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


int add_count, push_count;

List newList(void){
	List l = (List)malloc(sizeof(struct _list));
	l->key = NULL;
	l->next = NULL;
	return l;
}

void add_list(void* newItem,List l){
	++add_count;
	if (l->key == NULL){
		l->key = newItem;
	}else{
		List curr = l;
		while (curr -> next != NULL){
			curr = curr->next;
		}
		List newLink = (List)malloc(sizeof(struct _list));
		newLink->key = newItem;
		newLink->next = NULL;
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
		if (last->key != NULL) foo(last->key);
		free(last);
	}
}

void* list_get(int index,List l){
	assert(index >= 0);
	List curr = l;
	while (curr != NULL && index > 0){
		curr = curr->next;
		index--;
	}
	if (curr == NULL) return NULL;
	return curr->key;
}

void printList(List l,void (*foo)(void*)){
	List curr = l;
	while (curr != NULL){
		foo(curr->key);
		curr = curr->next;
	}
	printf("end\n");
}

void* list_findMatch(List l,int (*foo)(void*,void*),void* target){
	List curr = l;
	int found = 0;
	while (!found && curr != NULL){
		if (foo(curr->key,target)){
			found = 1;
		}else{
			curr = curr->next;
		}
	}
	if (curr != NULL) return curr->key;
	return NULL;
}


void* list_getKey(List l){
	return l->key;
}

List list_pop(List l){
	return l->next;
}

List list_push (List l,void* newItem){
	++push_count;
	List newLink = (List)malloc(sizeof (struct _list));
	newLink->key = newItem;
	newLink->next = l;
	return newLink;
}

List list_insert(List l,void* newContent,comparator c){
    List temp = l;
    List last = temp;
    while (temp != NULL){
        if (!c(temp->key,newContent)){
            break;
        }
        last = temp;
        temp = temp->next;
    }
    List newLink = (List)malloc(sizeof(struct _list));
    newLink->key = newContent;
    if (temp == last){ // insert at front
        newLink->next = l;
        l = newLink;
    }else if (temp == NULL){ //insert at the end
        newLink->next = NULL;
        last->next = newLink;
    }else{
        newLink->next = temp;
        last->next = newLink;
    }
    return l;
}

int list_aggregate(List l, counter h){
   List temp = l;
   int result = 0;
   while (temp != NULL){
      result += h(temp->key);
      temp = temp->next;
   }
   return result;
}

