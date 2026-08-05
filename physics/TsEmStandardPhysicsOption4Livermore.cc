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
// The body below follows G4EmStandardPhysics_option4::ConstructProcess()
// (Geant4 11.3) line by line. The only deviations are marked "LIVERMORE
// SWAP" and concern the low-energy e-/e+ ionisation models.

#include "TsEmStandardPhysicsOption4Livermore.hh"

#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4LossTableManager.hh"
#include "G4EmParameters.hh"
#include "G4EmBuilder.hh"
#include "G4EmStandUtil.hh"

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4RayleighScattering.hh"
#include "G4KleinNishinaModel.hh"
#include "G4BetheHeitler5DModel.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermorePolarizedRayleighModel.hh"
#include "G4PhotoElectricAngularGeneratorPolarized.hh"
#include "G4LowEPComptonModel.hh"
#include "G4LowEPPolarizedComptonModel.hh"

#include "G4hMultipleScattering.hh"
#include "G4GoudsmitSaundersonMscModel.hh"
#include "G4WentzelVIModel.hh"
#include "G4CoulombScattering.hh"
#include "G4eCoulombScatteringModel.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4Generator2BS.hh"
#include "G4SeltzerBergerModel.hh"
#include "G4eBremsstrahlungRelModel.hh"
#include "G4ePairProduction.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4eplusAnnihilation.hh"
#include "G4eplusTo2or3GammaModel.hh"

#include "G4ionIonisation.hh"
#include "G4LindhardSorensenIonModel.hh"
#include "G4NuclearStopping.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4GenericIon.hh"

#include "G4PhysicsListHelper.hh"
#include "G4EmModelActivator.hh"
#include "G4GammaGeneralProcess.hh"

#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(TsEmStandardPhysicsOption4Livermore);

TsEmStandardPhysicsOption4Livermore::TsEmStandardPhysicsOption4Livermore(G4int ver, const G4String&)
  : G4EmStandardPhysics_option4(ver)
{
	// The EM parameters are those of opt4, set by the base constructor.
	SetPhysicsName("G4EmStandard_opt4_liv");
}

TsEmStandardPhysicsOption4Livermore::~TsEmStandardPhysicsOption4Livermore()
{}

