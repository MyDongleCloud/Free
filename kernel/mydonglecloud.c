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
#include <linux/version.h>

#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_VERBOSE 3

struct mydonglePriv {
	int debug;
	int buzzerCount;
	struct pwm_device *buzzerS;
	int buzzerON;
	int buzzerFreq;
	struct work_struct workBuzzer;
	char model[8];
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

static ssize_t show_model(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%s\n", ip->model);
}

static DEVICE_ATTR(model, 0440, show_model, NULL);

static ssize_t show_hardwareVersion(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", ip->hardwareVersion);
}

static DEVICE_ATTR(hardwareVersion, 0440, show_hardwareVersion, NULL);

static ssize_t show_serialNumber(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mydonglePriv *ip = dev_get_drvdata(dev);
	if (strcmp(ip->model, "Pro") == 0) {
		const char *sz = NULL;
		of_property_read_string(of_root, "serial-number", &sz);
		return sprintf(buf, "%s", sz);
	} else if (strcmp(ip->model, "Std") == 0) {
#define ADDRESS_ID 0xC0067000
		void __iomem *regs = ioremap(ADDRESS_ID + 0x04, 4);
		char szSerial[17];
		sprintf(szSerial, "%08x", readl(regs));
		iounmap(regs);
		return sprintf(buf, "%s", szSerial);
	} else
		return sprintf(buf, "1234567890abcdef");
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
	&dev_attr_buzzer.attr,
	&dev_attr_buzzerFreq.attr,
	&dev_attr_model.attr,
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
	struct device *dev = &pdev->dev;
	struct mydonglePriv *ip = devm_kzalloc(dev, sizeof(struct mydonglePriv), GFP_KERNEL);
	if (!ip)
		return -ENOMEM;

	platform_set_drvdata(pdev, ip);
	myip = (struct mydonglePriv *)ip;
	ip->debug = 0;
	const char *mm;
	of_property_read_string(of_root, "model", &mm);
	strcpy(ip->model, strstr(mm, "Raspberry Pi") != NULL ? "Pro" : "Std");
	ip->hardwareVersion = 10;
	ip->buzzerCount = 0;
	INIT_WORK(&ip->workBuzzer, mydongle_workBuzzer);

	ip->buzzerS = pwm_get(&pdev->dev, NULL);
	if (IS_ERR(ip->buzzerS))
		printk("MyDongle-Dongle: Requesting PWM failed");

	timer_setup(&my_timer_buzzer, my_timer_buzzer_callback, 0);

	printk("MyDongleCloud: Exit probe");
	return sysfs_create_group(&dev->kobj, &mydongle_attr_group);
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,12,0)
static int mydongle_remove(struct platform_device *pdev) {
#else
static void mydongle_remove(struct platform_device *pdev) {
#endif
	int ret;

	ret = del_timer(&my_timer_buzzer);
	if (ret)
		printk("MyDongle-Dongle: Timer buzzer still in use\n");

	sysfs_remove_group(NULL, &mydongle_attr_group);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,12,0)
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
