#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include<linux/slab.h>
#include <linux/delay.h>
#include "action.h"
#define ctl_base_addr           0x3F200004  //gpio10-gpio19 功能寄存器
#define height_data_addr        0x3F20001C  //输出高电平1-32位寄存器
#define low_data_addr           0x3F200028  //输出低电平1-32位寄存器
#define gpio_data         0x3F200034  //gpio输入寄存器1-32

struct return_data{
    char data[4];
};
struct devinfo{
//    int irq_number;
    int device_maijor;
    char *device_name;
    const struct file_operations *file_action;
    struct class *device_class;
    char *device_class_name;
    struct device *device_node;
    unsigned int *ctl_gpio_addr;
    unsigned int *height_gpio_addr;
    unsigned int *low_gpio_addr;
    unsigned int *gpio_data_addr;
    unsigned int data[5];
    unsigned char tmp_data;
    int sleep;
};
struct devinfo  *dev_info;
bool yanzheng(struct devinfo data){
    char *v;
    if(data.data[0]+data.data[1]+data.data[2]+data.data[3]==data.data[4]){
        return true;
    }
    return false;
}
ssize_t file_read(struct file *fp, char __user *buff,size_t count,loff_t *fpos){

    unsigned int ctl_gpio_data=0;  //gpio控制寄存器
    unsigned int gpio_data_int=0; //gpio输入寄存器;
    dev_info->ctl_gpio_addr=ioremap(ctl_base_addr,4);
    dev_info->height_gpio_addr=ioremap(height_data_addr,4);
    dev_info->low_gpio_addr=ioremap(low_data_addr,4);
    dev_info->gpio_data_addr=ioremap(gpio_data,4);
    dev_info->sleep=1000;
 //初始化
    //读取控制寄存器现有的值
    ctl_gpio_data=readl(dev_info->ctl_gpio_addr);
    //设置GPIO16为输入模式
    writel(~(~ctl_gpio_data|(0x1<<(3*6))),dev_info->ctl_gpio_addr);
    ctl_gpio_data=readl(dev_info->ctl_gpio_addr);
    int status=(ctl_gpio_data>>(3*6-1))<<(32-3)>>(32-3);
    int in=0;
    int out=1;
    printk("控制寄存器值GPIO16 %d",status);
    if(status==in){
        printk("GPIO16是输入!");
    }
    //判断设备是否就绪,也就是高电平待机
    gpio_data_int=readl(dev_info->gpio_data_addr);
    status=gpio_data_int>>(16-1)<<(32-1)>>(32-1);
    if(status==1){
        printk("设备就绪!");
    }else {
        printk("设备未连接");
        return -1;
    }
    //触发:输出并发送低电平18毫秒后再设置为高电平(可能被传感器拉低了)30微妙
    ctl_gpio_data=readl(dev_info->ctl_gpio_addr);
    writel(ctl_gpio_data|(0x001<<(3*6)),dev_info->ctl_gpio_addr);
    ctl_gpio_data=readl(dev_info->ctl_gpio_addr);
    status=ctl_gpio_data>>(3*6)<<(32-3)>>(32-3);
    if(status==out){
        printk("GPIO16输出模式");
    }else {
        printk("GPiO16无法设置输出模式");
        return -1;
    }
    gpio_data_int=readl(dev_info->low_gpio_addr);
    writel(gpio_data_int|(0x1<<16),dev_info->low_gpio_addr);
    gpio_data_int=readl(dev_info->gpio_data_addr);
    status=gpio_data_int>>16<<31>>31;
    if(status==0){
        printk("GPIO16输出低电平");
    }else {
        printk("GPIO16无法输出低电平");
        return -1;
    }
    mdelay(18);
    gpio_data_int=readl(dev_info->height_gpio_addr);
    writel(gpio_data_int|(0x1<<16),dev_info->height_gpio_addr);
    status=readl(dev_info->gpio_data_addr)>>16<<31>>31;
    udelay(30);
    //  触发完成
    //设置为INPUT模式
    ctl_gpio_data=readl(dev_info->ctl_gpio_addr);
    writel(~(~ctl_gpio_data&(0x000<<(3*6))),dev_info->ctl_gpio_addr);
    ctl_gpio_data=readl(dev_info->ctl_gpio_addr);
    status=ctl_gpio_data>>(3*6)<<(32-3)>>(32-3);
    if(status==0){
        printk("GPIO16%s","输入模式");
    }else {
        printk("GPIO16%s","无法设置为输入模式");
        return -1;
    }
    dev_info->sleep=10000;
    while (dev_info->sleep>0) {
        gpio_data_int=readl(dev_info->gpio_data_addr);
        status=gpio_data_int>>16<<(32-1)>>(32-1);
        if(status==0){
            break;
        }else {
            printk("%d微妙未收到<dht11响应低电平",dev_info->sleep*3);
        }
        dev_info->sleep--;
        udelay(3);
    }
    udelay(80-3);
    dev_info->sleep=10000;
    while (dev_info->sleep>0) {
        gpio_data_int=readl(dev_info->gpio_data_addr);
        status=gpio_data_int>>16<<(32-1)>>(32-1);
        if(status==1){
            break;
        }else {
            printk("%d微妙未收到<dht11响应高电平",dev_info->sleep*3);
        }
        dev_info->sleep--;
        udelay(3);
    }
    udelay(80-3);
    //获取数据
    int i;
    dev_info->tmp_data=0;
    for(i=0;i<40;i++){
        dev_info->tmp_data<<=1;
        dev_info->sleep=10000;
        while (dev_info->sleep>0) {
            gpio_data_int=readl(dev_info->gpio_data_addr);
            status=gpio_data_int>>16<<(32-1)>>(32-1);
            if(status==0){
                break;
            }
            dev_info->sleep--;
        }
        udelay(50);
        dev_info->sleep=1000;
        while (dev_info->sleep>0) {
            gpio_data_int=readl(dev_info->gpio_data_addr);
            status=gpio_data_int>>16<<(32-1)>>(32-1);
            if(status==1){
                break;
            }
            dev_info->sleep--;
        }
        udelay(28);
        gpio_data_int=readl(dev_info->gpio_data_addr);
        status=gpio_data_int>>16<<(32-1)>>(32-1);
        if(status==1){
            dev_info->tmp_data|=0x1;
            udelay(40);
        }
        if((i+1)%8==0 & i>0){
           dev_info->data[(i+1)/8-1]=dev_info->tmp_data;
        }

    }
    bool ok;
    ok=yanzheng(*dev_info);
    if(ok){
        struct return_data w_s_d;
        i=0;
        for(i;i<4;i++){
            w_s_d.data[i]=dev_info->data[i];
        }
        copy_to_user(buff,&w_s_d,sizeof (struct return_data));
        return count;
    }else {
        return -1;
    }

};
int file_open(struct inode *inode,struct file *fp){
    printk("open file");
    return  0;
}
int file_close(struct inode *inode,struct file *fp){
    return 0;
};

