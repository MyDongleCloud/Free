#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/kernel_stat.h>

#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_VERBOSE 3

struct mydonglePriv {
	int debug;

	int gpioLed1;
	int gpioLed2;
	int gpioLed3;
	int gpioLed4;

	int buzzerCount;
	struct pwm_device *buzzerS;
	int buzzerON;
	int buzzerFreq;
	struct work_struct workBuzzer;


	int hardwareVersion;
};

static struct mydonglePriv *myip;
static struct timer_list my_timer_buzzer;

static void mydongle_workBuzzer(struct work_struct *w) {
	struct mydonglePriv *ip = container_of(w, struct mydonglePriv, workBuzzer);
	if (ip->buzzerON) {
		pwm_config(ip->buzzerS, 166000, 322000);
		pwm_enable(ip->buzzerS);
	} else
		pwm_disable(ip->buzzerS);
}

static int inBuzzer;
static void buzzer_(struct mydonglePriv *ip, int enable) {
	ip->buzzerON = enable;
	if (enable)
		inBuzzer = 1;
	schedule_work(&ip->workBuzzer);
}

static void my_timer_buzzer_callback(struct timer_list *tl) {
	struct mydonglePriv *ip = myip;
	if (ip->buzzerCount >= 0)
		ip->buzzerCount--;
	if (ip->buzzerCount >= 0)
		buzzer_(ip, ip->buzzerCount % 2);
	if (ip->buzzerCount >= 0)
		mod_timer(&my_timer_buzzer, jiffies + msecs_to_jiffies(100));
	else
		inBuzzer = 0;
}

static ssize_t write_buzzer(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	ip->buzzerCount = i == 2 ? 1 : i;
	mod_timer(&my_timer_buzzer, jiffies + msecs_to_jiffies(i == 2 ? 300 : i == 1 ? 150 : 100));
	buzzer_(ip, 1);
	return count;
}

static DEVICE_ATTR(buzzer, 0220, NULL, write_buzzer);

static ssize_t write_buzzerFreq(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;
	if (i == 0)
		pwm_disable(ip->buzzerS);
	else {
		long f = 1000 * 1000 * 1000 / i;
		pwm_config(ip->buzzerS, f / 2, f);
		pwm_enable(ip->buzzerS);
	}
	return count;
}

static DEVICE_ATTR(buzzerFreq, 0220, NULL, write_buzzerFreq);

static ssize_t show_led1(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", gpio_get_value(ip->gpioLed1));
}

static ssize_t write_led1(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(ip->gpioLed1, i);
	return count;
}

static DEVICE_ATTR(led1, 0660, show_led1, write_led1);

static ssize_t show_led2(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", gpio_get_value(ip->gpioLed2));
}

static ssize_t write_led2(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(ip->gpioLed2, i);
	return count;
}

static DEVICE_ATTR(led2, 0660, show_led2, write_led2);

static ssize_t show_led3(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", gpio_get_value(ip->gpioLed3));
}

static ssize_t write_led3(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(ip->gpioLed3, i);
	return count;
}

static DEVICE_ATTR(led3, 0660, show_led3, write_led3);

static ssize_t show_led4(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", gpio_get_value(ip->gpioLed4));
}

static ssize_t write_led4(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	gpio_set_value(ip->gpioLed4, i);
	return count;
}

static DEVICE_ATTR(led4, 0660, show_led4, write_led4);

static ssize_t show_hardwareVersion(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", ip->hardwareVersion);
}

static DEVICE_ATTR(hardwareVersion, 0440, show_hardwareVersion, NULL);

static ssize_t show_serialNumber(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%s", "1234567890abcdef");
}

static DEVICE_ATTR(serialNumber, 0440, show_serialNumber, NULL);

static ssize_t show_debug(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", ip->debug);
}

static ssize_t write_debug(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	ip->debug = i;
	return count;
}

static DEVICE_ATTR(debug, 0660, show_debug, write_debug);

static ssize_t write_printk(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	printk("MyDongleCloud: %s", buf);
	if (strlen(buf) > 0 && buf[strlen(buf) - 1] != '\n')
		printk("\n");
	return count;
}

static DEVICE_ATTR(printk, 0220, NULL, write_printk);

static ssize_t write_buzzerClick(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	unsigned long i;
        if (kstrtoul(buf, 10, &i))
		return -EINVAL;

	pwm_config(ip->buzzerS, 75000 / 2, 75000);
	pwm_enable(ip->buzzerS);
	udelay(i);
	pwm_disable(ip->buzzerS);
	return count;
}

static DEVICE_ATTR(buzzerClick, 0220, NULL, write_buzzerClick);

