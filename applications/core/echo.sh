#!/bin/bash
ROOT="../.."
if [ -f "$ROOT/variables.sh" ]; then
	. "$ROOT/variables.sh"
fi
. "$ROOT/ghost.sh"


# Define build setup
SRC="src-echo"
OBJ="obj-echo"
ARTIFACT_NAME="echo.bin"

# Include application build tasks
. "../applications.sh"
