#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DEBUG
#ifdef DEBUG
#define dbg(x) printf x
#else
#define dbg(x)
#endif

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
    EventType type;
    int printerId;
    int srcPrinterId;

    int jobId;
    LL arrivalTime;
    int pages;
} Event;

int is_higher_priority(Event *a, Event *b)
{
    if (a->arrivalTime != b->arrivalTime)
        return a->arrivalTime < b->arrivalTime;

    if (a->type != b->type)
        return a->type < b->type;

    switch (a->type)
    {
    case EV_COMPLETE:
    case EV_OVERHEAT:
    case EV_REST_END:
    case EV_START:
        return a->printerId < b->printerId;

    case EV_ARRIVE:
    case EV_ASSIGN:
        return a->jobId < b->jobId;

    case EV_REASSIGN:
        if (a->srcPrinterId != b->srcPrinterId)
            return a->srcPrinterId < b->srcPrinterId;
        return a->jobId < b->jobId;
    }
    return 0;
}

typedef struct
{
    Event *data;
    int capacity;
    int size;
} PriorityQueue;

PriorityQueue *pq_create(int capacity)
{
    PriorityQueue *pq = (PriorityQueue *)calloc(1, sizeof(PriorityQueue));
    assert(pq);
    pq->capacity = capacity;
    pq->data = (Event *)calloc(capacity, sizeof(Event));
    assert(pq->data);
    return pq;
}

void pq_destroy(PriorityQueue *pq)
{
    if (!pq)
        return;

    free(pq->data);
    free(pq);
}

void swap(Event *a, Event *b)
{
    Event t;

    t = *a;
    *a = *b;
    *b = t;
}

