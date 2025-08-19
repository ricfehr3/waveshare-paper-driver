#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
 
static int __init helloworld_init(void) { 
    pr_info("Nugget Biscuit!\n"); 
    return 0; 
} 
 
static void __exit helloworld_exit(void) { 
    pr_info("Nugget and a Biscuit!\n"); 
} 
 
module_init(helloworld_init); 
module_exit(helloworld_exit); 
MODULE_AUTHOR("Rickey Fehr <ric@rf3.xyz>"); 
MODULE_LICENSE("GPL");
