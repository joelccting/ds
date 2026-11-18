/**
 * Assignment 4
 * Problem 1: Huffman Encoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define LEN (100005)
#define OCC_LEN (96)

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

typedef struct Data
{
    int freq;
    char min;
} Data;

typedef struct PriorityQueue
{
    int size;
    int capacity;
    Data *data;
} PriorityQueue;

/**
 * @brief Create a priority queue
 * @param[in] capacity The size of the priority queue
 * @return A pointer of the object if success; null if failure
 */
PriorityQueue *pq_create(int capacity)
{
    PriorityQueue *retp = NULL;
    retp = (PriorityQueue *)calloc(1, sizeof(PriorityQueue));
    assert(retp);
    retp->capacity = capacity;
    retp->data = (Data *)calloc(capacity, sizeof(Data));
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

    free(pq->data);
    free(pq);
}

/**
 * @brief private function. Swap two events
 * @param[in] a The pointer to a event
 * @param[in] b The pointer to the other event
 * @return None
 */
static void swap(Data *a, Data *b)
{
    if (!a || !b)
    {
        err(("Null pointer\n"));
        return;
    }

    Data t;
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
int is_higher_priority(Data *a, Data *b)
{
    if (!a || !b)
    {
        err(("Null pointer\n"));
        return 0;
    }

    if (a->freq != b->freq)
    {
        return (a->freq < b->freq);
    }

    return (a->min < b->min);

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
    while (index > 0 && is_higher_priority(pq->data + index, pq->data + parent))
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

    if (left < pq->size && is_higher_priority(pq->data + left, pq->data + smallest))
    {
        smallest = left;
    }
    if (right < pq->size && is_higher_priority(pq->data + right, pq->data + smallest))
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
int pq_push(PriorityQueue *pq, Data *d)
{
    if (!pq)
    {
        err(("Null pointer\n"));
        return -1;
    }

    if (pq->size >= pq->capacity)
    {
        pq->capacity *= 2;
        Data *tmp = (Data *)realloc(pq->data, sizeof(Data) * pq->capacity);
        assert(tmp);
        pq->data = tmp;
    }

    if (d)
    {
        pq->data[pq->size] = *d;
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
int pq_top(PriorityQueue *pq, Data *d)
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
        *d = pq->data[0];

    return 0;
}

/**
 * @brief Pop an event out of a priority queue
 * @param[in] pq A pointer to the object
 * @param[in] d A pointer to an event
 * @return 0 if success; otherwise if failure
 */
int pq_pop(PriorityQueue *pq, Data *d)
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
    {
        *d = pq->data[0];
        pq->data[0] = pq->data[pq->size - 1];
        pq->size--;
        heapify_down(pq, 0);
    }

    return 0;
}

int main()
{
    char instr[LEN];
    int occ_cnt[OCC_LEN];

    fgets(instr, LEN - 1, stdin);
    int len = strlen(instr);
    instr[len - 1] = 0;
    dbg(("%s\n", instr));

    memset(occ_cnt, 0, sizeof(int) * OCC_LEN);
    for (int i = 0; instr[i] != 0; i++)
    {
        ++occ_cnt[instr[i] - 32];
    }

    for (int i = 0; i < OCC_LEN; i++)
    {
        if (occ_cnt[i])
            dbg(("ascii %d: %d\n", i, occ_cnt[i]));
    }

    return 0;
}