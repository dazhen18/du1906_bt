#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

ifdef CONFIG_CUPID_BOARD_V2
$(warning "config board is cupid")
COMPONENT_ADD_INCLUDEDIRS :=    cupid/include
COMPONENT_SRCDIRS := . cupid
else
ifdef CONFIG_MINI_BOARD
$(warning "config board is mini board")
COMPONENT_ADD_INCLUDEDIRS :=    mini_board/include
COMPONENT_SRCDIRS := . mini_board
else   # other case: CONFIG_ESP32_KORVO_DU1906_BOARD
$(warning "config board is esp_korvo_du1906 or mars")
COMPONENT_ADD_INCLUDEDIRS :=    krovo_du1906/include
COMPONENT_SRCDIRS := . krovo_du1906
endif
endif

ifdef CONFIG_MARS_BOARD
$(warning "config board is esp_x6bc_v10")
COMPONENT_ADD_INCLUDEDIRS :=    mars_x6bc/include
COMPONENT_SRCDIRS := . mars_x6bc
endif

CFLAGS += -Wno-enum-compare
