# TARGET SETTINGS ==============================================================
MAIN       = list-locales
PREFIX     = /usr/local
BINDIR     = $(PREFIX)/bin

# TOOL SETTINGS ================================================================
CROSS_COMPILE :=
CC         = $(CROSS_COMPILE)gcc
CPP        = $(CROSS_COMPILE)g++
OBJCOPY    = $(CROSS_COMPILE)objcopy
OBJDUMP    = $(CROSS_COMPILE)objdump
SIZE       = $(CROSS_COMPILE)size
FIND       = find
XARGS      = xargs
RM         = rm -rf
SED        = sed
AVRDUDE    = avrdude
SREC_CAT   = srec_cat
MKDIR      = mkdir
INSTALL    = install

# GCC SETTINGS =================================================================
C_STD      = gnu99
DEPEND     = -E -MD -MP -MF

CPP_STD    =


INCLUDE    = -I./ -I../../src -I../../lib/librrf/src -I../../lib/qoi
DEFINES    = -DSCREEN_70E=1
OPTIMIZE   =
WARN       = -W -Wall -Wundef -Wextra

CFLAGS     = -std=$(C_STD) $(OPTIMIZE) $(WARN) -Wstrict-prototypes $(INCLUDE) $(DEFINES) -g
CPPFLAGS     = $(OPTIMIZE) $(WARN) $(INCLUDE) $(DEFINES) -g
LDFLAGS    =

# MAKE SETTINGS =============================================================
ifneq ($(V),1)
Q := @
endif

ECHO=@echo
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Linux)
        ECHO=@echo -e
endif

# SOURCES ========================================================================
MAIN_SRCS := list-locales.cpp
MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)
MAIN_DEPS := $(MAIN_SRCS:.cpp=.d)

# RULES ========================================================================

all: main
main: $(MAIN)

-include $(MAIN_DEPS)

%.d: %.c
	$(ECHO) "  DEP\t$@"
	$(Q)$(CC) $(CFLAGS) $(DEPEND) $@ -c $< 1>/dev/null

%.o: %.c
	$(ECHO) "  CC\t$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.cpp
	$(ECHO) "  DEP\t$@"
	$(Q)$(CPP) $(CPPFLAGS) $(DEPEND) $@ -c $< 1>/dev/null

%.o: %.cpp
	$(ECHO) "  CPP\t$@"
	$(Q)$(CPP) $(CPPFLAGS) -c -o $@ $<

$(MAIN): $(MAIN_OBJS) $(MAIN_DEPS)
	$(ECHO) "  LD\t$@"
	$(Q)$(MKDIR) -p $(@D)
	$(Q)$(CPP) $(LDFLAGS) -o $@ $(MAIN_OBJS) -Wl,-Map=$(MAIN).map

clean:
	$(FIND) . -regex '.*\.\(d\|map\|o\)$\' | $(XARGS) $(RM)
	$(RM) $(MAIN)

install: $(MAIN)
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) $(MAIN) $(DESTDIR)$(BINDIR)

.PHONY: all clean
