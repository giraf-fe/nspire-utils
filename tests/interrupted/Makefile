EXE = interrupted

DISTDIR = build
OBJDIR = obj
ASMDIR = asm
SRCDIR = .

GCC = nspire-gcc
AS  = nspire-as
GXX = nspire-g++
LD  = nspire-ld
GENZEHN = genzehn
OBJDUMP = arm-none-eabi-objdump

ZEHNFLAGS = --name "$(EXE)" --author "giraf-fe" --notice "disables all interrupts and moves the vector table" --240x320-support true \
			--ndless-min 53 --ndless-max 62 --color-support true --uses-lcd-blit false

SHAREDFLAGS =  -Wall -Wextra -Wpedantic -marm -finline-functions -march=armv5te -mtune=arm926ej-s -mfpu=auto -O2 -ffunction-sections -fdata-sections -mno-unaligned-access \
			   -I ../../include
GCCFLAGS = $(SHAREDFLAGS) -Wno-incompatible-pointer-types -std=c99
GXXFLAGS = $(SHAREDFLAGS) -std=c++20
LDFLAGS = -Wall -lnspireio


OBJS = $(patsubst %.c, %.o, $(shell find $(SRCDIR) -name \*.c))
OBJS += $(patsubst %.cpp, %.o, $(shell find $(SRCDIR) -name \*.cpp))
OBJS += $(patsubst %.S, %.o, $(shell find $(SRCDIR) -name \*.S))

LIBNSPUTILS = ../../libnspireutils.a

vpath %.tns $(DISTDIR)
vpath %.elf $(DISTDIR)

all: $(EXE).tns

%.o: %.c
	mkdir -p $(dir $(OBJDIR)/$@)
	mkdir -p $(dir $(ASMDIR)/$@)
	# Compile
	$(GCC) $(GCCFLAGS) -c $< -o $(OBJDIR)/$@
	# Disassemble the generated object file
	$(OBJDUMP) -D -C $(OBJDIR)/$@ > $(ASMDIR)/$(patsubst %.o,%.s,$@)

%.o: %.cpp
	mkdir -p $(dir $(OBJDIR)/$@)
	mkdir -p $(dir $(ASMDIR)/$@)
	# Compile
	$(GXX) $(GXXFLAGS) -c $< -o $(OBJDIR)/$@
	# Disassemble the generated object file
	$(OBJDUMP) -D -C $(OBJDIR)/$@ > $(ASMDIR)/$(patsubst %.o,%.s,$@)
	
%.o: %.S
	mkdir -p $(dir $(OBJDIR)/$@)
	mkdir -p $(dir $(ASMDIR)/$@)
	# Assemble
	$(AS) -c $< -o $(OBJDIR)/$@
	# Disassemble
	$(OBJDUMP) -D $(OBJDIR)/$@ > $(ASMDIR)/$(patsubst %.o,%.s,$@)

$(EXE).elf: $(OBJS)
	mkdir -p $(DISTDIR)
	$(LD) $(addprefix $(OBJDIR)/,$(OBJS)) $(LIBNSPUTILS) -o $(DISTDIR)/$@ $(LDFLAGS)

$(EXE).tns: $(EXE).elf
	$(GENZEHN) --input $(DISTDIR)/$^ --output $(DISTDIR)/$@.zehn $(ZEHNFLAGS)
	make-prg $(DISTDIR)/$@.zehn $(DISTDIR)/$@
	rm $(DISTDIR)/$@.zehn

clean:
	rm -f $(addprefix $(OBJDIR)/,$(OBJS)) $(DISTDIR)/$(EXE).tns $(DISTDIR)/$(EXE).elf $(DISTDIR)/$(EXE).zehn
	rm -rf $(ASMDIR)