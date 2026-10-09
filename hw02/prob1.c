#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef enum
{
    UP,
    LEFT,
    DOWN,
    RIGHT,
    END
} Dir;

typedef struct
{
    int x;
    int y;
} Pos;

typedef struct
{
    Pos pos;
    int dir;
} Node;

typedef struct
{
    Pos initPos;
    Pos currPos;
    int health;
    Node *stack;
} Robot;

typedef struct
{
    int nRows;
    int nCols;
    int nRobots;
    char **map;
    Robot *rbts;
} fiExp;

/**
 * @brief init fiExp object
 * @param[out]
 * @param[in]
 * @return None
 */
fiExp *initFiExp(int H, int W, int M)
{
    fiExp *obj = (fiExp *)malloc(sizeof(fiExp));
    assert(obj);
    memset(obj, 0, sizeof(fiExp));
    obj->nRows = H;
    obj->nCols = W;
    obj->nRobots = M;
    obj->map = (char **)malloc(sizeof(char *) * (H + 1));
    assert(obj->map);
    memset(obj->map, 0, sizeof(char *) * (H + 1));

    for (int i = 0; i <= H; ++i)
    {
        obj->map[i] = (char *)malloc(sizeof(char) * (W + 2));
        assert(obj->map[i]);
        memset(obj->map[i], 0, sizeof(char) * (W + 2));
    }

    obj->rbts = (Robot *)malloc(sizeof(Robot) * (M + 1));
    assert(obj->rbts);
    memset(obj->rbts, 0, sizeof(Robot) * (M + 1));
    return obj;
}

void deleteFiExp(fiExp *obj)
{
    assert(obj);
    assert(obj->map);
    for (int i = 0; i <= obj->nRows; ++i)
    {
        if (obj->map[i])
            free(obj->map[i]);
    }
    free(obj->map);
    free(obj);
}

void printFiExp(fiExp *obj)
{
    printf("%s:\n", __func__);
    for (int i = 1; i <= obj->nRows; ++i)
        printf("%s\n", obj->map[i]);
    for (int i = 1; i <= obj->nRobots; ++i)
    {
        printf("init position (%d,%d), health=%d\n", obj->rbts[i].initPos.x, obj->rbts[i].initPos.y, obj->rbts[i].health);
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

    for (int i = 1; i <= H; ++i)
        scanf(" %s", obj->map[i]);

    for (int i = 1; i <= M; ++i)
        scanf("%d %d %d", &obj->rbts[i].initPos.x, &obj->rbts[i].initPos.y, &obj->rbts[i].health);

    printFiExp(obj);

    deleteFiExp(obj);
    return 0;
}