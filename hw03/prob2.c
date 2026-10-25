/**
 * Assignment 3
 * Problem 2: Social network post management
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

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

enum
{
    ADD,
    DELETE,
    PIN,
    UNPIN,
    MOVE,
    TOT_NUM_OPS
};

typedef struct _post
{
    int id;
    bool is_pinned;
    struct _post *next;
} Post;

typedef struct _personal_profile_page
{
    Post dummy_head;
    Post *last_pinned;
    Post *tail;
    int size;
} Profile;

Profile *profile_create(void)
{
    Profile *retp = NULL;
    retp = (Profile *)calloc(1, sizeof(Profile));
    assert(retp);
    retp->last_pinned = retp->tail = &retp->dummy_head;
    return retp;
}

void profile_destroy(Profile *obj)
{
    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    Post *prev = &obj->dummy_head,
         *curr = prev->next;
    while (curr)
    {
        prev = curr->next;
        free(curr);
        curr = prev;
    }

    free(obj);
}

void profile_print(Profile *obj)
{
    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    Post *p = obj->dummy_head.next;
    int cnt = 1;
    while (p)
    {
#if defined(DEBUG)
        printf("[%s%d]%d%s", p->is_pinned ? "*" : "", cnt - 1, p->id, (cnt == obj->size) ? "" : " ");
        cnt++;
#else
        printf("%d%s", p->id, (cnt++ == obj->size) ? "" : " ");
#endif
        p = p->next;
    }

    printf("\n");
}

/**
 * @brief Create a new post with id and append it to the end of the profile
 * @param[in] obj Pointer to the obj
 * @param[in] id ID
 * @return None
 */
void profile_add(Profile *obj, int id)
{
    // Post *prev = &obj->dummy_head,
    //      *curr = prev->next;

    // dbg(("+%s\n", __func__));
    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    // while (curr)
    // {
    //     prev = curr;
    //     curr = curr->next;
    // }

    Post *new = (Post *)calloc(1, sizeof(Post));
    assert(new);
    new->id = id;
    // prev->next = new;
    obj->tail->next = new;
    obj->tail = new;
    obj->size++;
    dbg(("tail->id=%d\n", obj->tail->id));
}

/**
 * @brief Remove the post at position x from the profile
 *
 */
void profile_delete(Profile *obj, const int x)
{
    Post *prev = &obj->dummy_head,
         *curr = prev->next;
    int pos = 0;

    // dbg(("+%s\n", __func__));

    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    if (obj->size <= x)
    {
        dbg(("No %d to delete\n", x));
        return;
    }

    while (x != pos)
    {
        prev = curr;
        curr = curr->next;
        ++pos;
    }

    if (curr == obj->last_pinned)
        obj->last_pinned = prev;

    prev->next = curr->next;
    if (curr->next == NULL) /* tail */
        obj->tail = prev;
    free(curr);
    obj->size--;
    dbg(("tail->id=%d\n", obj->tail->id));
}

/**
 * @brief Pin the post at position x
 */
void profile_pin(Profile *obj, const int x)
{
    Post *prev = &obj->dummy_head,
         *curr = prev->next;
    int pos = 0;

    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    if (obj->size <= x)
    {
        dbg(("No %d to pin\n", x));
        return;
    }

    while (x != pos)
    {
        prev = curr;
        curr = curr->next;
        ++pos;
    }

    if (curr->is_pinned == true)
        return;

    dbg(("curr->id=%d,%s\n", curr->id, curr->is_pinned ? "pin" : "unpin"));

    if (curr == obj->tail)
        obj->tail = prev;

    prev->next = curr->next;

    if (&obj->dummy_head == obj->last_pinned) // no pinned posts
    {
        curr->next = obj->dummy_head.next;
        obj->dummy_head.next = curr;
        obj->last_pinned = curr;
    }
    else
    {
        dbg(("pos=%d\n", pos));
        curr->next = obj->last_pinned->next;
        obj->last_pinned->next = curr;
        obj->last_pinned = curr;
    }

    dbg(("last_pinned->id=%d\n", obj->last_pinned->id));
    curr->is_pinned = true;
    dbg(("tail->id=%d\n", obj->tail->id));
}

/**
 * @brief Unpin the post at position x
 */
void profile_unpin(Profile *obj, const int x)
{
    Post *prev = &obj->dummy_head,
         *curr = prev->next;
    int pos = 0;

    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    if (obj->size <= x)
    {
        dbg(("No %d to unpin\n", x));
        return;
    }

    while (x != pos)
    {
        prev = curr;
        curr = curr->next;
        ++pos;
    }

    if (curr->is_pinned == false)
        return;

    if (curr == obj->last_pinned)
        obj->last_pinned = prev;

    prev->next = curr->next;
    curr->next = NULL;
    obj->tail->next = curr;
    obj->tail = curr;
    curr->is_pinned = false;
    dbg(("tail->id=%d\n", obj->tail->id));
}

