// 1_bytes.c - заменить и вывести байты числа типа int от младшего к старшему, используя битовые операции и указатели

// gcc 1_bytes.c -o bytes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SIZE_INT 11

//вывести байты числа типа int от младшего к старшему, используя битовые операции
void printBytesUseBO(int value)
{
	printf("\nFunction: %s\n", __FUNCTION__);

	printf("value                    : %d\n", value);
	printf("value hex                : %x\n", value);

	unsigned char bytes[4] = {0};
	bytes[0] =  value & 0x000000ff;
	bytes[1] = (value & 0x0000ff00) >> 8;
	bytes[2] = (value & 0x00ff0000) >> 16;
	bytes[3] = (value & 0xff000000) >> 24;

	printf("Bytes hex ascending order: %x %x %x %x\n", bytes[0], bytes[1], bytes[2], bytes[3]);

	return;
}

//заменить выбранный байт на введенное число, используя битовые операции
int replaceByteUseBO(int value, unsigned char new_value, unsigned char byte_num)
{
	printf("\nFunction: %s\n", __FUNCTION__);

	printf("value             : %d\n", value);
	printf("value hex         : %x\n", value);
	printf("new_value         : %d\n", new_value);
	printf("new_value hex     : %x\n", new_value);
	printf("byte_num          : %d\n", byte_num);

	if(byte_num < 0 || byte_num > 255)
	{
		printf("Error: new value from 0 to 255\n");
		return -1;
	}
	if(byte_num < 0 || byte_num > 3)
	{
		printf("Error: byte number from 0 to 3\n");
		return -1;
	}

	int shift = 8 * byte_num;
	int mask = 0xff << shift;
	return (~mask & value) | (new_value << shift);
}

//вывести байты числа типа int от младшего к старшему, используя указатели
void printBytesUsePtr(int value)
{
	printf("\nFunction: %s\n", __FUNCTION__);

	printf("value                    : %d\n", value);
	printf("value hex                : %x\n", value);

    unsigned char *byte = (unsigned char *)&value;

	unsigned char bytes[4] = {0};
	bytes[0] = *byte;
	bytes[1] = *(++byte);
	bytes[2] = *(++byte);
	bytes[3] = *(++byte);

	printf("Bytes hex ascending order: %x %x %x %x\n", bytes[0], bytes[1], bytes[2], bytes[3]);

    return;
}

//заменить выбранный байт на введенное число, используя указатели
int replaceByteUsePtr(int value, unsigned char new_value, unsigned char byte_num)
{
	printf("\nFunction: %s \n", __FUNCTION__);

	printf("value             : %d\n", value);
	printf("value hex         : %x\n", value);
	printf("new_value         : %d\n", new_value);
	printf("new_value hex     : %x\n", new_value);
	printf("byte_num          : %d\n", byte_num);

	if(byte_num < 0 || byte_num > 255)
	{
		printf("Error: new value from 0 to 255\n");
		return -1;
	}
	if(byte_num < 0 || byte_num > 3)
	{
		printf("Error: byte number from 0 to 3\n");
		return -1;
	}

    unsigned char *byte = (unsigned char *)&value;
	*(byte + byte_num) = new_value;

	return value;
}

int inputInt()
{
    char buffer[SIZE_INT] = {0};
    char *copy = NULL;
    int num = 0;
    bool succsess = false;
    while(!succsess)
    {
        fgets(buffer, SIZE_INT, stdin);
        if (strchr(buffer,'\n') == NULL)
        {
            while ((getchar()) != '\n');

            if(buffer[0] != '+' && buffer[0] != '-')
            {
                buffer[0] == ' ';//переполнение
            }
        }

        copy = strtok(buffer,"\n");

        char *endptr;
        num = strtol(copy, &endptr, 10);
        if (endptr == copy)
        {
            printf("No digits were found\n");
        } 
        else if (*endptr != '\0')
        {
            printf("Invalid character: %c\n", *endptr);
        }
        else
        {
            succsess = true;
        }
    }
    return num;
}
// int inputInt()
// {
//     char buffer[SIZE_INT] = {0};
//     fgets(buffer, SIZE_INT, stdin); 
//     int num = atoi(buffer);
//     if (strchr(buffer,'\n') == NULL)
//     {
//         while ((getchar()) != '\n');
//     }
//     return num;
// }

int main()
{
	int value = 0;
	unsigned char new_value = 0;
	unsigned char byte_num = 0;

	char buffer[255] = {0};

    printf("Enter value: ");
	value = inputInt();
	printBytesUseBO(value);

    printf("\nEnter new byte value from 0 to 255: ");
	new_value = inputInt();

    printf("Enter the byte number to change from 0 to 3: ");
	byte_num = inputInt();
	
	//с помощью битовых операций
	int ret1 = replaceByteUseBO(value, new_value, byte_num);
	printBytesUseBO(ret1);

	//с помощью указателей
	int ret2 = replaceByteUsePtr(value, new_value, byte_num);
	printBytesUsePtr(ret2);
}