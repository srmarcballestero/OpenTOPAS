#!/bin/bash

src_dir="$HOME/GitHub/OpenTOPAS"
extensions_flag=""
build_name="build"

while [[ $# -gt 0 ]]; do
    case $1 in
        --extensions-dir)
            extensions_flag="-DTOPAS_EXTENSIONS_DIR=$src_dir/extensions/"
            shift
            ;;
        --build-name)
            build_name="$2"
            shift 2
            ;;
        *)
            echo "Unknown option $1"
            exit 1
            ;;
    esac
done

cd $src_dir

mkdir -p builds/$build_name
mkdir -p installs/$build_name

rm -rf builds/$build_name/*
rm -rf installs/$build_name/*

cd builds/$build_name

export Geant4_DIR=$HOME/Applications/GEANT4/geant4-install
export GDCM_DIR=$HOME/Applications/GDCM/gdcm-install

if ! cmake ../.. -DCMAKE_INSTALL_PREFIX=../../installs/$build_name $extensions_flag; then
    echo "Error: CMake configuration failed"
    exit 1
fi

if ! make -j20 install; then
    echo "Error: Build and installation failed"
    exit 1
fi

echo "Done. The new executable is in $src_dir/installs/$build_name/bin"

echo "Creating setup script at $src_dir/installs/$build_name/setup_env.sh"
cat <<EOL > "../../installs/$build_name/setup_env.sh"
#!/bin/bash

topas_dir="\$HOME/GitHub/OpenTOPAS/installs/$build_name"

export TOPAS_G4_DATA_DIR="\$HOME/Applications/GEANT4/G4DATA"
if [ -d "\$HOME/Applications/GEANT4/geant4-install/lib64" ]; then
    export LD_LIBRARY_PATH="\$HOME/Applications/GEANT4/geant4-install/lib64:\$LD_LIBRARY_PATH"
else
    export LD_LIBRARY_PATH="\$HOME/Applications/GEANT4/geant4-install/lib:\$LD_LIBRARY_PATH"
fi

export QT_QPA_PLATFORM_PLUGIN_PATH="\$topas_dir/Frameworks"
export LD_LIBRARY_PATH="\$topas_dir/lib:\$LD_LIBRARY_PATH"
EOL

chmod +x "../../installs/$build_name/setup_env.sh"

echo "Creating run script at $src_dir/installs/$build_name/run_topas.sh"
cat <<EOL > "../../installs/$build_name/run_topas.sh"
#!/bin/bash
topas_dir="\$HOME/GitHub/OpenTOPAS/installs/$build_name"
source \$topas_dir/setup_env.sh
\$topas_dir/bin/topas \$@
EOL

chmod +x "../../installs/$build_name/run_topas.sh"
echo "You can run the new executable using $src_dir/installs/$build_name/run_topas.sh"