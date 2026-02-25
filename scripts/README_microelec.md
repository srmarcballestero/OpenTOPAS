# MicroElec Material Workaround

## Problem

Geant4's MicroElec physics requires material structure data files for **all materials** present in the simulation, even if MicroElec is only active in specific regions. The data files are located in:

```
$G4EMLOW/microelec/Structure/Data_<MATERIAL>.dat
```

By default, only these materials are supported:
- Si (Silicon)
- Cu (Copper)
- Ge (Germanium)
- Ag (Silver/Gold)
- Al (Aluminum)
- C (Carbon)
- Ni (Nickel)
- Ti (Titanium)
- W (Tungsten)
- KAPTON
- SILICON_DIOXIDE

If your simulation contains **any other material** (e.g., AIR, WATER, Vacuum, Steel, etc.), the simulation will crash during initialization with:

```
G4Exception : em0002
issued by : G4MicroElecMaterialStructure::ReadMaterialFile
file .../Data_<MATERIAL>.dat not found!
*** Fatal Exception *** core dump ***
```

## Solution

Run the provided script to create dummy material structure files:

```bash
./scripts/create_dummy_microelec_materials.sh
```

This script creates placeholder files for common materials (AIR, WATER, Vacuum, Steel, Brass, etc.) with energy limits set to `1e10 eV`. This ensures:

1. ✅ No initialization crash
2. ✅ MicroElec models **never activate** for these materials (limits too high)
3. ✅ MicroElec only works in the designated "microelec" region with supported materials

## Adding More Materials

If you encounter a crash for a material not in the script, manually create a dummy file:

```bash
cat > $G4EMLOW/microelec/Structure/Data_<YOUR_MATERIAL>.dat << 'EOF'
<YOUR_MATERIAL> Compound

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
```

Replace `<YOUR_MATERIAL>` with the exact material name from the error message.

## Important Notes

- This is a **workaround** for a Geant4 MicroElec limitation
- The dummy files do NOT enable MicroElec for those materials
- MicroElec physics still only works correctly in regions with officially supported materials (Si, Cu, Al, etc.)
- The high energy limits (1e10 eV) ensure MicroElec models never activate for dummy materials
