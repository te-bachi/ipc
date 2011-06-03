#ifndef __LIST_H__
#define __LIST_H__

typedef struct _List List;
typedef struct _ListEntry ListEntry;
typedef struct _ListIter ListIter;

typedef struct _List {
    ListEntry *head;
    ListEntry *tail;
    int        size;
} List;

typedef struct _ListEntry {
    void      *data;
    ListEntry *previous;
    ListEntry *next;
} ListEntry;

typedef struct _ListIter {
    ListEntry *current;
} ListIter;

List *list_new();
void list_add(List *list, void *data);
ListIter *list_iter_new(List *list);
void *list_iter_next(ListIter *iter);
void list_iter_free(ListIter *iter);
void list_free(List* list);

#endif
