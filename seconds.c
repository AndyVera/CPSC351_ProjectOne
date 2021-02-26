#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/hash.h>
#include <linux/gcd.h>
#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>


#define BUFFER_SIZE 128
#define PROC_NAME "seconds"

struct load_time {
        unsigned long strt;
        struct list_head list;
};

static LIST_HEAD(load_time_list);
void append_list(unsigned long time){
        struct load_time* a = kmalloc(sizeof(*a), GFP_KERNEL);
        a->strt = time;
        INIT_LIST_HEAD(&a->list);
        list_add_tail(&a->list, &load_time_list);
}

//--------------------------------------------------------------------------
ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);

static struct file_operations proc_ops = {
        .owner = THIS_MODULE,
        .read = proc_read,
};

int proc_init(void) {
        unsigned long a = jiffies;
        append_list(a);
        proc_create(PROC_NAME, 0, NULL, &proc_ops);
        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
        //printk(KERN_INFO "In proc_init(), jiffies is: %lu\n", jiffies);
        return 0;
}

void proc_exit(void) {
        remove_proc_entry(PROC_NAME, NULL);
        printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
        struct load_time* ptr;
        unsigned long begin= 0;
        list_for_each_entry(ptr, &load_time_list, list){
                if(ptr->strt != 0){
                        begin = ptr->strt;
                }
        }

        int rv = 0;

        char buffer[BUFFER_SIZE];
        static int completed = 0;

        if (completed) {
                completed = 0;
                return 0;
        }

        completed = 1;
        
        rv = sprintf(buffer, "Number of seconds elapse since the module was loaded are: %lu seconds\n", (jiffies-begin)/HZ);
        copy_to_user(usr_buf, buffer, rv);

        return rv;
}

module_init( proc_init );
module_exit( proc_exit );

//=========== insren.sh code ============================

/*
sudo -p "osc"
sudo dmesg -c 
clear

printf "beginning script...\n\n"
sudo insmod seconds.ko
sudo dmesg
printf "\n"

sleep 5
cat /proc/seconds
sleep 5
cat /proc/seconds
sleep 5
cat /proc/seconds

printf "\n"
sudo rmmod seconds
sudo dmesg
printf "\n...end of script\n\n"
*/
//======================================================

//====================== OUTPUT ========================
/*
beginning script...


[ 1405.133591] /proc/seconds created

Number of seconds elapse since the module was loaded are: 5 seconds
Number of seconds elapse since the module was loaded are: 10 seconds
Number of seconds elapse since the module was loaded are: 15 seconds

[ 1405.133591] /proc/seconds created
[ 1420.183942] /proc/seconds removed

...end of script
*/
//================= END OF OUTPUT =======================
