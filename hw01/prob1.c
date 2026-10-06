/**
 * Problem 1 Flood-control simulation system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct
{
    int nDams;      // number of dams
    int *damHeight; // heights of dams
    int nHoles;     // number of holes
    int *holes;     // position of holes
    int *prefixMax; // prefix-max
    int *suffixMax; // suffix-max
    int waterTotal; // original total water amount
    int *secStart;  // the start position of a section which i-th dam is in
    int *secEnd;    // the end position of a section which i-th dam is in
    int *oSecWater; // original water amount in a section before removing any dam
    int *prefixMaxcopy; // a copy of prefixMax, changeable any time without recalculating *prefixMax and *suffixMax
    int *suffixMaxcopy;
} Object;

#define FREE(x)      \
    do               \
    {                \
        if (x)       \
            free(x); \
    } while (0)

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))

/**
 * @brief Free allocated space for data
 * @param[in] obj a pointer to memory storing data for the whole program
 * @return None
 */
void deleteObject(Object *obj)
{
    assert(obj);
    FREE(obj->damHeight);
    FREE(obj->holes);
    FREE(obj->prefixMax);
    FREE(obj->suffixMax);
    FREE(obj->secStart);
    FREE(obj->secEnd);
    FREE(obj->oSecWater);
    FREE(obj->prefixMaxcopy);
    FREE(obj->suffixMaxcopy);
    FREE(obj);
}

/**
 * @brief Allocating required memory space
 * @param[in] n the number of dams
 * @return a pointer to the memory
 */
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

    p->oSecWater = (int *)malloc((n + 5) * sizeof(int));
    assert(p->oSecWater);
    memset(p->oSecWater, 0, (n + 5) * sizeof(int));

    p->prefixMaxcopy = (int *)malloc((n + 5) * sizeof(int));
    assert(p->prefixMaxcopy);
    memset(p->prefixMaxcopy, 0, (n + 5) * sizeof(int));

    p->suffixMaxcopy = (int *)malloc((n + 5) * sizeof(int));
    assert(p->suffixMaxcopy);
    memset(p->suffixMaxcopy, 0, (n + 5) * sizeof(int));

    return p;
}

/**
 * @brief Calculate the water amount stored in a section
 * @param[in] obj pointer to data
 * @param[in] flag 1: use original prefixMax/suffixMax to calculate; 0: use a modified copy to calculate
 * @param[in] start start position of a section
 * @param[in] end end position of a section
 * @return the water amount stored in a section
 */
int getSectionWater(Object *obj, int flag, int start, int end)
{
    int retval = 0;
    for (int i = start; i <= end; ++i)
    {
        if (flag)
            retval += MAX(0, MIN(obj->prefixMax[i], obj->suffixMax[i]) - obj->damHeight[i]);
        else
            retval += MAX(0, MIN(obj->prefixMaxcopy[i], obj->suffixMaxcopy[i]) - obj->damHeight[i]);
    }
    return retval;
}

/**
 * @brief Calculate total water amount and remember the value
 * @param[in] obj pointer to data
 * @return None
 */
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

/**
 * @brief get total water amount
 * @param[in] obj Pointer to data
 * @return Total water amount
 */
int getWaterTotal(Object *obj)
{
    assert(obj);
    return obj->waterTotal;
}

/**
 * @brief set regional prefix/suffix max
 * @param[in] obj pointer to data
 * @param[in] flag 1: write to original prefix/suffix max memory; 0: write to copy memory instead
 * @param[in] begin start of the section
 * @param[in] end end of the section
 * @return None
 */
void setPreSufMax2(Object *obj, int flag, int begin, int end)
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

    if (flag)
        obj->prefixMax[begin] = obj->damHeight[begin];
    else
        obj->prefixMaxcopy[begin] = obj->damHeight[begin];

    for (int i = begin + 1; i <= end; ++i)
    {
        if (flag)
            obj->prefixMax[i] = MAX(obj->damHeight[i], obj->prefixMax[i - 1]);
        else
            obj->prefixMaxcopy[i] = MAX(obj->damHeight[i], obj->prefixMaxcopy[i - 1]);
    }

    if (flag)
        obj->suffixMax[end] = obj->damHeight[end];
    else
        obj->suffixMaxcopy[end] = obj->damHeight[end];

    for (int i = end - 1; i >= begin; --i)
    {
        if (flag)
            obj->suffixMax[i] = MAX(obj->damHeight[i], obj->suffixMax[i + 1]);
        else
            obj->suffixMaxcopy[i] = MAX(obj->damHeight[i], obj->suffixMaxcopy[i + 1]);
    }
    // }
}
/**
 * @brief A wrapper of setPreSufMax2, handling all dams
 * @param[in] obj Pointer to data
 * @return None
 */
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
        setPreSufMax2(obj, 1, begin, end);
        for (int j = begin; j <= end; ++j)
        {
            obj->secStart[j] = begin;
            obj->secEnd[j] = end;
            obj->oSecWater[j] = getSectionWater(obj, 1, begin, end);
        }
        // printf("[%d] end=%d\n", __LINE__, end);
    }
}

/**
 * @brief Get the start and end position of i-th dam
 * @param[in] obj Pointer to data
 * @param[in] idx current dam position
 * @param[out] start start position of the section
 * @param[out] end end position of the section
 */
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

/**
 * @brief Iterate/remove each dam and decide water amount reaches the max at which position
 * @param[in] obj Pointer to data
 * @param[out] pos Pointer to the position which water amount reaches the max when removed
 * @param[out] maxWater The max water amount
 */
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
            // int oSecWater = getSectionWater(obj, begin, end);
            int oSecWater = obj->oSecWater[i];

            int h = obj->damHeight[i];
            obj->damHeight[i] = 0;
            setPreSufMax2(obj, 0, begin, end);
            int nSecWater = getSectionWater(obj, 0, begin, end);
            obj->damHeight[i] = h;
            // setPreSufMax2(obj, begin, end);

            delta = nSecWater - oSecWater;
        }

        if (obj->waterTotal + delta > *maxWater)
        {
            *pos = i;
            *maxWater = obj->waterTotal + delta;
        }
    }
}

/**
 * @brief Debug
 * @param[in] Pointer to data
 */
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

/**
 * @brief Entry of the program
 */
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

        /* Dams range from 1 to nDams; [0], [nDams + 1] are fake dams. */
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

        /* Calculate prefixMax and suffixMax */
        setPreSufMax(obj);

        /* Calculate total water amount */
        setWaterTotal(obj);
        // dbgprint(obj);

        /* Iterate and decide the answer */
        int remove, maxWater;
        removeDam(obj, &remove, &maxWater);

        /* Print results */
        printf("Case %d:\n", i);
        printf("Remove: %d\n", remove);
        printf("Maximum water: %d%s", maxWater, (i != nTestCases) ? "\n\n" : "");
        deleteObject(obj);
    }
    return 0;
}