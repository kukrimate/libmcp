#
# libmcp2210 global make configuration
#

# backends
export USE_LIBUSB := 1
export USE_HIDRAW := 0

# tools
export CC     := cc
export CFLAGS := -std=c99 -pedantic -D_XOPEN_SOURCE=700 -Wall -Wextra
