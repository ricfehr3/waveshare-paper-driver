#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/spi/spi.h> 
#include <linux/of.h> 
#include <linux/delay.h> 
#include <linux/gpio/consumer.h>

const unsigned char lut_full_update[] =
{
    0x22, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x11, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct epaper_dev_data {
    struct spi_device *spi;
    struct gpio_desc  *dc;
    struct gpio_desc  *res;
    struct gpio_desc  *busy;
};

static int epaper_write_data(struct epaper_dev_data *m, u8 data);
static int epaper_write_cmd(struct epaper_dev_data *m, u8 cmd);
static int epaper_probe(struct spi_device *spi);
static void epaper_remove(struct spi_device *spi);
static void epaper_shutdown(struct spi_device *spi);

static const struct of_device_id epaper_of_match[] = {
    { .compatible = "waveshare,epaperdev" },
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


void epaper_write_lut(struct epaper_dev_data *m) {
    //SendCommand(WRITE_LUT_REGISTER);
    epaper_write_cmd(m, 0x32);
    /* the length of look-up table is 30 bytes */
    for (int i = 0; i < 30; i++) {
        epaper_write_data(m, lut_full_update[i]);
    }
}


void epaper_set_cursor(
		struct epaper_dev_data *m,
		unsigned char x_start, 
		unsigned char y_start)
{
    epaper_write_cmd(m, 0x4E); // SET_RAM_X_ADDRESS_COUNTER
    epaper_write_data(m, x_start & 0xFF);
    //epaper_write_data(m, (x_start >> 3) & 0xFF); ????

    epaper_write_cmd(m, 0x4F); // SET_RAM_Y_ADDRESS_COUNTER
    epaper_write_data(m, y_start & 0xFF);
    epaper_write_data(m, (y_start >> 8) & 0xFF);
}


void epaper_set_windows(
		struct epaper_dev_data *m,
		unsigned char x_start, 
		unsigned char y_start, 
		unsigned char x_end, 
		unsigned char y_end)
{
    epaper_write_cmd(m, 0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
    epaper_write_data(m, (x_start>>3) & 0xFF);
    epaper_write_data(m, (x_end>>3) & 0xFF);
	
    epaper_write_cmd(m, 0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
    epaper_write_data(m, y_start & 0xFF);
    epaper_write_data(m, (y_start >> 8) & 0xFF);
    epaper_write_data(m, y_end & 0xFF);
    epaper_write_data(m, (y_end >> 8) & 0xFF);
}

static int wait_for_busy_low(struct gpio_desc *desc, unsigned int timeout_ms)
{
	unsigned long deadline = jiffies + msecs_to_jiffies(timeout_ms);

	for (;;) {
		if (gpiod_get_value_cansleep(desc) == 0)
			return 0;   /* success */

		if (time_after(jiffies, deadline))
			return -ETIMEDOUT;

		msleep(1); /* small sleep to avoid busy loop */
	}
}

static void epaper_busy_wait(struct epaper_dev_data *m) {
	int ret = wait_for_busy_low(m->busy, 5000);
	pr_info("Epaper busy wait %d\n", ret);
}

static int epaper_write_data(struct epaper_dev_data *m, u8 data)
{
	gpiod_set_value(m->dc, 1);	

	struct spi_transfer t = {
		.tx_buf = &data,
		.len    = sizeof(data),
	};

	return spi_sync_transfer(m->spi, &t, 1);
}

static int epaper_write_cmd(struct epaper_dev_data *m, u8 cmd)
{
	gpiod_set_value(m->dc, 0);

	struct spi_transfer t = {
		.tx_buf = &cmd,
		.len    = sizeof(cmd),
	};
	return spi_sync_transfer(m->spi, &t, 1);
}

static int epaper_read_reg(struct epaper_dev_data *m, u8 cmd, u8* val)
{
	int ret = 0;

	ret = epaper_write_cmd(m, cmd);
	if(ret) return ret;

	gpiod_set_value(m->dc, 1);	

	struct spi_transfer t = {
		.rx_buf = val,
		.len    = sizeof(*val),
	};
	return spi_sync_transfer(m->spi, &t, 1);
}

static int set_lut(struct epaper_dev_data *data) {
    static const unsigned char lut[] =
    {
        0x22, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x11, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    int ret = 0;

    ret = epaper_write_cmd(data, 32); //WRITE_LUT_REGISTER
    if(ret != 0) return ret;

    for (int i = 0; i < 30; i++) 
    {
        ret = epaper_write_data(data, lut[i]);
	if(ret != 0) break;
    }

    return ret;
}

static int epaper_probe(struct spi_device *spi) {
    printk("Epaper Probe\n");

    struct epaper_dev_data *data;
    int ret = 0;

    data = devm_kzalloc(&spi->dev, sizeof(*data), GFP_KERNEL);
    if(!data) {
	pr_info("Epaper Alloc Fail\n");
        return -ENOMEM;
    }

    pr_info("Epaper Allocated\n");

    data->spi = spi;

    data->dc = devm_gpiod_get(&spi->dev, "dc", GPIOD_OUT_LOW);
    data->res = devm_gpiod_get(&spi->dev, "res", GPIOD_OUT_LOW);
    data->busy = devm_gpiod_get(&spi->dev, "busy", GPIOD_IN);

    int wait = 500;

    if (IS_ERR(data->dc))
	return dev_err_probe(&spi->dev, PTR_ERR(data->dc), "Failed to get DC GPIO\n");
    if (IS_ERR(data->res))
	return dev_err_probe(&spi->dev, PTR_ERR(data->res), "Failed to get RES GPIO\n");
    if (IS_ERR(data->busy))
	return dev_err_probe(&spi->dev, PTR_ERR(data->busy), "Failed to get BUSY GPIO\n");

    // TODO: Reset is active low, make sure this makes sense
    gpiod_set_value(data->res, 1);
    msleep(wait);
    gpiod_set_value(data->res, 0);
    msleep(wait);
    gpiod_set_value(data->res, 1);
    msleep(wait);


    pr_info("Epaper Gpio DC Good\n");

    spi_set_drvdata(spi, data);

    spi->mode = SPI_MODE_0;
    spi->bits_per_word = 8;
    spi->max_speed_hz = 100000;
    //spi->max_speed_hz = 20000;

    ret = spi_setup(spi);
    if (ret) {
	dev_err(&spi->dev, "spi_setup failed: %d\n", ret);
	return ret;
    }

    pr_info("Epaper Allocated\n");

    //unsigned EPD_HEIGHT = 122;
    //unsigned EPD_WIDTH = 250;
    unsigned EPD_HEIGHT = 250;
    unsigned EPD_WIDTH = 122;

    //ret = epaper_write_reg(data, 0x01, 0xA5);
    msleep(10);
    u8 val;
    ret = epaper_write_cmd(data, 0x12);
    epaper_busy_wait(data);
    ret = epaper_write_cmd(data, 0x01);
    ret = epaper_write_data(data, 0xF9);
    ret = epaper_write_data(data, 0x00);
    ret = epaper_write_data(data, 0x00);

    ret = epaper_write_cmd(data, 0x11);
    ret = epaper_write_data(data, 0x03);

    epaper_set_windows(data, 0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epaper_set_cursor(data, 0, 0);

    ret = epaper_write_cmd(data, 0x22); // Load temperature value
    ret = epaper_write_data(data, 0xB1);	
    ret = epaper_write_cmd(data, 0x20);
    epaper_busy_wait(data);

    ret = epaper_write_cmd(data, 0x1A); //  Write to temperature register
    ret = epaper_write_data(data, 0x64);
    ret = epaper_write_data(data, 0x00);	

    ret = epaper_write_cmd(data, 0x22); //  Load temperature value
    ret = epaper_write_data(data, 0x91);
    ret = epaper_write_cmd(data, 0x20);	
    epaper_busy_wait(data);


    // BorderWaveform
    ret = epaper_write_cmd(data, 0x3C);
    ret = epaper_write_data(data, 0x05);

    // Display update control
    ret = epaper_write_cmd(data, 0x21);
    ret = epaper_write_data(data, 0x00);
    ret = epaper_write_data(data, 0x80);

    epaper_write_lut(data);
    epaper_busy_wait(data);

    // Read Build in temperature sensor
    //ret = epaper_write_cmd(data, 0x18);
    //ret = epaper_write_data(data, 0x80);
    //epaper_busy_wait(data);

    // CLEAR
    int w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    int h = EPD_HEIGHT;
    ret = epaper_write_cmd(data, 0x24);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
		if(i ^ j) {
		    epaper_write_data(data, 0xff);
		} else {
		    epaper_write_data(data, 0x00);
		}
	    //epaper_write_data(data, 0x00);
        }
    }
    epaper_busy_wait(data);

    //DISPLAY REFRESH
    ret = epaper_write_cmd(data, 0x22);
    ret = epaper_write_data(data, 0xf7);
    ret = epaper_write_cmd(data, 0x20);
    epaper_busy_wait(data);
    //ret = epaper_
    //epaper_read_reg(data, 0x1B, &val);
    //pr_info("Epaper temp register read %d\n", (int)val);

    //ret = epaper_write_cmd(data, 0x12);
	//if (ret)
		//return dev_err_probe(&spi->dev, ret, "init write failed\n");

    /*
    ret = epaper_write_cmd(data, 0x12);
    msleep(500);

    ret = epaper_write_cmd(data, 0x01);
    ret = epaper_write_data(data, 0xF9);
    ret = epaper_write_data(data, 0x00);
    ret = epaper_write_data(data, 0x00);
    
    ret = epaper_write_cmd(data, 0x11);
    ret = epaper_write_data(data, 0x03);

    ret = epaper_write_cmd(data, 0x3C);
    ret = epaper_write_data(data, 0x05);

    ret = epaper_write_cmd(data, 0x21);
    ret = epaper_write_data(data, 0x00);
    ret = epaper_write_data(data, 0x80);

    ret = epaper_write_cmd(data, 0x18);
    ret = epaper_write_data(data, 0x80);
    msleep(500);
    */
    /*
    ret = epaper_write_data(data, (EPD_HEIGHT - 1) & 0xFF);
    ret = epaper_write_data(data, ((EPD_HEIGHT - 1) >> 8) & 0xFF);
    ret = epaper_write_data(data, 0x00);
    ret = epaper_write_cmd(data, 0x0C);
    ret = epaper_write_data(data, 0xD7);
    ret = epaper_write_data(data, 0xD6);
    ret = epaper_write_data(data, 0x9D);
    ret = epaper_write_cmd(data, 0x2C);
    ret = epaper_write_data(data, 0xA8);
    ret = epaper_write_cmd(data, 0x3A); // SET_DUMMY_LINE_PERIOD
    ret = epaper_write_data(data, 0x1A);
    ret = epaper_write_cmd(data, 0x3B); // SET_GATE_TIME
    ret = epaper_write_data(data, 0x08);
    ret = epaper_write_cmd(data, 0x11); // DATA_ENTRY_MODE_SETTING
    ret = epaper_write_data(data, 0x03);
    */
    //ret = set_lut(data);



    pr_info("Epaper All Good\n");

    return ret;
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
