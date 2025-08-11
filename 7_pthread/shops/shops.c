// shops.c - программа

// 5 магазинов. В них загружается случайное количество товара от 950 до 1050.
// Одномоментно в магазине может находится только один покупатель или погрузчик.
// 3 покупателя. Они имеют потребность покупателя от 4500 до 5500.
// Покупатель заходят в магазин и покупают там все, что есть. После этого он засыпает на 2 секунды.
// 1 погрузчик. Загружает в магазин 500 единиц товара. После этого он засыпает на 1 секунду.
// Цикл повторяется, пока покупатели не насититься.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define SHOPS 5
#define CUSTOMERS 3

struct shop
{
    pthread_spinlock_t lock;
    //pthread_mutex_t mut;
    int sum;
};

int rnd(int *seed, int min, int max)
{
    return min + rand_r(seed) / (RAND_MAX / (max - min + 1) + 1); // потокобезопасная
}

void *customer(void *args)
{
    struct shop *shops = (struct shop *)args;
    pthread_t pid = pthread_self();
    unsigned int seed = pid;
    int demand = rnd(&seed, 4500, 5500);
    printf("cust %ld, demand %d\n", pid, demand);

    while (demand > 0)
    {
        int i = rnd(&seed, 0, SHOPS - 1);
        pthread_spin_lock(&shops[i].lock);
        //pthread_mutex_lock(&shops[i].mut);
        demand -= shops[i].sum;
        printf("cust %ld, demand, %d shop %d: %d\n", pid, demand, i, shops[i].sum);
        shops[i].sum = 0;
        pthread_spin_unlock(&shops[i].lock);
        //pthread_mutex_unlock(&shops[i].mut);
        sleep(2);
    }
    printf("cust %ld closed\n", pid);
    pthread_exit(0);
}

void *provider(void *args)
{
    struct shop *shops = (struct shop *)args;
    pthread_t pid = pthread_self();
    unsigned int seed = pid;

    while (1)
    {
        int i = rnd(&seed, 0, SHOPS - 1);
        pthread_spin_lock(&shops[i].lock);
        //pthread_mutex_lock(&shops[i].mut);
        shops[i].sum += 500;
        printf("prov %ld, shop %d: %d\n", pid, i, shops[i].sum);
        pthread_spin_unlock(&shops[i].lock);
        //pthread_mutex_unlock(&shops[i].mut);
        sleep(1);
    }
    pthread_exit(0);
}

int main()
{
    int seed = time(NULL);
    struct shop shops[SHOPS];

    for (int i = 0; i < SHOPS; i++)
    {
        shops[i].sum = rnd(&seed, 950, 1050);
        if (pthread_spin_init(&shops[i].lock, 0) != 0)
        {
            perror("pthread_spin_init");
            exit(1);
        }
        // if (pthread_mutex_init(&shops[i].mut, NULL) == -1) 
        // {                                  
        //     perror("mutex_init error");                                                 
        //     exit(1);                                                                    
        // }
        printf("shop %d: %d\n", i, shops[i].sum);
    }

    pthread_t cust[CUSTOMERS];
    for (int i = 0; i < CUSTOMERS; i++)
    {
        pthread_create(&cust[i], NULL, &customer, &shops);
    }
    pthread_t prov;
    pthread_create(&prov, NULL, &provider, &shops);


    for (int i = 0; i < CUSTOMERS; i++)
    {
        pthread_join(cust[i], NULL);

    }
    pthread_cancel(prov);
    pthread_join(prov, NULL);

    for (int i = 0; i < CUSTOMERS; i++)
        pthread_spin_destroy(&shops[i].lock);

    // for (int i = 0; i < SHOPS; i++)
    //     pthread_mutex_destroy(&shops[i].mut);

    return 0;
}
