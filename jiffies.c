#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/hash.h>
#include <linux/gcd.h>
#include <linux/jiffies.h>

#define BUFFER_SIZE 128
#define PROC_NAME "jiffies"

ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);

static struct file_operations proc_ops = {
        .owner = THIS_MODULE,
        .read = proc_read,
};

int proc_init(void) {
        proc_create(PROC_NAME, 0, NULL, &proc_ops);
        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
        return 0;
}

void proc_exit(void) {
        remove_proc_entry(PROC_NAME, NULL);
        printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
        int rv = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0;

        if (completed) {
                completed = 0;
                return 0;
        }

        completed = 1;
        rv = sprintf(buffer, "When /proc/jiffies is read, Jiffies is: %lu\n", jiffies);

        // copies the contents of buffer to userspace usr_buf
        copy_to_user(usr_buf, buffer, rv);

        return rv;
}


/* Macros for registering module entry and exit points. */
module_init( proc_init );
module_exit( proc_exit );

//=========== insren.sh code ============================

/*
sudo -p "osc"
sudo dmesg -c 
clear
printf "beginning script...\n\n"
sudo insmod jiffies.ko
sudo dmesg
printf "\n"

cat /proc/jiffies
sleep 1
cat /proc/jiffies
sleep 1
cat /proc/jiffies

printf "\n"

sudo dmesg -c
sudo rmmod jiffies 
sudo dmesg 
printf "\n...end of script\n\n"
*/
//======================================================

//====================== OUTPUT ========================
/*
beginning script...

[ 1656.027260] /proc/jiffies created

When /proc/jiffies is read, Jiffies is: 4295306287
When /proc/jiffies is read, Jiffies is: 4295306553
When /proc/jiffies is read, Jiffies is: 4295306804

[ 1656.027260] /proc/jiffies created
[ 1658.126202] /proc/jiffies removed

...end of script
*/
//================= END OF OUTPUT =======================
