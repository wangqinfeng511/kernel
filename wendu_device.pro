DEFINES += __KERNEL__
DEFINES += MODULE
INCLUDEPATH +=/lib/modules/5.0.0-29-generic/build/include
SOURCES += \
#        main.c
    device.c \
    action.cpp

HEADERS += \
    action.h
