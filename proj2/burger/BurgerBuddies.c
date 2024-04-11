#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SLEEP 3         // set sleep time

typedef struct {
    u_int8_t id;        // 服务当前客户的服务员id
    sem_t* order;       // 客户是否点单
    sem_t* takeaway;    // 客户是否将汉堡拿走，不拿走服务员不能继续服务
    sem_t* served;      // 服务员是否拿到了汉堡
} cashier_arg_t;


sem_t sem_customer;         // 是否有顾客到来，没有的话收银员会休息
sem_t sem_customer_private; // 用于当前客户获取服务员信息
sem_t sem_cashier;          // 可服务的cashier的个数
sem_t sem_rack;             // burger的个数
sem_t sem_vacant;           // 还能制作多少个burger，初始值为rack的大小

pthread_mutex_t mutex;

cashier_arg_t cashier_cur;  // 用来记录当前的服务员，需上 sem_customer_private 锁访问

void* cook(void* arg) {
    while(1) {
        sem_wait(&sem_vacant);
        sleep(rand() % SLEEP);  // 制作汉堡的随机延时
        printf("Cook [%d] cooks a burger and puts it into the rack.\n", *(int*)arg);
        sem_post(&sem_rack);
    }
}

void* cashier(void* arg) {
    while(1) {
        sem_wait(&sem_customer);
        pthread_mutex_lock(&mutex);
        int cashier_id = *(int*)arg;
        cashier_arg_t cash;
        cash.id = cashier_id;
        sem_t food, takeaway, serve;
        sem_init(&food, 0, 0);
        sem_init(&takeaway, 0, 0);
        sem_init(&serve, 0, 0);
        cash.order = &food;
        cash.takeaway = &takeaway;
        cash.served = &serve;
        cashier_cur = cash;
        pthread_mutex_unlock(&mutex);
        /* 在申明自己可以提供服务之前，修改服务员的信息，用于和客户之间进行同步 */
        sem_post(&sem_cashier);
        sem_wait(&food);        // 等用户点单
        printf("Cashier [%d] accepts an order.\n", *(int*)arg);
        sem_wait(&sem_rack);
        printf("Cashier [%d] takes a burger from the rack to the customer.\n", *(int*)arg);
        sem_post(&serve);       // 申明自己已经上餐了，等待客户拿走
        sem_wait(&takeaway);    // 等客户拿走
        sem_post(&sem_vacant);
        sem_destroy(&food);
        sem_destroy(&takeaway);
        sem_destroy(&serve);
    }
}

void* customer(void* arg) {
        cashier_arg_t cur;
        printf("Customer [%d] comes.\n", *(int*)arg);
        sem_wait(&sem_customer_private);    // 获取收银员的信息，必须上锁避免信息被覆盖或者修改
        sem_post(&sem_customer);
        sem_wait(&sem_cashier);
        cur = cashier_cur;
        printf("Customer [%d] is being served by cashier [%d].\n", *(int*) arg, cur.id);
        sem_post(cur.order);        // 点单
        sem_post(&sem_customer_private);
        sem_wait(cur.served);       // 等服务员上餐
        printf("Customer [%d] receives the burger and leaves.\n", *(int*)arg);
        sem_post(cur.takeaway);     // 申明自己已经拿走了汉堡，服务员可以休息/服务下一个客户
}

int main(int argc, char** argv) {
    if (argc != 5) {
        printf("Usage: %s <numCooks> <numCashiers> <numCustomers> <numRacks>\n", argv[0]);
        exit(1);
    }

    int numCooks = atoi(argv[1]);
    int numCashiers = atoi(argv[2]);
    int numCustomers = atoi(argv[3]);
    int numRacks = atoi(argv[4]);

    printf("Cooks [%d], Cashiers [%d], Customers [%d], Rack[%d]\n", numCooks, numCashiers, numCustomers, numRacks);
    printf("Begin run.\n\n");

    sem_init(&sem_cashier, 0, 0);
    sem_init(&sem_rack, 0, 0);
    sem_init(&sem_customer, 0, 0);
    sem_init(&sem_vacant, 0, numRacks);
    sem_init(&sem_customer_private, 0, 1);

    pthread_t cooks[numCooks], cashiers[numCashiers], customers[numCustomers];
    int ids[numCooks + numCashiers + numCustomers];

    for(int i = 0; i < numCooks; i++) {
        ids[i] = i + 1;
        pthread_create(&cooks[i], NULL, cook, &ids[i]);
    }

    for(int i = 0; i < numCashiers; i++) {
        ids[numCooks + i] = i + 1;
        pthread_create(&cashiers[i], NULL, cashier, &ids[numCooks + i]);
    }

    for(int i = 0; i < numCustomers; i++) {
        ids[numCooks + numCashiers + i] = i + 1;
        pthread_create(&customers[i], NULL, customer, &ids[numCooks + numCashiers + i]);
    }

    for (int i = 0; i < numCustomers; i++) {
        pthread_join(customers[i], NULL);
    }

    printf("\nAll customers have left, DONE.\n");

    for (int i = 0; i < numCashiers; ++i) {
        pthread_cancel(cashiers[i]);
    }

    for (int i = 0; i < numCooks; ++i) {
        pthread_cancel(cooks[i]);
    }

    sem_destroy(&sem_cashier);
    sem_destroy(&sem_rack);
    sem_destroy(&sem_customer);
    sem_destroy(&sem_vacant);
    sem_destroy(&sem_customer_private);
    pthread_mutex_destroy(&mutex);

    return 0;
}