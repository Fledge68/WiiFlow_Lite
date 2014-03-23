
PREFIX = powerpc-eabi-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

MACHDEP = -mcpu=750 -mno-eabi -mhard-float
CFLAGS = $(MACHDEP) -O1 -Werror -Wall -pipe -ffunction-sections -finline-functions-called-once -mno-sdata
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,-T,link.ld -L.
ASFLAGS = -D_LANGUAGE_ASSEMBLY -DHW_RVL

TARGET_LINKED = boot.elf
TARGET = ../../data/extldr.bin

CFILES = string.c sync.c main.c
OBJS = crt0.o string.o sync.o main.o

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
