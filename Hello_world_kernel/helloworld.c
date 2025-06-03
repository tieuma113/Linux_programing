#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KHANH_TIEUMA");
MODULE_DESCRIPTION("THIS IS A HELLO WORLD MODULE");


static int __init hello_init(void){
    printk(KERN_INFO "Hello world kernel\n");
    return 0;
}

static void __exit hello_exit(void){
    printk(KERN_INFO "Exit kernel Hello world\n");
    return;
}

module_init(hello_init);
module_exit(hello_exit);