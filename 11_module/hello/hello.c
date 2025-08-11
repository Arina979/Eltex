// hello.c – простейший модуль ядра.

// sudo make
// sudo insmod hello.ko
// sudo dmesg || tail -n5
// sudo rmmod hello

#include <linux/kernel.h> /* необходим для pr_info() */
#include <linux/module.h> /* необходим для всех модулей */

int init_module(void)
{
    pr_info("Hello world.\n");
    /* Если вернётся не 0, значит, init_module провалилась; модули загрузить не
    получится. */
    return 0;
}
void cleanup_module(void)
{
    pr_info("Goodbye world.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("I");
MODULE_DESCRIPTION("Hello");