static struct attribute *mydongle_attributes[] = {
	&dev_attr_led1.attr,
	&dev_attr_led2.attr,
	&dev_attr_led3.attr,
	&dev_attr_led4.attr,
	&dev_attr_buzzer.attr,
	&dev_attr_buzzerFreq.attr,
	&dev_attr_hardwareVersion.attr,
	&dev_attr_serialNumber.attr,
	&dev_attr_debug.attr,
	&dev_attr_printk.attr,
	&dev_attr_buzzerClick.attr,
	NULL,
};

static struct attribute_group mydongle_attr_group = {
	.attrs = mydongle_attributes,
};

static int mydongle_probe(struct platform_device *pdev) {
	printk("MyDongleCloud: Enter probe");
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct mydonglePriv *ip = devm_kzalloc(dev, sizeof(struct mydonglePriv), GFP_KERNEL);
	if (!ip)
		return -ENOMEM;

	platform_set_drvdata(pdev, ip);

	ip->debug = 0;

	struct device_node *node = of_find_compatible_node(NULL, NULL, "mydonglecloud");
	of_property_read_u32(node, "led1", &ip->gpioLed1);
	ip->gpioLed1 += 571;
	of_property_read_u32(node, "led2", &ip->gpioLed2);
	ip->gpioLed2 += 571;
	of_property_read_u32(node, "led3", &ip->gpioLed3);
	ip->gpioLed3 += 571;
	of_property_read_u32(node, "led4", &ip->gpioLed4);
	ip->gpioLed4 += 571;

	ip->hardwareVersion = 10;
	printk("MyDongleCloud: hardware_Version:%d\n", ip->hardwareVersion);

	ret = gpio_request(ip->gpioLed1, "GPIO_LED1");
	if (ret < 0) {
		printk("MyDongleCloud-Dongle: Failed to request GPIO %d for GPIO_LED1\n", ip->gpioLed1);
		//return 0;
	}
	gpio_direction_output(ip->gpioLed1, 0);

	ret = gpio_request(ip->gpioLed2, "GPIO_LED2");
	if (ret < 0) {
		printk("MyDongleCloud-Dongle: Failed to request GPIO %d for GPIO_LED2\n", ip->gpioLed2);
		//return 0;
	}
	gpio_direction_output(ip->gpioLed2, 0);

	ret = gpio_request(ip->gpioLed3, "GPIO_LED3");
	if (ret < 0) {
		printk("MyDongleCloud-Dongle: Failed to request GPIO %d for GPIO_LED3\n", ip->gpioLed3);
		//return 0;
	}
	gpio_direction_output(ip->gpioLed3, 0);

	ret = gpio_request(ip->gpioLed4, "GPIO_LED4");
	if (ret < 0) {
		printk("MyDongle-Dongle: Failed to request GPIO %d for GPIO_LED4\n", ip->gpioLed4);
		//return 0;
	}
	gpio_direction_output(ip->gpioLed4, 0);

	myip = (struct mydonglePriv *)ip;

	ip->buzzerCount = 0;
	INIT_WORK(&ip->workBuzzer, mydongle_workBuzzer);

	ip->buzzerS = pwm_get(&pdev->dev, NULL);
	if (IS_ERR(ip->buzzerS))
		printk("MyDongle-Dongle: Requesting PWM failed");

	timer_setup(&my_timer_buzzer, my_timer_buzzer_callback, 0);

	printk("MyDongleCloud: Exit probe");
	return sysfs_create_group(&dev->kobj, &mydongle_attr_group);
}

#ifdef KERNEL612
static void mydongle_remove(struct platform_device *pdev) {
#else
static int mydongle_remove(struct platform_device *pdev) {
#endif
	int ret;

	ret = del_timer(&my_timer_buzzer);
	if (ret)
		printk("MyDongle-Dongle: Timer buzzer still in use\n");

	sysfs_remove_group(NULL, &mydongle_attr_group);
#ifndef KERNEL612
	return 0;
#endif
}

static const struct of_device_id mydongle_of[] = {
    { .compatible = "mydonglecloud", },
    {},
};
MODULE_DEVICE_TABLE(of, mydongle_of); 

static struct platform_driver mydongle_driver = {
	.driver = {
		.name = "mydonglecloud",
		.of_match_table = of_match_ptr(mydongle_of),
	},
	.probe = mydongle_probe,
	.remove = mydongle_remove,
};

static int __init mydongle_init(void) {
	return platform_driver_register(&mydongle_driver);
}
module_init(mydongle_init);

static void __exit mydongle_exit(void) {
	platform_driver_unregister(&mydongle_driver);
}
module_exit(mydongle_exit);

MODULE_DESCRIPTION("Driver for MyDongle Cloud");
MODULE_AUTHOR("Gregoire Gentil");
MODULE_LICENSE("GPL");
