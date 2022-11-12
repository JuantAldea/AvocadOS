#include <stdio.h>

#define BUFFER_SIZE 3

struct circular_buffer {
    int *buffer[BUFFER_SIZE]; // array podría reservarse en heap dinámicamente
    int head, end;
    int count;
};

void init(struct circular_buffer *cb)
{
    cb->head = 0;
    cb->end = 0;
    cb->count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        cb->buffer[i] = NULL;
    }
}

int full(struct circular_buffer *cb)
{
    return cb->count == BUFFER_SIZE;
}

int empty(struct circular_buffer *cb)
{
    return !cb->count;
}

int push_back(struct circular_buffer *cb, int *value)
{
    if (full(cb)) {
        return 0;
    }

    cb->buffer[cb->head] = value;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count++;
    return 1;
}

int *pop_front(struct circular_buffer *cb)
{
    if (empty(cb)) {
        return NULL;
    }

    int *value = cb->buffer[cb->end];
    cb->end = (cb->end + 1) % BUFFER_SIZE;
    cb->count--;
    return value;
}

void print(struct circular_buffer *cb)
{
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%d, ", cb->buffer[i] ? *cb->buffer[i] : -1);
    }
    printf("\n");
}

int main(void)
{
    struct circular_buffer buffer;

    init(&buffer);

    int array[] = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
    int p;
    int *ptr;
    ptr = pop_front(&buffer);
    printf("pop?: %d; %d\n", ptr != NULL, ptr ? *ptr : -1);
    for (int i = 0; i < 10; i++) {
        p = push_back(&buffer, &array[i]);
        printf("pushed?: %d\n", p);
    }

    ptr = pop_front(&buffer);
    printf("pop?: %d; %d\n", ptr != NULL, ptr ? *ptr : -1);
    ptr = pop_front(&buffer);
    p = push_back(&buffer, &array[7]);
    printf("pushed?: %d\n", p);
    printf("pop?: %d; %d\n", ptr != NULL, ptr ? *ptr : -1);
    ptr = pop_front(&buffer);
    printf("pop?: %d; %d\n", ptr != NULL, ptr ? *ptr : -1);
    ptr = pop_front(&buffer);
    printf("pop?: %d; %d\n", ptr != NULL, ptr ? *ptr : -1);
    ptr = pop_front(&buffer);
    printf("pop?: %d; %d\n", ptr != NULL, ptr ? *ptr : -1);

    //  int *ptr = pop_front(&buffer);
    //  printf("PTR %p, %d\n", ptr, ptr ? *ptr : -1);
    //  ptr = pop_front(&buffer);
    //  printf("PTR %p, %d\n", ptr, ptr ? *ptr : -1);
    print(&buffer);
}