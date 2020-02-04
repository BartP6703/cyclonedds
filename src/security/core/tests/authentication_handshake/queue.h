#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdbool.h>

typedef struct _queue_t {
    char *name;
    uint8_t *buffer;
    char **names;
    size_t rp;
    size_t wp;
    size_t max;
    bool full;
} queue_t;

queue_t *queue_init(char *name, uint8_t *buffer, char **names, size_t size);
void queue_free(queue_t *cbuf);
void queue_reset(queue_t *cbuf);
bool queue_empty(queue_t *cbuf);
bool queue_full(queue_t *cbuf);
size_t queue_capacity(queue_t *cbuf);
size_t queue_size(queue_t *cbuf);

int enqueue(queue_t *queue, uint8_t data, char *name);
int dequeue(queue_t *queue, uint8_t *data, char **name);

#endif //QUEUE_H_
