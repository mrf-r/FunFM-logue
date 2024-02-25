
ifndef PLATFORMDIR
$(warning PLATFORMDIR not defined, building for nts-1)
PLATFORMDIR := $(abspath ../logue-sdk/platform/nutekt-digital)
endif

GCC_BIN_PATH ?= /bin
LDDIR := $(PLATFORMDIR)/dummy-osc/ld
include $(PLATFORMDIR)/dummy-osc/Makefile
