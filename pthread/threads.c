#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_spinlock_t lock;

void *customer(void *args)
{
    pthread_t seed = pthread_self();
    pthread_t copy = seed;

    int *shops = (int *)args;
    int demand = 0;

    while (demand < 10000)
    {
        int i = rand_r((unsigned int *)&copy) % 4;
        pthread_spin_lock(&lock);
        demand += shops[i];
        shops[i] = 0;
        printf("cust %lu shop %d: %d\n", seed, i, demand);
        pthread_spin_unlock(&lock);
        sleep(2);
    }
    printf("cust %lu %d\n", seed, demand);
    pthread_exit(0);
}

void *provider(void *args)
{
    pthread_t seed = pthread_self();
    pthread_t copy = seed;

    int *shops = (int *)args;

    while (1)
    {
        int i = rand_r((unsigned int *)&copy) % 4;
        pthread_spin_lock(&lock);
        shops[i] += 200;
        printf("prov %lu shop %d: %d\n", seed, i, shops[i]);
        pthread_spin_unlock(&lock);
        sleep(1);
    }
    pthread_exit(0);
}

int main()
{
    int shops[5];
    int seed = 0;

    for (int i = 0; i < 5; i++)
    {
        shops[i] = rand_r(&seed) % 100 + 900;
        printf("rand %d: %d\n", i, shops[i]);
    }

    pthread_t cust[3];
    pthread_t prov;

    int ret;
    ret = pthread_spin_init(&lock, 0);
    if (ret != 0)
    {
        perror("pthread_spin_init");
        exit(1);
    }

    for (int k = 0; k < 3; k++)
        pthread_create(&cust[k], NULL, &customer, shops);

    pthread_create(&prov, NULL, &provider, shops);

    for (int k = 0; k < 3; k++)
        pthread_join(cust[k], NULL);

    pthread_cancel(prov);
    pthread_join(prov, NULL);

    pthread_spin_destroy(&lock);

    exit(0);
}
