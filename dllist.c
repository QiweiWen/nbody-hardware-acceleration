#include "dllist.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int sane (dllist_t* lst){
	if (lst->first && lst->last && lst->first != lst->last){
		return (lst->last->last != NULL);
	}
	return 1;
}

dllist_t* new_dllist (void){
	dllist_t* res = malloc (sizeof (dllist_t));
	memset (res, 0, sizeof (dllist_t));
	return res;
}

void delete_dllist (dllist_t* list, freefunc_t freefunc){
	dlnode_t* curr = list->first, 
			* last = curr;
#ifdef DEBUG
	int debug_sum = 0;
#endif
	while (curr != NULL){
		if (freefunc){
			freefunc (curr->key);
		}
		last = curr;
		curr = curr->next;
		free (last);
#ifdef DEBUG
		debug_sum++;
#endif
	}
#ifdef DEBUG
	assert(debug_sum == list->num);
#endif
	free (list);
}


void      insert_dllist (dllist_t* list, void* key){
	dlnode_t* node = malloc (sizeof (dlnode_t));
	node->key = key;
	insert_dllist_link (list, node);
}

void	insert_dllist_link (dllist_t* list, dlnode_t* link){
	list->num++;
	link -> last = NULL;
	link->next = list->first;
	if (list->first){
		list->first->last = link;
	}
	if (!list->last){
		list->last = link;
	}
	list->first = link;
	assert (sane (list));
}

void  append_dllist (dllist_t* to, dllist_t* from){
	assert (sane (to));
	assert (sane (from));
	if (from->num == 0) return;
	to->num += from->num;
	if (!to->first){
		
		to->first = from->first;
		to->last = from->last;

		assert (sane (to));
		return;
	}
	to->last->next = from->first;
	from->first->last = to->last;
	to->last = from->last;

	assert (sane (to));
	from->num = 0;
	from->first = NULL;
	from->last = NULL;

}

void printdllist (dllist_t* lst, printfunc_t printfunc){
	dlnode_t* curr = lst->first;
	printf ("num: %d\n", lst->num);
	while (curr != NULL){
		if (printfunc){
			printfunc (curr->key);
			printf ("->");
		}
		curr = curr->next;	
	}
	printf ("\n");
}	

void dllist_delete_node (dllist_t* lst, dlnode_t* node, int free_node, freefunc_t freefunc){
	if (lst->num == 1){
		if (node == lst->first){
			if (free_node){
				if (freefunc){
					freefunc(node->key);
				}
				free (node);
			}
			memset (lst, 0, sizeof (dllist_t));
		}
	}else{
		lst->num--;
		if (node == lst->first){
			lst->first = lst->first->next;
			lst->first->last = NULL;	
		}else if (node == lst->last){
			lst->last = lst->last->last;
			lst->last->next = NULL;
		}else{
			if (node->last){
				node->last->next = node->next;
			}
			if (node->next){
				node->next->last = node->last;	
			}
		}

		if (free_node){
			if (freefunc){
				freefunc(node->key);
			}
			free (node);
		}
	}
	assert (sane (lst));
}
