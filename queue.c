#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    if (!q)
        return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    if (!q)
        return;

    while (q->head) {
        list_ele_t *del = q->head;
        q->head = q->head->next;
        free(del->value);
        free(del);
    }
    free(q);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    if (!q)
        return false;

    list_ele_t *newh = malloc(sizeof(list_ele_t));
    if (!newh)
        return false;

    size_t len = strnlen(s, 1024);

    newh->value = (char *) malloc((len + 1) * sizeof(char));
    if (!newh->value) {
        free(newh);
        return false;
    }
    strncpy(newh->value, s, len + 1);
    newh->next = q->head;

    if (!q->head)
        q->tail = newh;

    q->head = newh;
    q->size += 1;
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    if (!q)
        return false;

    list_ele_t *newh = malloc(sizeof(list_ele_t));
    if (!newh)
        return false;

    size_t len = strnlen(s, 1024);
    newh->value = (char *) malloc(len + 1);
    if (!newh->value) {
        free(newh);
        return false;
    }
    strncpy(newh->value, s, len + 1);
    newh->next = NULL;

    if (!q->head) {
        q->head = newh;
    } else {
        q->tail->next = newh;
    }
    q->tail = newh;
    q->size += 1;

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return true if successful.
 * Return false if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 * The space used by the list element and the string should be freed.
 */
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    if (!q || !q->head)
        return false;

    list_ele_t *del = q->head;
    q->head = del->next;
    q->size -= 1;
    if (!q->head)
        q->tail = NULL;

    if (sp) {
        size_t len = strnlen(del->value, 1024);
        strncpy(sp, del->value, bufsize);
        if (len >= bufsize) {
            sp[bufsize - 1] = '\0';
        } else {
            sp[len] = '\0';
        }
    }

    free(del->value);
    free(del);

    return true;
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    if (!q)
        return 0;
    return q->size;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if (!q || q->size <= 1)
        return;

    list_ele_t *prev = q->head;
    list_ele_t *this = q->head->next;
    while (this) {
        list_ele_t *next = this->next;
        this->next = prev;
        prev = this;
        this = next;
    }

    q->tail = q->head;
    q->tail->next = NULL;

    q->head = prev;
}

static void split(list_ele_t *head, list_ele_t **list_1, list_ele_t **list_2)
{
    list_ele_t *slow = head;
    list_ele_t *fast = head->next;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    *list_2 = slow->next;
    *list_1 = head;
    slow->next = NULL;
}


static list_ele_t *merge(list_ele_t *list_1, list_ele_t *list_2)
{
    list_ele_t *head = NULL;
    list_ele_t **cursor = &head;

    while (list_1 && list_2) {
        if (strncmp(list_1->value, list_2->value, 1024) <= 0) {
            *cursor = list_1;
            list_1 = list_1->next;
        } else {
            *cursor = list_2;
            list_2 = list_2->next;
        }
        cursor = &((*cursor)->next);
    }

    if (list_1) {
        *cursor = list_1;
    } else if (list_2) {
        *cursor = list_2;
    }

    return head;
}

static list_ele_t *merge_sort(list_ele_t *head)
{
    if (!head || !head->next)
        return head;

    list_ele_t *list_1;
    list_ele_t *list_2;
    split(head, &list_1, &list_2);

    list_1 = merge_sort(list_1);
    list_2 = merge_sort(list_2);

    return merge(list_1, list_2);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(queue_t *q)
{
    if (!q || q->size <= 1)
        return;

    q->head = merge_sort(q->head);

    while (q->tail->next) {
        q->tail = q->tail->next;
    }
}
