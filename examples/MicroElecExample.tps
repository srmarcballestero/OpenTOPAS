# ====================================================
# TOPAS MicroElec Physics Example
# ====================================================
# This example demonstrates how to use the MicroElec
# physics module in TOPAS for semiconductor simulations
# ====================================================

# World volume
s:Ge/World/Type = "TsBox"
s:Ge/World/Material = "Vacuum"
d:Ge/World/HLX = 1.0 cm
d:Ge/World/HLY = 1.0 cm
d:Ge/World/HLZ = 1.0 cm

# MicroElec target region (e.g., Silicon detector)
s:Ge/Target/Type = "TsBox"
s:Ge/Target/Parent = "World"
s:Ge/Target/Material = "G4_Cu"
d:Ge/Target/HLX = 0.5 cm
d:Ge/Target/HLY = 0.5 cm
d:Ge/Target/HLZ = 0.01 mm
d:Ge/Target/TransZ = 0.0 cm

b:Ts/UseQt = "True"

# Define MicroElec region - CRITICAL for MicroElec physics to activate
# Note: TOPAS automatically converts region names to lowercase
s:Ge/Target/AssignToRegionNamed = "microelec"

# Physics settings
sv:Ph/Default/Modules = 2 "g4em-microelec" "g4em-standard_opt4"

# Set very low production cuts to track low-energy electrons
d:Ph/Default/ForRegion/microelec/CutForAllParticles = 1 nm
d:Ph/Default/ForRegion/microelec/CutForElectron = 1 nm
d:Ph/Default/ForRegion/microelec/CutForPositron = 1 nm
d:Ph/Default/ForRegion/microelec/CutForGamma = 1 nm

# Critical: Set tracking cut for electrons to very low energy
# This overrides the default 990 eV Geant4 cut
d:Ph/Default/SetProductionCutLowerEdge = 1 eV
d:Ph/Default/LowestElectronEnergy = 1 eV

# Particle source
s:So/Demo/Type = "Beam"
s:So/Demo/Component = "BeamPosition"
s:So/Demo/BeamParticle = "Proton"
d:So/Demo/BeamEnergy = 70 MeV
u:So/Demo/BeamEnergySpread = 0.0
s:So/Demo/BeamPositionDistribution = "Flat"
s:So/Demo/BeamPositionCutoffShape = "Rectangle"
d:So/Demo/BeamPositionCutoffX = 0.1 cm
d:So/Demo/BeamPositionCutoffY = 0.1 cm
s:So/Demo/BeamAngularDistribution = "None"
i:So/Demo/NumberOfHistoriesInRun = 1

# Beam position
s:Ge/BeamPosition/Type = "Group"
s:Ge/BeamPosition/Parent = "World"
d:Ge/BeamPosition/TransZ = 0.5 cm

# Scoring
# s:Sc/DoseAtTarget/Quantity = "DoseToMedium"
# s:Sc/DoseAtTarget/Component = "Target"
# b:Sc/DoseAtTarget/OutputToConsole = "True"

# Graphics (optional - comment out for batch mode)
s:Gr/ViewA/Type = "OpenGL"
s:Gr/ViewA/ColorBy = "ParticleType"


# Run settings
i:Ts/ShowHistoryCountAtInterval = 100
b:Ts/PauseBeforeQuit = "False"
i:Ts/NumberOfThreads = 0
