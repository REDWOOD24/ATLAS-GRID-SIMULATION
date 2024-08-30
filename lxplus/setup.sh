#!/bin/sh

if [ "${ATLAS_LOCAL_ROOT_BASE}" = "" ]; then
  export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
fi
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
lsetup "cmake 3.21.3"
lsetup git
lsetup "views LCG_105 x86_64-el9-gcc13-opt"

