export PATH	:=	$(DEVKITPPC)/bin:$(PATH)
PREFIX = powerpc-eabi-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

MACHDEP = -mcpu=750 -mno-eabi
CFLAGS = $(MACHDEP) -O1 -Werror -Wall -pipe -mno-sdata
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,-T,link.ld -L.
ASFLAGS = -D_LANGUAGE_ASSEMBLY -DHW_RVL -x assembler-with-cpp

TARGET_LINKED = boot.elf
TARGET = extldr.bin

CFILES = string.c ios.c utils.c cache.c usbgecko.c main.c
OBJS = crt0.o memory.o string.o ios.o utils.o cache.o usbgecko.o main.o

DEPDIR = .deps

all: $(TARGET)

%.o: %.s
	@echo "$<"
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.S
	@echo "$<"
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.c
	@echo "$<"
	@$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

$(TARGET_LINKED): $(OBJS)
	@echo "linking ... $@"
	@$(CC) -g -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

$(TARGET): $(TARGET_LINKED)
	@echo "output ... $@"
	@$(OBJCOPY) -O binary $< $@

clean:
	@echo "clean ..."
	@rm -rf $(TARGET_LINKED) $(TARGET) $(OBJS) $(DEPDIR)
