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

#ifndef TsMicroElecMaterialStructure_h
#define TsMicroElecMaterialStructure_h 1

#include "G4MicroElecMaterialStructure.hh"
#include "globals.hh"
#include <vector>

/// \brief TOPAS-patched version of G4MicroElecMaterialStructure
///
/// This class extends G4MicroElecMaterialStructure to handle materials
/// that don't have MicroElec data files. Instead of throwing a fatal
/// exception, it returns a flag indicating whether the material is
/// supported by the MicroElec database.
///
/// This allows TOPAS simulations to have materials outside the MicroElec
/// region without causing fatal errors.

class TsMicroElecMaterialStructure
{
public:
  /// Constructor - attempts to load material data, sets fIsValid flag
  explicit TsMicroElecMaterialStructure(const G4String& matName);

  ~TsMicroElecMaterialStructure() = default;

  /// Returns true if the material data was successfully loaded
  G4bool IsValid() const { return fIsValid; }

  /// Returns the material name
  G4String GetMaterialName() const { return materialName; }

  /// Get number of energy levels
  G4int GetNLevels() const { return nLevels; }

  /// Get energy of a specific level
  G4double Energy(G4int level);

  /// Get Z for a specific shell
  G4double GetZ(G4int Shell);

  /// Get limit energy for a level
  G4double GetLimitEnergy(G4int level);

  /// Get inelastic model energy limits
  G4double GetInelasticModelLowLimit(G4int pdg);
  G4double GetInelasticModelHighLimit(G4int pdg);

  /// Get elastic model energy limits
  G4double GetElasticModelLowLimit() const { return limitElastic[0]; }
  G4double GetElasticModelHighLimit() const { return limitElastic[1]; }

  /// Check if shell is weakly bound
  G4bool IsShellWeaklyBound(G4int level);

  /// Get work function
  G4double GetWorkFunction() const { return workFunction; }

  /// Get energy gap
  G4double GetEnergyGap() const { return energyGap; }

  /// Check if material is a compound
  G4bool IsCompound() const { return isCompound; }

private:
  /// Read material data file (returns true if successful)
  G4bool ReadMaterialFile();

  /// Convert unit string to value
  G4double ConvertUnit(const G4String& unitName);

  G4bool fIsValid;  ///< True if material data loaded successfully

  G4String materialName;
  G4int nLevels;
  G4int Z;
  G4bool isCompound;

  G4double workFunction;
  G4double energyGap;
  G4double initialEnergy;

  std::vector<G4double> energyConstant;
  std::vector<G4double> LimitEnergy;
  std::vector<G4double> EADL_Enumerator;
  std::vector<G4bool> isShellWeaklyBoundVector;
  std::vector<G4int> compoundShellZ;

  G4double limitInelastic[4];  // [e- low, e- high, p low, p high]
  G4double limitElastic[2];    // [low, high]
};

#endif
