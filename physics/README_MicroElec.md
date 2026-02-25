# TOPAS MicroElec Physics Module

## Overview

The TsEmMicroElecPhysics module provides integration of Geant4's MicroElec library into TOPAS as a modular physics constructor. MicroElec is designed for simulating particle interactions in semiconductor materials and microelectronic devices at very low energies (down to 0.1 eV).

## Features

### Supported Particles
- **Electrons** (e-)
- **Positrons** (e+)
- **Protons** (p)
- **Alpha particles** (alpha)
- **Generic ions** (GenericIon)

### Physics Processes

1. **Elastic Scattering** (`G4MicroElecElastic`)
   - Model: `G4MicroElecElasticModel_new`
   - Energy range: 0.1 eV - 100 MeV (electrons)

2. **Inelastic Scattering/Ionization** (`G4MicroElecInelastic`)
   - Model: `G4MicroElecInelasticModel_new`
   - Energy range: 0.1 eV - 10 MeV (electrons), 100 eV - 10 MeV (heavy particles)

3. **LO Phonon Scattering** (`G4MicroElecLOPhononScattering`)
   - Model: `G4MicroElecLOPhononModel`
   - Designed for dielectric materials (SiO2, Al2O3)
   - Energy range: 0.1 eV - 10 MeV

4. **Surface Processes** (`G4MicroElecSurface`)
   - Handles particle interactions at material boundaries
   - Work function-dependent transmission

### EM Parameters
- Minimum energy: 0.1 eV
- Maximum energy: 10 TeV
- Lowest electron energy: 0 eV
- Bins per decade: 20
- CSDA range: Enabled
- Auger cascade: Enabled
- Angular generator for ionization: Enabled

## Usage

### 1. Basic Setup

Add the MicroElec physics module to your TOPAS parameter file:

```
sv:Ph/Default/Modules = 2 "g4em-standard_opt4" "g4em-microelec"
```

### 2. Define MicroElec Region

**CRITICAL**: MicroElec processes are only active in regions named "microelec". You must assign your target geometry to this region:

**Note**: TOPAS automatically converts all region names to lowercase, so use lowercase "microelec" in your parameter file.

```
s:Ge/Target/Type = "TsBox"
s:Ge/Target/Material = "G4_Si"
s:Ge/Target/AssignToRegionNamed = "microelec"
```

### 3. Complete Example

See `examples/MicroElecExample.txt` for a complete working example.

## Region-Based Physics

The MicroElec module uses a two-region approach:

### World Region
- Standard EM processes are active
- Multiple scattering: `G4UrbanMscModel`
- Ionization: Standard Geant4 models
- MicroElec processes use dummy models (inactive)

### MicroElec Region
- MicroElec processes become active
- Standard processes are deactivated below certain energies:
  - e- MSC: deactivated below 100 MeV
  - e- ionization: deactivated below 10 MeV
  - Proton ionization: deactivated below 10 MeV
- MicroElec models take over at low energies

## Materials

MicroElec models work best with semiconductor and dielectric materials:
- Silicon (G4_Si)
- Silicon Dioxide (G4_SILICON_DIOXIDE)
- Aluminum Oxide (G4_ALUMINUM_OXIDE)
- Other materials supported via `G4MicroElecMaterialStructure`

## Energy Ranges

### Electrons
- Elastic: 0.1 eV - 100 MeV
- Inelastic: 0.1 eV - 10 MeV
- LO Phonon: 0.1 eV - 10 MeV

### Protons
- Inelastic: 100 eV - 10 MeV

### Heavy Ions (alpha, GenericIon)
- Inelastic: 100 eV - 10 MeV

## Implementation Details

### Files
- **Header**: `physics/TsEmMicroElecPhysics.hh`
- **Source**: `physics/TsEmMicroElecPhysics.cc`
- **Registration**: `physics/TsModularPhysicsList.cc` (line 143)

### Registration String
```cpp
"g4em-microelec"
```

### Constructor Pattern
The module follows the TOPAS physics constructor pattern:
- Inherits from `G4VPhysicsConstructor`
- Provides two constructors (with/without `TsParameterManager`)
- Implements `ConstructParticle()` and `ConstructProcess()`
- Uses Geant4 physics constructor factory

### Process Registration
Processes are registered globally but use `G4DummyModel` in the World region. The actual MicroElec models are activated only in the "MicroElec" region using `G4EmConfigurator`.

## Atomic Deexcitation

The module automatically sets up atomic deexcitation:
- Fluorescence: Enabled
- Auger electrons: Enabled
- PIXE (Particle-Induced X-ray Emission): Enabled

## Comparison with DNA Physics

| Feature | MicroElec | DNA |
|---------|-----------|-----|
| Target | Semiconductors | Biological tissue (water) |
| Materials | Si, SiO2, Al2O3 | Water, DNA components |
| Min Energy | 0.1 eV | ~7.4 eV |
| Phonons | Yes (LO phonons) | No |
| Surface | Yes | No |

## References

1. Geant4 MicroElec example: `examples/advanced/microelectronics/`
2. Geant4 Physics Reference Manual (Chapter on MicroElec)
3. TOPAS Documentation: Physics Modules

## Notes

- MicroElec physics is computationally intensive due to very low energy tracking
- Production cuts should be set appropriately (typically < 1 μm)
- Use single-threaded mode for initial testing
- The region name "MicroElec" is hard-coded in the implementation

## Troubleshooting

### MicroElec not activating
- Verify region is named exactly "MicroElec" (case-sensitive)
- Check that geometry is assigned to the region
- Ensure module is listed in `Ph/Default/Modules`

### Simulation very slow
- MicroElec tracks particles to very low energies
- Consider increasing production cuts if appropriate
- Reduce number of histories for testing

### Compilation errors
- Verify Geant4 version supports MicroElec (Geant4 10.3+)
- Ensure Geant4 was built with appropriate options
