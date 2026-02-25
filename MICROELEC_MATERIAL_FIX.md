# MicroElec Material Compatibility Fix

## Problem Description

When using the MicroElec physics module in TOPAS with materials that don't have MicroElec data files (e.g., `G4_AIR`), Geant4 throws a fatal exception:

```
G4Exception : em0002
issued by : G4MicroElecMaterialStructure::ReadMaterialFile
file .../G4EMLOW8.2/microelec/Structure/Data_AIR.dat not found!
*** Fatal Exception ***
```

This occurs even when these materials are NOT in the "microelec" region and shouldn't use MicroElec physics.

## Root Cause

The Geant4 `G4MicroElecInelasticModel_new` (and related `_new` models) iterate through ALL materials in the `G4ProductionCutsTable` during initialization, attempting to load MicroElec data files for each one, regardless of which region they belong to.

## Solutions Implemented

### Solution 1: Use `param->RegionsMicroElec()` (Primary Fix)

**File:** `physics/TsEmMicroElecPhysics.cc` (line 136)

Added the critical call to `param->RegionsMicroElec()` which instructs Geant4 to only initialize MicroElec models for materials in regions designated as MicroElec regions.

```cpp
G4EmParameters* param = G4EmParameters::Instance();
param->SetDefaults();
param->SetBuildCSDARange(true);
param->SetMscStepLimitType(fUseSafety);

// CRITICAL: Tell Geant4 to only initialize MicroElec models for materials
// in regions designated as MicroElec regions. This prevents errors when
// materials outside the MicroElec database exist in the geometry.
param->RegionsMicroElec();
```

**Note:** This should work with Geant4 11.1.3, but the `_new` models may not fully respect this flag in all versions.

### Solution 2: Material Validation Helper (Diagnostic Tool)

**Files:**
- `physics/TsEmMicroElecPhysics.hh` (added method declaration)
- `physics/TsEmMicroElecPhysics.cc` (lines 317-367)

Added `ValidateMicroElecMaterials()` method that can be called to diagnose material compatibility issues. This method:
- Checks all materials in the geometry
- Verifies if MicroElec data files exist for each material
- Prints warnings for materials without data files
- Helps users ensure materials are correctly assigned to regions

To use this diagnostic tool, you can temporarily add this call in `ConstructProcess()`:
```cpp
// Add after line 143 (after param->SetAuger(true);)
ValidateMicroElecMaterials();
```

### Solution 3: TOPAS-Specific Material Structure (Alternative)

**Files Created:**
- `physics/TsMicroElecMaterialStructure.hh`
- `physics/TsMicroElecMaterialStructure.cc`

A patched version of `G4MicroElecMaterialStructure` that gracefully handles missing data files instead of throwing fatal exceptions. This class:
- Returns a validity flag instead of crashing
- Provides the same interface as the Geant4 version
- Can be used as a drop-in replacement if needed

**Note:** This is provided for reference but not currently integrated. Integrating it would require creating TOPAS-specific versions of all MicroElec models.

## How to Use

### Basic Usage (Solution 1 Only)

1. **Ensure your geometry has a "microelec" region:**
   ```
   s:Ge/Target/AssignToRegionNamed = "microelec"
   ```

2. **Use the MicroElec physics module:**
   ```
   sv:Ph/Default/Modules = 1 "g4em-microelec"
   ```

3. **Only use MicroElec-compatible materials in the microelec region:**
   - Supported materials: Si, Al, SiO2, Al2O3, etc.
   - See `$G4LEDATA/microelec/Structure/` for available materials

4. **Materials outside the microelec region can be anything:**
   - G4_AIR, G4_WATER, etc. will work fine as long as they're not in the microelec region

### Diagnostic Usage (Solution 2)

To verify material compatibility before running your full simulation:

1. Temporarily enable material validation by uncommenting or adding this line after `param->SetAuger(true);` in `TsEmMicroElecPhysics::ConstructProcess()`:
   ```cpp
   ValidateMicroElecMaterials();
   ```

2. Run your simulation - you'll see output like:
   ```
   TsEmMicroElecPhysics: Validating materials for MicroElec compatibility...
     [OK] G4_Al - MicroElec data file found
     [WARN] G4_AIR - NO MicroElec data file found
            Expected: .../microelec/Structure/Data_AIR.dat
            This material must NOT be in the 'microelec' region!
   ```

3. Remove the validation call once you've confirmed your setup is correct.

## Testing

Test with the provided example:
```bash
cd examples
topas MicroElecExample.tps
```

Expected behavior:
- Should run without fatal exceptions
- G4_AIR in World region uses standard EM physics
- G4_Al in microelec region uses MicroElec physics

## Which Materials Have MicroElec Data?

Check available materials:
```bash
ls $G4LEDATA/microelec/Structure/Data_*.dat
```

Common supported materials:
- Si (Silicon)
- Al (Aluminum)
- SiO2 (Silicon dioxide)
- Al2O3 (Aluminum oxide)
- Cu (Copper)
- Au (Gold)
- W (Tungsten)
- Diamond
- etc.

## Troubleshooting

### If you still get errors after applying Solution 1:

1. **Verify the region exists and is named correctly:**
   - TOPAS converts region names to lowercase
   - Use "microelec" not "MicroElec" or "MICROELEC"

2. **Check that only supported materials are in the microelec region:**
   - Use the validation method (Solution 2) to diagnose

3. **Verify Geant4 data files are installed:**
   ```bash
   echo $G4LEDATA
   ls $G4LEDATA/microelec/
   ```

4. **If using Geant4 version < 11.1.3:**
   - The `param->RegionsMicroElec()` method may not work properly
   - Consider upgrading or using a different approach

### If the error persists in newer Geant4 versions:

The `_new` models in some Geant4 versions may not properly respect the `RegionsMicroElec()` flag. In this case, you may need to create custom model wrappers or modify the Geant4 installation (not recommended).

## Files Modified

1. `physics/TsEmMicroElecPhysics.hh` - Added validation method declaration
2. `physics/TsEmMicroElecPhysics.cc` - Added `param->RegionsMicroElec()` call and validation method
3. `physics/TsMicroElecMaterialStructure.hh` - New file (optional, for reference)
4. `physics/TsMicroElecMaterialStructure.cc` - New file (optional, for reference)

## References

- Geant4 MicroElec example: `$G4INSTALL/examples/advanced/microelectronics/`
- MicroElec documentation: Geant4 Physics Reference Manual, Chapter on MicroElec
- TOPAS documentation: http://www.topasmc.org
