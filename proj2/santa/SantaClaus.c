#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_REINDEER 9
#define NUM_ELVES 10
#define NUM_ELVES_FOR_SANTA 3
#define ROUNDS 3
#define SLEEP 5
#define RED "\x1B[31m"
#define RESET "\x1B[0m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define MAGENTA "\x1B[35m"


sem_t santaSem, reindeerSem, elfSem;                // 唤醒圣诞老人的信号量、让驯鹿系上绳子的信号量和让精灵接受帮助的信号量
sem_t elfallow;                                     // 当有三只精灵在等待后，不再允许精灵进入等待队列
sem_t reindeerEnd;                                  // 巡回是否结束的信号量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // 用于限制访问资源的互斥锁
int reindeerCount = 0, elfCount = 0;                // 存储精灵和驯鹿的个数
int sleighCount = 0, helpCount = 0;                 // 存储圣诞老人出去送礼物/帮助精灵的次数
int hitchedCount = 0;                               // 存储已经系上绳索的驯鹿的个数
int elvesBuf[NUM_ELVES_FOR_SANTA];                  // 存储需要帮助的三只精灵的id
int flag_r = 1;                                     // 在圣诞老人巡回结束后，必须9只驯鹿全部释放资源后，才能有新的驯鹿进来等待
int flag_e = 1;                                     // 在圣诞老人帮完三个精灵后，必须三个精灵全部释放资源后，才能有新的精灵进来等待
int flag_exit = 0;                                  // 当循环条件满足的时候，存在部分精灵仍在等待，需要手动的去释放


void* SantaClaus(void* arg) {
    while (1) {
        int f = 0;
        pthread_mutex_lock(&mutex);
        if (sleighCount >= ROUNDS && helpCount >= ROUNDS) {
            printf(YELLOW "Santa Claus DONE.\n" RESET);
            f = 1;
            flag_exit = 1;
            /* 释放信号量资源，防止精灵和驯鹿忙等 */
            for (int i = 0; i < NUM_ELVES_FOR_SANTA; ++i) sem_post(&elfSem);
            for (int i = 0; i < NUM_ELVES; ++i) sem_post(&elfallow);
            for (int i = 0; i < NUM_REINDEER; ++i) sem_post(&reindeerSem);
        }
        pthread_mutex_unlock(&mutex);
        if (f) break;

        sem_wait(&santaSem);
        printf(MAGENTA "Santa Claus is waken up.\n" RESET);
        int rc, ec, sc, hc;

        pthread_mutex_lock(&mutex);
        rc = reindeerCount;
        ec = elfCount;
        sc = sleighCount;
        hc = helpCount;
        pthread_mutex_unlock(&mutex);

        /* 优先处理驯鹿 */
        if (rc == NUM_REINDEER) {
            printf(RED "Santa Claus: preparing sleigh.\nSleigh count : %d, Help elves count : %d.\n" RESET, sc + 1, hc);
            pthread_mutex_lock(&mutex);
            flag_r = 0;
            sleighCount++;
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < NUM_REINDEER; i++) {
                sem_post(&reindeerSem);
            }

            while (1) {
                int cnt;
                pthread_mutex_lock(&mutex);
                cnt = hitchedCount;
                pthread_mutex_unlock(&mutex);
                if (cnt == NUM_REINDEER) break;
            }

            printf(GREEN "Santa Claus together with %d reindeers are going to send gifts to children around the world.\n" RESET, NUM_REINDEER);
            sleep(rand() % SLEEP);              // 模拟送礼物
            printf(MAGENTA "Santa Claus finishes sending gift to children.\n" RESET);
            for (int i = 0; i < NUM_REINDEER; ++i) sem_post(&reindeerEnd);
            /* 模拟结束，释放驯鹿资源 */
            while (1) {
                int cnt;
                pthread_mutex_lock(&mutex);
                cnt = reindeerCount;
                pthread_mutex_unlock(&mutex);
                if (!cnt) break;
            }
            pthread_mutex_lock(&mutex);
            flag_r = 1;
            hitchedCount = 0;
            pthread_mutex_unlock(&mutex);
        } else if (ec == NUM_ELVES_FOR_SANTA) {
            printf(RED "Santa Claus: helping elves.\nSleigh count : %d, Help elves count : %d.\n" RESET, sc, hc + 1);
            pthread_mutex_lock(&mutex);
            flag_e = 0;
            helpCount++;
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < NUM_ELVES_FOR_SANTA; i++) {
                printf(GREEN "Elf %d is being helped.\n" RESET, elvesBuf[i]);
                // sem_post(&elfSem);  // 因为 flag_e 的存在，即使被帮助的精灵已经进入下一个循环了，也会在原地死循环
            }
            sleep(rand() % SLEEP);
            printf(MAGENTA "Santa Claus finishes helping the elves.\n" RESET);
            for (int i = 0; i < NUM_ELVES_FOR_SANTA; ++i) {
                sem_post(&elfSem);
            }

            while (1) {
                int cnt;
                pthread_mutex_lock(&mutex);
                cnt = elfCount;
                pthread_mutex_unlock(&mutex);
                if (!cnt) break;
            }
            pthread_mutex_lock(&mutex);
            flag_e = 1;
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < NUM_ELVES_FOR_SANTA; ++i) sem_post(&elfallow);
        }
    }
    return NULL;
}

