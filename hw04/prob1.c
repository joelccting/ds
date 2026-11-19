/**
 * Assignment 4
 * Problem 1: Huffman Encoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define LEN (100005)
#define OCC_LEN (95)
#define POOL_SZ (95 * 2)

#if defined(DEBUG)
#define dbg(x)                      \
    do                              \
    {                               \
        printf("D[%d] ", __LINE__); \
        printf x;                   \
    } while (0)
#else
#define dbg(x)
#endif

#define err(x)                      \
    do                              \
    {                               \
        printf("E[%d] ", __LINE__); \
        printf x;                   \
    } while (0)

typedef struct node Node;
struct node
{
    int occ_cnt;
    char ch;
    struct node *left;
    struct node *right;
};

typedef struct priority_queue PriorityQueue;
struct priority_queue
{
    int size;
    int capacity;
    Node **data;
};

/**
 * @brief Create a priority queue
 * @param[in] capacity The size of the priority queue
 * @return A pointer of the object if success; null if failure
 */
PriorityQueue *pq_create(int capacity)
{
    assert(capacity >= 0);
    PriorityQueue *retp = NULL;
    retp = (PriorityQueue *)calloc(1, sizeof(PriorityQueue));
    assert(retp);
    retp->capacity = capacity;
    retp->data = (Node **)calloc(capacity, sizeof(Node *));
    assert(retp->data);
    return retp;
}

/**
 * @brief Release the allocated memory of a priority queue
 * @param[in] pq A pointer to the priority queue
 * @return None
 */
void pq_destroy(PriorityQueue *pq)
{
    if (!pq)
    {
        err(("Null pointer\n"));
        return;
    }

    for (int i = pq->size - 1; i > -1; --i)
        free(pq->data[i]);

    free(pq->data);
    free(pq);
}

/**
 * @brief private function. Swap two events
 * @param[in] a The pointer to a event
 * @param[in] b The pointer to the other event
 * @return None
 */
static void swap(Node **a, Node **b)
{
    if (!a || !b)
    {
        err(("Null pointer\n"));
        return;
    }

    Node *t;
    t = *a;
    *a = *b;
    *b = t;
}

/**
 * @brief private function
 * @param[in] a An item
 * @param[in] b The other item
 * @return 1 if a < b; 0 if a == b; -1 if otherwise
 */
int is_higher_priority(Node *a, Node *b)
{
    if (!a || !b)
    {
        err(("Null pointer\n"));
        return 0;
    }

    if (a->occ_cnt != b->occ_cnt)
    {
        return (a->occ_cnt < b->occ_cnt);
    }

    return (a->ch < b->ch);

    return 0;
}

/**
 * @brief Private function. Moving an item up.
 * @param[in] pq A pointer to the object
 * @param[in] index Current position of the element
 * @return None
 */
static void heapify_up(PriorityQueue *pq, int index)
{
    if (!pq)
    {
        err(("Null pointer\n"));
        return;
    }

    int parent = (index - 1) / 2;
    while (index > 0 && is_higher_priority(pq->data[index], pq->data[parent]))
    {
        swap(&pq->data[index], &pq->data[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

/**
 * @brief private function. Moving an item down.
 * @param[in] pq A pointer to the object
 * @param[in] index Current position of the element
 * @return None
 */
void heapify_down(PriorityQueue *pq, int index)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return;
    }

    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < pq->size && is_higher_priority(pq->data[left], pq->data[smallest]))
    {
        smallest = left;
    }
    if (right < pq->size && is_higher_priority(pq->data[right], pq->data[smallest]))
    {
        smallest = right;
    }

    if (smallest != index)
    {
        swap(&pq->data[index], &pq->data[smallest]);
        heapify_down(pq, smallest);
    }
}

/**
 * @brief Push an item into a priority queue
 * @param[in] pq A pointer to the object
 * @param[in] e A pointer to an item
 * @return 0 if success; otherwise if failure
 */
int pq_push(PriorityQueue *pq, Node *d)
{
    if (!pq)
    {
        err(("Null pointer\n"));
        return -1;
    }

    if (pq->size >= pq->capacity)
    {
        pq->capacity *= 2;
        Node **tmp = (Node **)realloc(pq->data, sizeof(Node *) * pq->capacity);
        assert(tmp);
        pq->data = tmp;
    }

    if (d)
    {
        pq->data[pq->size] = d;
        pq->size++;
        heapify_up(pq, pq->size - 1);
    }

    return 0;
}

/**
 * @brief Peep the top event
 * @param[in] pq A pointer to the object
 * @param[out] d A pointer to an event
 * @return 0 if success; otherwise if failure
 */
int pq_top(PriorityQueue *pq, Node *d)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (pq->size == 0)
    {
        dbg(("Pque empty\n"));
        return 1;
    }

    if (d)
        *d = *pq->data[0];

    return 0;
}