void heapifyUp(PriorityQueue *pq, int index)
{
    int parent = (index - 1) / 2;
    while (index > 0 && is_higher_priority(pq->data + index, pq->data + parent))
    {
        swap(&pq->data[index], &pq->data[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapifyDown(PriorityQueue *pq, int index)
{
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < pq->size && is_higher_priority(pq->data + left, pq->data + smallest))
    {
        smallest = left;
    }
    if (right < pq->size && is_higher_priority(pq->data + right, pq->data + smallest))
    {
        smallest = right;
    }

    if (smallest != index)
    {
        swap(&pq->data[index], &pq->data[smallest]);
        heapifyDown(pq, smallest);
    }
}

void pq_push(PriorityQueue *pq, Event *e)
{
    // if (pq->size == pq->capacity)
    // {
    //     pq->capacity *= 2;
    //     pq->data = (Event *)realloc(pq->data, sizeof(Event) * pq->capacity);
    // }

    pq->data[pq->size] = *e;
    pq->size++;
    heapifyUp(pq, pq->size - 1);
}

int pq_top(PriorityQueue *pq, Event *e)
{
    if (pq->size == 0)
    {
        return -1;
    }

    *e = pq->data[0];
    return 0;
}

int pq_pop(PriorityQueue *pq, Event *e)
{
    if (pq->size == 0)
    {
        return -1;
    }

    *e = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    heapifyDown(pq, 0);

    return 0;
}

typedef struct
{
    int id;
    int arrivalTime;
    int pages;
} Job;

typedef struct
{
    int front;
    int rear;
    int size;
    Job *data;
} Queue;

int isEmpty(Queue *que)
{
    if (que->front == que->rear)
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

int enque(Queue *que, Job *data)
{
    if (isFull(que))
        return -1;

    que->rear = (que->rear + 1) % que->size;
    que->data[que->rear] = *data;
    return 0;
}

int deque(Queue *que, Job *job)
{
    if (isEmpty(que))
        return -1;

    que->front = (que->front + 1) % que->size;
    *job = que->data[que->front];
    return 0;
}

typedef struct
{
    int id;
    int speed;      // required time per page
    int heat_limit; // max number of consecutively printing pages before overheating
    int rest_time;  // required rest arrivalTime
    int n_tasks;
    Queue *task_que; // task_que queue
    int n_to_print_pages;
} Printer;

int printer_get_pages(Printer *p, int tick, int *n_printed, int *n_to_print)
{
    Queue *que = p->task_que;
    int retval = 0;
    if (isEmpty(que))
        return 0;

    int front = (que->front + 1) % que->size;
    Job *j = &p->task_que->data[front];
    *n_printed = (tick - j->arrivalTime) / p->speed;
    retval += (j->pages - *n_printed);

    front = (front + 1) % que->size;

    int rear = que->rear;
    if (front != rear)
    {
        if (front <= rear)
        {
            for (int i = front; i <= rear; ++i)
                retval += que->data[i].pages;
        }
        else
        {
            for (int i = front; i <= que->size; ++i)
                retval += que->data[i].pages;
            for (int i = 0; i <= rear; ++i)
                retval += que->data[i].pages;
        }
    }
    *n_to_print = retval;
    return 1;
}

void printer_set_pages(Printer *p, int tick)
{
    int n_printed, n_to_print;
    printer_get_pages(p, tick, &n_printed, &n_to_print);
    p->n_to_print_pages = n_to_print;
    dbg(("id %d, printed %d, to print %d\n", p->id, n_printed, n_to_print));
}

typedef struct
{
    Printer *printer;
    int nPrinters;

    PriorityQueue *pq;
} CopyCenter;

/**
 * @param[in] nPrinters: number of printers
 * @param[in] nJobs: number of job
 */
CopyCenter *cc_create(int nPrinters, int nJobs)
{
    CopyCenter *cc = NULL;
    cc = (CopyCenter *)calloc(1, sizeof(CopyCenter));
    assert(cc);
    cc->nPrinters = nPrinters;
    cc->printer = (Printer *)calloc(nPrinters + 1, sizeof(Printer));
    assert(cc->printer);
    for (int i = 1; i <= nPrinters; ++i)
    {
        Printer *p = &cc->printer[i];
        p->task_que = (Queue *)calloc(1, sizeof(Queue));
        assert(p->task_que);
        p->task_que->front = p->task_que->rear = 0;
        p->task_que->size = nJobs;
        p->task_que->data = (Job *)calloc(nJobs + 1, sizeof(Job));
        assert(p->task_que->data);
    }

    cc->pq = pq_create(nJobs);

    return cc;
}

void cc_destroy(CopyCenter *cc)
{
    if (!cc)
        return;

    pq_destroy(cc->pq);

    for (int i = 1; i <= cc->nPrinters; ++i)
    {
        Printer *p = &cc->printer[i];
        free(p->task_que->data);
        free(p->task_que);
    }

    free(cc->printer);
    free(cc);
}

/**
 * @brief get the expected-to-finish-earliest printer
 * @return the printer id
 */
int cc_getEarliest(CopyCenter *cc, int tick)
{
    int retval = INT_MAX, val, retidx = -1;
    for (int i = 1; i <= cc->nPrinters; ++i)
    {
        Printer *p = &cc->printer[i];
        val = tick + p->rest_time + p->n_to_print_pages * p->speed;
        if (val < retval)
        {
            retidx = i;
            retval = val;
        }

        dbg(("printer %d: %d\n", i, val));
    }

    return retidx;
}

void cc_process(CopyCenter *cc)
{
    const char etstr[][15] = {"", "COMPLELE", "OVERHEAT", "REST END", "ARRIVE", "REASSIGN", "ASSIGN", "START"};
    Event e;
    while (!pq_pop(cc->pq, &e))
    {
        int tick = e.arrivalTime;

        switch (e.type)
        {
        case EV_ARRIVE:
            printf("Time %d: %s Job %d, pages=%d\n", tick, etstr[e.type], e.jobId, e.pages);
            e.type = EV_ASSIGN;
            pq_push(cc->pq, &e);
            break;
        case EV_ASSIGN:
            e.printerId = cc_getEarliest(cc, tick);
            Queue *q = cc->printer[e.printerId].task_que;
            Job jb;
            jb.arrivalTime = e.arrivalTime;
            jb.id = e.jobId;
            jb.pages = e.pages;
            enque(q, &jb);
            printer_set_pages(&cc->printer[e.printerId], tick);
            printf("Time %d: %s Job %d to Printer %d\n", tick, etstr[e.type], e.jobId, e.printerId);
            e.type = EV_START;
            pq_push(cc->pq, &e);
            break;
        case EV_START:
            printf("Time %d: %s Printer %d, Job %d, remaining pages=%d\n",
                   tick, etstr[e.type], e.printerId, e.jobId, e.pages);
            break;
        case EV_COMPLETE:
        case EV_OVERHEAT:
        case EV_REST_END:

        case EV_REASSIGN:

        default:
            break;
        }
    }
}

int main()
{
    int M, // number of printers
        N; // number of job
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
    }

    for (int i = 1; i <= N; ++i)
    {
        Event e;
        e.jobId = i;
        e.type = EV_ARRIVE;
        scanf("%lld %d", &e.arrivalTime, &e.pages);
        pq_push(cc->pq, &e);
    }

    cc_process(cc);
    printf("Finish arrivalTime: ");

    cc_destroy(cc);

    return 0;
}