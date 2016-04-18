#ifndef DLLIST_H
#define DLLIST_H



typedef struct dlnode{
	struct dlnode* last;
	struct dlnode* next;
	void* key;
}dlnode_t;

typedef struct dllist{
	dlnode_t* first;
	dlnode_t* last;
	int num;
}dllist_t;

typedef void (*freefunc_t) (void*);
typedef void (*printfunc_t) (void*);

dllist_t* new_dllist (void);
void delete_dllist (dllist_t*, freefunc_t);
void      insert_dllist (dllist_t*, void* key);
void	insert_dllist_link (dllist_t* list, dlnode_t* link);
void printdllist (dllist_t* lst, printfunc_t printfunc);
void  append_dllist (dllist_t* to, dllist_t* from);
void dllist_delete_node (dllist_t* lst, dlnode_t* node, int free_node, freefunc_t freefunc);
#endif
