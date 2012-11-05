#!/bin/bash

# Rebuild the file README.packages.
# This will only work on an apt-based system such as ubuntu.
# On other systems, just read the file and don't regenerate it.
# (Or fix this script.)

cat packages.header > README.packages
dpkg -l | egrep "$(tr '\n' '|' < packages.in )xxxxx" | egrep '^ii' | awk '{print $2}' | perl -pwe 's/^/  /;' >> README.packages
