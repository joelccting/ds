#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef DEBUG
#define dbgprint(x) (printf("[DEBUG %d] ", __LINE__), printf x)
#else
#define dbgprint(x) ((void)0)
#endif

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
    DIR_MAX
} Dir;

typedef enum
{
    ACTIVE,
    WIN,
    DEAD,
    NOPATH,
    STATUS_MAX
} Status;

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
};

typedef struct
{
    Status status;
    Pos initPos;
    Pos currPos;
    Dir currDir;
    int health;
    int wins;
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

/**
 * @brief Check collision
 * @param[in] obj Pointer to data
 * @param[in] maze Pointer to the maze
 * @param[in] num The ith robot from 1 to obj->nRobots
 * @return 0 still active; -1 dead
 */
int collideCheck(fiExp *obj, char **maze, int num)
{
    int ret = 0;
    Robot *currRbt = &obj->robots[num];
    for (int i = 1; i <= obj->nRobots; ++i)
    {
        if (i == num)
            continue;

        Robot *othRbt = &obj->robots[i];
        if (othRbt->status > WIN)
            continue;

        if (currRbt->currPos.x == othRbt->currPos.x && currRbt->currPos.y == othRbt->currPos.y)
        {
            if (currRbt->health > othRbt->health)
            {
                othRbt->status = DEAD;
                currRbt->wins++;
                currRbt->health -= currRbt->wins;
                if (currRbt->health <= 0)
                {
                    currRbt->status = DEAD;
                    ret = -1;
                }
            }
            else if (currRbt->health < othRbt->health)
            {
                currRbt->status = DEAD;
                ret = -1;
                othRbt->wins++;
                othRbt->health -= othRbt->wins;
                if (othRbt->health <= 0)
                    othRbt->status = DEAD;
            }
            else
            {
                currRbt->status = othRbt->status = DEAD;
                ret = -1;
            }

            if (currRbt->status > WIN)
                break;
        }
    }

    return ret;
}

/**
 * @brief move robot i one step
 * @param[in] obj pointer to data
 * @param[in] maze pointer to the map
 * @param[in] i ith robot
 */
void step(fiExp *obj, char **maze, int i)
{
    bool isEvPnt = false, isSuccMove = false, isDead = false;
    Robot *robot = &obj->robots[i];

    if (robot->status > ACTIVE)
        return;

    Pos *currPos = &robot->currPos, next;
    Dir *currDir = &robot->currDir;
    // obj->robots[i].mark[currPos.x][currPos.y] = 1;
    // push(&obj->robots[i], currPos.x, currPos.y, currDir);

    // while (!isEmpty(&obj->robots[i]) && !found)
    // {

    while (*currDir < DIR_MAX && !isEvPnt && !isSuccMove)
    {
        next.x = currPos->x + move[*currDir].dx;
        next.y = currPos->y + move[*currDir].dy;

        if (next.x == obj->evPnt.x && next.y == obj->evPnt.y)
        { /* reach evaculation point */
            isEvPnt = true;
            // push(&obj->robots[i], n.x, n.y, n.dir);
            push(robot, next.x, next.y, DIR_MAX);
            robot->status = WIN;
        }
        else if (maze[next.x][next.y] == '.' && robot->mark[next.x][next.y] == 0)
        { /* move to a new position */
            isSuccMove = true;
            robot->mark[next.x][next.y] = 1;
            push(robot, next.x, next.y, *currDir);
            currPos->x = next.x;
            currPos->y = next.y;
            *currDir = UP;
            if (collideCheck(obj, maze, i))
            {
                isDead = true;
                break;
            }
        }
        else // change direction
        {
            ++(*currDir);
        }
    }

    // if (isEvPnt)
    // {
    //     printf("The path is:\n");
    //     for (int k = 0; k <= robot->top; ++k)
    //     {
    //         printf("(%d,%d)", robot->stack[k].x, robot->stack[k].y);
    //     }
    // }
    // else
    if (*currDir >= DIR_MAX)
    { /* backtracking  */
        Node n;
        pop(robot, &n);

        if (n.x == robot->initPos.x && n.y == robot->initPos.y)
        { /* NO PATH */
            robot->status = NOPATH;
            return;
        }

        currPos->x = n.x;
        currPos->y = n.y;
        *currDir = n.dir;
    }
    else if (isDead)
    {
        return;
    }
}

/**
 * @return Number of active robots
 */

int isRbtActive(fiExp *obj)
{
    int ret = 0;

    for (int i = 1; i <= obj->nRobots; ++i)
    {
        Robot *r = &obj->robots[i];
        if (r->status == ACTIVE)
            ++ret;
    }

    return ret;
}

void sim(fiExp *obj, char **maze)
{
    while (isRbtActive(obj))
        for (int i = 1; i <= obj->nRobots; ++i)
        {
            step(obj, maze, i);
        }
}

void pntOutput(fiExp *obj)
{
    const char *str[] = {"ACTIVE", "WIN", "DIE", "NO PATH"};
    for (int i = 1; i <= obj->nRobots; ++i)
    {
        Robot *r = &obj->robots[i];
        printf("Robot %d: %s\n", i, str[r->status]);

        if (r->status != NOPATH)
        {
            printf("Path: ");
            for (int j = 0; j <= r->top; ++j)
            {
                printf("(%d,%d)%s", r->stack[j].x, r->stack[j].y, (j != r->top) ? " -> " : "");
            }
            printf("\n");
        }
    }
}

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
        Robot *r = &obj->robots[i];
        r->initPos.x = r->currPos.x = x;
        r->initPos.y = r->currPos.y = y;
        r->health = h;
        r->mark[r->currPos.x][r->currPos.y] = 1;
        push(r, r->currPos.x, r->currPos.y, UP);
    }

    // printFiExp(obj);
    sim(obj, maze);
    pntOutput(obj);

    for (int i = 1; i <= H; ++i)
        free(maze[i]);
    free(maze);
    deleteFiExp(obj);
    
    return 0;
}