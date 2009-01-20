# arch/arm/plat-s5pc1xx/Makefile
#
# Copyright 2008 Openmoko, Inc.
# Copyright 2008 Simtec Electronics
#
# Licensed under GPLv2

obj-y				:=
obj-m				:=
obj-n				:= dummy.o
obj-				:=

# Core files

obj-y				+= dev-uart.o devs.o
obj-y				+= cpu.o
obj-y				+= s5pc1xx-time.o
obj-y				+= irq.o
obj-y				+= irq-eint.o
obj-y				+= clock.o
obj-y				+= gpiolib.o

# CPU support

obj-$(CONFIG_CPU_S5PC100_INIT)	+= s5pc100-init.o
obj-$(CONFIG_CPU_S5PC100_CLOCK)	+= s5pc100-clock.o
obj-$(CONFIG_PM)                += pm.o
obj-$(CONFIG_PM)                += sleep.o

# Device setup

obj-$(CONFIG_S5PC1XX_SETUP_I2C0) += setup-i2c0.o
obj-$(CONFIG_S5PC1XX_SETUP_I2C1) += setup-i2c1.o
obj-$(CONFIG_S5PC1XX_ADC)        += adc.o
obj-$(CONFIG_S5PC1XX_PWM)        += pwm-s5pc100.o
