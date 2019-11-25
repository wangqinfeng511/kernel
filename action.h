#ifndef ACTION_H
#define ACTION_H

enum action_value{
    out=2,
    in=3,
    low=0,
    height=1,
};
class action
{
public:
    action();
    void set_gpio_ctl(int gpio,action_value action);
};

#endif // ACTION_H