void* Reindeer(void* arg) {
    while (1) {
        int f = 0, f1;
        pthread_mutex_lock(&mutex);
        if (sleighCount >= ROUNDS && helpCount >= ROUNDS) f = 1;
        f1 = flag_r;
        pthread_mutex_unlock(&mutex);
        if (f) break;
        if (!f1) continue;

        sleep(rand() % SLEEP);
        int id = *(int*)arg;
        printf("Reindeer %d is waiting for the santa claus.\n", id);
        pthread_mutex_lock(&mutex);
        reindeerCount++;
        if (reindeerCount == NUM_REINDEER) {
            sem_post(&santaSem);
        }
        pthread_mutex_unlock(&mutex);
        sem_wait(&reindeerSem);
        int nf;
        pthread_mutex_lock(&mutex);
        nf = flag_exit;
        pthread_mutex_unlock(&mutex);
        if (nf) break;
        printf(GREEN "Reindeer %d getting hitched.\n" RESET, id);
        pthread_mutex_lock(&mutex);
        ++hitchedCount;
        pthread_mutex_unlock(&mutex);
        sem_wait(&reindeerEnd);
        pthread_mutex_lock(&mutex);
        --reindeerCount;
        // printf("Reindeer %d leaves, current count : %d.\n", id, reindeerCount);
        pthread_mutex_unlock(&mutex);
    }
    printf(YELLOW "Reindeer %d DONE.\n" RESET, *(int*)arg);
    return NULL;
}

void* Elf(void* arg) {
    while (1) {
        int f = 0, f1;
        pthread_mutex_lock(&mutex);
        if (sleighCount >= ROUNDS && helpCount >= ROUNDS) f = 1;
        f1 = flag_e;
        pthread_mutex_unlock(&mutex);
        if (f) break;
        if (!f1) continue;
        
        sleep(rand() % SLEEP);
        // printf("Elf %d is waiting for the santa claus.\n", *(int*)arg);
        sem_wait(&elfallow);
        int nf;
        pthread_mutex_lock(&mutex);
        nf = flag_exit;
        pthread_mutex_unlock(&mutex);

        if (nf) break;
        
        int id = *(int*)arg;
        // printf("This is elf %d\n", id);
        pthread_mutex_lock(&mutex);
        if (elfCount < NUM_ELVES_FOR_SANTA) {
            elvesBuf[elfCount] = id;
            printf("Elf %d is waiting for the santa claus.\n", id);
            ++elfCount;
            if (elfCount == NUM_ELVES_FOR_SANTA) sem_post(&santaSem);
        } else {
            // 因为 elfallow 资源的限制，永远不会执行到
            printf("Elf %d goes back.\n", id);
            continue;
        }
        pthread_mutex_unlock(&mutex);
        sem_wait(&elfSem);
        pthread_mutex_lock(&mutex);
        --elfCount;
        pthread_mutex_unlock(&mutex);       
    }
    printf(YELLOW "Elf %d DONE.\n" RESET, *(int*)arg);
    return NULL;
}

int main() {
    pthread_t santa, reindeers[NUM_REINDEER], elves[NUM_ELVES];
    int ids[NUM_REINDEER > NUM_ELVES ? NUM_REINDEER : NUM_ELVES];

    if (sem_init(&santaSem, 0, 0) == -1) {
        printf("Failed to initialize santaSem.\n");
        exit(1);
    }
    if (sem_init(&reindeerSem, 0, 0) == -1) {
        printf("Failed to initialize reindeerSem.\n");
        exit(1);
    }
    if (sem_init(&elfSem, 0, 0) == -1) {
        printf("Failed to initialize elfSem.\n");
        exit(1);
    }
    if (sem_init(&elfallow, 0, NUM_ELVES_FOR_SANTA) == -1) {
        printf("Failed to initialize elfallow.\n");
        exit(1);
    }
    if (sem_init(&reindeerEnd, 0, 0) == -1) {
        printf("Failed to initialize reindeerEnd.\n");
        exit(1);
    }

    if (pthread_create(&santa, NULL, SantaClaus, NULL) != 0) {
        printf("Failed to create santa claus thread.\n");
        exit(1);
    }

    for (int i = 0; i < NUM_REINDEER; i++) {
        ids[i] = i;
        if (pthread_create(&reindeers[i], NULL, Reindeer, &ids[i]) != 0) {
            printf("Failed to create reindeer thread #%d\n", i);
            exit(1);
        }
    }
    for (int i = 0; i < NUM_ELVES; i++) {
        ids[i] = i;
        if (pthread_create(&elves[i], NULL, Elf, &ids[i]) != 0) {
            printf("Failed to create elf thread #%d\n", i);
            exit(1);
        }
    }

    if (pthread_join(santa, NULL) != 0) {
        printf("Failed to join santa thread\n");
        exit(1);
    }
    for (int i = 0; i < NUM_REINDEER; i++) {
        if (pthread_join(reindeers[i], NULL) != 0) {
            printf("Failed to join reindeer thread #%d\n", i);
            exit(1);
        }
    }
    for (int i = 0; i < NUM_ELVES; i++) {
        if (pthread_join(elves[i], NULL) != 0) {
            printf("Failed to join elf thread #%d\n", i);
            exit(1);
        }
    }

    if (sem_destroy(&santaSem) == -1) {
        printf("Failed to destroy santaSem.\n");
        exit(1);
    }
    if (sem_destroy(&reindeerSem) == -1) {
        printf("Failed to destroy reindeerSem.\n");
        exit(1);
    }
    if (sem_destroy(&elfSem) == -1) {
        printf("Failed to destroy elfSem.\n");
        exit(1);
    }
    if (sem_destroy(&elfallow) == -1) {
        printf("Failed to destroy elfallow.\n");
        exit(1);
    }
    if (sem_destroy(&reindeerEnd) == -1) {
        printf("Failed to destroy reindeerEnd.\n");
        exit(1);
    }
    if (pthread_mutex_destroy(&mutex) != 0) {
        printf("Failed to destroy mutex.\n");
        exit(1);
    }
    return 0;
}