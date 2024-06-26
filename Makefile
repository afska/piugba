# === SETUP ===========================================================

# --- No implicit rules ---
.SUFFIXES:

# --- Paths ---
export BASE_DIR = $(GBA_DIR)\projects\piugba

export TONCLIB := $(DEVKITPRO)/libtonc
include  $(BASE_DIR)/tonc_rules

# --- Main path ---

export PATH	:=	$(DEVKITARM)/bin:$(PATH)

# === PROJECT DETAILS =================================================
# PROJ		: Base project name
# TITLE		: Title for ROM header (12 characters)
# LIBS		: Libraries to use, formatted as list for linker flags
# BUILD		: Directory for build process temporaries. Should NOT be empty!
# SRCDIRS	: List of source file directories
# DATADIRS	: List of data file directories
# INCDIRS	: List of header file directories
# LIBDIRS	: List of library directories
# General note: use `.' for the current dir, don't leave the lists empty.

export PROJ	?= $(notdir $(CURDIR))
TITLE		:= $(PROJ)

LIBS		:= -ltonc -lgba -lgba-sprite-engine -lugba

BUILD		:= build
SRCDIRS		:= src \
						 src/data/content \
						 src/data/content/_compiled_sprites \
						 src/data/custom \
						 src/gameplay \
						 src/gameplay/debug \
						 src/gameplay/models \
						 src/gameplay/multiplayer \
						 src/gameplay/save \
						 src/gameplay/video \
						 src/objects \
						 src/objects/base \
						 src/objects/score \
						 src/objects/score/combo \
						 src/objects/ui \
						 src/player \
						 src/player/core \
						 src/scenes \
						 src/scenes/base \
						 src/scenes/ui \
						 src/utils \
						 src/utils/gbfs \
						 src/utils/flashcartio \
						 src/utils/flashcartio/everdrivegbax5 \
						 src/utils/flashcartio/ezflashomega \
						 src/utils/flashcartio/fatfs \
						 src/utils/pool \
						 libs
DATADIRS	:=
INCDIRS		:= src
LIBDIRS		:= $(TONCLIB) $(LIBGBA) $(BASE_DIR)/libs/libgba-sprite-engine $(BASE_DIR)/libs/libugba

# --- switches ---

bMB		:= 0	# Multiboot build
bTEMPS	:= 0	# Save gcc temporaries (.i and .S files)
bDEBUG2	:= 0	# Generate debug info (bDEBUG2? Not a full DEBUG flag. Yet)

# === BUILD FLAGS =====================================================
# This is probably where you can stop editing
# NOTE: I've noticed that -fgcse and -ftree-loop-optimize sometimes muck
#	up things (gcse seems fond of building masks inside a loop instead of
#	outside them for example). Removing them sometimes helps

# --- Architecture ---

ARCH    := -mthumb-interwork -mthumb
RARCH   := -mthumb-interwork -mthumb
IARCH   := -mthumb-interwork -marm -mlong-calls

# --- Main flags ---

CFLAGS		:= -mcpu=arm7tdmi -mtune=arm7tdmi -Ofast
CFLAGS		+= -Wall
CFLAGS		+= $(INCLUDE)
CFLAGS		+= -ffast-math -fno-strict-aliasing

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -ffunction-sections -fdata-sections

ASFLAGS		:= $(ARCH) $(INCLUDE)
LDFLAGS 	:= $(ARCH) -Wl,--print-memory-usage,-Map,$(PROJ).map,--gc-sections

# --- switched additions ----------------------------------------------

# --- Multiboot ? ---
ifeq ($(strip $(bMB)), 1)
	TARGET	:= $(PROJ).mb
else
	TARGET	:= $(PROJ)
endif

# --- Save temporary files ? ---
ifeq ($(strip $(bTEMPS)), 1)
	CFLAGS		+= -save-temps
	CXXFLAGS	+= -save-temps
endif

# --- Debug info ? ---

ifeq ($(strip $(bDEBUG)), 1)
	CFLAGS		+= -DDEBUG -g
	CXXFLAGS	+= -DDEBUG -g
	ASFLAGS		+= -DDEBUG -g
	LDFLAGS		+= -g
