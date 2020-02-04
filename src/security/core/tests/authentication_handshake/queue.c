#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "dds/ddsrt/heap.h"
#include "queue.h"

#define ADVANCE(queue)                                \
    do {                                              \
        if (queue->full) {                            \
            queue->wp = (queue->wp + 1) % queue->max; \
        }                                             \
        queue->rp = (queue->rp + 1) % queue->max;     \
        queue->full = (queue->rp == queue->wp);       \
    } while (0)

#define RETREAT(queue)                                \
    do {                                              \
        queue->full = false;                          \
        queue->wp = (queue->wp + 1) % queue->max;     \
    } while (0)

int enqueue(queue_t *queue, uint8_t data, char *name)
{
    int r = -1;
    assert(queue && queue->buffer);
    if (!queue_full(queue)) {
        queue->buffer[queue->rp] = data;
        queue->names[queue->rp] = name;
        ADVANCE(queue);
        r = 0;
    }
    return r;
}

int dequeue(queue_t *queue, uint8_t *data, char **name)
{
    assert(queue && data && queue->buffer);
    int r = -1;
    if (!queue_empty(queue)) {
        *data = queue->buffer[queue->wp];
        *name = queue->names[queue->wp];
        RETREAT(queue);
        r = 0;
    }
    return r;
}

queue_t *queue_init(char *name, uint8_t *buffer, char **names, size_t size)
{
    assert(buffer && size);
    queue_t *queue = ddsrt_malloc(sizeof(queue_t));
    assert(queue);
    queue->name = name;
    queue->buffer = buffer;
    queue->names = names;
    queue->max = size;
    queue_reset(queue);
    assert(queue_empty(queue));
    return queue;
}

void queue_free(queue_t *queue)
{
    assert(queue);
    ddsrt_free(queue);
}

void queue_reset(queue_t *queue)
{
    assert(queue);
    queue->rp = 0;
    queue->wp = 0;
    queue->full = false;
}

size_t queue_size(queue_t *queue)
{
    assert(queue);
    size_t size = queue->max;
    if (!queue->full) {
        if (queue->rp >= queue->wp) {
            size = queue->rp - queue->wp;
        } else {
            size = queue->max + queue->rp - queue->wp;
        }
    }
    return size;
}

size_t queue_capacity(queue_t *queue)
{
    assert(queue);
    return queue->max;
}

bool queue_empty(queue_t *queue)
{
    assert(queue);
    return (!queue->full && (queue->rp == queue->wp));
}

bool queue_full(queue_t *queue)
{
    assert(queue);
    return queue->full;
}
