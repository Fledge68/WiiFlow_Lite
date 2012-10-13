
#PREFIX = powerpc-gekko-
PREFIX = $(DEVKITPPC)/bin/powerpc-eabi-
#PREFIX = /home/megazig/Wii/bootmii-utils/bin/powerpc-elf-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

MACHDEP = -mcpu=750 -mno-eabi -mhard-float -DTINY -DDEBUG
#CFLAGS = $(MACHDEP) -Os -Wall -pipe -ffunction-sections -finline-functions-called-once -mno-sdata --combine -fwhole-program -ffreestanding
CFLAGS = $(MACHDEP) -O1 -Wall -pipe -ffunction-sections -finline-functions-called-once -mno-sdata
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,-T,link.ld -L.
ASFLAGS = -D_LANGUAGE_ASSEMBLY -DHW_RVL -DTINY

TARGET_LINKED = boot.elf
TARGET = booter.bin

CFILES = apploader.c cios.c cache.c debug.c di.c disc.c ios.c patchcode.c usb.c utils.c video.c \
fst.c multidol.c wip.c main.c
#OBJS = crt0.o _all.o
OBJS = crt0.o apploader.o cios.o cache.o debug.o di.o disc.o ios.o patchcode.o patchhook.o usb.o utils.o video.o \
fst.o multidol.o wip.o main.o

DEPDIR = .deps

LIBS = 

all: $(TARGET)

%.o: %.s
	@echo " ASSEMBLE    $<"
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.S
	@echo " ASSEMBLE    $<"
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.c
	@echo " COMPILE     $<"
	@$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

%.o: %.cpp
	@echo " COMPILE     $<"
	@$(CXX) $(CFLAGS) $(DEFINES) -c $< -o $@

#_all.o: $(CFILES)
#	@echo " COMPILE ALL "
#	@mkdir -p $(DEPDIR)
#	@$(CC) $(CFLAGS) $(DEFINES) -Wp,-MMD,$(DEPDIR)/$(*F).d,-MQ,"$@",-MP -c $(CFILES) -o $@

$(TARGET_LINKED): $(OBJS)
	@echo " LINK        $@"
	@$(CC) -g -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

$(TARGET): $(TARGET_LINKED)
	@echo " OBJCOPY     $@"
	@$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(TARGET_LINKED) $(TARGET) $(OBJS) $(DEPDIR)
