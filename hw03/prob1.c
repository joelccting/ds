/**
 * Assignment 3
 * Problem 1: Multi-stage linked list reorganization
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SZ (128)

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

typedef struct _node
{
    int val;
    struct _node *next;
} Node;

/**
 * Singly-linked list
 */
typedef struct _sllst
{
    Node dummy_head;
    int size;
} Sllst;

Sllst *sllst_create(void)
{
    Sllst *retp = (Sllst *)calloc(1, sizeof(Sllst));
    assert(retp);
    return retp;
}

void sllst_destroy(Sllst *obj)
{
    if (!obj)
        return;
    Node *curr = obj->dummy_head.next, *next;

    while (curr)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(obj);
}

void sllst_insert(Sllst *obj, int val, int pos)
{
    if (!obj)
    {
        err(("Null pointer!\n"));
        return;
    }
    Node *prev = &obj->dummy_head,
         *curr = prev->next;
    int cnt = 0;
    while (cnt != pos)
    {
        prev = curr;
        curr = curr->next;
        ++cnt;
    }

    Node *p = (Node *)calloc(1, sizeof(Node));
    p->val = val;
    p->next = curr;
    prev->next = p;
    ++obj->size;
}

void sllst_delete(Sllst *obj, int pos)
{
    Node *prev = &obj->dummy_head,
         *curr = prev->next;
    int cnt = 0;
    while (cnt != pos)
    {
        prev = curr;
        curr = curr->next;
        ++cnt;
    }

    prev->next = curr->next;
    free(curr);
    --obj->size;
}

enum
{
    LEFT_1_PREV = 0,
    LEFT_1_CURR,
    RIGHT_1_PREV,
    RIGHT_1_CURR,
    LEFT_2_PREV,
    LEFT_2_CURR,
    RIGHT_2_PREV,
    RIGHT_2_CURR,
    TOT_POINTERS
};

void sllst_swap(Sllst *obj, int l1, int r1, int l2, int r2)
{
    Node *prev = &obj->dummy_head,
         *curr = prev->next;
    int cnt = 0;
    Node *p[TOT_POINTERS];
    while (curr)
    {
        if (cnt == l1)
        {
            p[LEFT_1_PREV] = prev;
            p[LEFT_1_CURR] = curr;
        }
        else if (l1 != r1 && cnt == r1)
        {
            p[RIGHT_1_PREV] = prev;
            p[RIGHT_1_CURR] = curr;
        }
        else if (cnt == l2)
        {
            p[LEFT_2_PREV] = prev;
            p[LEFT_2_CURR] = curr;
        }
        else if (l2 != r2 && cnt == r2)
        {
            p[RIGHT_2_PREV] = prev;
            p[RIGHT_2_CURR] = curr;
        }

        prev = curr;
        curr = curr->next;
        ++cnt;
    }

    Node *tmp;
    if (l1 != r1 && l2 != r2)
    {
        p[LEFT_1_PREV]->next = p[LEFT_2_CURR];
        p[LEFT_2_PREV]->next = p[LEFT_1_CURR];
        tmp = p[RIGHT_1_CURR]->next;
        p[RIGHT_1_CURR]->next = p[RIGHT_2_CURR]->next;
        p[RIGHT_2_CURR]->next = tmp;
    }
    else if (l1 == r1 && l2 != r2)
    {
        p[LEFT_1_PREV]->next = p[LEFT_2_CURR];
        p[LEFT_2_PREV]->next = p[LEFT_1_CURR];
        tmp = p[LEFT_1_CURR]->next;
        p[LEFT_1_CURR]->next = p[RIGHT_2_CURR]->next;
        p[RIGHT_2_CURR]->next = tmp;
    }
    else if (l1 != r1 && l2 == r2)
    {
        p[LEFT_1_PREV]->next = p[LEFT_2_CURR];
        p[LEFT_2_PREV]->next = p[LEFT_1_CURR];
        tmp = p[RIGHT_1_CURR]->next;
        p[RIGHT_1_CURR]->next = p[LEFT_2_CURR]->next;
        p[LEFT_2_CURR]->next = tmp;
    }
    else if (l1 == r1 && l2 == r2)
    {
        p[LEFT_1_PREV]->next = p[LEFT_2_CURR];
        p[LEFT_2_PREV]->next = p[LEFT_1_CURR];
        tmp = p[LEFT_1_CURR]->next;
        p[LEFT_1_CURR]->next = p[LEFT_2_CURR]->next;
        p[LEFT_2_CURR]->next = tmp;
    }
}

