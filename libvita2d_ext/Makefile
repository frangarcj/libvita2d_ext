TARGET_LIB = libvita2d_ext.a
OBJS       = source/vita2d_ext.o source/vita2d_ext_draw.o source/utils.o
INCLUDES   = include
SHADERS    = shader/compiled/clear_v_gxp.o shader/compiled/clear_f_gxp.o \
             shader/compiled/color_v_gxp.o shader/compiled/color_f_gxp.o \
             shader/compiled/texture_v_gxp.o shader/compiled/texture_f_gxp.o \
             shader/compiled/texture_tint_f_gxp.o

PREFIX  ?= ${VITASDK}/arm-vita-eabi
CC      = arm-vita-eabi-gcc
AR      = arm-vita-eabi-ar
CFLAGS  = -Wl,-q -Wall -O3 -I$(INCLUDES) -ffat-lto-objects -flto
ASFLAGS = $(CFLAGS)

all: $(TARGET_LIB)

debug: CFLAGS += -DDEBUG_BUILD
debug: all

$(TARGET_LIB): $(SHADERS) $(OBJS)
	$(AR) -rc $@ $^

clean:
	rm -rf $(TARGET_LIB) $(OBJS)

install: $(TARGET_LIB)
	@mkdir -p $(DESTDIR)$(PREFIX)/lib/
	cp $(TARGET_LIB) $(DESTDIR)$(PREFIX)/lib/
	@mkdir -p $(DESTDIR)$(PREFIX)/include/
	cp include/vita2d_ext.h $(DESTDIR)$(PREFIX)/include/

