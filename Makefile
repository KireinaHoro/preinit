INCDIR	= include
SRCDIR	= src
LIBDIR	= lib
OUTDIR	= out
MKDIR	= mkdir -p $(OBJDIR) $(LIBDIR) $(OUTDIR)

CHOST	= aarch64-unknown-linux-gnu
CC	= $(CHOST)-gcc
CXX	= $(CHOST)-g++
CFLAGS	= -I$(INCDIR)
CXXFLAGS= $(CFLAGS) -Wall

_TARGETS= init
TARGETS	= $(patsubst %,$(OUTDIR)/%,$(_TARGETS))

_OBJS	= util/log_facility.o $(addsuffix .o,$(_TARGETS))
OBJS	= $(patsubst %,$(SRCDIR)/%,$(_OBJS))

$(TARGETS) : $(OBJS)
	$(MKDIR)
	$(CXX) -o $(OUTDIR)/$@ $(OBJS)

parentdir = $(notdir $(abspath $(dir $1)))

$(SRCDIR)/%.o : $(SRCDIR)/%.cc
	$(CXX) -c $(CXXFLAGS) -I$(INCDIR)/$(call parentdir,$@) -o $@ $<

$(SRCDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) -I$(INCDIR)/$(call parentdir,$@) -o $@ $<
