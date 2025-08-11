// examples.c - примеры общения модулей с пользовательским проcтранством

// linux version = 6.14.0

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/rwlock.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/sysfs.h>

#define MSG_LEN 80              // Максимальная длина сообщения устройства
#define FILE_NAME "test"        // Имя файлов, как оно показано в /dev, /proc, /sys
static char msg[MSG_LEN] = {0}; // msg, которое устройство будет выдавать при запросе
static DECLARE_RWSEM(sem);      // Блокировка чтения/записи
// device file
static int major; // Старший номер, присвоенный драйверу устройства
static struct class *cls;
// proc
static struct proc_dir_entry *proc_file;
// sys
static struct kobject *mymodule;

// Чтение из ядра в пользовательское пространство

// root:
// cat /dev/test
static ssize_t file_read(struct file *filp,   /* см. include/linux/fs.h */
                         char __user *buffer, /* буфер для данных. */
                         size_t length,       /* длина буфера. */
                         loff_t *offset)
{
    int rc = 0;
    down_read(&sem);
    rc = simple_read_from_buffer(buffer, length, offset, msg, MSG_LEN);
    pr_info("Read %s\n", msg);
    up_read(&sem);
    return rc;
}

// root:
// cat /proc/test
static ssize_t procfile_read(struct file *filp, char __user *buffer,
                             size_t length, loff_t *offset)
{
    int rc = 0;
    down_read(&sem);
    rc = simple_read_from_buffer(buffer, length, offset, msg, MSG_LEN);
    pr_info("Read proc %s\n", msg);
    up_read(&sem);
    return rc;
}

// root:
// cat /sys/kernel/mymodule/msg
static ssize_t sysfile_show(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf)
{
    int rc = 0;
    down_read(&sem);
    memcpy(buf, msg, MSG_LEN);
    rc = strlen(buf);
    pr_info("Read sys %s\n", msg);
    up_read(&sem);
    return rc;
}

// Запись из пользовательского пространства в ядро

// root:
// echo 'Hi' > /dev/test
static ssize_t file_write(struct file *filp, const char __user *buff,
                          size_t size, loff_t *off)
{
    size_t rc = 0;

    if (size > MSG_LEN)
    {
        pr_alert("Buffer size is larger than allowed.\n");
        return -EINVAL;
    }

    down_write(&sem);
    memset(msg, 0, MSG_LEN);
    rc = simple_write_to_buffer(msg, MSG_LEN, off, buff, size);
    pr_info("Write %s\n", msg);
    up_write(&sem);

    return rc;
}

// root:
// echo 'Hi' > /proc/test
static ssize_t procfile_write(struct file *file, const char __user *buff,
                              size_t size, loff_t *off)
{
    size_t rc = 0;

    if (size > MSG_LEN)
    {
        pr_alert("Buffer size is larger than allowed.\n");
        return -EINVAL;
    }

    down_write(&sem);
    memset(msg, 0, MSG_LEN);
    rc = simple_write_to_buffer(msg, MSG_LEN, off, buff, size);
    pr_info("Write proc %s\n", msg);
    up_write(&sem);

    return rc;
}

// root:
// echo 'Hi' > /sys/kernel/mymodule/msg
static ssize_t sysfile_store(struct kobject *kobj,
                             struct kobj_attribute *attr, char *buf,
                             size_t count)
{
    size_t rc = 0;

    if (count > MSG_LEN)
    {
        pr_alert("Buffer size is larger than allowed.\n");
        return -EINVAL;
    }

    down_write(&sem);
    memset(msg, 0, MSG_LEN);
    memcpy(msg, buf, count);
    rc = strlen(buf);
    pr_info("Write sys %s\n", msg);
    up_write(&sem);
    return rc;
}

// device file
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = file_read,
    .write = file_write,
};
// proc
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};
// sys
static struct kobj_attribute myvariable_attribute =
    __ATTR(msg, 0660, sysfile_show, (void *)sysfile_store);

static int __init device_init(void)
{
    pr_info("Module device_file loaded.\n");

    major = register_chrdev(0, FILE_NAME, &fops);
    if (major < 0)
    {
        pr_alert("Registering char device failed with %d\n", major);
        return major;
    }
    pr_info("I was assigned major number %d.\n", major);

    // device file
    cls = class_create(FILE_NAME);
    if (IS_ERR(cls))
    {
        pr_alert("Failed to create class\n");
        unregister_chrdev(major, FILE_NAME);
        return PTR_ERR(cls);
    }

    struct device *dev = device_create(cls, NULL, MKDEV(major, 0), NULL, FILE_NAME);
    pr_info("Device created on /dev/%s\n", FILE_NAME);
    if (IS_ERR(dev))
    {

        device_destroy(cls, MKDEV(major, 0));
        pr_alert("Device creation /dev/%s error\n", FILE_NAME);
        return -EINVAL;
    }

    // proc
    proc_file = proc_create(FILE_NAME, 0644, NULL, &proc_file_fops);
    if (NULL == proc_file)
    {
        proc_remove(proc_file);
        pr_alert("Error: Could not initialize /proc/%s\n", FILE_NAME);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", FILE_NAME);

    // sys
    int error = 0;
    mymodule = kobject_create_and_add("mymodule", kernel_kobj);
    if (!mymodule)
    {
        pr_alert("Failed to create and register structure in /sys\n");
        return -ENOMEM;
    }
    error = sysfs_create_file(mymodule, &myvariable_attribute.attr);
    if (error)
    {
        pr_info("Failed to create the myvariable file in /sys/kernel/mymodule\n");
        return error;
    }

    return 0;
}
static void __exit device_exit(void)
{
    // device file
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    /* Отмена регистрации устройства. */
    unregister_chrdev(major, FILE_NAME);

    // proc
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", FILE_NAME);

    // sys
    kobject_put(mymodule);

    pr_info("Module device_file unloaded.\n");
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");