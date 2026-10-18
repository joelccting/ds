#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if defined(DEBUG)
#define dbg(x)                      \
    do                              \
    {                               \
        printf("D(%d) ", __LINE__); \
        printf x;                   \
    } while (0)
#else
#define dbg(x)
#endif

#define err(x)                      \
    do                              \
    {                               \
        printf("E(%d) ", __LINE__); \
        printf x;                   \
    } while (0)

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
    int n_pages;
} Event;

int event_set(Event *e, EventType type, int printer_id, int src_printer_id, int job_id, LL arrival_time, int n_pages)
{
    if (!e)
    {
        err(("Null\n"));
        return -1;
    }
    e->arrival_time = arrival_time;
    e->job_id = job_id;
    e->n_pages = n_pages;
    e->printer_id = printer_id;
    e->src_printer_id = src_printer_id;
    e->type = type;

    return 0;
}

int _is_higher_priority(Event *a, Event *b)
{
    if (!a || !b)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time;

    if (a->type != b->type)
    {
        // int prio_map[] = {0, 1, 3, 2, 4, 5, 6, 7};
        // return prio_map[a->type] < prio_map[b->type];
        return a->type < b->type;
    }

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
    PriorityQueue *pq = NULL;
    pq = (PriorityQueue *)calloc(1, sizeof(PriorityQueue));
    assert(pq);
    pq->capacity = capacity;
    pq->data = (Event *)calloc(capacity, sizeof(Event));
    assert(pq->data);
    return pq;
}

void pq_destroy(PriorityQueue *pq)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return;
    }

    free(pq->data);
    free(pq);
}

void _swap(Event *a, Event *b)
{
    if (!a || !b)
    {
        err(("Found null pointer\n"));
        return;
    }

    Event t;
    t = *a;
    *a = *b;
    *b = t;
}

void _heapifyUp(PriorityQueue *pq, int index)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return;
    }

    int parent = (index - 1) / 2;
    while (index > 0 && _is_higher_priority(pq->data + index, pq->data + parent))
    {
        _swap(&pq->data[index], &pq->data[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

void _heapifyDown(PriorityQueue *pq, int index)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return;
    }

    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < pq->size && _is_higher_priority(pq->data + left, pq->data + smallest))
    {
        smallest = left;
    }
    if (right < pq->size && _is_higher_priority(pq->data + right, pq->data + smallest))
    {
        smallest = right;
    }

    if (smallest != index)
    {
        _swap(&pq->data[index], &pq->data[smallest]);
        _heapifyDown(pq, smallest);
    }
}

int pq_push(PriorityQueue *pq, Event *e)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (pq->size == pq->capacity)
    {
        //     pq->capacity *= 2;
        //     pq->data = (Event *)realloc(pq->data, sizeof(Event) * pq->capacity);
        err(("Pque full\n"));
        return 1;
    }

    if (e)
    {
        pq->data[pq->size] = *e;
        pq->size++;
        _heapifyUp(pq, pq->size - 1);
    }

    return 0;
}

int pq_top(PriorityQueue *pq, Event *e)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (pq->size == 0)
    {
        dbg(("Pque empty\n"));
        return 1;
    }

    if (e)
        *e = pq->data[0];

    return 0;
}

int pq_pop(PriorityQueue *pq, Event *e)
{
    if (!pq)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (pq->size == 0)
    {
        dbg(("Pque empty\n"));
        return 1;
    }

    if (e)
    {
        *e = pq->data[0];
        pq->data[0] = pq->data[pq->size - 1];
        pq->size--;
        _heapifyDown(pq, 0);
    }

    return 0;
}

typedef struct job
{
    int id;
    LL arrival_time;
    int n_pages;
} Job;

void job_set(Job *j, int id, LL arrival_time, int n_pages)
{
    if (!j)
        return;

    j->id = id;
    j->arrival_time = arrival_time;
    j->n_pages = n_pages;
}

typedef struct queue
{
    int front;
    int rear;
    int size;
    Job *data;
} Queue;

int isEmpty(Queue *que)
{
    if (!que)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (que->front == que->rear)
    {
        dbg(("is empty!\n"));
        return 1;
    }

    return 0;
}

int isFull(Queue *que)
{
    if (!que)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    int rear = (que->rear + 1) % que->size;
    if (que->front == rear)
    {
        dbg(("is full!\n"));
        return 1;
    }

    return 0;
}

