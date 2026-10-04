#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#define LEN (128)
typedef struct
{
    // int cellIdx;
    char type;
    int val;
} specCell;

typedef struct
{
    int nCells;          // N
    int maxStepsPerMove; // K
    int maxMoves;        // R
    int nSpecialCells;   // C
    int minScore;        // G
    specCell *sc;

} Map;

Map *initMap(int N, int K, int R, int C, int G)
{
    Map *m = NULL;
    m = (Map *)malloc(sizeof(Map));
    assert(m);
    memset(m, 0, sizeof(Map));
    m->nCells = N;
    m->maxStepsPerMove = K;
    m->maxMoves = R;
    m->nSpecialCells = C;
    m->minScore = G;
    m->sc = (specCell *)malloc((N + 1) * sizeof(specCell));
    assert(m->sc);
    memset(m->sc, 0, (N + 1) * sizeof(specCell));
    return m;
}

void deleteMap(Map *m)
{
    assert(m);
    assert(m->sc);
    free(m->sc);
    free(m);
}

typedef struct
{
    int roll;
    int from;
    int moveTo;
    char effect[LEN + 1];
    int final;
    int score;
    int bonus;
    int penalty;
    int shield;
} turnLog;

typedef struct
{
    turnLog *item;
    int *dice;
    int totSucRoutes;
    int shRoute;
    int shRouteTurns;
    int hiScoreRoute;
    int hiScore;
} Log;

typedef struct
{
    int turn;
    int pos;
    int moves;
    int score;
    int shield;
    int bonus;
    int penalty;
    unsigned long long treasureMask;
} playerState;

void printLog(playerState *player, Log *log)
{
    if (log->totSucRoutes > 1)
        printf("\n");

    printf("Route %d:\n", log->totSucRoutes);
    printf("Dice: ");
    for (int i = 1; i <= player->moves; ++i)
    {
        printf("%d%s", log->dice[i], (i != player->moves) ? " " : "\n");
    }

    for (int i = 1; i <= player->moves; ++i)
    {
        printf("Turn %d: ", i);
        printf("roll=%d, ", log->item[i].roll);
        printf("from=%d, move_to=%d, ", log->item[i].from, log->item[i].moveTo);
        printf("effect=%s, ", log->item[i].effect); // TODO
        printf("final=%d, ", log->item[i].final);
        printf("score=%d, ", log->item[i].score);
        printf("bonus=%d, ", log->item[i].bonus);
        printf("penalty=%d ", log->item[i].penalty);
        printf("shield=%d\n", log->item[i].shield);
    }

    printf("Result: Win in %d turns, score=%d\n", player->moves, player->score);
}

void printSummary(Log *log)
{
    if (log->totSucRoutes)
    {
        printf("\nSummary:\n");
        printf("Total successful routes: %d\n", log->totSucRoutes);
        printf("Shortest route: Route %d, turns=%d\n", log->shRoute, log->shRouteTurns);
        printf("Highest score: Route %d, score=%d", log->hiScoreRoute, log->hiScore);
    }
    else
    {
        printf("No successful route");
    }
}

Log *initLog(int nCells)
{
    Log *p = NULL;
    p = (Log *)malloc(sizeof(Log));
    assert(p);
    memset(p, 0, sizeof(Log));

    p->item = (turnLog *)malloc((nCells + 1) * sizeof(turnLog));
    assert(p->item);
    memset(p->item, 0, (nCells + 1) * sizeof(turnLog));

    p->dice = (int *)malloc((nCells + 1) * sizeof(int));
    assert(p->dice);
    memset(p->dice, 0, (nCells + 1) * sizeof(int));

    p->hiScore = -1;
    p->hiScoreRoute = -1;
    p->shRoute = 1000;
    p->shRouteTurns = 1000;
    return p;
}

void deleteLog(Log *obj)
{
    assert(obj->item);
    assert(obj->dice);
    free(obj->item);
    free(obj->dice);
    free(obj);
}

playerState *initplayerState(int nCells, int maxMoves)
{
    playerState *obj = NULL;
    obj = (playerState *)malloc(sizeof(playerState));
    assert(obj);
    memset(obj, 0, sizeof(playerState));
    obj->pos = 1;

    return obj;
}

void deleteplayerState(playerState *obj)
{
    assert(obj);
    free(obj);
}