/**
 * @brief Pop an event out of a priority queue
 * @param[in] pq A pointer to the object
 * @return NULL if failure; otherwise if success;
 */
Node *pq_pop(PriorityQueue *pq)
{
    Node *retp = NULL;

    if (!pq)
    {
        err(("Found null pointer\n"));
        return NULL;
    }

    if (pq->size == 0)
    {
        dbg(("Pque is empty\n"));
        return NULL;
    }

    retp = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    heapify_down(pq, 0);

    return retp;
}

void build_codes(Node *curr, char path[], int depth, char codes[OCC_LEN][100])
{
    dbg(("depth=%d\n", depth));

    if (!curr->left && !curr->right)
    {
        if (depth == 0)
        {
            strcpy(codes[curr->ch - 32], "0");
        }
        else
        {
            path[depth] = 0;
            strcpy(codes[curr->ch - 32], path);
        }

        return;
    }

    if (curr->left)
    {
        dbg(("\n"));
        path[depth] = '0';
        build_codes(curr->left, path, depth + 1, codes);
    }

    if (curr->right)
    {
        dbg(("\n"));
        path[depth] = '1';
        build_codes(curr->right, path, depth + 1, codes);
    }
}

/**
 * @brief Post-order traversal to release allocated memory
 */
void tree_destroy(Node *root)
{
}

int main()
{
    char instr[LEN];
    int occ_cnt[OCC_LEN];
    char codes[OCC_LEN][100] = {{0}};
    char path[POOL_SZ] = {0};

    fgets(instr, LEN - 1, stdin);
    int len = strlen(instr);
    if (len > 0 && instr[len - 1] == '\n')
        instr[len - 1] = 0;
    dbg(("%s\n", instr));

    /* count occurences of characters */
    memset(occ_cnt, 0, sizeof(int) * OCC_LEN);
    for (int i = 0; instr[i] != 0; i++)
    {
        ++occ_cnt[instr[i] - 32];
    }

#if 0 // defined(DEBUG)
    for (int i = 0; i < OCC_LEN; i++)
    {
        if (occ_cnt[i])
            dbg(("ascii %d: %d\n", i, occ_cnt[i]));
    }
#endif

    PriorityQueue *pq = pq_create(OCC_LEN);
    for (int i = 0; i < OCC_LEN; ++i)
    {

        if (!occ_cnt[i])
            continue;
        Node *p = (Node *)calloc(1, sizeof(Node));
        assert(p);
        p->ch = i + 32;
        p->occ_cnt = occ_cnt[i];
        pq_push(pq, p);
    }

#if 0 // defined(DEBUG)
    Node d;
    while (!pq_pop(pq, &d))
    {
        dbg(("chval=%d: freq=%d\n", d.ch, d.occ_cnt));
    }
#endif

    /* Build the tree */
    while (pq->size > 1)
    {
        Node *left = pq_pop(pq);
        assert(left);
        Node *right = pq_pop(pq);
        assert(right);
        Node *parent = (Node *)calloc(1, sizeof(Node));
        parent->left = left;
        parent->right = right;
        parent->occ_cnt = left->occ_cnt + right->occ_cnt;
        parent->ch = ((left->ch < right->ch) ? left->ch : right->ch);
        pq_push(pq, parent);
    }

    Node *root = pq_pop(pq);
    build_codes(root, path, 0, codes);
    for (int i = 0; i < OCC_LEN; ++i)
    {
        if (strlen(codes[i]))
        {
            if (i == 0)
                printf("<space> %s\n", codes[i]);
            else
                printf("%c %s\n", (char)(i + 32), codes[i]);
        }
    }

    pq_destroy(pq);

    return 0;
}