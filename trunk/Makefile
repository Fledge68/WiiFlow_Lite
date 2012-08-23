#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	boot
BUILD		:=	build
SOURCES		:=	source \
				source/banner \
				source/channel \
				source/cheats \
				source/config \
				source/data \
				source/devicemounter \
				source/devicemounter/libwbfs \
				source/fileOps \
				source/gc \
				source/gecko \
				source/gui \
				source/homebrew \
				source/libwbfs \
				source/list \
				source/loader \
				source/memory \
				source/menu \
				source/music \
				source/network \
				source/plugin \
				source/unzip \
				source/xml \
				source/wstringEx

DATA		:=	data \
				data/images \
				data/sounds \
				data/help

INCLUDES	:=	source
#---------------------------------------------------------------------------------
# Default build shell script options
#---------------------------------------------------------------------------------
ios			:=	249
#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS	 =	-g -O2 -Wall -Wextra -Wno-multichar $(MACHDEP) $(INCLUDE) -DHAVE_CONFIG_H
CXXFLAGS =	$(CFLAGS)

LDFLAGS	 =	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map,--section-start,.init=0x80A00000,-wrap,malloc,-wrap,free,-wrap,memalign,-wrap,calloc,-wrap,realloc,-wrap,malloc_usable_size

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lcustomfat -lcustomntfs -lcustomext2fs -lpng -lm -lz -lwiiuse -lbte -lasnd -logc -lfreetype -lvorbisidec -lmad -ljpeg -lmodplay

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(CURDIR)/portlibs
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
SVNREV		:=	$(shell bash ./scripts/svnrev.sh)

export CFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

TXTFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.txt)))

BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
TTFFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.ttf)))
PNGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.png)))
JPGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.jpg)))

MP3FILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.mp3)))
OGGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.ogg)))
PCMFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.pcm)))
WAVFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.wav)))

DOLFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.dol)))
ELFFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.elf)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(JPGFILES:.jpg=.jpg.o) $(PNGFILES:.png=.png.o) $(DOLFILES:.dol=.dol.o) \
					$(OGGFILES:.ogg=.ogg.o) $(WAVFILES:.wav=.wav.o) $(MP3FILES:.mp3=.mp3.o) \
					$(ELFFILES:.elf=.elf.o) $(BINFILES:.bin=.bin.o) $(TXTFILES:.txt=.txt.o) \
					$(CURDIR)/data/magic_patcher.o

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)

#---------------------------------------------------------------------------------
.PHONY: $(BUILD) all clean run
#---------------------------------------------------------------------------------
$(BUILD):
	@echo Building for  IOS $(ios).
	@bash ./scripts/buildtype.sh $(ios)
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

#---------------------------------------------------------------------------------
run:
	wiiload $(TARGET).dol

#---------------------------------------------------------------------------------
gdb:
	@echo Loading GDB with symbols from boot.elf, type quit to exit.
	@powerpc-eabi-gdb boot.elf

#---------------------------------------------------------------------------------
addr:
	@echo Loading addr2line with symbols from boot.elf..
	@echo Press ctrl+c to exit.
	@echo Enter an address from the stack dump:
	@powerpc-eabi-addr2line -f -e boot.elf

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES) alt_ios_gen.o

#---------------------------------------------------------------------------------
$(BUILD)/alt_ios_gen.o: alt_ios_gen.c

#---------------------------------------------------------------------------------
# This rule links in binary data with the .txt extension
#---------------------------------------------------------------------------------
%.txt.o:	%.txt
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	:	%.jpg
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .png extension
#---------------------------------------------------------------------------------
%.png.o	:	%.png
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .ogg extension
#---------------------------------------------------------------------------------
%.ogg.o : %.ogg
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .wav extension
#---------------------------------------------------------------------------------
%.wav.o	:	%.wav
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .dol extension
#---------------------------------------------------------------------------------
%.dol.o : %.dol
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .elf extension
#---------------------------------------------------------------------------------
%.elf.o : %.elf
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
