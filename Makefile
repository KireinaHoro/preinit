INCDIR	= include
SRCDIR	= src
OUTDIR	= out
MKDIR	= mkdir -p $(OUTDIR)

CHOST	= aarch64-unknown-linux-gnu
CC	= $(CHOST)-gcc
CXX	= $(CHOST)-g++
STRIP	= $(CHOST)-strip
CFLAGS	= -I$(INCDIR) -static
CXXFLAGS= $(CFLAGS) -Wall -std=c++17

_TARGETS= init
TARGETS	= $(patsubst %,$(OUTDIR)/%,$(_TARGETS))

_DEPS	= util/log_facility.o

_OBJS	= $(addsuffix .o,$(_TARGETS)) $(_DEPS)
OBJS	= $(patsubst %,$(SRCDIR)/%,$(_OBJS))

LIBS	=

$(TARGETS) : $(OBJS)
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)
	$(STRIP) $@

$(SRCDIR)/%.o : $(SRCDIR)/%.cpp $(INCDIR)/%.h
	$(CXX) -c $(CXXFLAGS) -I$(INCDIR)/$(dir $*) -o $@ $<

$(SRCDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.h
	$(CXX) -c $(CXXFLAGS) -I$(INCDIR)/$(dir $*) -o $@ $<

$(SRCDIR)/%.o : $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) -c $(CFLAGS) -I$(INCDIR)/$(dir $*) -o $@ $<

.PHONY : clean
clean :
	rm -f $(OBJS) $(OUTDIR)/*
