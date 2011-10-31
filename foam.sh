#!/bin/bash

# Execute FoamVis assuming all libraries are in the same folder 
# as the executable.
DYLD_LIBRARY_PATH=`pwd` ./foam "$@"
