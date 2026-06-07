#******************************************************************************
# Makefile — Wormhole Portal Token Bridge Ledger Plugin
#
# Build targets for all Ledger devices (nanos, nanosp, nanox, stax).
# Expects the ethereum-plugin-sdk submodule at ethereum-plugin-sdk/.
#
# Set BOLOS_SDK or ETHEREUM_PLUGIN_SDK to the SDK path.
#******************************************************************************

ifeq ($(BOLOS_SDK),)
ifneq ($(ETHEREUM_PLUGIN_SDK),)
BOLOS_SDK := $(ETHEREUM_PLUGIN_SDK)
endif
endif

ifneq ($(BOLOS_SDK),)
include $(BOLOS_SDK)/Makefile.defines
endif

# Plugin name (must match the directory name expected by the SDK).
APPNAME = "Wormhole Portal"

# Source files.
APP_SOURCE_PATH += src

# Build directories per device.
TARGET_DIRS = nanos nanosp nanox stax

.PHONY: all clean build

all: $(TARGET_DIRS)

# Build for all device targets using the SDK.
$(TARGET_DIRS):
	$(MAKE) -C $(BOLOS_SDK)/plugin $@ TARGET=$@

# Convenience: build for a single device or all.
build: all

clean:
	rm -rf $(TARGET_DIRS) bin/ build/