const struct file_operations actions_ji={
        .read=file_read,
        .open=file_open,
        .release=file_close
};
static int __init device_init(void){
   dev_info=kmalloc(sizeof (struct devinfo),GFP_KERNEL);
   dev_info->file_action=&actions_ji;
   dev_info->device_maijor=0;
   dev_info->device_name="dht11";
   dev_info->device_maijor=register_chrdev(dev_info->device_maijor,dev_info->device_name,dev_info->file_action);
    printk("device  number %d",dev_info->device_maijor);
    if(dev_info->device_maijor<0){
        printk("register device error %d",dev_info->device_maijor);
        return -1;
    }
    dev_info->device_class_name="dht_class";
    printk("create class %s",dev_info->device_class_name);
    dev_info->device_class=class_create(THIS_MODULE,dev_info->device_class_name);
    if(IS_ERR(dev_info->device_class)){
       goto err_create_class;
    }
    printk("create device_node");
    dev_info->device_node=device_create(dev_info->device_class,NULL,MKDEV(dev_info->device_maijor,0),NULL,dev_info->device_name);
    if(IS_ERR(dev_info->device_node)){
        goto  err_create_device_node;
    }
    return 0;
err_create_class:
       printk("create device class error");
       unregister_chrdev(dev_info->device_maijor,dev_info->device_name);
       return -1;

err_create_device_node:
       printk("create device node error");
       class_destroy(dev_info->device_class);
       return  -1;
};
static void __exit device_exit(void){
    device_destroy(dev_info->device_class,MKDEV(dev_info->device_maijor,0));
    class_destroy(dev_info->device_class);
    unregister_chrdev(dev_info->device_maijor,dev_info->device_name);
};
module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");