Sllst *sllst_fold(Sllst *obj, int n_groups, int *group)
{
    dbg(("+%s\n", __func__));
    Sllst *retp = sllst_create();

#if 0 // defined(DEBUG)
    for (int i = 0; i < n_groups; ++i)
        printf("%d ", group[i]);
    printf("\n");
#endif

    Node **arr = (Node **)calloc(obj->size, sizeof(Node *));
    Node *h = obj->dummy_head.next;
    int i = 0;
    while (h)
    {
        arr[i++] = h;
        h = h->next;
    }

    Node *tail = &retp->dummy_head;
    int left, right;
    for (int i = 0; i < n_groups; ++i)
    {
        left = group[i],
        right = group[i + 1] - 1;
        dbg(("(%d,%d)\n", left, right));

        if (left > obj->size)
            break;

        if (left <= obj->size - 1 && right >= obj->size)
            right = obj->size - 1;

        while (left <= right)
        {
            tail->next = arr[left];
            tail = arr[left];
            tail->next = NULL;
            ++left;
            if (left > right)
                break;
            tail->next = arr[right];
            tail = arr[right];
            tail->next = NULL;
            --right;
        }
    }

    right = group[n_groups];
    dbg(("right=%d size=%d\n", right, obj->size));
    // dbg(("arr[%d] val=%d\n", right, arr[right]->val));
    while (right < obj->size)
    {
        tail->next = arr[right];
        tail = arr[right];
        tail->next = NULL;
        ++right;
    }

    // tail->next = arr[right];

    free(arr);
    obj->dummy_head.next = NULL;
    dbg(("-%s\n", __func__));
    return retp;
}

void sllst_print(Sllst *obj)
{
    dbg(("%s\n", __func__));
    Node *p = obj->dummy_head.next;
    if (!p)
        dbg(("Null pointer\n"));

    while (p)
    {
        printf("%d", p->val);
        p = p->next;
        if (p)
            printf(" ");
    }
    printf("\n");
}

enum
{
    INSERT,
    DELETE,
    SWAP,
    TOT_MODOP
};

typedef struct _mod_ops
{
    int type;
    int operand[4];
} Modops;

void parse_cmd(char *str, Modops *m)
{
    if (!str || !m)
    {
        err(("Null pointers\n"));
    }

    char *p = strtok(str, " ");
    if (*p == 'I')
        m->type = INSERT;
    else if (*p == 'D')
        m->type = DELETE;
    else if (*p == 'S')
        m->type = SWAP;
    else
        err(("Wrong input\n"));

    dbg(("m->type=%d\n", m->type));

    p = strtok(NULL, " \n");
    int i = 0;
    while (p)
    {
        m->operand[i] = atoi(p);
        dbg(("m->operand[%d]=%d\n", i, m->operand[i]));
        p = strtok(NULL, " ");
        ++i;
    }
}

int main()
{
    int n, // number of nodes in the init linked list
        m, // number of modification operations
        k, // number of group sizes for folding
        val;
    char str[SZ];
    Modops mop;

    scanf("%d", &n);
    Sllst *obj = sllst_create();
    for (int i = 0; i < n; ++i)
    {
        scanf("%d ", &val);
        sllst_insert(obj, val, i);
    }

    // sllst_swap(obj, 0, 1, 2, 4);
    scanf("%d ", &m);
    for (int i = 0; i < m; ++i)
    {
        fgets(str, SZ - 1, stdin);
        int len = strlen(str);
        if (str[len - 1] == '\n')
            str[len - 1] = 0;
        parse_cmd(str, &mop);
        switch (mop.type)
        {
        case INSERT:
            sllst_insert(obj, mop.operand[1], mop.operand[0]);
            break;
        case DELETE:
            sllst_delete(obj, mop.operand[0]);
            break;
        case SWAP:
            sllst_swap(obj, mop.operand[0], mop.operand[1], mop.operand[2], mop.operand[3]);
            break;
        default:
            err(("Should not come here\n"));
            break;
        }
    }

    scanf("%d ", &k);
    int *grp = (int *)calloc(k + 1, sizeof(int));
    assert(grp);
    for (int i = 1; i <= k; ++i)
    {
        scanf("%d", grp + i);
        grp[i] += grp[i - 1];
    }
#if defined(DEBUG)
    sllst_print(obj);
#endif
    Sllst *obj2 = sllst_fold(obj, k, grp);
    sllst_print(obj2);
    free(grp);
    sllst_destroy(obj2);
    sllst_destroy(obj);
    return 0;
}