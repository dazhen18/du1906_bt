#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_SRCDIRS := .
COMPONENT_ADD_INCLUDEDIRS := include 
ifdef CONFIG_CUPID_BOARD_V2
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib -lbds_light_sdk_cupid
else
ifdef CONFIG_MARS_BOARD
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib -lbds_light_sdk_mars
else
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib -lbds_light_sdk
endif
endif

