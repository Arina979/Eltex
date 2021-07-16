#include <stdio.h>
#include <stdlib.h>

void f1()
{
    int N;
    printf("Enter the size of the matrix ");
    scanf("%d", &N);

    int tek = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            tek++;
            printf("%5d", tek);
        }
        printf("\n");
    }
}

void f2()
{
    int *a;
    int n = 0;

    printf("Enter size masive ");
    scanf("%d", &n);

    a = (int *)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++)
    {
        printf("a[%d] = ", i);
        scanf("%d", &a[i]);
    }

    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);

    printf("\n");

    for (int i = 0; i < n / 2; i++)
    {
        int swap = a[i];
        a[i] = a[n - i - 1];
        a[n - i - 1] = swap;
    }

    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);

    free(a);
}

void f3()
{
    int n;

    printf("Enter the size of the matrix ");
    scanf("%d", &n);

    int **a = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        a[i] = (int *)malloc(n * sizeof(int));
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i + j >= n - 1)
                a[i][j] = 1;
            else
                a[i][j] = 0;
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%5d ", a[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < n; i++)
    {
        free(a[i]);
    }
    free(a);
}

void f4()
{
    int n;

    printf("Enter the size of the matrix ");
    scanf("%d", &n);

    int **a = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        a[i] = (int *)malloc(n * sizeof(int));
    }

    if (n % 2 != 0)
    {
        a[(n / 2)][(n / 2)] = (n * n); // центр матрицы
    }

    int tek = 1;
    int i = 0;
    int j = 0;
    a[i][j] = 0;

    int min = 0;
    int max = n;

    while (min < n/2)
    {
        i = min;
        j = min;
        for (j; j < max; j++) //j+
        {
            a[i][j] = tek;
            tek++;
        }
        j--;
        i++;
        for (i; i < max; i++) //i+
        {
            a[i][j] = tek;
            tek++;
        }
        i--;
        j--;
        for (j; j >= min; j--) //j-
        {
            a[i][j] = tek;
            tek++;
        }
        j++;
        i--;
        for (i; i > min; i--) //i-
        {
            a[i][j] = tek;
            tek++;
        }
        max--;
        min++;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%5d ", a[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < n; i++)
    {
        free(a[i]);
    }
    free(a);
}

int main()
{
    //f1();
    f2();
    //f3();
    //f4();
}