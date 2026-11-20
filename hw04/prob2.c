/**
 * Assignment 4
 * Problem 2: Huffman Decoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
 * @param[in] idx Current position of the element
 * @return None
 */
static void heapify_up(PriorityQueue *pq, int idx)
{
    if (!pq)
    {
        err(("Null pointer\n"));
        return;
    }

    int parent = (idx - 1) / 2;
    while (idx > 0 && is_higher_priority(pq->data[idx], pq->data[parent]))
    {
        swap(&pq->data[idx], &pq->data[parent]);
        idx = parent;
        parent = (idx - 1) / 2;
    }
}

/**
 * @brief private function. Moving an item down.
 * @param[in] pq A pointer to the object
 * @param[in] idx Current position of the element
 * @return None
 */
void heapify_down(PriorityQueue *pq, int idx)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return;
    }

    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < pq->size && is_higher_priority(pq->data[left], pq->data[smallest]))
    {
        smallest = left;
    }
    if (right < pq->size && is_higher_priority(pq->data[right], pq->data[smallest]))
    {
        smallest = right;
    }

    if (smallest != idx)
    {
        swap(&pq->data[idx], &pq->data[smallest]);
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

Node *tree_build(PriorityQueue *pq)
{
    Node *retp = NULL;

    assert(pq);

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

    retp = pq_pop(pq);

    return retp;
}

void tree_destroy(Node *root)
{
    if (!root)
        return;

    if (root->left)
        tree_destroy(root->left);

    if (root->right)
        tree_destroy(root->right);

    free(root);
}

/**
 * @brief Parse the input string
 */
int parse(char *str, int *freq)
{
    // dbg(("%s\n", str));
    int retval = 0;
    char *pch;
    if (!str)
    {
        err(("Null pointer\n"));
        return retval;
    }

    pch = strtok(str, " ");
    // dbg(("%s\n", pch));
    if (!strcmp(str, "<space>"))
        retval = 32;
    else if (33 <= pch[0] && pch[0] <= 126)
        retval = pch[0];

    pch = strtok(NULL, " ");
    if (freq)
        *freq = atoi(pch);

    return retval;
}

void decode(Node *node, char *str, int *idx)
{

    if (!node->left && !node->right)
    {
        dbg(("idx=%d\n", *idx));
        printf("%c", node->ch);
        return;
    }

    if (str[*idx] == '0')
    {
        *idx += 1;
        if (node->left)
        {
            decode(node->left, str, idx);
        }
        else
        {
            printf("%c", node->ch);
            return;
        }
    }
    else if (str[*idx] == '1' && node->right)
    {

        *idx += 1;
        decode(node->right, str, idx);
    }
}

int main()
{
    int N, // number of distinct characters
        len;
    char buf[LEN];
    scanf("%d ", &N);
    PriorityQueue *pq = pq_create(OCC_LEN);
    for (int i = 0; i < N; ++i)
    {
        fgets(buf, LEN, stdin);
        len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        {
            buf[len - 1] = 0;
            --len;
        }

        int freq;
        int c = parse(buf, &freq);
        dbg(("%c: %d\n", (char)c, freq));
        Node *p = (Node *)calloc(1, sizeof(Node));
        assert(p);
        p->ch = c;
        p->occ_cnt = freq;
        pq_push(pq, p);
    }

    fgets(buf, LEN, stdin);
    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    {
        buf[len - 1] = 0;
        --len;
    }

    Node *root = NULL;
    if (len)
    {

        root = tree_build(pq);
        int idx = 0;
        if (N == 1)
        {
            if (!root->left && !root->right)
                while (idx < len)
                {
                    printf("%c", root->ch);
                    ++idx;
                }
        }
        else
        {
            while (idx < len)
            {
                decode(root, buf, &idx);
                dbg(("idx=%d\n", idx));
            }
        }
    }
    printf("\n");

    if (root)
        tree_destroy(root);
    pq_destroy(pq);
    return 0;
}