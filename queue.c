#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

static inline void swap_element(element_t *a, element_t *b)
{
    char *tmp = a->value;
    a->value = b->value;
    b->value = tmp;
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = (struct list_head *) malloc(sizeof(struct list_head));
    if (!q)
        return NULL;

    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *node;
    element_t *safe;

    list_for_each_entry_safe (node, safe, l, list) {
        list_del(&node->list);
        q_release_element(node);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    size_t len = strlen(s);

    element->value = malloc(strlen(s) + 1);
    if (!element->value) {
        free(element);
        return false;
    }

    strncpy(element->value, s, len);
    element->value[len] = '\0';

    list_add(&element->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    size_t len = strlen(s);

    element->value = malloc(strlen(s) + 1);
    if (!element->value) {
        free(element);
        return false;
    }

    strncpy(element->value, s, len);
    element->value[len] = '\0';

    list_add_tail(&element->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *node = head->next;
    list_del_init(node);

    element_t *element = container_of(node, element_t, list);
    if (sp) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *node = head->prev;
    list_del_init(node);

    element_t *element = container_of(node, element_t, list);
    if (sp) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    struct list_head *mid = head->next;
    for (struct list_head *fast = head->next->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        mid = mid->next;
    }
    list_del(mid);

    element_t *element = container_of(mid, element_t, list);
    q_release_element(element);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head)
        return false;

    LIST_HEAD(dup_list);

    struct list_head *node = (head)->next;
    struct list_head *next = node->next;
    bool prev_same = false;

    for (; next != (head); node = next, next = node->next) {
        element_t *left = list_entry(node, element_t, list);
        element_t *right = list_entry(next, element_t, list);
        bool same = !strcmp(left->value, right->value);
        if (same || prev_same)
            list_move(node, &dup_list);
        prev_same = same;
    }
    if (prev_same)
        list_move(node, &dup_list);

    list_for_each_safe (node, next, &dup_list) {
        list_del(node);
        element_t *entry = list_entry(node, element_t, list);
        q_release_element(entry);
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *h1 = head->next;
    struct list_head *h2 = h1->next;

    while (h1 != head && h2 != head) {
        swap_element(list_entry(h1, element_t, list),
                     list_entry(h2, element_t, list));

        h1 = h2->next;
        h2 = h1->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *h = head->next;
    struct list_head *t = head->prev;

    while (h != t) {
        swap_element(list_entry(h, element_t, list),
                     list_entry(t, element_t, list));

        if (h->next == t) {
            break;
        }

        h = h->next;
        t = t->prev;
    }
}

static struct list_head *merge_sort(struct list_head *start,
                                    struct list_head *end)
{
    if (start == end)
        return start;

    struct list_head *mid = start;
    struct list_head *flag = start;

    // If more than 2 nodes, split list into 2 parts and do merge sort on them
    while (flag->next && flag->next->next) {
        flag = flag->next->next;
        mid = mid->next;
    }

    flag = mid->next;
    mid->next = NULL;

    struct list_head *left = merge_sort(start, mid);
    struct list_head *right = merge_sort(flag, end);

    struct list_head *result = NULL;
    struct list_head **result_p = &result;

    // Merge two sorted lists
    for (; left && right; result_p = &(*result_p)->next) {
        element_t *l = list_entry(left, element_t, list);
        element_t *r = list_entry(right, element_t, list);

        struct list_head **this_p =
            strcmp(l->value, r->value) <= 0 ? &left : &right;
        *result_p = *this_p;
        *this_p = (*this_p)->next;
    }
    *result_p = left ? left : right;

    return result;
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *start = head->next;
    struct list_head *end = head->prev;

    end->next = NULL;
    head->next = merge_sort(start, end);

    // Rebuild all prev links
    struct list_head *node = head;
    for (; node->next; node = node->next) {
        node->next->prev = node;
    }
    node->next = head;
    head->prev = node;
}
