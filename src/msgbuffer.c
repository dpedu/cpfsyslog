#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


struct msgbuffer_entry {
    char* data;
    struct msgbuffer_entry* next;
};


struct msgbuffer_entry* msgbuffer = NULL;


void buff_push(char* data) {
    struct msgbuffer_entry* oldhead = msgbuffer;
    msgbuffer = malloc(sizeof(struct msgbuffer_entry));
    msgbuffer->data = data; // or strdup(data) if data should be moved to heap
    msgbuffer->next = oldhead;
}


char* buff_pop() {
    // assert(msgbuffer != NULL); // Popped when no items present
    if(msgbuffer == NULL) return NULL;
    char* result = msgbuffer->data;
    struct msgbuffer_entry* oldhead = msgbuffer;
    msgbuffer = msgbuffer->next;
    free(oldhead);
    return result;
}


int buff_count() {
    int len = 0;
    struct msgbuffer_entry* current = msgbuffer;
    while (current != NULL) {
        current = current->next;
        len++;
    }
    return len;
}


void buff_freeall() {
    struct msgbuffer_entry* current = msgbuffer;
    struct msgbuffer_entry* next = msgbuffer;
    while (current != NULL) {
        next = current->next;
        free(current->data);  // maybe not desirable in reuse
        free(current);
        current = next;
    }
    msgbuffer = NULL;
}


char* buff_pop_head() {
    // assert(msgbuffer != NULL); // Popped when no items present
    if(msgbuffer == NULL) return NULL;
    char* result = NULL;
    struct msgbuffer_entry* previous = msgbuffer;
    struct msgbuffer_entry* current = msgbuffer;
    while (current->next != NULL) {
        previous = current;
        current = current->next;
    }
    result = current->data;
    previous->next = NULL;
    if(current == msgbuffer) msgbuffer = NULL;
    free(current);
    return result;
}


void buff_push_head(char* data) {
    struct msgbuffer_entry* new = malloc(sizeof(struct msgbuffer_entry));
    new->data = data;
    new->next = NULL;
    if(msgbuffer == NULL) {
        msgbuffer = new;
    } else {
        struct msgbuffer_entry* current = msgbuffer;
        while (current->next != NULL) {
            current = current->next;
        }
        current -> next = new;
    }
}
