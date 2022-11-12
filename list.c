#include <stdio.h>
#include <stdlib.h>

struct payload {
    int data[5];
};

struct node {
    struct node *next;
    struct node *prev;
    struct payload data;
};

void free_if_not_null(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

void delete_list(struct node **list)
{
    if (!*list) {
        return;
    }

    struct node *ptr = *list;
    do {
        struct node *to_free = ptr;
        ptr = ptr->next;
        free(to_free);
    } while ((*list) != ptr);

    *list = NULL;
}

void print_list(const struct node *const list)
{
    printf("List %p:", (void *)list);

    if (!list) {
        printf("\n");
        return;
    }

    const struct node *ptr = list;
    do {
        printf("%d ", ptr->data.data[0]);
        ptr = ptr->next;
    } while (list != ptr);

    printf("\n");
}

void print_list_reverse(const struct node *const list)
{
    printf("List %p:", (void *)list);

    if (!list) {
        printf("\n");
        return;
    }

    const struct node *ptr = list->prev;
    do {
        printf("%d ", ptr->data.data[0]);
        ptr = ptr->prev;
    } while (list->prev != ptr);

    printf("\n");
}

/*
struct node *add_node_after(struct node **list, int value) {
  struct node *new_node = (struct node *)malloc(sizeof(struct node));
  new_node->data = value;

  if (!*list) {
    new_node->next = new_node;
    new_node->prev = new_node;
    *list = new_node;
    return new_node;
  }

  new_node->next = (*list)->next;
  new_node->prev = *list;

  (*list)->next->prev = new_node;
  (*list)->next = new_node;

  return new_node;
}
*/

struct node *add_node_before(struct node **list, const struct payload value)
{
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    new_node->data = value;

    if (!*list) {
        // si la list está vacía, la lista pasa a ser el nodo recien creado
        new_node->next = new_node;
        new_node->prev = new_node;
        *list = new_node;
        return new_node;
    }

    new_node->next = (*list);
    new_node->prev = (*list)->prev;

    (*list)->prev->next = new_node;
    (*list)->prev = new_node;

    return new_node;
}

struct node *push_back(struct node **list, const struct payload value)
{
    return add_node_before(list, value);
}

struct node *push_front(struct node **list, const struct payload value)
{
    *list = push_back(list, value);
    return *list;
}

struct node *extract_node(struct node **list)
{
    if (!*list) {
        return NULL;
    }

    struct node *ptr = *list;

    if ((*list)->next == *list) {
        // sólo hay un nodo -> es la cabecera y ya apunta a si misma
        *list = NULL;
        return ptr;
    }

    ptr->prev->next = (*list)->next;
    ptr->next->prev = (*list)->prev;

    *list = (*list)->next;
    // se extrae el nodo, es decir se extrae la cabeza de la lista **list
    // y la cabecera de la lista pasa a ser el siguiente

    ptr->next = ptr;
    ptr->prev = ptr;
    return ptr;
}

struct node *pop_back(struct node **list)
{
    if (!*list) {
        return NULL;
    }

    struct node *ptr = (*list)->prev;
    // si es la cabeza, hay que actualizar la cabeza,
    // en otro caso se modifica otro puntero
    return extract_node(ptr->next == ptr ? list : &ptr);
}

struct node *pop_front(struct node **list)
{
    if (!*list) {
        return NULL;
    }

    // quitar el primero es mover la cabecera y quitar el último
    *list = (*list)->next;
    return pop_back(list);
}

struct node *find_index(struct node *const list, unsigned int index)
{
    if (!list) {
        return NULL;
    }

    struct node *ptr = list;
    unsigned int i = 0;
    do {
        if (i == index) {
            return ptr;
        }

        ptr = ptr->next;
        i++;
    } while (list != ptr);

    return NULL;
}

struct node *pop_index(struct node **list, unsigned int index)
{
    if (!*list) {
        return NULL;
    }

    struct node *ptr = find_index(*list, index);

    if (!ptr) {
        return NULL;
    }

    return extract_node(*list == ptr ? list : &ptr);
}

struct node *push_index(struct node **list, unsigned int index, struct payload data)
{
    // no se inserta en una lista vacía si no es al principio
    if (!*list && index) {
        return NULL;
    }

    // push_front se encarga de actualizar la cabeza
    if (!index) {
        return push_front(list, data);
    }

    // no es la cabeza, hay que encontrar el nodo
    struct node *ptr = *list;
    unsigned int i = 0;
    do {
        if (i == index) {
            break;
        }

        ptr = ptr->next;
        i++;
    } while ((*list) != ptr);

    // se puede después de él ultimo, pero no más allá
    if ((*list) == ptr && i != index) {
        return NULL;
    }

    // en este punto ptr apunta al nodo que está en el indice,
    // así que sólo hay que insertar detrás de él para desplazarlo
    return push_back(&ptr, data);
}

struct node *swap(struct node **n1, struct node **n2)
{
    if (!*n1 || !*n2) {
        return NULL;
    }

    struct node *const n1_prev = (*n1)->prev;
    struct node *const n2_next = (*n2)->next;

    n1_prev->next = *n2;
    n2_next->prev = *n1;

    if ((*n1)->next == *n2 || (*n1)->prev == *n2) {
        (*n1)->prev = *n2;
        (*n2)->next = *n1;
    } else {
        (*n1)->next->prev = *n2;
        (*n2)->prev->next = *n1;
        (*n1)->prev = (*n2)->prev;
        (*n2)->next = (*n1)->next;
    }

    (*n1)->next = n2_next;
    (*n2)->prev = n1_prev;
    struct node *const tmp = *n1;
    *n1 = *n2;
    *n2 = tmp;
    return *n1;
}

int main(void)
{
    struct node *list = NULL;
    print_list(list);
    // add_node_after(&list, 123);
    // add_node_after(&list, 678);
    // add_node_after(&list, 888);
    print_list(list);
    for (int i = 0; i < 5; i++) {
        push_back(&list, (struct payload){ .data = { i } });
    }

    print_list(list);

    // free_if_not_null(extract_node(&list));
    // free_if_not_null(extract_node(&list->next));
    // free_if_not_null(extract_node(&list->next->next));

    struct node *ptr = list->prev;
    free_if_not_null(extract_node(&list));
    print_list(list);
    ptr = list->prev;
    free_if_not_null(extract_node(&list));
    print_list(list);
    free_if_not_null(pop_back(&list));
    print_list(list);
    free_if_not_null(pop_back(&list));
    print_list(list);
    free_if_not_null(pop_back(&list));
    print_list(list);
    free_if_not_null(pop_back(&list));
    print_list(list);

    for (int i = 0; i < 5; i++) {
        push_back(&list, (struct payload){ .data = { i } });
    }

    for (int i = 0; i < 5; i++) {
        print_list(list);
        free_if_not_null(pop_front(&list));
    }

    for (int i = 0; i < 5; i++) {
        push_back(&list, (struct payload){ .data = { i } });
    }

    print_list(list);
    free(pop_index(&list, 100));
    print_list(list);
    free(pop_index(&list, 0));
    print_list(list);
    free(pop_index(&list, 1));
    print_list(list);
    free(pop_index(&list, 2));
    print_list(list);
    free(pop_index(&list, 1));
    print_list(list);
    free(pop_index(&list, 0));
    print_list(list);

    push_index(&list, 0, (struct payload){ .data = { 1000 } });
    push_index(&list, 1, (struct payload){ .data = { 2000 } });
    push_index(&list, 2, (struct payload){ .data = { 3000 } });
    push_index(&list, 3, (struct payload){ .data = { 4000 } });
    print_list(list);

    ptr = list;
    struct node *ptr2 = list->next;
    printf("addr %p\n", (void *)list);
    swap(&list, &ptr);
    printf("addr %p\n", (void *)list);
    print_list(list);
    swap(&ptr, &ptr2);
    printf("addr %p\n", (void *)list);
    print_list(list);
    swap(&ptr, &ptr2);
    printf("addr %p\n", (void *)list);
    print_list(list);
    swap(&list, &ptr2);
    printf("addr %p\n", (void *)list);
    print_list(list);

    // free_if_not_null(pop_back(&list));
    /*
    print_list(list);
    free(pop_index(&list, 2));
    free(pop_index(&list, 2));
    free(pop_index(&list, 3));
    print_list(list);
    push_index(&list, 1, 333);
    push_index(&list, 1, 444);
    push_index(&list, 5, 7777);
    print_list(list);
    push_index(&list, 1, 333);
    push_index(&list, 1, 444);
    push_index(&list, 5, 7777);
    free(pop_index(&list, 0));
    free(pop_index(&list, 8));
    free(pop_index(&list, 7));
    free(pop_index(&list, 4));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    free(pop_index(&list, 0));
    push_index(&list, 1, 333);
    push_index(&list, 0, 333);
    push_index(&list, 5, 333);
    push_index(&list, 0, 222);
    push_index(&list, 2, 444);
    print_list(list);
    // free(extract_node(&(list->next)));
    printf("###############\n");
    print_list(list);
    printf("###############\n");
    free_if_not_null(extract_node(&list));
    print_list(list);
    free_if_not_null(extract_node(&list));
    print_list(list);
    free_if_not_null(extract_node(&list));
    print_list(list);
    free_if_not_null(extract_node(&list));
    print_list(list);
    printf("###############\n");
  */
    // move_element(&list, 1, 2);

    // print_list_reverse(list);

    // print_list(list);

    // ptr = pop_back(&list);
    // free(ptr);
    //  print_list(list);

    // ptr = pop_front(&list);
    // free(ptr);
    // print_list(list);
    delete_list(&list);
    for (int i = 0; i < 128; i++) {
        push_back(&list, (struct payload){ .data = { i } });
    }
    print_list(list);
    delete_list(&list);
}
