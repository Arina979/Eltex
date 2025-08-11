// 2_matrix.c - задачи по работе с матрицами

// gcc 2_matrix.c -o matrix

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

//вывести квадратную матрицу по заданному N
void f1()
{
    int N;
    printf("Enter the size of the matrix: ");
    scanf("%d", &N);

    int cur = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            cur++;
            printf("%5d", cur);
        }
        printf("\n");
    }
}

//вывести заданный массив размером N в обратном порядке
void f2()
{
    int *a;
    int n = 0;

    printf("Enter size masive: ");
    scanf("%d", &n);

    a = (int *)malloc(n * sizeof(int));
    if(a == 0)
    {
        printf("Allocation Failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++)
    {
        printf("a[%d] = ", i);
        scanf("%d", &a[i]);
    }

    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);

    printf("\n");

    for (int i = n - 1; i >= 0; i--)
        printf("%d ", a[i]);

    printf("\n");

    free(a);
}

//заполнить верхний треугольник матрицы 1, а нижний 0
void f3()
{
    int n;

    printf("Enter the size of the matrix: ");
    scanf("%d", &n);

    int **a = (int **)malloc(n * sizeof(int *));
    if(a == 0)
    {
        printf("Allocation Failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
    {
        a[i] = (int *)malloc(n * sizeof(int));
        if(a[i] == 0)
        {
            printf("Allocation Failed");
            exit(EXIT_FAILURE);
        }
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

//заполнить матрицу числами от 1 до N^2 улиткой
void f4()
{
    int n;

    printf("Enter the size of the matrix: ");
    scanf("%d", &n);

    int **a = (int **)malloc(n * sizeof(int *));
    if(a == 0)
    {
        printf("Allocation Failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
    {
        a[i] = (int *)malloc(n * sizeof(int));
        if(a[i] == 0)
        {
            printf("Allocation Failed");
            exit(EXIT_FAILURE);
        }
    }

    if (n % 2 != 0)
    {
        a[(n / 2)][(n / 2)] = (n * n); // центр матрицы
    }

    int cur = 1;
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
            a[i][j] = cur;
            cur++;
        }
        j--;
        i++;
        for (i; i < max; i++) //i+
        {
            a[i][j] = cur;
            cur++;
        }
        i--;
        j--;
        for (j; j >= min; j--) //j-
        {
            a[i][j] = cur;
            cur++;
        }
        j++;
        i--;
        for (i; i > min; i--) //i-
        {
            a[i][j] = cur;
            cur++;
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
    //f1(); //вывести квадратную матрицу по заданному N
    //f2(); //вывести заданный массив размером N в обратном порядке
    //f3(); //заполнить верхний треугольник матрицы 1, а нижний 0
    f4(); //заполнить матрицу числами от 1 до N^2 улиткой
}