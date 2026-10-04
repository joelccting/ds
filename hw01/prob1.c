#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct
{
    int nDams;
    int *damHeight;
    int nHoles;
    int *holes;
    int *prefixMax;
    int *suffixMax;
    int waterTotal;
    int *secStart;
    int *secEnd;
} Object;

#define FREE(x)      \
    do               \
    {                \
        if (x)       \
            free(x); \
    } while (0)

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))

void deleteObject(Object *obj)
{
    assert(obj);
    FREE(obj->damHeight);
    FREE(obj->holes);
    FREE(obj->prefixMax);
    FREE(obj->suffixMax);
    FREE(obj->secStart);
    FREE(obj->secEnd);
    FREE(obj);
}

Object *initObject(int n)
{
    Object *p = NULL;
    p = (Object *)malloc(sizeof(Object));
    assert(p);
    memset(p, 0, sizeof(Object));
    p->nDams = n;

    p->damHeight = (int *)malloc((n + 5) * sizeof(int));
    assert(p->damHeight);
    memset(p->damHeight, 0, (n + 5) * sizeof(int));

    p->holes = (int *)malloc((n + 5) * sizeof(int));
    assert(p->holes);
    memset(p->holes, 0, (n + 5) * sizeof(int));

    p->prefixMax = (int *)malloc((n + 5) * sizeof(int));
    assert(p->prefixMax);
    memset(p->prefixMax, 0, (n + 5) * sizeof(int));

    p->suffixMax = (int *)malloc((n + 5) * sizeof(int));
    assert(p->suffixMax);
    memset(p->suffixMax, 0, (n + 5) * sizeof(int));

    p->secStart = (int *)malloc((n + 5) * sizeof(int));
    assert(p->secStart);
    memset(p->secStart, 0, (n + 5) * sizeof(int));

    p->secEnd = (int *)malloc((n + 5) * sizeof(int));
    assert(p->secEnd);
    memset(p->secEnd, 0, (n + 5) * sizeof(int));
    return p;
}

int getSectionWater(Object *obj, int start, int end)
{
    int retval = 0;
    for (int i = start; i <= end; ++i)
    {
        retval += MAX(0, MIN(obj->prefixMax[i], obj->suffixMax[i]) - obj->damHeight[i]);
    }
    return retval;
}

void setWaterTotal(Object *obj)
{
    assert(obj);
    obj->waterTotal = 0;
    for (int i = 1; i <= obj->nDams; ++i)
    {
        if (obj->damHeight[i] == -1)
        {
            continue;
        }

        obj->waterTotal += MAX(0, MIN(obj->prefixMax[i], obj->suffixMax[i]) - obj->damHeight[i]);
    }
}

int getWaterTotal(Object *obj)
{
    assert(obj);
    return obj->waterTotal;
}

void setPreSufMax2(Object *obj, int begin, int end)
{
    assert(obj);
    if (begin > end)
        return;

    // for (int i = 1; i <= obj->nHoles; ++i)
    // {
    //     // 1 or 2 dams save no water
    //     if ((obj->holes[i] - obj->holes[i - 1]) <= 3)
    //     {
    //         continue;
    //     }

    // int begin = obj->holes[i - 1] + 1,
    //     end = obj->holes[i] - 1;

    obj->prefixMax[begin] = obj->damHeight[begin];
    for (int i = begin + 1; i <= end; ++i)
    {
        obj->prefixMax[i] = MAX(obj->damHeight[i], obj->prefixMax[i - 1]);
    }

    obj->suffixMax[end] = obj->damHeight[end];
    for (int i = end - 1; i >= begin; --i)
    {
        obj->suffixMax[i] = MAX(obj->damHeight[i], obj->suffixMax[i + 1]);
    }
    // }
}

void setPreSufMax(Object *obj)
{
    assert(obj);

    for (int i = 0; i <= obj->nHoles - 1; ++i)
    {
        // 1 or 2 dams save no water
        // if ((obj->holes[i] - obj->holes[i - 1]) <= 3)
        // {
        //     continue;
        // }

        int begin = obj->holes[i] + 1,
            end = obj->holes[i + 1] - 1;
        for (int j = begin; j <= end; ++j)
        {
            obj->secStart[j] = begin;
            obj->secEnd[j] = end;
        }
        // printf("[%d] end=%d\n", __LINE__, end);
        setPreSufMax2(obj, begin, end);
    }
}

