obj-y 	+= SAR_Sensor_Hardware.o
obj-y	+= sx9310/
obj-n	+= example/

DEBUG = n

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DASH_HW_DEBUG
else
	DEBFLAGS = -O -g
endif
ccflags-y += $(DEBFLAGS)

