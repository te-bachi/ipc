#include <stdlib.h>
#include "List.h"

/*
#include <stdio.h>
*/

List *list_new() {
    List *list;
    
    list = (List *) malloc(sizeof(List));
    
    return list;
}

void list_add(List *list, void *data) {
    ListEntry *entry;
    
    entry = (ListEntry *) malloc(sizeof(ListEntry));
    entry->data = data;
    
    if (list->size == 0) {
        entry->next      = NULL;
        entry->previous  = NULL;
        list->head       = entry;
        list->tail       = entry;
    } else {
        entry->next      = NULL;
        entry->previous  = list->head;
        list->tail->next = entry;
        
        list->tail       = entry;
    }
    list->size++;
}

ListIter *list_iter_new(List *list) {
    ListIter *iter;
    
    iter = (ListIter *) malloc(sizeof(ListIter));
    iter->current = list->head;
    
    return iter;
}

void *list_iter_next(ListIter *iter) {
    void *data;
    
    data = NULL;
    
    if (iter->current != NULL) {
        data          = iter->current->data;
        iter->current = iter->current->next;
    }
    
    return data;
}

void list_iter_free(ListIter *iter) {
    free(iter);
}

void list_free(List* list) {
    ListEntry *oldEntry;
    ListEntry *newEntry;
    
    // Lösche alle Einträge
    oldEntry = list->head;
    while (oldEntry != NULL) {
        newEntry = oldEntry->next;
        free(oldEntry);
        oldEntry = newEntry;
    }
    
    // Lösche Liste selbst
    free(list);
    
}

/*
int main(int argc, char *argv[]) {
    List     *list;
    ListIter *iter;
    void     *data;
    int       number;
    
    list = list_new();
    iter = list_iter_new(list);
    
    // Iteriere über leere Liste
    while ((data = list_iter_next(iter)) != NULL) {
        number = (int) data;
        printf("==> %d\n", number);
    }
    
    // Fügt erstes Element hinzu
    list_add(list, (void *) 7);
    
    // Iteriere über Liste
    iter = list_iter_new(list);
    while ((data = list_iter_next(iter)) != NULL) {
        number = (int) data;
        printf("==> %d\n", number);
    }
    printf("\n");
    
    // Fügt weitere Elemente hinzu
    list_add(list, (void *) 4);
    list_add(list, (void *) 5);
    list_add(list, (void *) 2);
    list_add(list, (void *) 6);
    list_add(list, (void *) 1);
    
    
    iter = list_iter_new(list);
    while ((data = list_iter_next(iter)) != NULL) {
        number = (int) data;
        printf("==> %d\n", number);
    }
    printf("\n");
    
    list_free(list);
    
    return 0;
}
*/