int enque(Queue *que, Job *data)
{
    if (!que)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (isFull(que))
    {
        dbg(("Que is full\n"));
        return 1;
    }

    if (data)
    {
        que->rear = (que->rear + 1) % que->size;
        que->data[que->rear] = *data;
    }

    return 0;
}

/**
 * @return 0 if success; otherwise if failure
 */
int gettop(Queue *que, Job *job)
{
    if (!que)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (isEmpty(que))
    {
        dbg(("Que is empty\n"));
        return 1;
    }

    int front = (que->front + 1) % que->size;
    if (job)
        *job = que->data[front];

    return 0;
}

int deque(Queue *que, Job *job)
{
    if (!que)
    {
        err(("Found null pointer\n"));
        return -1;
    }

    if (isEmpty(que))
    {
        dbg(("Que is empty\n"));
        return 1;
    }
    if (job)
    {
        que->front = (que->front + 1) % que->size;
        *job = que->data[que->front];
    }

    return 0;
}

typedef enum printer_state
{
    PS_IDLE = 0,
    PS_BUSY,
    PS_RESTING,
    PS_TOTAL_NUMBER
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
    if (!p || !j)
    {
        err(("Found null pointer\n"));
        return;
    }

    Queue *q = p->task_que;

    enque(q, j);
    p->n_remaining_pages += j->n_pages;
}

LL printer_get_end_time(Printer *p, LL tick, int new_job_pages)
{
#if defined(DEBUG)
    const char str[][20] = {"IDLE", "BUSY", "RESTING"};
#endif
    LL retval;
    if (!p)
        return 0LL;

    switch (p->state)
    {
    case PS_RESTING:
    {
        LL start_time = MAX(tick, p->rest_end_time);
        retval = start_time + (LL)(p->n_remaining_pages + new_job_pages) * p->speed;
        break;
    }
    case PS_BUSY:
    {
        retval = p->current_job_start_time + (LL)(p->n_remaining_pages + new_job_pages) * p->speed;
        break;
    }
    case PS_IDLE:
    default:
        retval = tick + (LL)new_job_pages * p->speed;
        break;
    }

    dbg(("Printer %d at %s turn available at %lld\n", p->id, str[p->state], retval));

    return retval;
}

PrinterState printer_get_state(Printer *p)
{
    if (!p)
        return PS_TOTAL_NUMBER;

    return p->state;
}

