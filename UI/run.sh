#!/bin/bash
# Launcher script to run the application with clean environment
# This avoids library conflicts from snap packages

env -i \
    HOME="$HOME" \
    DISPLAY="$DISPLAY" \
    XAUTHORITY="$XAUTHORITY" \
    PATH=/usr/local/bin:/usr/bin:/bin \
    ./build/start
