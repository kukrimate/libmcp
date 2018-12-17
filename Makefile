OBJ = mcp2210.o
HDR = hid.h mcp2210.h

# Operating system dependent
ifeq ($(shell uname -s),Linux)
OBJ  += hid_linux.o
LIBS += -ludev
else
$(error Your platform is not supported at the moment!)
endif

APPOBJ = conftool.o
APPHDR = config.h

.PHONY: all
all: libmcp2210.so libmcp2210.a mcp2210_conf

# Shared library
libmcp2210.so: $(OBJ) $(HDR)
	$(CC) $(LDFLAGS) -shared $(OBJ) -o $@ $(LIBS)

# Static library
libmcp2210.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)

# Configuration tool
mcp2210_conf: $(APPOBJ) $(APPHDR)
	$(CC) $(LDFLAGS) $(APPOBJ) libmcp2210.a -o $@ $(LIBS)

# Compile C code
%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) libmcp2210.so libmcp2210.a $(APPOBJ) mcp2210_conf
