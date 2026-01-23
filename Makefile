LIB = libnspireutils

GCC = nspire-gcc
AS  = nspire-as
GXX = nspire-g++
LD  = nspire-ld
AR  := $(shell (which arm-elf-ar arm-none-eabi-ar arm-linux-gnueabi-ar | head -1) 2>/dev/null)

SHAREDFLAGS = -Wall -Wextra -Wpedantic -Werror -marm -march=armv5te -mtune=arm926ej-s -mfpu=auto -Os -ffunction-sections -fdata-sections -mno-unaligned-access

GCCFLAGS = $(SHAREDFLAGS) -std=c99
GXXFLAGS = $(SHAREDFLAGS) -std=c++20 -Iinclude
LDFLAGS = -Wall

DISTDIR = .
OBJDIR = obj
SRCDIR = src

OBJS = $(patsubst %.c, %.o, $(shell find $(SRCDIR) -name \*.c 2>/dev/null))
OBJS += $(patsubst %.cpp, %.o, $(shell find $(SRCDIR) -name \*.cpp 2>/dev/null))
OBJS += $(patsubst %.S, %.o, $(shell find $(SRCDIR) -name \*.S 2>/dev/null))
vpath %.a $(DISTDIR)


all: $(LIB).a tests

TESTDIRS = $(patsubst %/,%,$(dir $(wildcard tests/*/Makefile)))

.PHONY: all clean tests $(TESTDIRS)

tests: $(TESTDIRS)

$(TESTDIRS): $(LIB).a
	$(MAKE) -C $@

%.o: %.c
	mkdir -p $(dir $(OBJDIR)/$@)
	$(GCC) $(GCCFLAGS) -c $< -o $(OBJDIR)/$@

%.o: %.cpp
	mkdir -p $(dir $(OBJDIR)/$@)
	$(GXX) $(GXXFLAGS) -c $< -o $(OBJDIR)/$@
	
%.o: %.S
	mkdir -p $(dir $(OBJDIR)/$@)
	$(AS) -c $< -o $(OBJDIR)/$@

$(LIB).a: $(OBJS)
	mkdir -p $(DISTDIR)
	$(AR) rcs $(DISTDIR)/$@ $(addprefix $(OBJDIR)/,$^)

clean:
	rm -rf $(OBJDIR) $(DISTDIR)/$(LIB).a
	for dir in $(TESTDIRS); do $(MAKE) -C $$dir clean; done