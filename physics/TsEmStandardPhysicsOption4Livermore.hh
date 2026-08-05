// * File created by @srmarcballestero (marc.ballestero-ribo@psi.ch)
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
// *                                                                  *
// ********************************************************************
//
// G4EmStandardPhysics_option4 with the low-energy electron ionisation model
// swapped from Penelope to Livermore. Everything else (EM parameters, gamma
// models, msc, bremsstrahlung, hadron and ion physics) is identical to opt4;
// the EM parameter block is inherited from the opt4 constructor unchanged.
//
// Two consequences of the swap are worth keeping in mind:
//   * G4LivermoreIonisationModel is defined for e- only (it throws a fatal
//     exception if attached to e+), so positrons fall back to MollerBhabha
//     over the whole range, exactly as in G4EmLivermorePhysics.
//   * The model's intrinsic range is 12 eV - 100 GeV; here it is used below
//     100 keV, matching the Penelope crossover it replaces.
//
// TOPAS module name: "g4em-standard_opt4_liv".

#ifndef TsEmStandardPhysicsOption4Livermore_h
#define TsEmStandardPhysicsOption4Livermore_h 1

#include "G4EmStandardPhysics_option4.hh"
#include "globals.hh"

class TsEmStandardPhysicsOption4Livermore : public G4EmStandardPhysics_option4
{
public:
  explicit TsEmStandardPhysicsOption4Livermore(G4int ver = 1, const G4String& name = "");

  ~TsEmStandardPhysicsOption4Livermore() override;

  void ConstructProcess() override;
};

#endif
