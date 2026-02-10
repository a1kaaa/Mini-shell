.PHONY: all clean fclean make_dir

# Disable implicit rules
.SUFFIXES:
CC=gcc
SRCDIR=src
OBJDIR=obj
EXEC=shell
EXECDIR=bin
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS=-Wall -g
CPPFLAGS=-Iinclude

# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions 
#LIBS += -lsocket -lnsl -lrt
LIBS+=-lpthread

all: make_dir $(EXECDIR)/$(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(EXECDIR)/$(EXEC):  $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

make_dir:
	-mkdir $(OBJDIR)
	-mkdir $(EXECDIR)

clean:
	-rm -rf $(OBJDIR)

fclean:
	make clean
	-rm -rf $(EXECDIR)
