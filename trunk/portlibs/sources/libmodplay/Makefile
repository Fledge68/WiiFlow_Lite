#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

# MACHDEP	= -DGEKKO -O2 -mcpu=750 -meabi -mhard-float -ffunction-sections -fdata-sections -fmodulo-sched

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	modplay
BUILD		?=	build
SOURCES		:=	source
DATA		:=
INCLUDES	:=

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

# CFLAGS		= -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CFLAGS		= -ffast-math -O3 -pipe -mrvl -mcpu=750 -meabi -mhard-float -Wall $(MACHDEP) $(INCLUDE) -DGEKKO -DHAVE_CONFIG_H
CXXFLAGS	=	$(CFLAGS)
# LDFLAGS 	= -g $(MACHDEP) -Wl,-Map,$(notdir $@).map
ARFLAGS 	= rcs
ASFLAGS		= -D_LANGUAGE_ASSEMBLY -DHW_RVL

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export TOPDIR		:=	$(CURDIR)
export OUTPUT		:=	$(CURDIR)/lib$(TARGET)
export VPATH		:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
						$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
						$(foreach dir,$(STUBSOURCES),$(CURDIR)/$(dir))
export DEPSDIR		:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))

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
export LIBPATHS		:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
						-L$(LIBOGC_LIB)

export OUTPUT		:=	$(CURDIR)/lib$(TARGET)

export LD			:=	$(CC)

export OFILES		:=	$(CFILES:.c=.o)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) lib$(TARGET).a

#---------------------------------------------------------------------------------
install:
	@echo Installing ...
	@mkdir -p ../../include/$(TARGET)
	@install -v -m 644 lib$(TARGET).a ../../lib
	@install -v -m 644 source/defines.h ../../include/$(TARGET)
	@install -v -m 644 source/envelope.h ../../include/$(TARGET)
	@install -v -m 644 source/mixer.h ../../include/$(TARGET)
	@install -v -m 644 source/modplay.h ../../include/$(TARGET)
	@install -v -m 644 source/modplay_core.h ../../include/$(TARGET)


#---------------------------------------------------------------------------------
else

DEPENDS = $(OFILES:.o=.d) $(OSTUBFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).a: $(OFILES)
	@rm -f $(OUTPUT).a
	@$(AR) $(ARFLAGS) $(OUTPUT).a $(OFILES)
	@echo built ... $(notdir $@)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
