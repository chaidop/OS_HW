#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/sched.h>

static struct kobject *example_kobject;
volatile int roots = 0;

static ssize_t find_roots_syscall(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    printk("find_roots system call called by process %d\n", current->pid);
	pid_t pp;
	pp = current->pid;
	struct task_struct * cc = current;
	for( ; ; cc = cc->real_parent){	
		printk("id: %d, name: %s\n",cc->pid, cc->comm);
		if(cc->pid == 1){
			break;
		}
	}
    return sprintf(buf, "%d\n",pp);
}

struct kobj_attribute foo_attribute = __ATTR(find_roots, 0660, find_roots_syscall, NULL);

static int __init mymodule_init (void){
    int error = 0;
    example_kobject = kobject_create_and_add("team13", kernel_kobj);
    if(!example_kobject)
        return -ENOMEM;
    error = sysfs_create_file(example_kobject, &foo_attribute.attr);
    if(error){
        printk("failed to create the foo file in /sys/kernel/kobject_example \n");
    }
    return error;
}

static void __exit mymodule_exit(void)
{
    printk("Module uninitialized successfully \n");
    kobject_put(example_kobject);
}

module_init(mymodule_init);
module_exit(mymodule_exit);
MODULE_LICENSE("GPL");

