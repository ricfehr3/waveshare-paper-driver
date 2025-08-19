#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/spi/spi.h> 
#include <linux/of.h> 

static int hello_probe(struct spi_device *spi);
static void hello_remove(struct spi_device *spi);
static void hello_shutdown(struct spi_device *spi);

static const struct of_device_id hello_of_match[] = {
    { .compatible = "hellovendor,hellodev" },
    { /*  /(6_6 )\  */}
};
MODULE_DEVICE_TABLE(of, hello_of_match);

static const struct spi_device_id hello_id_table[] = {
    { "hellodev", 0 },
    { /*  /(6_6 )\  */}
};

MODULE_DEVICE_TABLE(spi, hello_id_table);

static struct spi_driver hello_driver =  {
    .id_table = hello_id_table,
    .probe = hello_probe,
    .remove = hello_remove,
    .shutdown = hello_shutdown,
    .driver = {
        .name = "hello",
        .of_match_table = of_match_ptr(hello_of_match),
    },
};

static int hello_probe(struct spi_device *spi) {
    printk("Hello Probe\n"); 
    return 0;
}

static void hello_remove(struct spi_device *spi) {
    pr_info("Hello Remove\n"); 
}

static void hello_shutdown(struct spi_device *spi) {
    pr_info("Hello Shutdown\n"); 
}

static int __init helloworld_init(void) { 
    pr_info("Nugget Biscuit!\n"); 
    return spi_register_driver(&hello_driver);
} 
 
static void __exit helloworld_exit(void) { 
    pr_info("Nugget and a Biscuit!\n"); 
    spi_unregister_driver(&hello_driver);
} 
 
module_init(helloworld_init); 
module_exit(helloworld_exit); 

MODULE_AUTHOR("Rickey Fehr <ric@rf3.xyz>"); 
MODULE_LICENSE("GPL");
