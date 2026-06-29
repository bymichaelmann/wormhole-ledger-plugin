#******************************************************************************
# Makefile — Wormhole Portal Token Bridge Ledger Plugin
#
# Build using the Ledger app-builder Docker container:
#   docker run --rm -it -v "$(pwd):/app" \
#     ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest
#   make BOLOS_SDK=$NANOS_SDK
#******************************************************************************

APPNAME      = "Wormhole Portal"
APPVERSION_M = 1
APPVERSION_N = 0
APPVERSION_P = 0

include ethereum-plugin-sdk/standard_plugin.mk
