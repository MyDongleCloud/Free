/* Linux kernel driver for st7735s
 *
 * Copyright (C) 2019 by Always Innovating, Inc.
 * Author: Gregoire Gentil <gregoire@gentil.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/version.h>

#define WIDTH 128
#define HEIGHT 128
#define DEPTH 3
#define SCREENBUFFERSIZE WIDTH * HEIGHT * DEPTH

struct st7735sPriv {
	struct spi_device *spi;
	struct class *cls;
	struct device *dev;
	struct cdev cdev;
	struct work_struct work;
	struct spi_transfer spi_transfer;
	struct spi_message spi_message;
	unsigned char spiTx[WIDTH * HEIGHT * 2];
	int debug;
	int backlight;
	int wd;
	int nrst;
	int rotation;
	unsigned char *framebuffer;
	size_t framebuffersize;
};

static DECLARE_WAIT_QUEUE_HEAD(st7735s_wait);

//Functions
static void screenCommand(struct st7735sPriv *priv, unsigned char cmd) {
	gpio_set_value(priv->wd, 0);
	memset(&priv->spi_message, 0, sizeof(struct spi_message));
	memset(&priv->spi_transfer, 0, sizeof(struct spi_transfer));
	priv->spi_transfer.tx_buf = &cmd;
	priv->spi_transfer.len = 1;
	spi_message_init(&priv->spi_message);
	spi_message_add_tail(&priv->spi_transfer, &priv->spi_message);
	spi_sync(priv->spi, &priv->spi_message);
	gpio_set_value(priv->wd, 1);
}

static void screenData(struct st7735sPriv *priv, unsigned char data) {
	memset(&priv->spi_message, 0, sizeof(struct spi_message));
	memset(&priv->spi_transfer, 0, sizeof(struct spi_transfer));
	priv->spi_transfer.tx_buf = &data;
	priv->spi_transfer.len = 1;
	spi_message_init(&priv->spi_message);
	spi_message_add_tail(&priv->spi_transfer, &priv->spi_message);
	spi_sync(priv->spi, &priv->spi_message);
}

static void init_(struct st7735sPriv *priv) {
	printk("MyDongle-ST7735S: screenInit\n");

	screenCommand(priv, 0x11); //Sleep out
	screenCommand(priv, 0x11); //Sleep out
	msleep(50);
	screenCommand(priv, 0xB1);
	screenData(priv, 0x01);
	screenData(priv, 0x2c);
	screenData(priv, 0x2d);
	screenCommand(priv, 0xB2);
	screenData(priv, 0x02);
	screenData(priv, 0x35);
	screenData(priv, 0x36);
	screenCommand(priv, 0xB3);
	screenData(priv, 0x01);
	screenData(priv, 0x2c);
	screenData(priv, 0x2d);
	screenData(priv, 0x01);
	screenData(priv, 0x2c);
	screenData(priv, 0x2d);
	screenCommand(priv, 0xB4); //Dot inversion
	screenData(priv, 0x03);
	screenCommand(priv, 0xC0);
	screenData(priv, 0xa2);
	screenData(priv, 0x02);
	screenData(priv, 0x84);
	screenCommand(priv, 0xC1);
	screenData(priv, 0XC5);
	screenCommand(priv, 0xC2);
	screenData(priv, 0x0a);
	screenData(priv, 0x00);
	screenCommand(priv, 0xC3);
	screenData(priv, 0x8a);
	screenData(priv, 0x2A);
	screenCommand(priv, 0xC4);
	screenData(priv, 0x8a);
	screenData(priv, 0xEE);
	screenCommand(priv, 0xC5); //VCOM
	screenData(priv, 0x0e);
	screenCommand(priv, 0x36); //MX, MY, RGB mode
	screenData(priv, 0xc8); //0xa8 \BA\E1\C6\C1
	screenCommand(priv, 0xE0);
	screenData(priv, 0x0f);
	screenData(priv, 0x1a);
	screenData(priv, 0x0f);
	screenData(priv, 0x18);
	screenData(priv, 0x2f);
	screenData(priv, 0x28);
	screenData(priv, 0x20);
	screenData(priv, 0x22);
	screenData(priv, 0x1f);
	screenData(priv, 0x1b);
	screenData(priv, 0x23);
	screenData(priv, 0x37);
	screenData(priv, 0x00);
	screenData(priv, 0x07);
	screenData(priv, 0x02);
	screenData(priv, 0x10);
	screenCommand(priv, 0xE1);
	screenData(priv, 0x0f);
	screenData(priv, 0x1b);
	screenData(priv, 0x0f);
	screenData(priv, 0x17);
	screenData(priv, 0x33);
	screenData(priv, 0x2c);
	screenData(priv, 0x29);
	screenData(priv, 0x2e);
	screenData(priv, 0x30);
	screenData(priv, 0x39);
	screenData(priv, 0x3f);
	screenData(priv, 0x3B);
	screenData(priv, 0x00);
	screenData(priv, 0x01);
	screenData(priv, 0x04);
	screenData(priv, 0x13);
	screenCommand(priv, 0x3A); //65k mode
	screenData(priv, 0x05);

	screenCommand(priv, 0x29); //Display on
}

static void screenArea_(struct st7735sPriv *priv, int xs, int xe, int ys, int ye) {
	screenCommand(priv, 0x2B);
	screenData(priv, 0);
	screenData(priv, xs);
	screenData(priv, 0);
	screenData(priv, xe);

	screenCommand(priv, 0x2A);
	screenData(priv, 0);
	screenData(priv, ys);
	screenData(priv, 0);
	screenData(priv, ye);

	screenCommand(priv, 0x2C);
}

static void updateTx(struct st7735sPriv *priv, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	screenArea_(priv, 32 + y, 32 + y + h - 1, x, x + w - 1);
	memset(&priv->spi_message, 0, sizeof(struct spi_message));
	memset(&priv->spi_transfer, 0, sizeof(struct spi_transfer));
	priv->spi_transfer.tx_buf = priv->spiTx;
	priv->spi_transfer.len = 2 * h * w;
	spi_message_init(&priv->spi_message);
	spi_message_add_tail(&priv->spi_transfer, &priv->spi_message);
	spi_sync(priv->spi, &priv->spi_message);
}

static unsigned int convert24to16(unsigned char r, unsigned char g, unsigned char b) {
	return ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}

static void update(struct st7735sPriv *priv, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	int xx, yy;
	for (yy = 0; yy < h; yy++)
		for (xx = 0; xx < w; xx++) {
			int posFb = DEPTH * (yy * w + xx);
			int posTx;
			if (priv->rotation == 0)
				posTx = (yy * w + xx) * 2;
			else if (priv->rotation == 1)
				posTx = ((w - 1 - xx) * h + yy) * 2;
			else if (priv->rotation == 2)
				posTx = ((h - 1 - yy) * w + w - 1 - xx) * 2;
			else if (priv->rotation == 3)
				posTx = (xx * h + h - 1 - yy) * 2;
			unsigned int c = convert24to16(priv->framebuffer[posFb + 2], priv->framebuffer[posFb + 1], priv->framebuffer[posFb + 0]);
			priv->spiTx[posTx] = c >> 8;
			priv->spiTx[posTx + 1] = c & 0xFF;
		}
	if (priv->rotation % 2)
		updateTx(priv, y, x, h, w);
	else
		updateTx(priv, x, y, w, h);
}

static void rect(struct st7735sPriv *priv, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
	int xx, yy;
	for (yy = 0; yy < h; yy++)
		for (xx = 0; xx < w; xx++) {
			priv->spiTx[2 * yy * w + 2 * xx] = color >> 8;
			priv->spiTx[2 * yy * w + 2 * xx + 1] = color & 0xFF;
		}
	updateTx(priv, x, y, w, h);
}

static ssize_t write_init(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	init_(priv);
	return count;
}

static DEVICE_ATTR(init, 0220, NULL, write_init);

static ssize_t write_rect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	int x, y, w, h, color;
	sscanf(buf, "%d %d %d %d %d", &x, &y, &w, &h, &color);
	rect(priv, x, y, w, h, color);
	return count;
}

static DEVICE_ATTR(rect, 0220, NULL, write_rect);

static ssize_t write_update(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	int x, y, w, h;
	sscanf(buf, "%d %d %d %d", &x, &y, &w, &h);
	update(priv, x, y, w, h);
	return count;
}

static DEVICE_ATTR(update, 0220, NULL, write_update);

///////////////////////////////////////////

static int st7735s_open(struct inode *inode, struct file *filp) {
	struct st7735sPriv *priv = container_of(inode->i_cdev, struct st7735sPriv, cdev);
	filp->private_data = priv;
	if (priv->debug >= 1)
		printk("MyDongle-ST7735S: open\n");
	return 0;
}

static ssize_t st7735s_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	struct st7735sPriv *priv = (struct st7735sPriv *)filp->private_data;
	int ret = copy_to_user(buf, priv->framebuffer, count);
	if (priv->debug >= 1)
		printk("MyDongle-ST7735S: read %lu\n", count);
	if (ret != 0)
		printk("MyDongle-ST7735S: copy_to_user %d\n", ret);
	//memcpy(buf, priv->framebuffer, count);
	return count;
}

static ssize_t st7735s_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	struct st7735sPriv *priv = (struct st7735sPriv *)filp->private_data;
	int ret = copy_from_user(priv->framebuffer, buf, count);
	if (priv->debug >= 1)
		printk("MyDongle-ST7735S: write %lu\n", count);
	if (ret != 0)
		printk("MyDongle-ST7735S: copy_from_user %d\n", ret);
	//memcpy(priv->framebuffer, buf, count);
	return count;
}

static int st7735s_mmap(struct file *filp, struct vm_area_struct *vma) {
	struct st7735sPriv *priv = (struct st7735sPriv *)filp->private_data;
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);

	if (size > priv->framebuffersize)
		return -EINVAL;

	if (remap_pfn_range(vma, vma->vm_start,virt_to_phys((void *)priv->framebuffer) >> PAGE_SHIFT, size, vma->vm_page_prot) < 0) {
	printk("MyDongle-ST7735S: remap_pfn_range failed\n");
		return -EIO;
	}
	return 0;
}

static int st7735s_release(struct inode *inode, struct file *filp) {
	struct st7735sPriv *priv = (struct st7735sPriv *)filp->private_data;
	if (priv->debug >= 1)
	printk("MyDongle-ST7735S: release\n");
	return 0;
}

static struct file_operations st7735s_fops = {
	.owner = THIS_MODULE,
	.open = st7735s_open,
	.read = st7735s_read,
	.write = st7735s_write,
	.mmap = st7735s_mmap,
	.release = st7735s_release,
};

static ssize_t show_debug(struct device *dev, struct device_attribute *attr, char *buf) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", priv->debug);
}

static ssize_t write_debug(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	unsigned long i;

		if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	priv->debug = i;
	return count;
}

static DEVICE_ATTR(debug, 0660, show_debug, write_debug);

static ssize_t show_rotation(struct device *dev, struct device_attribute *attr, char *buf) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", priv->rotation);
}

static ssize_t write_rotation(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	unsigned long i;

		if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	priv->rotation = i;
	return count;
}

static DEVICE_ATTR(rotation, 0660, show_rotation, write_rotation);

static ssize_t show_backlight(struct device *dev, struct device_attribute *attr, char *buf) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", gpio_get_value(priv->backlight));
}

static ssize_t write_backlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	unsigned long i;
		if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(priv->backlight, i);
	return count;
}

static DEVICE_ATTR(backlight, 0660, show_backlight, write_backlight);

static ssize_t show_wd(struct device *dev, struct device_attribute *attr, char *buf) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", gpio_get_value(priv->wd));
}

static ssize_t write_wd(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	unsigned long i;
		if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(priv->wd, i);
	return count;
}

static DEVICE_ATTR(wd, 0660, show_wd, write_wd);

#define hex2Int(a) (a >= '0' && a <= '9' ? (a - '0') : ( a - 'a' + 10))
static ssize_t write_spi(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	int len = strlen(buf) / 2;
	int i;
	for (i = 0; i < len; i++)
		priv->spiTx[i] = hex2Int(buf[2 * i]) * 16 + hex2Int(buf[2 * i + 1]);
	memset(&priv->spi_message, 0, sizeof(struct spi_message));
	memset(&priv->spi_transfer, 0, sizeof(struct spi_transfer));
	priv->spi_transfer.tx_buf = priv->spiTx;
	priv->spi_transfer.len = len;
	spi_message_init(&priv->spi_message);
	spi_message_add_tail(&priv->spi_transfer, &priv->spi_message);
	spi_sync(priv->spi, &priv->spi_message);
	return count;
}

static DEVICE_ATTR(spi, 0220, NULL, write_spi);

static ssize_t show_reset(struct device *dev, struct device_attribute *attr, char *buf) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", !gpio_get_value(priv->nrst));
}

static ssize_t write_reset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct st7735sPriv *priv = dev_get_drvdata(dev);
	unsigned long i;
		if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(priv->nrst, !i);
	return count;
}

static DEVICE_ATTR(reset, 0660, show_reset, write_reset);

static struct attribute *st7735s_attributes[] = {
	&dev_attr_init.attr,
	&dev_attr_rect.attr,
	&dev_attr_update.attr,
	&dev_attr_debug.attr,
	&dev_attr_rotation.attr,
	&dev_attr_backlight.attr,
	&dev_attr_wd.attr,
	&dev_attr_spi.attr,
	&dev_attr_reset.attr,
	NULL,
};

static struct attribute_group st7735s_attr_group = {
	.attrs = st7735s_attributes,
};

static int st7735s_probe(struct spi_device *spi) {
	struct st7735sPriv *priv;
	int error;
	int ret;
	struct device *dev = &spi->dev;
	struct device_node *node = of_find_compatible_node(NULL, NULL, "st7735s");

	printk("MyDongle-ST7735S: Enter probe");
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_3;
	error = spi_setup(spi);
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(dev, "failed to allocate driver private data\n");
		error = -ENOMEM;
		goto err0;
	}

	dev_set_drvdata(dev, priv);
	priv->spi = spi;
	priv->debug = 0;
	priv->rotation = 0;

	priv->framebuffersize = PAGE_ALIGN(SCREENBUFFERSIZE);
	priv->framebuffer = kmalloc(priv->framebuffersize, GFP_KERNEL);

	error = sysfs_create_group(&dev->kobj, &st7735s_attr_group);

#define VS10x3_MAJOR 200
	error = register_chrdev(VS10x3_MAJOR, "mydonglecloud_screen_f", &st7735s_fops);
	if (error)
		goto err0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,0,0)
	priv->cls = class_create(THIS_MODULE, "st7735s");
#else
	priv->cls = class_create("st7735s");
#endif
	if (IS_ERR(priv->cls)) {
		error = PTR_ERR(priv->cls);
		goto err0;
	}

	cdev_init(&priv->cdev, &st7735s_fops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops = &st7735s_fops;
	priv->dev = device_create(priv->cls, NULL, MKDEV(VS10x3_MAJOR, 0 /* minor */), NULL, "mydonglecloud_screen_f");
	cdev_add(&priv->cdev, MKDEV(VS10x3_MAJOR, 0), 1);

	of_property_read_u32(node, "bcklit", &priv->backlight);
	priv->backlight += 569;
	ret = gpio_request(priv->backlight, "BACKLIGHT");
	if (ret < 0) {
		printk("MyDongle-ST7735S: Failed to request GPIO %d for BACKLIGHT\n", priv->backlight);
		//return 0;
	}
	gpio_direction_output(priv->backlight, 1);

	of_property_read_u32(node, "wd", &priv->wd);
	priv->wd += 569;
	ret = gpio_request(priv->wd, "WD");
	if (ret < 0) {
		printk("MyDongle-ST7735S: Failed to request GPIO %d for WD\n", priv->wd);
		//return 0;
	}
	gpio_direction_output(priv->wd, 1);

	of_property_read_u32(node, "nrst", &priv->nrst);
	priv->nrst += 569;
	ret = gpio_request(priv->nrst, "NRST");
	if (ret < 0) {
		printk("MyDongle-ST7735S: Failed to request GPIO %d for NRST\n", priv->nrst);
		//return 0;
	}
	gpio_direction_output(priv->nrst, 1);

	printk("MyDongle-ST7735S: Exit probe");
	return 0;

 err0:
	dev_set_drvdata(dev, NULL);
	return error;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,0,0)
static int st7735s_remove(struct spi_device *spi) {
#else
static void st7735s_remove(struct spi_device *spi) {
#endif
	struct st7735sPriv *priv = spi_get_drvdata(spi);
	struct device *dev = &spi->dev;

	kfree(priv);

	sysfs_remove_group(&dev->kobj, &st7735s_attr_group);

	dev_set_drvdata(dev, NULL);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,0,0)
	return 0;
#endif
}

static const struct spi_device_id st7735s_id[] = {
	{ "st7735s", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, st7735s_id);

static const struct of_device_id st7735s_of[] = {
	{ .compatible = "st7735s", },
	{},
};
MODULE_DEVICE_TABLE(of, st7735s_of); 

static struct spi_driver st7735s_driver = {
	.driver = {
		.name = "st7735s",
		.of_match_table = of_match_ptr(st7735s_of),
	},
	.probe = st7735s_probe,
	.remove = st7735s_remove,
	.id_table = st7735s_id,
};

module_spi_driver(st7735s_driver);

MODULE_DESCRIPTION("Driver for ST7735S");
MODULE_AUTHOR("Gregoire Gentil");
MODULE_LICENSE("GPL");