void getSection(Object *obj, int idx, int *start, int *end)
{
    assert(start);
    assert(end);
    // for (int i = 1; i <= obj->nHoles; ++i)
    // {
    //     if (obj->holes[i] < idx)
    //     {
    //         continue;
    //     }
    //     else
    //     {
    //         *start = obj->holes[i - 1] + 1;
    //         *end = obj->holes[i] - 1;
    //         break;
    //     }
    // }
    *start = obj->secStart[idx];
    *end = obj->secEnd[idx];
}

// void setPreSufMax(Object *obj)
// {
//     assert(obj);

//     for (int i = 1; i <= obj->nHoles; ++i)
//     {
//         // 1 or 2 dams save no water
//         if ((obj->holes[i] - obj->holes[i - 1]) <= 3)
//         {
//             continue;
//         }

//         int begin = obj->holes[i - 1] + 1,
//             end = obj->holes[i] - 1;

//         obj->prefixMax[begin] = obj->damHeight[begin];
//         for (int i = begin + 1; i <= end; ++i)
//         {
//             obj->prefixMax[i] = MAX(obj->damHeight[i], obj->prefixMax[i - 1]);
//         }

//         obj->suffixMax[end] = obj->damHeight[end];
//         for (int i = end - 1; i >= begin; --i)
//         {
//             obj->suffixMax[i] = MAX(obj->damHeight[i], obj->suffixMax[i + 1]);
//         }
//     }
// }

void removeDam(Object *obj, int *pos, int *maxWater)
{
    assert(obj);
    assert(pos);
    assert(maxWater);
    *pos = 0;
    *maxWater = -1;

    int delta;
    for (int i = 1; i <= obj->nDams; ++i)
    {
        if (obj->damHeight[i] <= 0)
        {
            continue;
        }

        if (obj->damHeight[i] < obj->prefixMax[i] && obj->damHeight[i] < obj->suffixMax[i])
        {
            delta = obj->damHeight[i];
        }
        else
        {
            int begin, end;
            getSection(obj, i, &begin, &end);
            int oSecWater = getSectionWater(obj, begin, end);
            int h = obj->damHeight[i];
            obj->damHeight[i] = 0;
            setPreSufMax2(obj, begin, end);
            int nSecWater = getSectionWater(obj, begin, end);
            obj->damHeight[i] = h;
            setPreSufMax2(obj, begin, end);
            delta = nSecWater - oSecWater;
        }

        if (obj->waterTotal + delta > *maxWater)
        {
            *pos = i;
            *maxWater = obj->waterTotal + delta;
        }
    }
}

void dbgprint(Object *obj)
{
    assert(obj);

    printf("Number of dams: %d\n", obj->nDams);
    for (int i = 1; i <= obj->nDams; ++i)
    {
        printf("%d%s", obj->damHeight[i], (i < obj->nDams) ? " " : "");
    }
    printf("\n");

    printf("Prefix Max:\n");
    for (int i = 1; i <= obj->nDams; ++i)
    {
        printf("%d%s", obj->prefixMax[i], (i < obj->nDams) ? " " : "");
    }
    printf("\n");
    printf("Suffix Max:\n");
    for (int i = 1; i <= obj->nDams; ++i)
    {
        printf("%d%s", obj->suffixMax[i], (i < obj->nDams) ? " " : "");
    }
    printf("\n");
    printf("Total water: %d\n", obj->waterTotal);
}

int main()
{
    int nTestCases; // the number of test cases
    int nDams;      // the number of dams

    scanf("%d", &nTestCases);

    for (int i = 1; i <= nTestCases; ++i)
    {
        scanf("%d", &nDams);
        Object *obj = initObject(nDams);
        if (!obj)
        {
            return EXIT_FAILURE;
        }

        obj->holes[0] = 0;
        obj->nHoles = 1;
        for (int j = 1; j <= nDams; ++j)
        {
            scanf("%d", obj->damHeight + j);

            if (obj->damHeight[j] == -1)
            {
                obj->holes[obj->nHoles++] = j;
            }
        }
        obj->holes[obj->nHoles++] = nDams + 1;

        setPreSufMax(obj);
        setWaterTotal(obj);
        // dbgprint(obj);

        int remove, maxWater;
        removeDam(obj, &remove, &maxWater);
        printf("Case %d:\n", i);
        printf("Remove: %d\n", remove);
        printf("Maximum water: %d%s", maxWater, (i != nTestCases) ? "\n\n" : "");
        deleteObject(obj);
    }
    return 0;
}