# World
s:Ge/World/Type = "TsBox"
s:Ge/World/Material = "Vacuum"
d:Ge/World/HLX = 2 m
d:Ge/World/HLY = 2 m
d:Ge/World/HLZ = 4 m

s:Ge/BoxWithField/Type = "TsBox"
s:Ge/BoxWithField/Parent = "World"
d:Ge/BoxWithField/HLX = 1.0 m
d:Ge/BoxWithField/HLY = 1.0 m
d:Ge/BoxWithField/HLZ = 2.0 m
d:Ge/BoxWithField/TransX = 0 m
d:Ge/BoxWithField/TransY = 0 m
d:Ge/BoxWithField/TransZ = 0 m
sc:Ge/BoxWithField/Material = "Vacuum"

i:Gr/View/MagneticFieldArrowDensity = 5
i:Gr/View/ElectricFieldArrowDensity = 5


s:Ge/BoxWithField/Field = "ElectroMagneticFieldMap"
s:Ge/BoxWithField/ElectricField3DTable = "./mytests/mapped_electromagnetic/electric_field.csv"
s:Ge/BoxWithField/MagneticField3DTable = "./mytests/mapped_electromagnetic/magnetic_field.csv"

s:So/Demo/Type = "Beam" # Beam, Isotropic, Emittance or PhaseSpace
s:So/Demo/Component = "BeamPosition"
s:So/Demo/BeamParticle = "e-"
d:So/Demo/BeamEnergy = 6000 eV
u:So/Demo/BeamEnergySpread = 0.0
s:So/Demo/BeamPositionDistribution = "None" # None, Flat or Gaussian

s:So/Demo/BeamAngularDistribution = "None" # None, Flat or Gaussian

i:So/Demo/NumberOfHistoriesInRun = 3

# Graphics
b:Ts/UseQt = "True"
s:Gr/View/Type = "OpenGL"
b:Gr/View/IncludeAxes = "False"