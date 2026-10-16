#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define dbg(x) printf x

typedef long long LL;

typedef enum
{
    EV_COMPLETE = 1,
    EV_OVERHEAT,
    EV_REST_END,
    EV_ARRIVE,
    EV_REASSIGN,
    EV_ASSIGN,
    EV_START
} EventType;

typedef struct
{
    LL time;
    EventType type;
    int printer_id;
    int job_id;
    int src_printer_id;
    int pages;
} Event;

int is_higher_priority(Event *a, Event *b)
{
    if (a->time != b->time)
        return a->time < b->time;

    if (a->type != b->type)
        return a->type < b->type;

    switch (a->type)
    {
    case EV_COMPLETE:
    case EV_OVERHEAT:
    case EV_REST_END:
    case EV_START:
        return a->printer_id < b->printer_id;

    case EV_ARRIVE:
    case EV_ASSIGN:
        return a->job_id < b->job_id;

    case EV_REASSIGN:
        if (a->src_printer_id != b->src_printer_id)
            return a->src_printer_id < b->src_printer_id;
        return a->job_id < b->job_id;
    }
    return 0;
}

typedef enum
{
    IDLE = 0,
    COMPLETE,
    OVERHEAT,
    RESTEND,
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
    Status status;
} Job;

typedef struct
{
    int front;
    int rear;
    int size;
    int *idx;
} Queue;

int isEmpty(Queue *que)
{
    if (que->front == que->rear)
        ;
    {
        dbg(("ERROR: is empty!\n"));
        return 1;
    }

    return 0;
}

int isFull(Queue *que)
{
    int rear = (que->rear + 1) % que->size;
    if (que->front == rear)
    {
        dbg(("ERROR: is full!\n"));
        return 1;
    }

    return 0;
}

int enque(Queue *que, int idx)
{
    if (isFull(que))
        return -1;

    que->rear = (que->rear + 1) % que->size;
    que->idx[que->rear] = idx;
    return 0;
}

int deque(Queue *que)
{
    if (isEmpty(que))
        return -1;

    que->front = (que->front + 1) % que->size;
    return que->idx[que->front];
}

typedef enum
{
    PS_IDLE,
    PS_BUSY,
    PS_RESTING
} PrinterState;

typedef struct
{
    int id;
    int speed;      // time per page
    int heat_limit; // max number of consecutively printing pages before overheating
    int rest_time;  // required rest time

    Queue *tskq;    // tskq queue
    LL currRePages; // current remaining pages
    int currJobId;  // current job id
    LL cpPageCount; // consecutive printed page count
    PrinterState state;
} Printer;

typedef struct
{
    Job *jobs;
    int nJobs;
    int nEnqeue;
    Queue *arque; // arrival queue
    Printer *printer;
    int nPrinters;
} CopyCenter;

/**
 * @param[in] nPrn: number of printers
 * @param[in] nJob: number of jobs
 */
CopyCenter *cc_create(int nPrn, int nJob)
{
    CopyCenter *cc = NULL;
    cc = (CopyCenter *)calloc(1, sizeof(CopyCenter));
    assert(cc);
    cc->nPrinters = nPrn;
    cc->printer = (Printer *)calloc(nPrn + 1, sizeof(Printer));
    assert(cc->printer);
    cc->nJobs = nJob;
    cc->arque = (Queue *)calloc(nJob + 1, sizeof(Job *));
    assert(cc->arque);
    cc->jobs = (Job *)calloc(nJob + 1, sizeof(Job));
    assert(cc->jobs);

    for (int i = 1; i <= nPrn; ++i)
    {
        Printer *p = &cc->printer[i];
        p->tskq = (Queue *)calloc(1, sizeof(Queue));
        assert(p->tskq);
        // p->tskq->size = nJob + 1;
        p->tskq->front = p->tskq->rear = 0;
        p->tskq->idx = (int *)calloc(nJob + 1, sizeof(int));
        assert(p->tskq->idx);
    }

    return cc;
}

void cc_destroy(CopyCenter *cc)
{
    if (!cc)
        return;

    for (int i = 1; i <= cc->nPrinters; ++i)
    {
        Printer *p = &cc->printer[i];
        free(p->tskq->idx);
        free(p->tskq);
    }
    free(cc->arque);
    free(cc->printer);
    free(cc->jobs);
    free(cc);
}

/**
 * @return 0: no jobs; >0: the arrival job index; <0: errors
 */
int cc_setArrivJob(CopyCenter *cc, int tick)
{
    Job *jobs = cc->jobs;
    Queue *arque = cc->arque;
    int t = cc->nEnqeue + 1;

    while (t <= cc->nJobs)
    {
        printf("t=%d/%d\n", t, cc->nJobs);
        if (jobs[t].arrivTime == tick)
        {
            printf("Time %d: ARRIVE Job %d, pages=%d\n", tick, jobs[t].id, jobs[t].pages);
            enque(arque, jobs[t].id);
            cc->nEnqeue++;
        }
        else
        {
            break;
        }

        ++t;
    }

    return 0;
}

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
int getFiEarliest(CopyCenter *p, int M)
{
    int idx = INT_MAX;
    // LL finTime = LLONG_MAX;
    // for (int i = 1; i <= M; ++i)
    // {
    //     LL remTime = p->pnt[i].remPages * p->pnt[i].speed + p->pnt[i].rest_time;
    //     if (finTime > remTime)
    //     {
    //         finTime = remTime;
    //         idx = i;
    //     }
    // }

    return idx;
}

void step(CopyCenter *cc, int tick)
{
    // int front;
    cc_setArrivJob(cc, tick);
    // {
    //     printf("Time %d: ARRIVE Job %d, pages=%d\n", tick, jque->job[front].id, jque->job[front].pages);
    // }
    // for (int i = 1; i <= n; ++i)
    // {
    //     int pnt = getFiEarliest(pdata, pdata->size);
    //     int jobid = jque->job[++jque->front].id;
    //     printf("Time %d: ASSIGN Job %d to Printer %d\n", tick, jobid, pnt);
    //     enque(pdata->pnt, &jque->job[jque->front]);
    // }
}

/**
 * @return 1 if all done; 0 otherwise
 */
int cc_isDone(CopyCenter *cc)
{
    int sz = cc->nJobs;
    for (int i = 1; i <= sz; ++i)
    {
        Status s = cc->jobs[i].status;
        if (s != COMPLETE)
            return 0;
    }

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

    CopyCenter *cc = cc_create(M, N);

    for (int i = 1; i <= M; ++i)
    {
        Printer *p = &cc->printer[i];
        int s, h, r;
        scanf("%d %d %d", &s, &h, &r);
        p->id = i;
        p->speed = s;
        p->heat_limit = h;
        p->rest_time = r;
        p->state = PS_IDLE;
    }

    for (int i = 1; i <= N; ++i)
    {
        Job *j = &cc->jobs[i];
        int a, p;
        scanf("%d %d", &a, &p);
        j->id = i;
        j->arrivTime = a;
        j->pages = p;
    }

    qsort(cc->jobs + 1, N, sizeof(Job), cmpArrivTime);
    // for (int i = 1; i <= N; ++i)
    // {
    //     printf("%d: %d, %d\n", j->job[i].id, j->job[i].arrivTime, j->job[i].pages);
    // }

    int tick = 0;
    while (!cc_isDone(cc))
    {
        step(cc, tick);
        tick++;
        if (tick > 3)
            break;
    }

    printf("Finish time: %d", tick);

    cc_destroy(cc);

    return 0;
}