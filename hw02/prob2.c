#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef long long LL;

typedef enum
{
    COMPLETE,
    OVERHEAT,
    REST,
    ARRIVE,
    REASSIGN,
    ASSIGN,
    START
} Status;

typedef struct
{
    int id;
    int arrivTime;
    int pages;
    int isDone;
} Job;

typedef struct
{
    Job *job;
    int size;
    int front;
    int rear;
} JobData;

typedef struct
{
    int front;
    int rear;
    int size;
    Job *que;
} Queue;

int getArrivJob(JobData *jdata, int tick)
{
    int front = jdata->front;
    while (jdata->job[++front].arrivTime == tick)
    {
        printf("Time %d: ARRIVE Job %d, pages=%d\n", tick, jdata->job[front].id, jdata->job[front].pages);
    }

    return front - 1;
}

void freeArrivJob(Queue *q)
{
    if (q)
    {
        free(q->que);
        free(q);
    }
}

int isFull(Queue *q)
{
    int rear = q->rear;
    if ((rear + 1) % q->size == q->front)
        return 1;

    return 0;
}

int isEmpty(Queue *q)
{
    if (q->front == q->rear)
        return 1;

    return 0;
}

int enque(Queue *q, Job *j)
{
    if (isFull(q))
        return -1;

    q->rear = (q->rear + 1) % q->size;
    memcpy(q->que + q->rear, j, sizeof(Job));
    return 0;
}

int deque(Queue *q, Job *j)
{
    if (isEmpty(q))
        return -1;

    q->front = (q->front + 1) % q->size;
    memcpy(j, q->que + q->rear, sizeof(Job));
    return 0;
}

typedef struct
{
    int id;
    int speed;     // time per page
    int heatLimit; // max number of consecutively printing pages before overheating
    int restTime;  // required rest time
    Queue *task;   // task queue
    LL remPages;
    Status state;
} Printer;

typedef struct
{
    Printer *pnt;
    int size;
} PrinterData;

int cmpArrivTime(const void *a, const void *b)
{
    Job *pa = (Job *)a,
        *pb = (Job *)b;
    if (pa->arrivTime == pb->arrivTime)
    {
        return (pa->id > pb->id);
    }

    return (pa->arrivTime > pb->arrivTime);
}

/**
 * @brief get the printer that is expected to finish the earliest
 * @return the printer number
 */
int getFiEarliest(PrinterData *p, int M)
{
    int idx = INT_MAX;
    LL finTime = LLONG_MAX;
    for (int i = 1; i <= M; ++i)
    {
        LL remTime = p->pnt[i].remPages * p->pnt[i].speed + p->pnt[i].restTime;
        if (finTime > remTime)
        {
            finTime = remTime;
            idx = i;
        }
    }

    return idx;
}

void step(PrinterData *pdata, JobData *jdata, int tick)
{
    int n = getArrivJob(jdata, tick);
    for (int i = 1; i <= n; ++i)
    {
        int pnt = getFiEarliest(pdata, pdata->size);
        int jobid = jdata->job[++jdata->front].id;
        printf("Time %d: ASSIGN Job %d to Printer %d\n", tick, jobid, pnt);
        enque(pdata->pnt, &jdata->job[jdata->front]);
    }
}

/**
 * @return 1 if all done; 0 otherwise
 */
int isAllJobDone(JobData *j, int N)
{
    for (int i = 1; i <= N; ++i)
        if (!j->job[i].isDone)
            return 0;
    return 1;
}

int main()
{
    int M, // number of printers
        N; // number of jobs
    if (2 != scanf("%d %d", &M, &N))
    {
        fprintf(stderr, "Usage: M, N\n");
        return 0;
    }

    PrinterData *p = (PrinterData *)calloc(1, sizeof(PrinterData));
    assert(p);
    p->size = M;
    p->pnt = (Printer *)calloc(M + 1, sizeof(Printer));
    assert(p->pnt);

    for (int i = 1; i <= M; ++i)
    {
        int s, h, r;
        scanf("%d %d %d", &s, &h, &r);
        p->pnt[i].id = i;
        p->pnt[i].speed = s;
        p->pnt[i].heatLimit = h;
        p->pnt[i].restTime = r;

        p->pnt[i].task = (Queue *)calloc(1, sizeof(Queue));
        assert(p->pnt[i].task);
        p->pnt[i].task->size = N + 5;
        p->pnt[i].task->front = p->pnt[i].task->rear = 0;
        p->pnt[i].task->que = (Job *)calloc(p->pnt[i].task->size, sizeof(Job));
        assert(p->pnt[i].task->que);
    }

    JobData *j = (JobData *)calloc(1, sizeof(JobData));
    assert(j);
    j->size = N;
    j->front = 0;
    j->rear = N;
    j->job = (Job *)calloc(N + 1, sizeof(Job));
    assert(j);
    for (int i = 1; i <= N; ++i)
    {
        int a, p;
        scanf("%d %d", &a, &p);
        j->job[i].id = i;
        j->job[i].arrivTime = a;
        j->job[i].pages = p;
    }

    qsort(j->job + 1, N, sizeof(Job), cmpArrivTime);
    // for (int i = 1; i <= N; ++i)
    // {
    //     printf("%d: %d, %d\n", j->job[i].id, j->job[i].arrivTime, j->job[i].pages);
    // }

    int tick = 0;
    while (!isAllJobDone(j, N))
    {
        step(p, j, tick);
        tick++;
        if (tick > 3)
            break;
    }

    printf("Finish time: %d", tick);

    for (int i = 1; i <= M; ++i)
    {
        free(p->pnt[i].task->que);
        free(p->pnt[i].task);
    }

    free(p->pnt);
    free(p);
    free(j->job);
    free(j);

    return 0;
}