#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
    Job **jobQue;
} Queue;

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

void enque(Queue *q, Job *j)
{
    if (isFull(q))
        return;

    q->rear = (q->rear + 1) % q->size;
    q->jobQue[q->rear] = j;
}

void deque(Queue *q)
{
    if (isEmpty(q))
        return;

    q->front = (q->front + 1) % q->size;
}

typedef struct
{
    int id;
    int speed;     // time per page
    int heatLimit; // max number of consecutively printing pages
    int restTime;  // required rest time
    Queue *que;
} Printer;

int main()
{
    int M, // number of printers
        N; // number of jobs
    if (2 != scanf("%d %d", &M, &N))
    {
        fprintf(stderr, "Usage: M, N\n");
        return 0;
    }

    Printer *p = (Printer *)calloc(M + 1, sizeof(Printer));
    assert(p);

    for (int i = 1; i <= M; ++i)
    {
        int s, h, r;
        scanf("%d %d %d", &s, &h, &r);
        p[i].id = i;
        p[i].speed = s;
        p[i].heatLimit = h;
        p[i].restTime = r;
        p[i].que = (Queue *)calloc(1, sizeof(Queue));
        assert(p[i].que);
        p[i].que->size = N + 5;
        p[i].que->front = p[i].que->rear = 0;
        p[i].que->jobQue = (Job **)calloc(p[i].que->size, sizeof(Job *));
        assert(p[i].que->jobQue);
    }

    Job *j = (Job *)calloc(N + 1, sizeof(Job));
    assert(j);
    for (int i = 1; i <= N; ++i)
    {
        int a, p;
        scanf("%d %d", &a, &p);
        j[i].id = i;
        j[i].arrivalTime = a;
        j[i].pages = p;
    }

    for (int i = 1; i <= M; ++i)
    {
        free(p[i].que->jobQue);
        free(p[i].que);
    }
    free(p);
    free(j);

    return 0;
}