# World
s:Ge/World/Type = "TsBox"
s:Ge/World/Material = "Vacuum"
d:Ge/World/HLX = 2 m
d:Ge/World/HLY = 2 m
d:Ge/World/HLZ = 2 m

s:Ge/BoxWithField/Type = "TsBox"
s:Ge/BoxWithField/Parent = "World"
d:Ge/BoxWithField/HLX = 1.0 m
d:Ge/BoxWithField/HLY = 1.0 m
d:Ge/BoxWithField/HLZ = 1.0 m
d:Ge/BoxWithField/TransX = 0 m
d:Ge/BoxWithField/TransY = 0 m
d:Ge/BoxWithField/TransZ = 0 m
sc:Ge/BoxWithField/Material = "Vacuum"

# s:Ge/BoxWithField/Field = "MappedMagnet"
# s:Ge/BoxWithField/MagneticField3DTable = "../mytests/B_fit_withcup_at_-40percent_Opera.TABLE"
# i:Gr/View/MagneticFieldArrowDensity = 100


# s:Ge/BoxWithField/Field = "ElectricFieldMap"
# s:Ge/BoxWithField/ElectricField3DTable = "../mytests/test_field_table_E.csv"

s:Ge/BoxWithField/Field = "UniformElectroMagnetic"
u:Ge/BoxWithField/ElectricFieldDirectionX = 1.0
u:Ge/BoxWithField/ElectricFieldDirectionY = 1.0
u:Ge/BoxWithField/ElectricFieldDirectionZ = 0.0
d:Ge/BoxWithField/ElectricFieldStrength   = 5000 V/m
u:Ge/BoxWithField/MagneticFieldDirectionX = 0.0
u:Ge/BoxWithField/MagneticFieldDirectionY = 1.0
u:Ge/BoxWithField/MagneticFieldDirectionZ = 0.0
d:Ge/BoxWithField/MagneticFieldStrength   = 5.0 gauss

i:Gr/View/MagneticFieldArrowDensity = 10
i:Gr/View/ElectricFieldArrowDensity = 10

s:So/Demo/Type = "Beam" # Beam, Isotropic, Emittance or PhaseSpace
s:So/Demo/Component = "BeamPosition"
s:So/Demo/BeamParticle = "e-"
d:So/Demo/BeamEnergy = 600 eV
u:So/Demo/BeamEnergySpread = 0.0
s:So/Demo/BeamPositionDistribution = "None" # None, Flat or Gaussian

s:So/Demo/BeamAngularDistribution = "None" # None, Flat or Gaussian

i:So/Demo/NumberOfHistoriesInRun = 3

# Graphics
b:Ts/UseQt = "True"
s:Gr/View/Type = "OpenGL"
b:Gr/View/IncludeAxes = "False"