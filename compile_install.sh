#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

# ----------------------------
# User-configurable variables
# ----------------------------
TOPAS_SRC_DIR="$HOME/Projects/OpenTOPAS"
BUILD_NAME="build"
EXTENSIONS_FLAG=""
NO_GRAPHICS="ON"
REBUILD_GEANT4=false
NUM_JOBS=20

# Default Geant4 paths
GEANT4_SRC_DIR="$TOPAS_SRC_DIR/external/geant4"

GDCM_DIR="$HOME/Software/GDCM/gdcm-install"

# ----------------------------
# Functions
# ----------------------------
print_usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

Options:
  --extensions-dir             Use TOPAS extensions directory
  --build-name <name>          Specify build/install directory name (default: build)
  --geant4-install-dir <dir>   Specify Geant4 install directory
  --no-graphics                Disable Geant4 graphics
  --rebuild-geant4             Force rebuild of Geant4
EOF
}

build_geant4() {
    echo "Rebuilding Geant4..."
    echo "Installing Geant4 to $GEANT4_INSTALL_DIR"
    mkdir -p "$GEANT4_SRC_DIR/builds/$BUILD_NAME"
    mkdir -p "$GEANT4_INSTALL_DIR"
    pushd "$GEANT4_SRC_DIR/builds/$BUILD_NAME" > /dev/null

    cmake "$GEANT4_SRC_DIR" \
        -DGEANT4_INSTALL_DATA=ON \
        -DGEANT4_BUILD_MULTITHREADED=ON \
        -DCMAKE_INSTALL_PREFIX="$GEANT4_INSTALL_DIR" \
        -DCMAKE_PREFIX_PATH="/usr/lib/qt5" \
        -DGEANT4_USE_QT="$NO_GRAPHICS" \
        -DGEANT4_USE_OPENGL_X11="$NO_GRAPHICS" \
        -DGEANT4_USE_RAYTRACER_X11="$NO_GRAPHICS"

    make -j"$NUM_JOBS" install
    popd > /dev/null
}

build_topas() {
    echo "Building TOPAS..."
    mkdir -p "$TOPAS_SRC_DIR/builds/$BUILD_NAME"
    mkdir -p "$TOPAS_SRC_DIR/installs/$BUILD_NAME"

    # Optional: clean old builds
    rm -rf "$TOPAS_SRC_DIR/builds/$BUILD_NAME"/*
    rm -rf "$TOPAS_SRC_DIR/installs/$BUILD_NAME"/*

    pushd "$TOPAS_SRC_DIR/builds/$BUILD_NAME" > /dev/null
    cmake ../.. -DCMAKE_INSTALL_PREFIX="../../installs/$BUILD_NAME" $EXTENSIONS_FLAG
    make -j"$NUM_JOBS" install
    popd > /dev/null

    echo "TOPAS installed in $TOPAS_SRC_DIR/installs/$BUILD_NAME/bin"
}

create_setup_scripts() {
    local install_dir="$TOPAS_SRC_DIR/installs/$BUILD_NAME"

    # Setup environment script
    cat <<EOL > "$install_dir/setup_env.sh"
#!/bin/bash
TOPAS_DIR="$install_dir"

export TOPAS_G4_DATA_DIR="$GEANT4_INSTALL_DIR/share/Geant4/data"
if [ -d "$GEANT4_INSTALL_DIR/lib64" ]; then
    export LD_LIBRARY_PATH="$GEANT4_INSTALL_DIR/lib64:\$LD_LIBRARY_PATH"
else
    export LD_LIBRARY_PATH="$GEANT4_INSTALL_DIR/lib:\$LD_LIBRARY_PATH"
fi

export QT_QPA_PLATFORM_PLUGIN_PATH="\$TOPAS_DIR/Frameworks"
export LD_LIBRARY_PATH="\$TOPAS_DIR/lib:\$LD_LIBRARY_PATH"
EOL
    chmod +x "$install_dir/setup_env.sh"

    # Run TOPAS script
    cat <<EOL > "$install_dir/run_topas.sh"
#!/bin/bash
TOPAS_DIR="$install_dir"
source \$TOPAS_DIR/setup_env.sh
\$TOPAS_DIR/bin/topas \$@
EOL
    chmod +x "$install_dir/run_topas.sh"

    echo "Setup script created: $install_dir/setup_env.sh"
    echo "Run TOPAS with: $install_dir/run_topas.sh"
}


# ----------------------------
# Argument Parsing
# ----------------------------
while [[ $# -gt 0 ]]; do
    case $1 in
        --extensions-dir)
            EXTENSIONS_FLAG="-DTOPAS_EXTENSIONS_DIR=$TOPAS_SRC_DIR/extensions/"
            shift
            ;;
        --build-name)
            BUILD_NAME="$2"
            shift 2
            ;;
        --geant4-install-dir)
            GEANT4_INSTALL_DIR="$2"
            shift 2
            ;;
        --no-graphics)
            NO_GRAPHICS="OFF"
            shift
            ;;
        --rebuild-geant4)
            REBUILD_GEANT4=true
            shift
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# ----------------------------
# Main Execution
# ----------------------------

if [ "$REBUILD_GEANT4" = true ]; then
    # If rebuilding, use default install path unless user passed one
    GEANT4_INSTALL_DIR="${GEANT4_INSTALL_DIR:-$GEANT4_SRC_DIR/installs/$BUILD_NAME}"
    build_geant4
else
    # If not rebuilding, enforce that user supplied --geant4-install-dir
    if [ -z "${GEANT4_INSTALL_DIR:-}" ]; then
        echo "Error: --geant4-install-dir must be provided if --rebuild-geant4 is not used."
        print_usage
        exit 1
    fi
fi

# Automatically detect Geant4 version subdirectory
if [ -d "$GEANT4_INSTALL_DIR/lib" ]; then
    GEANT4_DIR=$(find "$GEANT4_INSTALL_DIR/lib" -maxdepth 1 -type d -name "Geant4-*" | head -n1)
elif [ -d "$GEANT4_INSTALL_DIR/lib64" ]; then
    GEANT4_DIR=$(find "$GEANT4_INSTALL_DIR/lib64" -maxdepth 1 -type d -name "Geant4-*" | head -n1)
else
    GEANT4_DIR=""
fi

if [ -z "$GEANT4_DIR" ]; then
    echo "Error: Could not detect Geant4 version directory in $GEANT4_INSTALL_DIR/lib or lib64"
    exit 1
fi

export Geant4_DIR="$GEANT4_INSTALL_DIR"
export GDCM_DIR="$GDCM_DIR"

# Build TOPAS
build_topas

# Create setup and run scripts
create_setup_scripts

echo "Installation completed successfully!"

