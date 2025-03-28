#!/bin/bash
ROOT="../.."
if [ -f "$ROOT/variables.sh" ]; then
	. "$ROOT/variables.sh"
fi
. "$ROOT/ghost.sh"

# Build configuration
ARTIFACT_NAME="js.bin"
CFLAGS_ADD="-DDUK_F_GHOST"

# Include application build tasks
. "../applications.sh"
