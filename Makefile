include config.mk

# dependencies for backends
ifeq ($(USE_LIBUSB),1)
export CFLAGS += $(shell pkg-config --cflags libusb-1.0)
export LIBS   += $(shell pkg-config --libs libusb-1.0)
else ifeq ($(USE_HIDRAW),1)
export LIBS   += -ludev
endif

.PHONY: all
all: lib conf

.PHONY: lib
lib:
	$(MAKE) -C $@

.PHONY: conf
conf: lib
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(MAKE) -C lib clean
	$(MAKE) -C conf clean
