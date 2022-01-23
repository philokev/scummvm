MODULE := engines/yahtzee

MODULE_OBJS = \
	yahtzee.o \
	console.o \
	controls.o \
	metaengine.o \
	util.o \
	gui/dialogs.o \
	gui/widget.o

# This module can be built as a plugin
ifeq ($(ENABLE_YAHTZEE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
