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

#include "TsEmMicroElecPhysics.hh"
#include "TsParameterManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4ProductionCutsTable.hh"
#include "G4MaterialCutsCouple.hh"
#include <fstream>

// MicroElec models and processes
#include "G4MicroElecElastic.hh"
#include "G4MicroElecElasticModel.hh"
#include "G4MicroElecElasticModel_new.hh"

#include "G4MicroElecInelastic.hh"
#include "G4MicroElecInelasticModel.hh"
#include "G4MicroElecInelasticModel_new.hh"

#include "G4MicroElecLOPhononScattering.hh"
#include "G4MicroElecLOPhononModel.hh"
#include "G4MicroElecSurface.hh"

// Standard EM processes and models
#include "G4LossTableManager.hh"
#include "G4EmConfigurator.hh"
#include "G4VEmModel.hh"
#include "G4DummyModel.hh"
#include "G4eIonisation.hh"
#include "G4hIonisation.hh"
#include "G4ionIonisation.hh"
#include "G4eMultipleScattering.hh"
#include "G4hMultipleScattering.hh"
#include "G4BraggModel.hh"
#include "G4BraggIonModel.hh"
#include "G4BetheBlochModel.hh"
#include "G4GoudsmitSaundersonMscModel.hh"
#include "G4WentzelVIModel.hh"
#include "G4MollerBhabhaModel.hh"
#include "G4IonFluctuations.hh"
#include "G4UniversalFluctuation.hh"

// Particles
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Proton.hh"
#include "G4Alpha.hh"
#include "G4GenericIon.hh"

// Atomic deexcitation
#include "G4UAtomicDeexcitation.hh"
#include "G4LossTableManager.hh"
#include "G4EmParameters.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(TsEmMicroElecPhysics);

TsEmMicroElecPhysics::TsEmMicroElecPhysics(TsParameterManager* pM)
  : G4VPhysicsConstructor("TsEmMicroElecPhysics"), fPm(pM), fVerbose(1)
{
	if (fPm && fPm->ParameterExists("Ph/Verbosity"))
		fVerbose = fPm->GetIntegerParameter("Ph/Verbosity");

	if (fVerbose > 0)
		G4cout << "TsEmMicroElecPhysics: MicroElec physics constructor initialized" << G4endl;
}

TsEmMicroElecPhysics::TsEmMicroElecPhysics(G4int ver)
  : G4VPhysicsConstructor("TsEmMicroElecPhysics"), fPm(nullptr), fVerbose(ver)
{
	if (fVerbose > 0)
		G4cout << "TsEmMicroElecPhysics: MicroElec physics constructor initialized" << G4endl;
}

TsEmMicroElecPhysics::~TsEmMicroElecPhysics()
{}

void TsEmMicroElecPhysics::ConstructParticle()
{
	// Bosons
	G4Gamma::GammaDefinition();

	// Leptons
	G4Electron::ElectronDefinition();
	G4Positron::PositronDefinition();

	// Baryons
	G4Proton::ProtonDefinition();

	// Ions
	G4Alpha::AlphaDefinition();
	G4GenericIon::GenericIonDefinition();
}

