#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif
export PORTLIBS	:=	../../portlibs/ppc
export PATH	:=	$(DEVKITPPC)/bin:$(PORTLIBS)/bin:$(PATH)
export LIBOGC_INC	:=	$(DEVKITPRO)/libogc/include
export LIBOGC_LIB	:=	$(DEVKITPRO)/libogc/lib/wii

PREFIX	:=	powerpc-gekko-

export AS	:=	$(PREFIX)as
export CC	:=	$(PREFIX)gcc
export CXX	:=	$(PREFIX)g++
export AR	:=	$(PREFIX)ar
export LD	:=	$(PREFIX)ld
export OBJCOPY	:=	$(PREFIX)objcopy

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	app_booter
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data  
INCLUDES	:=

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
MACHDEP := -mno-eabi -mno-sdata -mcpu=750
CFLAGS	:= -Wall -W -Os -ffreestanding -std=gnu99 -Wstrict-aliasing=2 $(MACHDEP) $(INCLUDE)
CXXFLAGS :=	$(CFLAGS)

#---------------------------------------------------------------------------------
# move loader to another location - THANKS CREDIAR - 0x81330000 for HBC
#---------------------------------------------------------------------------------
#LDFLAGS	=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map
#LDFLAGS = -g $(MACHDEP) -Wl,-Map,$(notdir $@).map -Wl,--section-start,.init=0x80003f00
Q := @
MAKEFLAGS += --no-print-directory
#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
#LIBS	:= 

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
#LIBDIRS	:= $(CURDIR)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean install

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).bin

#---------------------------------------------------------------------------------
install:
	@echo Installing to WiiFlow Source...
	@cp $(OUTPUT).bin ../../data/
	@echo DONE!

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).bin: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.bin: %.elf
	@echo "  OBJCOPY   $@"
	$(Q)$(OBJCOPY) -O binary $< $@

%.elf: link.ld $(OFILES)
	@echo "  LINK      $@"
	$(Q)$(LD) -n -T $^ $(LDFLAGS) -o $@

%.o: %.c 
	@echo "  COMPILE   $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	@echo "  ASSEMBLE  $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
	
-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
