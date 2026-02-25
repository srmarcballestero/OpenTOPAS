#!/bin/bash
# Script to create dummy MicroElec material structure files for unsupported materials
# This prevents G4Exception when using MicroElec physics with materials that don't have
# official MicroElec support. The dummy files have very high energy limits so MicroElec
# models will never actually activate for these materials.

MICROELEC_DIR="/home/marc/Applications/GEANT4/G4DATA/G4EMLOW8.2/microelec/Structure"

# List of common Geant4 materials that might appear in simulations
# but aren't supported by MicroElec
MATERIALS=(
    "AIR"
    "WATER"
    "Galactic"
    "Vacuum"
    "Steel"
    "el"
    "ss"
    "acuum"
    "opper"
    "per"
    "Brass"
    "AIR"
    "Aluminum"
    "STAINLESS-STEEL"
    "POLYETHYLENE"
    "PMMA"
    "LUCITE"
)

# Template for dummy material file
create_dummy_file() {
    local material=$1
    local filepath="${MICROELEC_DIR}/Data_${material}.dat"

    # Skip if file already exists
    if [ -f "$filepath" ]; then
        echo "File already exists: $filepath (skipping)"
        return
    fi

    echo "Creating dummy file: $filepath"

    cat > "$filepath" << EOF
${material} Compound

1 WorkFunction eV 0
1 EnergyGap eV 0

1 EnergyPeak eV 15.6
1 EnergyLimit eV 1
1 EADL noUnit -1
1 WeaklyBoundShell noUnit 1

1 WeaklyBoundInitialEnergy eV 0

1 DielectricModelLowEnergyLimit_e eV 1e10
1 DielectricModelHighEnergyLimit_e MeV 1e10
1 DielectricModelLowEnergyLimit_p eV 1e10
1 DielectricModelHighEnergyLimit_p MeV 1e10
1 ElasticModelLowEnergyLimit eV 1e10
1 ElasticModelHighEnergyLimit eV 1e10
EOF
}

# Check if directory exists
if [ ! -d "$MICROELEC_DIR" ]; then
    echo "ERROR: MicroElec Structure directory not found: $MICROELEC_DIR"
    exit 1
fi

echo "Creating dummy MicroElec material files..."
echo "Directory: $MICROELEC_DIR"
echo ""

for material in "${MATERIALS[@]}"; do
    create_dummy_file "$material"
done

echo ""
echo "Done! Created dummy files with energy limits set to 1e10 eV."
echo "MicroElec models will not activate for these materials."