#define MIN(x, y) (((x) <= (y)) ? (x) : (y))
#define MAX(x, y) (((x) <= (y)) ? (y) : (x))
void _search(Profile *obj, const int x, const int y, Post **left_prev, Post **right_prev)
{
    dbg(("x=%d,y=%d\n", x, y));
    if (!obj)
    {
        err(("Null pointer\n"));
        return;
    }

    if (obj->size <= x || obj->size <= y)
    {
        err(("TODO: illegal x=%d, y=%d\n", x, y));
        return;
    }

    int left = MIN(x, y),
        right = MAX(x, y);

    Post *prev = &obj->dummy_head,
         *curr = prev->next;
    int pos = 0;

    while (left != pos)
    {
        prev = curr;
        curr = curr->next;
        ++pos;
    }

    *left_prev = prev;

    while (right != pos)
    {
        prev = curr;
        curr = curr->next;
        ++pos;
    }

    *right_prev = prev;
}

/**
 * @brief Move the post at position x to be immediately after the post at position y
 */
void profile_move_after(Profile *obj, const int x, const int y)
{
    if (x == y)
        return;

    Post *left_prev, *right_prev;
    _search(obj, x, y, &left_prev, &right_prev);
    Post *left_curr = left_prev->next,
         *right_curr = right_prev->next;

    // dbg(("x_curr=%d, y_curr=%d\n", x_curr->id, y_curr->id));

    if (x < y)
    {
        left_prev->next = left_curr->next;
        left_curr->next = right_curr->next;
        right_curr->next = left_curr;
    }
    else
    {
        right_prev->next = right_curr->next;
        right_curr->next = left_curr->next;
        left_curr->next = right_curr;
    }

    if (obj->last_pinned == right_curr)
        if (x < y)
            obj->last_pinned = left_curr;

    dbg(("last_pinned->id=%d\n", obj->last_pinned->id));

    if (right_curr == obj->tail)
    {
        if (x < y)
            obj->tail = left_curr;
        else
            obj->tail = right_prev;
    }

    dbg(("tail->id=%d\n", obj->tail->id));
}

int parse(char *str, int *x, int *y)
{
    if (!str)
    {
        err(("Null pointer\n"));
        return -1;
    }

    int retval, i = 0;
    char *token = strtok(str, " ");
    while (token)
    {
        if (0 == i)
        {
            switch (token[0])
            {
            case 'A':
                retval = ADD;
                break;
            case 'D':
                retval = DELETE;
                break;
            case 'P':
                retval = PIN;
                break;
            case 'U':
                retval = UNPIN;
                break;
            case 'M':
                retval = MOVE;
                break;
            default:
                err(("SHOULD NOT BE HERE\n"));
                break;
            }
        }

        if (i == 1)
        {
            if (x)
                *x = atoi(token);
        }

        if (i == 3 && retval == MOVE)
        {
            if (y)
                *y = atoi(token);
        }

        ++i;
        token = strtok(NULL, " ");
    }

    // dbg(("retval=%d\n", retval));

    return retval;
}

int main()
{
    int n, // the initial number of posts
        m; // number of operations
    char buf[SZ];
    if (1 != scanf("%d ", &n))
    {
        err(("Wrong input\n"));
        return 0;
    }

    Profile *obj = profile_create();
    int id;
    for (int i = 0; i < n; ++i)
    {
        scanf("%d", &id);
        profile_add(obj, id);
    }

#if defined(DEBUG)
    profile_print(obj);
#endif

    scanf("%d ", &m);
    for (int i = 0; i < m; ++i)
    {
        fgets(buf, SZ - 1, stdin);
        int len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = 0;
        int x, y;
        int cmd = parse(buf, &x, &y);
        switch (cmd)
        {
        case ADD:
            profile_add(obj, x);
            break;
        case DELETE:
            profile_delete(obj, x);
            break;
        case PIN:
            profile_pin(obj, x);
            break;
        case UNPIN:
            profile_unpin(obj, x);
            break;
        case MOVE:
            profile_move_after(obj, x, y);
            break;
        default:
            err(("SHOULD NOT BE HERE\n"));
            break;
        }

#if defined(DEBUG)
        profile_print(obj);
#endif
    }

    profile_print(obj);
    profile_destroy(obj);
    return 0;
}