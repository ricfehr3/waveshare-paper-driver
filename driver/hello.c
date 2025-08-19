#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/spi/spi.h> 
#include <linux/of.h> 

static int epaper_probe(struct spi_device *spi);
static void epaper_remove(struct spi_device *spi);
static void epaper_shutdown(struct spi_device *spi);

static const struct of_device_id epaper_of_match[] = {
    { .compatible = "epapervendor,epaperdev" },
    { /*  /(6_6 )\  */}
};
MODULE_DEVICE_TABLE(of, epaper_of_match);

static const struct spi_device_id epaper_id_table[] = {
    { "epaperdev", 0 },
    { /*  /(6_6 )\  */}
};

MODULE_DEVICE_TABLE(spi, epaper_id_table);

static struct spi_driver epaper_driver =  {
    .id_table = epaper_id_table,
    .probe = epaper_probe,
    .remove = epaper_remove,
    .shutdown = epaper_shutdown,
    .driver = {
        .name = "epaper",
        .of_match_table = of_match_ptr(epaper_of_match),
    },
};

static int epaper_probe(struct spi_device *spi) {
    printk("Epaper Probe\n"); 
    return 0;
}

static void epaper_remove(struct spi_device *spi) {
    pr_info("Epaper Remove\n"); 
}

static void epaper_shutdown(struct spi_device *spi) {
    pr_info("Epaper Shutdown\n"); 
}

static int __init epaper_init(void) { 
    pr_info("Epaper Init\n"); 
    return spi_register_driver(&epaper_driver);
} 
 
static void __exit epaper_exit(void) { 
    pr_info("Epaper Exit\n"); 
    spi_unregister_driver(&epaper_driver);
} 
 
module_init(epaper_init); 
module_exit(epaper_exit); 

MODULE_AUTHOR("Rickey Fehr <ric@rf3.xyz>"); 
MODULE_LICENSE("GPL");