void TsEmStandardPhysicsOption4Livermore::ConstructProcess()
{
	if (verboseLevel > 1) {
		G4cout << "### " << GetPhysicsName() << " Construct Processes" << G4endl;
	}
	G4EmBuilder::PrepareEMPhysics();

	G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
	G4EmParameters* param = G4EmParameters::Instance();

	// processes used by several particles
	G4hMultipleScattering* hmsc = new G4hMultipleScattering("ionmsc");

	// nuclear stopping is enabled if the energy limit above zero
	G4double nielEnergyLimit = param->MaxNIELEnergy();
	G4NuclearStopping* pnuc = nullptr;
	if (nielEnergyLimit > 0.0) {
		pnuc = new G4NuclearStopping();
		pnuc->SetMaxKinEnergy(nielEnergyLimit);
	}

	// high energy limit for e+- scattering models and bremsstrahlung
	G4double highEnergyLimit = param->MscEnergyLimit();

	// Add gamma EM Processes
	G4ParticleDefinition* particle = G4Gamma::Gamma();
	G4bool polar = param->EnablePolarisation();

	// Photoelectric
	G4PhotoElectricEffect* pe = new G4PhotoElectricEffect();
	G4VEmModel* peModel = new G4LivermorePhotoElectricModel();
	pe->SetEmModel(peModel);
	if (polar) {
		peModel->SetAngularDistribution(new G4PhotoElectricAngularGeneratorPolarized());
	}

	// Compton scattering
	G4ComptonScattering* cs = new G4ComptonScattering;
	cs->SetEmModel(new G4KleinNishinaModel());
	G4VEmModel* cModel = nullptr;
	if (polar) {
		cModel = new G4LowEPPolarizedComptonModel();
	} else {
		cModel = new G4LowEPComptonModel();
	}
	cModel->SetHighEnergyLimit(20*CLHEP::MeV);
	cs->AddEmModel(0, cModel);

	// Gamma conversion
	G4GammaConversion* gc = new G4GammaConversion();
	G4VEmModel* conv = new G4BetheHeitler5DModel();
	gc->SetEmModel(conv);

	// default Rayleigh scattering is Livermore
	G4RayleighScattering* rl = new G4RayleighScattering();
	if (polar) {
		rl->SetEmModel(new G4LivermorePolarizedRayleighModel());
	}

	if (param->GeneralProcessActive()) {
		G4GammaGeneralProcess* sp = new G4GammaGeneralProcess();
		sp->AddEmProcess(pe);
		sp->AddEmProcess(cs);
		sp->AddEmProcess(gc);
		sp->AddEmProcess(rl);
		G4LossTableManager::Instance()->SetGammaGeneralProcess(sp);
		ph->RegisterProcess(sp, particle);
	} else {
		ph->RegisterProcess(pe, particle);
		ph->RegisterProcess(cs, particle);
		ph->RegisterProcess(gc, particle);
		ph->RegisterProcess(rl, particle);
	}

	// e-
	particle = G4Electron::Electron();

	// e-/e+ msc gs with Mott-correction
	// (Mott-correction is set through G4EmParameters)
	G4GoudsmitSaundersonMscModel* msc1 = new G4GoudsmitSaundersonMscModel();
	G4WentzelVIModel* msc2 = new G4WentzelVIModel();
	msc1->SetHighEnergyLimit(highEnergyLimit);
	msc2->SetLowEnergyLimit(highEnergyLimit);
	G4EmBuilder::ConstructElectronMscProcess(msc1, msc2, particle);

	G4eCoulombScatteringModel* ssm = new G4eCoulombScatteringModel();
	G4CoulombScattering* ss = new G4CoulombScattering();
	ss->SetEmModel(ssm);
	ss->SetMinKinEnergy(highEnergyLimit);
	ssm->SetLowEnergyLimit(highEnergyLimit);
	ssm->SetActivationLowEnergyLimit(highEnergyLimit);

	// ionisation
	// LIVERMORE SWAP: opt4 uses G4PenelopeIonisationModel below 100 keV.
	G4eIonisation* eioni = new G4eIonisation();
	eioni->SetFluctModel(G4EmStandUtil::ModelOfFluctuations());
	G4VEmModel* theIoniMod = new G4LivermoreIonisationModel();
	theIoniMod->SetHighEnergyLimit(0.1*CLHEP::MeV);
	eioni->AddEmModel(0, theIoniMod);

	// bremsstrahlung
	G4eBremsstrahlung* brem = new G4eBremsstrahlung();
	G4SeltzerBergerModel* br1 = new G4SeltzerBergerModel();
	G4eBremsstrahlungRelModel* br2 = new G4eBremsstrahlungRelModel();
	br1->SetAngularDistribution(new G4Generator2BS());
	br2->SetAngularDistribution(new G4Generator2BS());
	brem->SetEmModel(br1);
	brem->SetEmModel(br2);
	br1->SetHighEnergyLimit(CLHEP::GeV);

	G4ePairProduction* ee = new G4ePairProduction();

	// register processes
	ph->RegisterProcess(eioni, particle);
	ph->RegisterProcess(brem, particle);
	ph->RegisterProcess(ee, particle);
	ph->RegisterProcess(ss, particle);

	// e+
	particle = G4Positron::Positron();

	// e-/e+ msc gs with Mott-correction
	// (Mott-correction is set through G4EmParameters)
	msc1 = new G4GoudsmitSaundersonMscModel();
	msc2 = new G4WentzelVIModel();
	msc1->SetHighEnergyLimit(highEnergyLimit);
	msc2->SetLowEnergyLimit(highEnergyLimit);
	G4EmBuilder::ConstructElectronMscProcess(msc1, msc2, particle);

	ssm = new G4eCoulombScatteringModel();
	ss = new G4CoulombScattering();
	ss->SetEmModel(ssm);
	ss->SetMinKinEnergy(highEnergyLimit);
	ssm->SetLowEnergyLimit(highEnergyLimit);
	ssm->SetActivationLowEnergyLimit(highEnergyLimit);

	// ionisation
	// LIVERMORE SWAP: opt4 adds G4PenelopeIonisationModel below 100 keV here.
	// G4LivermoreIonisationModel is e- only (fatal exception on e+), so the
	// positron keeps MollerBhabha over the whole range, as in G4EmLivermorePhysics.
	eioni = new G4eIonisation();
	eioni->SetFluctModel(G4EmStandUtil::ModelOfFluctuations());

	// bremsstrahlung
	brem = new G4eBremsstrahlung();
	br1 = new G4SeltzerBergerModel();
	br2 = new G4eBremsstrahlungRelModel();
	br1->SetAngularDistribution(new G4Generator2BS());
	br2->SetAngularDistribution(new G4Generator2BS());
	brem->SetEmModel(br1);
	brem->SetEmModel(br2);
	br1->SetHighEnergyLimit(CLHEP::GeV);

	// annihilation
	auto anni = new G4eplusAnnihilation();
	if (param->Use3GammaAnnihilationOnFly()) {
		anni->SetEmModel(new G4eplusTo2or3GammaModel());
	}

	// register processes
	ph->RegisterProcess(eioni, particle);
	ph->RegisterProcess(brem, particle);
	ph->RegisterProcess(ee, particle);
	ph->RegisterProcess(anni, particle);
	ph->RegisterProcess(ss, particle);

	// generic ion
	particle = G4GenericIon::GenericIon();
	G4ionIonisation* ionIoni = new G4ionIonisation();
	ionIoni->SetEmModel(new G4LindhardSorensenIonModel());
	ph->RegisterProcess(hmsc, particle);
	ph->RegisterProcess(ionIoni, particle);
	if (nullptr != pnuc) { ph->RegisterProcess(pnuc, particle); }

	// muons, hadrons, ions
	G4EmBuilder::ConstructCharged(hmsc, pnuc);

	// extra configuration
	G4EmModelActivator mact(GetPhysicsName());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
