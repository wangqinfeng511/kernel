#include "action.h"
#include <linux/init.h>
#include <linux/mount.h>
action::action()
{

}
void action::set_gpio_ctl(int gpio,action_value action){
    printk("调用成功");
}
