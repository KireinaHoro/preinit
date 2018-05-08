INCDIR	= include
SRCDIR	= src
OUTDIR	= out
MKDIR	= mkdir -p $(OUTDIR)

CHOST	= aarch64-unknown-linux-gnu
CC	= $(CHOST)-gcc
CXX	= $(CHOST)-g++
STRIP	= $(CHOST)-strip
CFLAGS	= -I$(INCDIR) -static
CXXFLAGS= $(CFLAGS) -Wall

_TARGETS= init
TARGETS	= $(patsubst %,$(OUTDIR)/%,$(_TARGETS))

_DEPS	= util/log_facility.o

_OBJS	= $(addsuffix .o,$(_TARGETS)) $(_DEPS)
OBJS	= $(patsubst %,$(SRCDIR)/%,$(_OBJS))

$(TARGETS) : $(OBJS)
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	$(STRIP) $@

parentdir = $(notdir $(abspath $(dir $1)))

$(SRCDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.h
	$(CXX) -c $(CXXFLAGS) -I$(INCDIR)/$(call parentdir,$@) -o $@ $<

$(SRCDIR)/%.o : $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) -c $(CFLAGS) -I$(INCDIR)/$(call parentdir,$@) -o $@ $<

.PHONY : clean
clean :
	rm -f $(OBJS) $(OUTDIR)/*