else
	CFLAGS		+= -DNDEBUG
	CXXFLAGS	+= -DNDEBUG
	ASFLAGS		+= -DNDEBUG
endif

# --- Custom vars ? ---

GAMETITLE=piuGBA
GAMEMAKER=AGB
GAMECODE=AZCE # Megaman Zero (SRAM - 64kb)
MODE ?= auto
SONGS ?= src/data/content/songs
VIDEOLIB ?= src/data/content/piuGBA_videos
VIDEOENABLE ?= false
HQAUDIOLIB ?= src/data/content/piuGBA_audios
HQAUDIOENABLE ?= false
FAST ?= false
ENV ?= development
BOSS ?= true
ARCADE ?= false

ifeq ($(ENV), debug)
	CXXFLAGS += -DENV_DEBUG=true -DENV_DEVELOPMENT=true -DSENV_DEBUG=true -DSENV_DEVELOPMENT=true -DENV_ARCADE=$(ARCADE)
else ifeq ($(ENV), development)
	CXXFLAGS += -DENV_DEVELOPMENT=true -DSENV_DEVELOPMENT=true -DENV_ARCADE=$(ARCADE)
else
	CXXFLAGS += -DENV_ARCADE=$(ARCADE)
endif

# CXXFLAGS += -DCUSTOM_VAR_DEFINE

# === BUILD PROC ======================================================

ifneq ($(BUILD),$(notdir $(CURDIR)))

# Still in main dir:
# * Define/export some extra variables
# * Invoke this file again from the build dir
# PONDER: what happens if BUILD == "" ?

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=									\
	$(foreach dir, $(SRCDIRS) , $(CURDIR)/$(dir))	\
	$(foreach dir, $(DATADIRS), $(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

# --- List source and data files ---

CFILES		:=	$(foreach dir, $(SRCDIRS) , $(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir, $(SRCDIRS) , $(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir, $(SRCDIRS) , $(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir, $(DATADIRS), $(notdir $(wildcard $(dir)/*.*)))

# --- Set linker depending on C++ file existence ---
ifeq ($(strip $(CPPFILES)),)
	export LD	:= $(CC)
else
	export LD	:= $(CXX)
endif

# --- Define object file list ---
export OFILES	:=	$(addsuffix .o, $(BINFILES))					\
					$(CFILES:.c=.o) $(CPPFILES:.cpp=.o)				\
					$(SFILES:.S=.o)

# --- Create include and library search paths ---
export INCLUDE	:=	$(foreach dir,$(INCDIRS),-I$(CURDIR)/$(dir))	\
					$(foreach dir,$(LIBDIRS),-I$(dir)/include)		\
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	-L$(CURDIR) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

# --- Create BUILD if necessary, and run this makefile from there ---

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	arm-none-eabi-nm -Sn $(OUTPUT).elf > $(BUILD)/$(TARGET).map

all	: $(BUILD)

clean:
	@echo clean ...
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).gba $(TARGET).sav

else		# If we're here, we should be in the BUILD dir

DEPENDS	:= $(OFILES:.o=.d)

# --- Main targets ----

$(OUTPUT).gba	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)

-include $(DEPENDS)

endif		# End BUILD switch

# --- More targets ----------------------------------------------------

.PHONY: check-env clean assets start rebuild restart

check-env:
ifndef GBA_DIR
	$(warning Missing environment variable: GBA_DIR. See README.md)
	$(error "Aborting")
endif

assets: check-env
	./scripts/assets.sh

import: check-env
	node ./scripts/importer/src/importer.js --mode "$(MODE)" --directory "$(SONGS)" --videolib="$(VIDEOLIB)" --hqaudiolib="$(HQAUDIOLIB)" --boss=$(BOSS) --arcade=$(ARCADE) --fast=$(FAST) --videoenable=$(VIDEOENABLE) --hqaudioenable=$(HQAUDIOENABLE)
	cd src/data/content/_compiled_files && gbfs ../files.gbfs *

package: check-env $(BUILD)
	./scripts/package.sh "piugba.gba" "src/data/content/files.gbfs"

start: check-env package
	start "$(TARGET).out.gba"

rebuild: check-env clean package

restart: check-env rebuild
	start "$(TARGET).out.gba"

reimport: check-env import package
	start "$(TARGET).out.gba"

# EOF