void printer_set_state(Printer *p, PrinterState ps)
{
    if (!p)
        return;

    p->state = ps;

    if (PS_RESTING == ps)
    {
        p->consecutive_printed_pages = 0;
    }
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
 * @param[in] n_jobs: number of job
 */
CopyCenter *cc_create(int n_printers, int n_jobs)
{
    CopyCenter *cc = NULL;
    cc = (CopyCenter *)calloc(1, sizeof(CopyCenter));
    assert(cc);
    cc->n_printers = n_printers;
    cc->n_jobs = n_jobs;
    cc->printer = (Printer *)calloc(n_printers + 5, sizeof(Printer));
    assert(cc->printer);
    for (int i = 1; i <= n_printers; ++i)
    {
        Printer *p = &cc->printer[i];
        p->task_que = (Queue *)calloc(1, sizeof(Queue));
        assert(p->task_que);
        p->task_que->front = p->task_que->rear = 0;
        p->task_que->size = n_jobs + 4;
        p->task_que->data = (Job *)calloc(n_jobs + 5, sizeof(Job));
        assert(p->task_que->data);
    }

    cc->pq = pq_create(n_jobs + 5);

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
 * @return fin_tick tick to finish new-job pages
 */
int cc_get_earliest_printer(CopyCenter *cc, int printer_id, LL tick, int njpages, LL *fin_tick)
{
    if (!cc)
    {
        err(("\n"));
        return -1;
    }
    LL retval = LLONG_MAX, // target end time
        val;
    int retidx = -1;
    for (int i = 1; i <= cc->n_printers; ++i)
    {
        if (printer_id > 0 && i == printer_id)
            continue;

        Printer *p = &cc->printer[i];
        val = printer_get_end_time(p, tick, njpages);

        dbg(("Printer %d: target time %lld\n", i, val));

        if (val < retval)
        {
            retidx = i;
            retval = val;
        }
    }

    if (fin_tick)
        *fin_tick = retval;

    return retidx;
}

LL cc_sim(CopyCenter *cc)
{
    const char etstr[][15] = {"", "COMPLETE", "OVERHEAT", "REST END", "ARRIVE", "REASSIGN", "ASSIGN", "START"};
    Event e, ne;
    LL time, tick = 0;
    PrinterState old_state;

    if (!cc)
    {
        err(("Null pointer\n"));
        return -1LL;
    }

    while (!pq_pop(cc->pq, &e) && cc->n_completed_jobs < cc->n_jobs)
    // while (!pq_pop(cc->pq, &e))
    {
        tick = e.arrival_time;
        Printer *p = NULL;
        Queue *q = NULL;
        Job job;
        int new_printer_id;

        switch (e.type)
        {
        case EV_ARRIVE:
            printf("Time %lld: %s Job %d, pages=%d\n", tick, etstr[e.type], e.job_id, e.n_pages);
            // e.type = EV_ASSIGN;
            event_set(&ne, EV_ASSIGN, e.printer_id, e.src_printer_id, e.job_id, e.arrival_time, e.n_pages);
            pq_push(cc->pq, &ne);
            break;

        case EV_ASSIGN:
            new_printer_id = cc_get_earliest_printer(cc, e.printer_id, tick, e.n_pages, &time);
            p = &cc->printer[new_printer_id];
            q = p->task_que;
            printf("Time %lld: %s Job %d to Printer %d\n", tick, etstr[e.type], e.job_id, new_printer_id);

            job_set(&job, e.job_id, e.arrival_time + time, e.n_pages);
            old_state = printer_get_state(p);
            printer_add_job(p, &job);

            if (old_state == PS_IDLE)
            {
                printer_set_state(p, PS_BUSY);
                event_set(&ne, EV_START, new_printer_id, 0, e.job_id, tick, e.n_pages);
                pq_push(cc->pq, &ne);
            }
            printer_set_state(p, PS_BUSY);

            break;

        case EV_START:
            p = &cc->printer[e.printer_id];
            q = p->task_que;

            if (printer_get_state(p) == PS_RESTING)
            {
                dbg(("Printer %d is resting\n", p->id));
                break;
            }

            p->current_job_start_time = tick;
            printf("Time %lld: %s Printer %d, Job %d, remaining pages=%d\n", tick, etstr[e.type], e.printer_id, e.job_id, e.n_pages);
            int rempages = p->heat_limit - p->consecutive_printed_pages;
            if (e.n_pages <= rempages) /* can consecutively print without overheat */
            {
                // e.arrival_time = tick + e.n_pages * p->speed;
                // e.type = EV_COMPLETE;
                event_set(&ne, EV_COMPLETE, e.printer_id, e.src_printer_id, e.job_id, tick + e.n_pages * p->speed, e.n_pages);
                p->consecutive_printed_pages += e.n_pages;
            }
            else
            {
                // e.n_pages -= rempages;
                // e.arrival_time = tick + rempages * p->speed;
                // e.type = EV_OVERHEAT;
                event_set(&ne, EV_OVERHEAT, e.printer_id, e.src_printer_id, e.job_id, tick + rempages * p->speed, e.n_pages - rempages);
                p->consecutive_printed_pages += rempages;
            }
            // p->state = PS_BUSY;
            printer_set_state(p, PS_BUSY);
            pq_push(cc->pq, &ne);
            break;

        case EV_COMPLETE:
            cc->n_completed_jobs++;
            printf("Time %lld: %s Job %d in Printer %d\n", tick, etstr[e.type], e.job_id, e.printer_id);
            p = &cc->printer[e.printer_id];
            q = p->task_que;

            deque(q, &job);
            // p->consecutive_printed_pages += job.n_pages;
            p->n_remaining_pages -= job.n_pages;

            dbg(("Printer id:%d, p->heat_limit=%d, p->consecutive_printed_pages=%d\n", p->id, p->heat_limit, p->consecutive_printed_pages));
            if (p->heat_limit == p->consecutive_printed_pages)
            {
                p->rest_end_time = tick + p->rest_time;
                dbg(("printer %d idle, overheat, restart time=%lld\n", p->id, p->rest_end_time));
                // p->rest_end_time = tick + p->rest_time;
                // e.arrival_time = tick;
                // e.type = EV_OVERHEAT;
                event_set(&ne, EV_OVERHEAT, e.printer_id, e.src_printer_id, e.job_id, tick, 0);
                pq_push(cc->pq, &ne);
                break;
            }

            // deque(q, &job);
            // p->n_remaining_pages -= job.n_pages;
            if (0 == gettop(q, &job)) // Current printer's queue is not empty
            {
                // e.arrival_time = tick;
                // e.job_id = job.id;
                // e.n_pages = job.n_pages;
                // e.type = EV_START;

                event_set(&ne, EV_START, e.printer_id, e.src_printer_id, job.id, tick, job.n_pages);
                pq_push(cc->pq, &ne);
            }
            else
            {
                // p->state = PS_IDLE;
                // p->consecutive_printed_pages = 0;
                // printer_set_state(p, PS_IDLE);
                printer_set_state(p, PS_IDLE);
            }
            break;

        case EV_OVERHEAT:
            p = &cc->printer[e.printer_id];
            q = p->task_que;
            p->rest_end_time = tick + p->rest_time;
            // p->state = PS_RESTING;
            printer_set_state(p, PS_RESTING);
            printf("Time %lld: %s Printer %d, Restart time=%lld\n",
                   tick, etstr[e.type], e.printer_id, p->rest_end_time);

            event_set(&ne, EV_REST_END, e.printer_id, e.src_printer_id, e.job_id, p->rest_end_time, e.n_pages);
            pq_push(cc->pq, &ne);
            dbg(("Overheat with %d pages\n", e.n_pages));

            if (e.n_pages > 0)
            {
                deque(q, &job);
                p->n_remaining_pages -= e.n_pages;
                event_set(&ne, EV_REASSIGN, e.printer_id, p->id, e.job_id, tick, e.n_pages);
                pq_push(cc->pq, &ne);
            }

            break;

        case EV_REST_END:
            p = &cc->printer[e.printer_id];
            q = p->task_que;
            p->consecutive_printed_pages = 0;
            printf("Time %lld: %s Printer %d\n", tick, etstr[e.type], e.printer_id);

            if (!gettop(q, &job)) // Que is not empty
            {
                // e.arrival_time = tick;
                // e.job_id = job.id;
                // e.n_pages = job.n_pages;
                // e.type = EV_START;
                event_set(&ne, EV_START, e.printer_id, e.src_printer_id, job.id, tick, job.n_pages);
                pq_push(cc->pq, &ne);
                printer_set_state(p, PS_BUSY);
            }
            else
            {
                // p->state = PS_IDLE;
                printer_set_state(p, PS_IDLE);
            }
            break;

        case EV_REASSIGN:
            new_printer_id = cc_get_earliest_printer(cc, e.src_printer_id, tick, e.n_pages, &time);
            if (new_printer_id == e.src_printer_id)
            {
                dbg(("Printer %d: no new printer id\n", e.src_printer_id));
                break;
            }

            p = &cc->printer[new_printer_id];
            q = p->task_que;
            printf("Time %lld: %s Job %d, remaining pages=%d, from Printer %d to Printer %d\n",
                   tick, etstr[e.type], e.job_id, e.n_pages, e.src_printer_id, new_printer_id);

            // job.arrival_time = time;
            // job.id = e.job_id;
            // job.n_pages = e.n_pages;
            job_set(&job, e.job_id, tick, e.n_pages);
            old_state = printer_get_state(p);
            printer_add_job(p, &job);

            if (old_state == PS_IDLE)
            {
                printer_set_state(p, PS_BUSY);
                event_set(&ne, EV_START, new_printer_id, e.src_printer_id, job.id, tick, job.n_pages);
                pq_push(cc->pq, &ne);
            }

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
        fprintf(stderr, "Wrong inputs\n");
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
        LL arrival_time;
        int n_pages;
        scanf("%lld %d", &arrival_time, &n_pages);
        Event e;
        // e.job_id = i;
        // e.type = EV_ARRIVE;
        event_set(&e, EV_ARRIVE, 0, 0, i, arrival_time, n_pages);
        pq_push(cc->pq, &e);
    }

    printf("Finish time: %lld", cc_sim(cc));

    cc_destroy(cc);

    return 0;
}