void dfs(playerState *player, Map *map, Log *log)
{
    if (player->pos >= map->nCells)
    {
        // success
        if (player->score >= map->minScore && player->moves <= map->maxMoves)
        {
            log->totSucRoutes++;
            if (player->score > log->hiScore)
            {
                log->hiScore = player->score;
                log->hiScoreRoute = log->totSucRoutes;
            }

            if (log->shRouteTurns > player->moves)
            {
                log->shRouteTurns = player->moves;
                log->shRoute = log->totSucRoutes;
            }
            printLog(player, log);
            return;
        }
        else // failure
        {
            return;
        }
    }
    else // next.pos < map->nCells
    {
        // failure
        if (player->moves >= map->maxMoves)
        {
            return;
        }
    }

    // roll the dice
    for (int roll = 1; roll <= map->maxStepsPerMove; ++roll)
    {
        playerState next = *player; // get a copy so that recursion can recover the previous state

        ++next.moves;
        log->item[next.moves].effect[0] = '\0';
        log->dice[next.moves] = roll;
        log->item[next.moves].roll = roll;
        log->item[next.moves].score = next.score;
        log->item[next.moves].bonus = next.bonus;
        log->item[next.moves].penalty = next.penalty;
        log->item[next.moves].shield = next.shield;

        int effectiveRoll = roll + next.bonus - next.penalty;
        next.bonus = 0;
        next.penalty = 0;
        if (effectiveRoll > 0)
        {
            next.pos += effectiveRoll;

            log->item[next.moves].from = next.pos - effectiveRoll;
            log->item[next.moves].moveTo = next.pos;
            log->item[next.moves].final = next.pos;

            if (next.pos >= map->nCells)
            {
                // success
                log->item[next.moves].score = next.score;
                log->item[next.moves].bonus = next.bonus;
                log->item[next.moves].penalty = next.penalty;
                log->item[next.moves].shield = next.shield;
                log->item[next.moves].final = map->nCells;
                snprintf(log->item[next.moves].effect, LEN, "Reach goal");
                dfs(&next, map, log);
                continue;
            }
            else //(next.pos < map->nCells)
            {
                bool bLadder = (next.pos < map->nCells) && (map->sc[next.pos].type == 'L');
                bool bSnake = (next.pos < map->nCells) && (map->sc[next.pos].type == 'S');
                int flag = 0;
                while (next.pos < map->nCells && (bLadder || bSnake))
                {
                    if (bLadder)
                    {
                        int len = strlen(log->item[next.moves].effect);
                        if (len)
                        {
                            snprintf(log->item[next.moves].effect + len, LEN - len, ";Ladder %d->%d",
                                     next.pos, map->sc[next.pos].val);
                        }
                        else
                        {
                            snprintf(log->item[next.moves].effect, LEN, "Ladder %d->%d",
                                     next.pos, map->sc[next.pos].val);
                        }

                        next.pos = map->sc[next.pos].val;
                    }
                    else if (bSnake)
                    {
                        log->item[next.moves].shield = next.shield;
                        int len = strlen(log->item[next.moves].effect);
                        if (next.shield == 1)
                        {
                            next.shield = 0;
                            if (len)
                            {
                                snprintf(log->item[next.moves].effect + len, LEN - len,
                                         ";Snake blocked");
                            }
                            else
                            {
                                snprintf(log->item[next.moves].effect, LEN, "Snake blocked");
                            }

                            log->item[next.moves].final = next.pos;
                            log->item[next.moves].shield = 0;
                            break;
                        }
                        else //(next.shield != 1)
                        {
                            if (len)
                            {
                                snprintf(log->item[next.moves].effect + len, LEN - len, ";Snake %d->%d",
                                         next.pos, map->sc[next.pos].val);
                            }
                            else
                            {
                                snprintf(log->item[next.moves].effect, LEN, "Snake %d->%d",
                                         next.pos, map->sc[next.pos].val);
                            }

                            next.pos = map->sc[next.pos].val;
                        }
                    }

                    log->item[next.moves].final = next.pos;

                    if (next.pos >= map->nCells) // success
                    {
                        log->item[next.moves].final = map->nCells;
                        int len = strlen(log->item[next.moves].effect);
                        if (len)
                            snprintf(log->item[next.moves].effect + len, LEN - len, ";Reach goal");
                        else
                            snprintf(log->item[next.moves].effect, LEN, "Reach goal");
                        dfs(&next, map, log);
                        flag = 1;
                        break;
                    }

                    // reset
                    bLadder = (map->sc[next.pos].type == 'L');
                    bSnake = (map->sc[next.pos].type == 'S');
                } // while(next.pos < map->nCells && (bLadder || (bSnake && !bShield)))

                if (flag)
                    continue;

                int len = strlen(log->item[next.moves].effect);
                switch (map->sc[next.pos].type)
                {
                case 'L': // Ladder
                    // bLadder = true;
                    break;
                case 'S': // Snake
                    // bSnake = true;
                    break;
                case 'H': // Shield
                    next.shield = 1;
                    log->item[next.moves].shield = next.shield;
                    if (len)
                    {
                        snprintf(log->item[next.moves].effect + len, LEN - len,
                                 ";Shield");
                    }
                    else
                    {
                        snprintf(log->item[next.moves].effect, LEN,
                                 "Shield");
                    }
                    break;
                case 'P': // Penalty
                    next.penalty = map->sc[next.pos].val;
                    if (len)
                    {
                        snprintf(log->item[next.moves].effect + len, LEN - len,
                                 ";Penalty -%d", next.penalty);
                    }
                    else
                    {
                        snprintf(log->item[next.moves].effect, LEN,
                                 "Penalty -%d", next.penalty);
                    }
                    break;
                case 'B': // Bonus
                    next.bonus = map->sc[next.pos].val;
                    if (len)
                    {
                        snprintf(log->item[next.moves].effect + len, LEN - len,
                                 ";Bonus +%d", next.bonus);
                    }
                    else
                    {
                        snprintf(log->item[next.moves].effect, LEN,
                                 "Bonus +%d", next.bonus);
                    }
                    break;

                case 'T': // Treasure
                    if (next.treasureMask & (1ULL << next.pos))
                    {
                        next.score += map->sc[next.pos].val;
                        next.treasureMask &= ~(1ULL << next.pos);
                        if (len)
                        {
                            snprintf(log->item[next.moves].effect + len, LEN - len,
                                     ";Treasure +%d", map->sc[next.pos].val);
                        }
                        else
                        {
                            snprintf(log->item[next.moves].effect, LEN,
                                     "Treasure +%d", map->sc[next.pos].val);
                        }
                    }

                    break;

                default:
                    // if (len)
                    // {
                    //     snprintf(log->item[next.moves].effect + len, LEN - len,
                    //              "%s;None", log->item[next.moves].effect);
                    // }
                    // else
                    // {
                    if (!len)
                        snprintf(log->item[next.moves].effect, LEN, "None");
                    // }

                    break;
                }

                log->item[next.moves].score = next.score;
                log->item[next.moves].bonus = next.bonus;
                log->item[next.moves].penalty = next.penalty;
            }
        }
        else // (effectiveRoll <= 0)
        {
            log->item[next.moves].from = next.pos;
            log->item[next.moves].moveTo = next.pos;
            log->item[next.moves].final = next.pos;
            snprintf(log->item[next.moves].effect, LEN, "None");
            dfs(&next, map, log);
            continue;
        }

        dfs(&next, map, log);
    }
}

int main()
{
    int N, // number of cells on the board
        K, // maximum movement value for each move
        R, // maximum number of moves
        C, // number of special cells
        G; // minimum score to successfully complete the mission

    if (5 != scanf("%d %d %d %d %d", &N, &K, &R, &C, &G))
    {
        fprintf(stderr, "Wrong inputs\n");
        return 0;
    }

    Map *map = initMap(N, K, R, C, G);
    playerState *player = initplayerState(N, K);
    Log *log = initLog(N);

    // <cell_index><type><value>
    for (int i = 0; i < C; ++i)
    {
        int idx, val;
        char type;
        scanf("%d %c %d", &idx, &type, &val);
        map->sc[idx].type = type;
        map->sc[idx].val = val;
        switch (type)
        {
        case 'T':
            player->treasureMask |= (1ULL << idx);
            break;
        default:
            break;
        }
    }

    dfs(player, map, log);

    printSummary(log);

    deleteMap(map);
    deleteplayerState(player);
    deleteLog(log);

    return 0;
}