void TsEmMicroElecPhysics::ConstructProcess()
{
	if (fVerbose > 0)
		G4cout << "TsEmMicroElecPhysics: Constructing MicroElec processes" << G4endl;

	// ========================================================================
	// Configure EM parameters
	// ========================================================================
	G4EmParameters* param = G4EmParameters::Instance();
	param->SetDefaults();
	param->SetBuildCSDARange(true);
	param->SetMscStepLimitType(fUseSafety);

	// CRITICAL: Tell Geant4 to only initialize MicroElec models for materials
	// in regions designated as MicroElec regions. This prevents errors when
	// materials outside the MicroElec database exist in the geometry.
	param->RegionsMicroElec();

	param->SetMinEnergy(0.1*eV);
	param->SetMaxEnergy(10*TeV);
	param->SetLowestElectronEnergy(0*eV);
	param->SetNumberOfBinsPerDecade(20);
	param->ActivateAngularGeneratorForIonisation(true);
	param->SetAuger(true);

	// ========================================================================
	// Set up atomic deexcitation
	// ========================================================================
	G4VAtomDeexcitation* de = new G4UAtomicDeexcitation();
	G4LossTableManager::Instance()->SetAtomDeexcitation(de);
	de->SetFluo(true);
	de->SetAuger(true);
	de->SetPIXE(true);
	de->InitialiseForNewRun();

	// ========================================================================
	// Iterate over particles and add processes
	// ========================================================================
	auto particleIterator = GetParticleIterator();
	particleIterator->reset();

	while ((*particleIterator)())
	{
		G4ParticleDefinition* particle = particleIterator->value();
		G4ProcessManager* pmanager = particle->GetProcessManager();
		G4String particleName = particle->GetParticleName();

		// ====================================================================
		// ELECTRONS
		// ====================================================================
		if (particleName == "e-")
		{
			// NOTE: Standard MSC and ionisation are added by g4em-standard_opt4.
			// In the microelec region, standard models remain active above 1 keV
			// where they are reliable. MicroElec takes over below 1 keV where
			// standard EM accuracy degrades significantly.
			// We add MicroElec-specific processes (active only in microelec region)

			// MicroElec elastic (uses dummy model in World, activated in Target)
			G4MicroElecElastic* microElecElastic = new G4MicroElecElastic("e-_G4MicroElecElastic");
			microElecElastic->SetEmModel(new G4DummyModel(), 1);
			pmanager->AddDiscreteProcess(microElecElastic);

			// MicroElec inelastic (uses dummy model in World, activated in Target)
			G4MicroElecInelastic* microElecInelastic = new G4MicroElecInelastic("e-_G4MicroElecInelastic");
			microElecInelastic->SetEmModel(new G4DummyModel(), 1);
			pmanager->AddDiscreteProcess(microElecInelastic);

			// MicroElec LO phonon scattering (for dielectrics like SiO2, Al2O3)
			G4MicroElecLOPhononScattering* phononScattering = new G4MicroElecLOPhononScattering("e-_G4MicroElecLOPhonon");
			phononScattering->SetEmModel(new G4DummyModel(), 1);
			pmanager->AddDiscreteProcess(phononScattering);

			// MicroElec surface process (boundary interactions)
			G4MicroElecSurface* microElecSurface = new G4MicroElecSurface("e-_G4MicroElecSurface");
			microElecSurface->SetProcessManager(pmanager);
			pmanager->AddDiscreteProcess(microElecSurface);
		}

		// ====================================================================
		// PROTONS
		// ====================================================================
		else if (particleName == "proton")
		{
			// NOTE: Standard ionisation is added by g4em-standard_opt4.
			// In microelec region: MicroElec (100 eV - 2 MeV) replaces Bragg model
			// to avoid conflicts with MicroElec delta ray production. Standard
			// Bethe-Bloch model remains active above 2 MeV where it's reliable.

			// MicroElec inelastic (uses dummy model in World, activated in Target)
			G4MicroElecInelastic* microElecInelastic = new G4MicroElecInelastic("p_G4MicroElecInelastic");
			microElecInelastic->SetEmModel(new G4DummyModel(), 1);
			pmanager->AddDiscreteProcess(microElecInelastic);
		}

		// ====================================================================
		// ALPHA PARTICLES
		// ====================================================================
		else if (particleName == "alpha")
		{
			// NOTE: Standard ionisation is added by g4em-standard_opt4.
			// In microelec region: MicroElec (100 eV - 8 MeV) replaces BraggIon model
			// to avoid conflicts with MicroElec delta ray production. Standard
			// Bethe-Bloch model remains active above ~8 MeV (2 MeV/u) where it's reliable.

			// MicroElec inelastic (uses dummy model in World, activated in Target)
			G4MicroElecInelastic* microElecInelastic = new G4MicroElecInelastic("alpha_G4MicroElecInelastic");
			microElecInelastic->SetEmModel(new G4DummyModel(), 1);
			pmanager->AddDiscreteProcess(microElecInelastic);
		}

		// ====================================================================
		// GENERIC IONS
		// ====================================================================
		else if (particleName == "GenericIon")
		{
			// NOTE: Standard ionisation is added by g4em-standard_opt4.
			// In microelec region: MicroElec (100 eV - 10 MeV) replaces BraggIon model
			// to avoid conflicts with MicroElec delta ray production. Standard high-energy
			// models (Lindhard-Sorensen) remain active above 10 MeV where they're reliable.

			// MicroElec inelastic (uses dummy model in World, activated in Target)
			G4MicroElecInelastic* microElecInelastic = new G4MicroElecInelastic("ion_G4MicroElecInelastic");
			microElecInelastic->SetEmModel(new G4DummyModel(), 1);
			pmanager->AddDiscreteProcess(microElecInelastic);
		}
	}

	// ========================================================================
	// Configure region-specific models (for "microelec" region)
	// NOTE: TOPAS converts all region names to lowercase
	// NOTE: Models are instantiated here but only activated in the "microelec" region
	//       Models will only initialize their material data when actually used
	// ========================================================================
	G4EmConfigurator* em_config = G4LossTableManager::Instance()->EmConfigurator();
	G4VEmModel* mod;

	// ------------------------------------------------------------------------
	// Electrons in microelec region
	// ------------------------------------------------------------------------

	// Use Goudsmit-Saunderson MSC model between 1 keV and 100 MeV in microelec region
	G4GoudsmitSaundersonMscModel* msc_gs = new G4GoudsmitSaundersonMscModel();
	msc_gs->SetActivationLowEnergyLimit(1*keV);
	em_config->SetExtraEmModel("e-", "msc", msc_gs, "microelec", 1*keV, 100*MeV);

	// Use WentzelVI MSC model above 100 MeV in microelec region
	G4WentzelVIModel* msc_wentzel = new G4WentzelVIModel();
	msc_wentzel->SetActivationLowEnergyLimit(100*MeV);
	em_config->SetExtraEmModel("e-", "msc", msc_wentzel, "microelec", 100*MeV, 10*TeV);

	// Activate MicroElec elastic model in microelec region
	mod = new G4MicroElecElasticModel_new();
	em_config->SetExtraEmModel("e-", "e-_G4MicroElecElastic", mod, "microelec", 0.0, 1*keV);

	mod = new G4MollerBhabhaModel();
	mod->SetActivationLowEnergyLimit(1*keV);
	em_config->SetExtraEmModel("e-", "eIoni", mod, "microelec", 1*keV, 10*TeV, new G4UniversalFluctuation());

	// Activate MicroElec inelastic model in microelec region
	mod = new G4MicroElecInelasticModel_new();
	em_config->SetExtraEmModel("e-", "e-_G4MicroElecInelastic", mod, "microelec", 0.0, 1*keV);

	// Activate MicroElec LO phonon scattering model in microelec region
	mod = new G4MicroElecLOPhononModel();
	em_config->SetExtraEmModel("e-", "e-_G4MicroElecLOPhonon", mod, "microelec", 0.0, 10*MeV);

	// ------------------------------------------------------------------------
	// Protons in microelec region
	// ------------------------------------------------------------------------

	// Deactivate Bragg model below 2 MeV in microelec region (conflicts with MicroElec)
	mod = new G4BetheBlochModel();
	mod->SetActivationLowEnergyLimit(2*MeV);
	em_config->SetExtraEmModel("proton", "hIoni", mod, "microelec", 2*MeV, 10*TeV, new G4IonFluctuations());

	// Activate MicroElec inelastic model for protons (100 eV - 2 MeV)
	mod = new G4MicroElecInelasticModel_new();
	mod->SetActivationLowEnergyLimit(100*eV);
	em_config->SetExtraEmModel("proton", "p_G4MicroElecInelastic", mod, "microelec", 100*eV, 2*MeV);

	// ------------------------------------------------------------------------
	// Alpha particles in microelec region
	// ------------------------------------------------------------------------

	// Deactivate BraggIon model below 8 MeV in microelec region (conflicts with MicroElec)
	mod = new G4BetheBlochModel();
	mod->SetActivationLowEnergyLimit(8*MeV);
	em_config->SetExtraEmModel("alpha", "ionIoni", mod, "microelec", 8*MeV, 10*TeV, new G4IonFluctuations());

	// Activate MicroElec inelastic model for alphas (100 eV - 8 MeV)
	mod = new G4MicroElecInelasticModel_new();
	mod->SetActivationLowEnergyLimit(100*eV);
	em_config->SetExtraEmModel("alpha", "alpha_G4MicroElecInelastic", mod, "microelec", 0.0, 8*MeV);

	// ------------------------------------------------------------------------
	// Generic ions in microelec region
	// ------------------------------------------------------------------------

	// Deactivate BraggIon model below 10 MeV in microelec region (conflicts with MicroElec)
	mod = new G4BetheBlochModel();
	mod->SetActivationLowEnergyLimit(10*MeV);
	em_config->SetExtraEmModel("GenericIon", "ionIoni", mod, "microelec", 2*MeV, 10*TeV, new G4IonFluctuations());

	// Activate MicroElec inelastic model for generic ions (100 eV - 10 MeV)
	// Activate MicroElec inelastic model for generic ions (100 eV - 10 MeV)
	mod = new G4MicroElecInelasticModel_new();
	mod->SetActivationLowEnergyLimit(100*eV);
	em_config->SetExtraEmModel("GenericIon", "ion_G4MicroElecInelastic", mod, "microelec", 100*eV, 10*MeV);

	if (fVerbose > 0)
		G4cout << "TsEmMicroElecPhysics: MicroElec processes constructed successfully" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
