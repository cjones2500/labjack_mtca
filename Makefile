APPS = mtcat
SHLIB = libmtcat_lj

CFLAGS = -O2 -g -W -Wall -fPIC
LDFLAGS = -bind_at_load
#CC = clang
CC = gcc

SOMAKER = $(CC) 
#SOFLAGS =  -dynamiclib
SOFLAGS =  -shared -rdynamic -Wl,-soname,${SHLIB}.${SOSUFFIX}
#SOSUFFIX = dylib
SOSUFFIX = so
LIBS = -llabjackusb -lusb-1.0 -lm
LDFLAGS = -L.

SOURCESSCRATCH = $(wildcard *.c)
SOURCES = $(filter-out $(patsubst %, %.c, $(APPS)), $(SOURCESSCRATCH))
#SOURCES = $(wildcard OR*.cc)
OBJECTS = $(SOURCES:.c=.o)

all: depend $(SHLIB) $(APPS)

.depend depend:
	@echo Checking dependencies...
	@$(CC) -M *.c > .depend

$(SHLIB): $(OBJECTS) 
	$(SOMAKER) $(SOFLAGS) -o $(SHLIB).$(SOSUFFIX) $(OBJECTS) $(LIBS)

.c.o: 
	$(CC) $(CFLAGS) -c $<

%: %.o $(OBJECTS)
	#$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lmtcat_lj
	$(CC) $(CFLAGS) -o $@ $< -ldl

clean:
	@echo "Cleaning for: $(SHLIB) $(APPS)"
	@rm -f $(SHLIB).$(SOSUFFIX) $(APPS) *.o *~ .depend

ifneq ($(MAKECMDGOALS), clean)
-include .depend
endif

