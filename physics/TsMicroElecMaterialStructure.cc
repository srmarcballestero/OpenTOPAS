//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// *                                                                  *
// * This file is part of the TOPAS-nBio extensions to the            *
// * TOPAS Simulation Toolkit.                                        *
// *                                                                  *
// * Based on G4MicroElecMaterialStructure from Geant4 11.1.3         *
// * Modified to gracefully handle missing material data files        *
// *                                                                  *
// ********************************************************************
//

#include "TsMicroElecMaterialStructure.hh"
#include "G4SystemOfUnits.hh"
#include "G4EmParameters.hh"
#include <fstream>
#include <sstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

TsMicroElecMaterialStructure::TsMicroElecMaterialStructure(const G4String& matName)
  : fIsValid(false), materialName(matName), nLevels(0), Z(0), isCompound(false),
    workFunction(0), energyGap(0), initialEnergy(0)
{
  // Initialize arrays
  for (int i = 0; i < 4; ++i) limitInelastic[i] = 0.0;
  for (int i = 0; i < 2; ++i) limitElastic[i] = 0.0;

  if (matName == "Vacuum" || matName == "uum") {
    workFunction = 0;
    initialEnergy = 0;
    fIsValid = true;  // Vacuum is always "valid"
  }
  else {
    fIsValid = ReadMaterialFile();
  }
  nLevels = (G4int)energyConstant.size();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4bool TsMicroElecMaterialStructure::ReadMaterialFile()
{
  const char* path = G4FindDataDir("G4LEDATA");

  G4String processedName = materialName;
  if (processedName[0] == 'G' && processedName[1] == '4') {
    // Remove "G4_" prefix from NIST database materials
    processedName.erase(0, 3);
  }

  std::ostringstream fileName;
  fileName << path << "/microelec/Structure/Data_" << processedName << ".dat";
  std::ifstream fichier(fileName.str().c_str());

  if (!fichier) {
    // File not found - this is expected for materials not in MicroElec database
    // Just return false without throwing an exception
    if (G4EmParameters::Instance()->Verbose() > 0) {
      G4cout << "TsMicroElecMaterialStructure: Material '" << materialName
             << "' not found in MicroElec database (file: " << fileName.str() << ")" << G4endl;
      G4cout << "                                This material will use standard EM physics." << G4endl;
    }
    return false;
  }

  // File found - proceed with reading
  int varLength = 0;
  G4String nameParameter;
  G4String unitName;
  G4double unitValue;
  G4double data;
  G4String filler;
  G4String type;

  fichier >> filler >> type;
  materialName = filler;
  if (type == "Compound") {
    isCompound = true;
    Z = 0;
  }
  else {
    isCompound = false;
    Z = std::stoi(type);
  }

  while(!fichier.eof()) {
    getline(fichier, filler);
    std::stringstream line(filler);

    if (filler[0] == '#' || filler.empty()) {
      continue;
    }

    line >> varLength;
    line >> nameParameter;
    line >> unitName;
    unitValue = ConvertUnit(unitName);

    for (int i = 0; i < varLength; i++) {
      line >> data;
      data = data * unitValue;

      if(nameParameter == "WorkFunction") {
        workFunction = data;
      }
      if(nameParameter == "EnergyGap") {
        energyGap = data;
      }
      if(nameParameter == "EnergyPeak") {
        energyConstant.push_back(data);
      }
      if(nameParameter == "EnergyLimit") {
        LimitEnergy.push_back(data);
      }
      if(nameParameter == "EADL") {
        EADL_Enumerator.push_back(data);
      }
      if (nameParameter == "WeaklyBoundShell") {
        if (data == 0) {
          isShellWeaklyBoundVector.push_back(false);
        }
        else {
          isShellWeaklyBoundVector.push_back(true);
        }
      }
      if(nameParameter == "WeaklyBoundInitialEnergy") {
        initialEnergy = data;
      }
      if(nameParameter == "ShellAtomicNumber") {
        compoundShellZ.push_back(data);
      }
      if(nameParameter == "DielectricModelLowEnergyLimit_e") {
        limitInelastic[0] = data;
      }
      if(nameParameter == "DielectricModelHighEnergyLimit_e") {
        limitInelastic[1] = data;
      }
      if(nameParameter == "DielectricModelLowEnergyLimit_p") {
        limitInelastic[2] = data;
      }
      if(nameParameter == "DielectricModelHighEnergyLimit_p") {
        limitInelastic[3] = data;
      }
      if(nameParameter == "ElasticModelLowEnergyLimit") {
        limitElastic[0] = data;
      }
      if(nameParameter == "ElasticModelHighEnergyLimit") {
        limitElastic[1] = data;
      }
    }
  }
  fichier.close();

  return true;  // Successfully loaded
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double TsMicroElecMaterialStructure::Energy(G4int level)
{
  return (level >= 0 && level < nLevels) ? energyConstant[level] : 0.0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double TsMicroElecMaterialStructure::GetZ(G4int Shell)
{
  if (Shell >= 0 && Shell < nLevels) {
    if(!isCompound) {
      return Z;
    }
    else {
      return compoundShellZ[Shell];
    }
  }
  else {
    return 0;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double TsMicroElecMaterialStructure::ConvertUnit(const G4String& unitName)
{
  G4double unitValue = 0;
  if(unitName == "meV") {
    unitValue = 1e-3 * CLHEP::eV;
  }
  else if(unitName == "eV") {
    unitValue = CLHEP::eV;
  }
  else if(unitName == "keV") {
    unitValue = CLHEP::keV;
  }
  else if(unitName == "MeV") {
    unitValue = CLHEP::MeV;
  }
  else if(unitName == "noUnit") {
    unitValue = 1;
  }

  return unitValue;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double TsMicroElecMaterialStructure::GetLimitEnergy(G4int level)
{
  G4double E = LimitEnergy[level];
  if (IsShellWeaklyBound(level)) {
    E = energyGap + initialEnergy;
  }
  return E;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double TsMicroElecMaterialStructure::GetInelasticModelLowLimit(G4int pdg)
{
  G4double res = 0.0;
  if(pdg == 11) {
    res = limitInelastic[0];
  }
  else if(pdg == 2212) {
    res = limitInelastic[2];
  }
  return res;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double TsMicroElecMaterialStructure::GetInelasticModelHighLimit(G4int pdg)
{
  G4double res = 0.0;
  if(pdg == 11) {
    res = limitInelastic[1];
  }
  else if(pdg == 2212) {
    res = limitInelastic[3];
  }
  return res;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4bool TsMicroElecMaterialStructure::IsShellWeaklyBound(G4int level)
{
  return isShellWeaklyBoundVector[level];
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
