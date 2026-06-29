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

# Workaround for glyph naming mismatch: standard_plugin.mk sets
# ICONGLYPH=C_stax_<name>_64px but uses ICON_STAX=icons/stax_app_<name>.gif,
# which causes the glyph tool to generate C_stax_app_<name> (name from file).
# Point ICON_STAX at a file whose basename produces the expected constant.
ICON_STAX := icons/stax_wormhole_portal_64px.gif
