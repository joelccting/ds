#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DEBUG
#ifdef DEBUG
#define dbg(x)                     \
    do                             \
    {                              \
        printf("[%d] ", __LINE__); \
        printf x;                  \
    } while (0)

#else
#define dbg(x)
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef long long LL;

typedef enum event_type
{
    EV_COMPLETE = 1,
    EV_OVERHEAT,
    EV_REST_END,
    EV_ARRIVE,
    EV_REASSIGN,
    EV_ASSIGN,
    EV_START
} EventType;

typedef struct event
{
    EventType type;
    int printer_id;
    int src_printer_id;
    int job_id;
    LL arrival_time;
    int pages;
} Event;

int is_higher_priority(Event *a, Event *b)
{
    if (a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time;

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

typedef struct priority_queue
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

typedef struct job
{
    int id;
    LL arrival_time;
    int pages;
} Job;

typedef struct queue
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
        // dbg(("is empty!\n"));
        return 1;
    }

    return 0;
}

int isFull(Queue *que)
{

    int rear = (que->rear + 1) % que->size;
    if (que->front == rear)
    {
        // dbg(("is full!\n"));
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

int gettop(Queue *que, Job *job)
{
    if (isEmpty(que))
        return -1;
    int front = (que->front + 1) % que->size;
    if (job)
        *job = que->data[front];
    return 0;
}

int deque(Queue *que, Job *job)
{
    if (isEmpty(que))
        return -1;

    que->front = (que->front + 1) % que->size;
    if (job)
        *job = que->data[que->front];
    return 0;
}

typedef enum printer_state
{
    PS_IDLE = 0,
    PS_BUSY,
    PS_RESTING
} PrinterState;

typedef struct printer
{
    int id;
    int speed;      // required time per page
    int heat_limit; // max number of consecutively printing pages before overheating
    int rest_time;  // required rest arrival_time
    int n_tasks;
    Queue *task_que; // task_que queue
    int n_remaining_pages;
    PrinterState state;
    LL rest_end_time;
    int consecutive_printed_pages;
    LL current_job_start_time;
} Printer;

void printer_add_job(Printer *p, Job *j)
{
    Queue *q = p->task_que;

    enque(q, j);
    p->n_remaining_pages += j->pages;
    p->state = PS_BUSY;
}

LL printer_get_end_time(Printer *p, LL tick, int new_job_pages)
{
    PrinterState ps = p->state;
    LL val;
    switch (ps)
    {
    case PS_RESTING:
        p->rest_end_time = MAX(p->rest_end_time - tick, 0LL);
        val = p->rest_end_time + (p->n_remaining_pages + new_job_pages) * p->speed;
        break;
    case PS_BUSY:
        val = tick + (p->n_remaining_pages + new_job_pages) * p->speed;
        break;
    case PS_IDLE:
    default:
        val = tick + new_job_pages * p->speed;
        // dbg(("tick %lld, new job pages %d, speed %d\n", tick, new_job_pages, p->speed));
        break;
    }

    return val;
}

PrinterState printer_get_state(Printer *p)
{
    return p->state;
}

typedef struct copy_center
{
    Printer *printer;
    int n_printers;
    int n_jobs;
    int n_completed_jobs;
    PriorityQueue *pq;
} CopyCenter;

/**
 * @param[in] n_printers: number of printers
 * @param[in] nJobs: number of job
 */
CopyCenter *cc_create(int n_printers, int nJobs)
{
    CopyCenter *cc = NULL;
    cc = (CopyCenter *)calloc(1, sizeof(CopyCenter));
    assert(cc);
    cc->n_printers = n_printers;
    cc->n_jobs = nJobs;
    cc->printer = (Printer *)calloc(n_printers + 1, sizeof(Printer));
    assert(cc->printer);
    for (int i = 1; i <= n_printers; ++i)
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

    for (int i = 1; i <= cc->n_printers; ++i)
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
 * @param[in] printer_id the current printer id. 0 if all printers need to be evaluated.
 * @param[in] njpages new-job pages
 * @return the printer id
 * @return extime time to finish new-job pages
 */
int cc_getEarliest(CopyCenter *cc, int printer_id, LL tick, int njpages, LL *extime)
{
    LL retval = LLONG_MAX, // target end time
        val;
    int retidx = -1;
    for (int i = 1; i <= cc->n_printers; ++i)
    {
        if (printer_id > 0 && i == printer_id)
            continue;

        Printer *p = &cc->printer[i];
        val = printer_get_end_time(p, tick, njpages);

        // dbg(("Printer %d: target time %lld\n", i, val));

        if (val < retval)
        {
            retidx = i;
            retval = val;
        }
    }
    if (extime)
        *extime = retval;
    return retidx;
}

LL cc_process(CopyCenter *cc)
{
    const char etstr[][15] = {"", "COMPLETE", "OVERHEAT", "REST END", "ARRIVE", "REASSIGN", "ASSIGN", "START"};
    Event e;
    LL time, tick = 0;

    while (!pq_pop(cc->pq, &e) && cc->n_completed_jobs < cc->n_jobs)
    {
        tick = e.arrival_time;
        Printer *p = NULL;
        Queue *q = NULL;

        switch (e.type)
        {
        case EV_ARRIVE:
            printf("Time %lld: %s Job %d, pages=%d\n", tick, etstr[e.type], e.job_id, e.pages);
            e.type = EV_ASSIGN;
            pq_push(cc->pq, &e);
            break;

        case EV_ASSIGN:
            e.printer_id = cc_getEarliest(cc, 0, tick, e.pages, &time);
            p = &cc->printer[e.printer_id];
            printf("Time %lld: %s Job %d to Printer %d\n", tick, etstr[e.type], e.job_id, e.printer_id);
            // dbg(("complete time: %lld\n", time));
            Job jb;
            jb.arrival_time = e.arrival_time + time;
            jb.id = e.job_id;
            jb.pages = e.pages;
            if (p->state == PS_IDLE)
            {
                e.type = EV_START;
                pq_push(cc->pq, &e);
            }

            printer_add_job(p, &jb);
            break;

        case EV_START:
            p = &cc->printer[e.printer_id];
            printf("Time %lld: %s Printer %d, Job %d, remaining pages=%d\n",
                   tick, etstr[e.type], e.printer_id, e.job_id, e.pages);
            int rempages = p->heat_limit - p->consecutive_printed_pages;
            if (e.pages <= rempages)
            {
                e.arrival_time = tick + e.pages * p->speed;
                e.type = EV_COMPLETE;
                p->consecutive_printed_pages += e.pages;
                p->state = PS_BUSY;
            }
            else
            {
                e.pages -= rempages;
                e.arrival_time = tick + rempages * p->speed;
                e.type = EV_OVERHEAT;
                p->consecutive_printed_pages += rempages;
            }
            pq_push(cc->pq, &e);
            break;

        case EV_COMPLETE:
            cc->n_completed_jobs++;
            printf("Time %lld: %s Job %d in Printer %d\n",
                   tick, etstr[e.type], e.job_id, e.printer_id);
            p = &cc->printer[e.printer_id];
            q = p->task_que;
            Job b;
            deque(q, &b);
            p->n_remaining_pages -= b.pages;
            if (0 == gettop(q, &b))
            {
                e.arrival_time = tick;
                e.job_id = b.id;
                e.pages = b.pages;
                e.type = EV_START;
                pq_push(cc->pq, &e);
            }
            else
            {
                p->state = PS_IDLE;
                if (p->heat_limit == p->consecutive_printed_pages)
                {
                    p->rest_end_time = tick + p->rest_time;
                    e.arrival_time = tick;
                    e.type = EV_OVERHEAT;
                    pq_push(cc->pq, &e);
                }
            }
            break;

        case EV_OVERHEAT:
            p = &cc->printer[e.printer_id];
            p->rest_end_time = tick + p->rest_time;
            p->state = PS_RESTING;
            printf("Time %lld: %s Printer %d, Restart time=%lld\n",
                   tick, etstr[e.type], e.printer_id, p->rest_end_time);
            e.arrival_time = p->rest_end_time;
            e.type = EV_REST_END;
            pq_push(cc->pq, &e);

            q = p->task_que;
            if (0 == gettop(q, &b))
            {
                if (b.pages > p->heat_limit)
                {
                    e.arrival_time = tick;
                    e.type = EV_REASSIGN;
                    e.job_id = b.id;
                    e.pages = b.pages - p->heat_limit;
                    e.src_printer_id = p->id;
                    pq_push(cc->pq, &e);
                    deque(q, NULL);
                }
            }

            break;

        case EV_REST_END:
            printf("Time %lld: %s Printer %d\n",
                   tick, etstr[e.type], e.printer_id);
            p = &cc->printer[e.printer_id];
            p->consecutive_printed_pages = 0;
            q = p->task_que;
            Job j;
            if (-1 != gettop(q, &j))
            {
                e.arrival_time = tick;
                e.job_id = j.id;
                e.pages = j.pages;
                e.type = EV_START;
                pq_push(cc->pq, &e);
                p->state = PS_BUSY;
            }
            else
            {
                p->state = PS_IDLE;
            }
            break;

        case EV_REASSIGN:
            e.printer_id = cc_getEarliest(cc, e.printer_id, tick, e.pages, &time);
            printf("Time %lld: %s Job %d, remaining pages=%d, from Printer %d to Printer %d\n",
                   tick, etstr[e.type], e.job_id, e.pages, e.src_printer_id, e.printer_id);
            e.type = EV_START;
            pq_push(cc->pq, &e);
            break;

        default:
            fprintf(stderr, "Unknown state: %d\n", e.type);
            break;
        }
    }

    return tick;
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
        e.job_id = i;
        e.type = EV_ARRIVE;
        scanf("%lld %d", &e.arrival_time, &e.pages);
        pq_push(cc->pq, &e);
    }

    printf("Finish time: %lld", cc_process(cc));

    cc_destroy(cc);

    return 0;
}