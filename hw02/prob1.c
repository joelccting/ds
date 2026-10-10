#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

/**
 * +---------> y
 * |
 * |
 * |
 * v
 * x
 */

typedef enum
{
    UP,
    LEFT,
    DOWN,
    RIGHT,
    DIR_END
} Dir;

typedef struct
{
    int dx;
    int dy;
} Delta;

const Delta move[] = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

typedef struct
{
    int x;
    int y;
} Pos;

typedef struct node Node;
struct node
{
    int x;
    int y;
    Dir dir;
    Node *next;
};

typedef struct
{
    Pos initPos;
    Pos currPos;
    int health;
    Node *stack;
    int top;
    int stkMax;
    int **mark;
} Robot;

int isFull(Robot *r)
{
    if (r->top >= r->stkMax)
        return 1;
    return 0;
}

int isEmpty(Robot *r)
{
    if (r->top < 0)
        return 1;
    return 0;
}

int push(Robot *r, int x, int y, Dir d)
{
    // Stack full
    if (isFull(r))
        return 1;

    r->top++;
    r->stack[r->top].x = x;
    r->stack[r->top].y = y;
    r->stack[r->top].dir = d;

    return 0;
}

int pop(Robot *r, Node *n)
{
    // Stack empty
    if (isEmpty(r))
        return 1;

    if (n)
    {
        n->x = r->stack[r->top].x;
        n->y = r->stack[r->top].y;
        n->dir = r->stack[r->top].dir;
    }

    --r->top;

    return 0;
}

typedef struct
{
    int nRows;
    int nCols;
    int nRobots;
    Robot *robots;
    Pos evPnt;
} fiExp;

/**
 * @brief init fiExp object
 * @param[out]
 * @param[in] H number of rows
 * @param[in] W number of columns
 * @param[in] M number of robots
 * @return None
 */
fiExp *initFiExp(int H, int W, int M)
{
    fiExp *obj = (fiExp *)calloc(1, sizeof(fiExp));
    assert(obj);

    obj->nRows = H;
    obj->nCols = W;
    obj->nRobots = M;

    obj->robots = (Robot *)calloc(M + 1, sizeof(Robot));
    assert(obj->robots);

    for (int i = 1; i <= obj->nRobots; ++i)
    {
        obj->robots[i].top = -1;
        obj->robots[i].stkMax = H * W + 4;
        obj->robots[i].stack = (Node *)calloc(H * W + 5, sizeof(Node));
        assert(obj->robots[i].stack);

        obj->robots[i].mark = (int **)calloc(obj->nRows + 1, sizeof(int *));
        assert(obj->robots[i].mark);
        for (int j = 1; j <= obj->nRows; ++j)
            obj->robots[i].mark[j] = (int *)calloc(obj->nCols + 1, sizeof(int));
    }

    return obj;
}

void deleteFiExp(fiExp *obj)
{
    assert(obj);
    for (int i = 1; i <= obj->nRobots; ++i)
    {
        if (obj->robots[i].stack)
            free(obj->robots[i].stack);

        if (obj->robots[i].mark)
            free(obj->robots[i].mark);
    }

    if (obj->robots)
        free(obj->robots);

    free(obj);
}

void printFiExp(fiExp *obj)
{
    printf("%s:\n", __func__);

    for (int i = 1; i <= obj->nRobots; ++i)
    {
        printf("init position (%d,%d), health=%d\n", obj->robots[i].initPos.x, obj->robots[i].initPos.y, obj->robots[i].health);
    }
}

void path(fiExp *obj, char **maze, int i)
{
    bool found = false;
    Pos curr = {obj->robots[i].currPos.x, obj->robots[i].currPos.y}, next;
    obj->robots[i].mark[curr.x][curr.y] = 1;
    push(&obj->robots[i], curr.x, curr.y, UP);

    while (!isEmpty(&obj->robots[i]) && !found)
    {
        Node n;
        pop(&obj->robots[i], &n);
        while (n.dir < DIR_END && !found)
        {
            next.x = n.x + move[n.dir].dx;
            next.y = n.y + move[n.dir].dy;

            if (next.x == obj->evPnt.x && next.y == obj->evPnt.y)
            {
                found = true;
                push(&obj->robots[i], n.x, n.y, n.dir);
                push(&obj->robots[i], next.x, next.y, DIR_END);
            }
            else if (maze[next.x][next.y] == '.' && obj->robots[i].mark[next.x][next.y] == 0)
            {
                obj->robots[i].mark[next.x][next.y] = 1;
                push(&obj->robots[i], n.x, n.y, ++n.dir);
                n.x = next.x;
                n.y = next.y;
                n.dir = UP;
            }
            else // change direction
            {
                ++n.dir;
            }
        }
    }

    if (found)
    {
        printf("The path is:\n");
        for (int k = 0; k <= obj->robots[i].top; ++k)
        {
            printf("(%d,%d)", obj->robots[i].stack[k].x, obj->robots[i].stack[k].y);
        }
    }
    else
    {
        printf("No path\n");
    }
}

// void sim(fiExp *obj, char **maze)
// {
//     // for (int i = 1; i <= obj->nRobots; ++i)
//     int i = 1;
//     {
//     }
// }

int main()
{
    int H, // number of rows
        W, // number of columns
        M; // number of robots

    if (3 != scanf("%d %d %d", &H, &W, &M))
    {
        fprintf(stderr, "Wrong inputs");
        return 0;
    }

    fiExp *obj = initFiExp(H, W, M);

    char **maze = (char **)calloc(H + 1, sizeof(char *));
    assert(maze);
    for (int i = 1; i <= H; ++i)
    {
        maze[i] = (char *)calloc(W + 1, sizeof(char));
        assert(maze[i]);
    }

    for (int i = 1; i <= H; ++i)
        scanf(" %s", maze[i] + 1);

    for (int i = 1; i <= H; ++i)
        for (int j = 1; j <= W; ++j)
            if (maze[i][j] == 'E')
            {
                obj->evPnt.x = i;
                obj->evPnt.y = j;
            }

    for (int i = 1; i <= M; ++i)
    {
        int x, y, h;
        scanf("%d %d %d", &x, &y, &h);
        obj->robots[i].initPos.x = obj->robots[i].currPos.x = x;
        obj->robots[i].initPos.y = obj->robots[i].currPos.y = y;
        obj->robots[i].health = h;
    }

    printFiExp(obj);
    // sim(obj, maze);
    path(obj, maze, 1);
    deleteFiExp(obj);
    return 0;
}