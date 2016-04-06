#ifndef _LIST_H
#define _LIST_H
typedef struct _list* List;
struct _list{
	void* key;
	List next;

};
typedef int (counter(void*));
typedef int (comparator)(void*,void*);
List newList(void);
void add_list(void*,List);
List delete_list(void*,List,void (*foo)(void*));
void* list_get(int,List);
void* list_findMatch(List,int (*foo)(void*,void*),void* target);
void destroyList(List,void (*foo)(void*));
void* list_getKey(List l);
List list_pop(List l);
void printList(List l,void (*foo)(void*));
List list_push (List l,void* newItem);
List list_insert(List l,void* newContent,comparator c);
int list_aggregate(List l, counter h);
#endif
