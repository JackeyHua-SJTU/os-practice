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

sem_t santaSem, reindeerSem, elfSem;                // 唤醒圣诞老人的信号量、让驯鹿系上绳子的信号量和让精灵接受帮助的信号量
sem_t elfallow;                                     // 当有三只精灵在等待后，不再允许精灵进入等待队列
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // 用于限制访问资源的互斥锁
int reindeerCount = 0, elfCount = 0;                // 存储精灵和驯鹿的个数
int sleighCount = 0, helpCount = 0;                 // 存储圣诞老人出去送礼物/帮助精灵的次数
int elvesBuf[NUM_ELVES_FOR_SANTA];                  // 存储需要帮助的三只精灵的id
int flag_r = 1;                                     // 在圣诞老人巡回结束后，必须9只驯鹿全部释放资源后，才能有新的驯鹿进来等待
int flag_e = 1;                                     // 在圣诞老人帮完三个精灵后，必须三个精灵全部释放资源后，才能有新的精灵进来等待
int flag_exit = 0;                                  // 当循环条件满足的时候，存在部分精灵仍在等待，需要手动的去释放

// TODO: add random sleep

void* SantaClaus(void* arg) {
    while (1) {
        int f = 0;
        pthread_mutex_lock(&mutex);
        if (sleighCount >= ROUNDS && helpCount >= ROUNDS) {
            f = 1;
            flag_exit = 1;
            for (int i = 0; i < NUM_ELVES_FOR_SANTA; ++i) sem_post(&elfSem);
            for (int i = 0; i < NUM_ELVES; ++i) sem_post(&elfallow);
        }
        pthread_mutex_unlock(&mutex);
        if (f) break;

        sem_wait(&santaSem);
        int rc, ec;

        pthread_mutex_lock(&mutex);
        rc = reindeerCount;
        ec = elfCount;
        pthread_mutex_unlock(&mutex);
        int sc, hc;
        pthread_mutex_lock(&mutex);
        sc = sleighCount;
        hc = helpCount;
        pthread_mutex_unlock(&mutex);
        // Santa gives priority to the reindeer.
        if (rc == NUM_REINDEER) {
            printf("Santa Claus: preparing sleigh.\nSleigh count : %d, Help elves count : %d.\n", sc + 1, hc);
            pthread_mutex_lock(&mutex);
            flag_r = 0;
            sleighCount++;
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < NUM_REINDEER; i++) {
                sem_post(&reindeerSem);
            }
            // TODO: add sync here
            printf("Santa Claus: make all kids in the world happy\n");
            while (1) {
                int cnt;
                pthread_mutex_lock(&mutex);
                cnt = reindeerCount;
                pthread_mutex_unlock(&mutex);
                if (!cnt) break;
            }
            pthread_mutex_lock(&mutex);
            flag_r = 1;
            pthread_mutex_unlock(&mutex);
        } else if (ec == NUM_ELVES_FOR_SANTA) {
            printf("Santa Claus: helping elves.\nSleigh count : %d, Help elves count : %d.\n", sc, hc + 1);
            pthread_mutex_lock(&mutex);
            flag_e = 0;
            helpCount++;
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < NUM_ELVES_FOR_SANTA; i++) {
                printf("Elf %d is being helped.\n", elvesBuf[i]);
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
    printf("Santa Claus DONE.\n");
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
        printf("Reindeer %d getting hitched.\n", id);
        pthread_mutex_lock(&mutex);
        --reindeerCount;
        pthread_mutex_unlock(&mutex);
    }
    printf("Reindeer %d DONE.\n", *(int*)arg);
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
            printf("Elf %d goes back.\n", id);
            continue;
        }
        pthread_mutex_unlock(&mutex);
        sem_wait(&elfSem);
        pthread_mutex_lock(&mutex);
        --elfCount;
        pthread_mutex_unlock(&mutex);       
    }
    printf("Elf %d DONE.\n", *(int*)arg);
    return NULL;
}

int main() {
    pthread_t santa, reindeers[NUM_REINDEER], elves[NUM_ELVES];
    int ids[NUM_REINDEER > NUM_ELVES ? NUM_REINDEER : NUM_ELVES];

    sem_init(&santaSem, 0, 0);
    sem_init(&reindeerSem, 0, 0);
    sem_init(&elfSem, 0, 0);
    sem_init(&elfallow, 0, NUM_ELVES_FOR_SANTA);

    pthread_create(&santa, NULL, SantaClaus, NULL);
    for (int i = 0; i < NUM_REINDEER; i++) {
        ids[i] = i;
        pthread_create(&reindeers[i], NULL, Reindeer, &ids[i]);
    }
    for (int i = 0; i < NUM_ELVES; i++) {
        ids[i] = i;
        pthread_create(&elves[i], NULL, Elf, &ids[i]);
    }

    pthread_join(santa, NULL);
    for (int i = 0; i < NUM_REINDEER; i++) {
        pthread_join(reindeers[i], NULL);
    }
    for (int i = 0; i < NUM_ELVES; i++) {
        pthread_join(elves[i], NULL);
    }

    // TODO: destroy the semaphore

    return 0;
}