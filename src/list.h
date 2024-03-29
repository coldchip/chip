#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct _ListNode {
   struct _ListNode * next;
   struct _ListNode * previous;
} ListNode;

typedef struct _List {
   ListNode sentinel;
} List;

void                 list_clear (List *);

ListNode            *list_insert (ListNode *, void *);
void *               list_remove (ListNode *);
ListNode            *list_move (ListNode *, void *, void *);

size_t               list_size (List *);

#define              list_begin(list) ((list) -> sentinel.next)
#define              list_end(list) (& (list) -> sentinel)

#define              list_empty(list) (list_begin (list) == list_end (list))

#define              list_next(iterator) ((iterator) -> next)
#define              list_previous(iterator) ((iterator) -> previous)

#define              list_front(list) ((void *) (list) -> sentinel.next)
#define              list_back(list) ((void *) (list) -> sentinel.previous)

#endif