#!/bin/bash

topas_dir="$HOME/GitHub/OpenTOPAS/installs/$1"

export TOPAS_G4_DATA_DIR=$HOME/Applications/GEANT4/G4DATA
export LD_LIBRARY_PATH=$HOME/Applications/GEANT4/geant4-install/lib64:$LD_LIBRARY_PATH

export QT_QPA_PLATFORM_PLUGIN_PATH=$topas_dir/Frameworks
export LD_LIBRARY_PATH=$topas_dir/lib:$LD_LIBRARY_PATH