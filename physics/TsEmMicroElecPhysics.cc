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
// MicroElec track-structure physics for the "microelec" region, designed to
// overlay a standard EM base list (em_opt3, em_opt4 or the opt4/Livermore
// variant). Above an electron threshold the region reproduces the base list's
// electron models; below the threshold MicroElec elastic/inelastic take over.
// Behaviour is selected at run time through TOPAS parameters (see
// ConstructProcess):
//
//   Ph/MicroElec/BaseList             = "opt3" | "opt4" | "opt4_liv"
//                                                       (default "opt4")
//   Ph/MicroElec/ElectronThreshold    = 1. keV          (default 1 keV)
//   Ph/MicroElec/ProtonThreshold      = 2. MeV          (default 2 MeV)
//   Ph/MicroElec/LowEnergyIonisation  = "Penelope" | "Livermore" | "None"
//                                       (default follows BaseList: Penelope for
//                                       opt4, Livermore for opt4_liv, None for
//                                       opt3; opt4 family only)
//   Ph/MicroElec/UsePenelope          = "True"          (deprecated; "False" is
//                                       an alias for LowEnergyIonisation="None")
//
// Optional surface work-function overrides (paired vectors, same length) let a
// non-database bulk share its skin's work function so no spurious escape barrier
// appears at their interface; unlisted materials keep WF = 0 (air/vacuum-like):
//   sv:Ph/MicroElec/SurfaceWorkFunctionMaterials = 1 "Brass"
//   dv:Ph/MicroElec/SurfaceWorkFunctionValues    = 1 4.2 eV
//
// IMPORTANT: this constructor only ADDS MicroElec processes and region model
// overrides; it does not register the base electron/photon processes. The
// matching base list MUST also be present in Ph/<list>/Modules, e.g.:
//   Ph/Default/Modules = 2 "g4em-standard_opt4" "g4em-microelec"
// or, for the Livermore variant of opt4:
//   Ph/Default/Modules = 2 "g4em-standard_opt4_liv" "g4em-microelec"
// and BaseList must match that base list for the substitution to be consistent.

#include "TsEmMicroElecPhysics.hh"
#include "TsParameterManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4ProductionCutsTable.hh"
#include "G4MaterialCutsCouple.hh"
#include <fstream>
#include <vector>

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
#include "G4UrbanMscModel.hh"
#include "G4GoudsmitSaundersonMscModel.hh"
#include "G4WentzelVIModel.hh"
#include "G4MollerBhabhaModel.hh"
#include "G4PenelopeIonisationModel.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4LindhardSorensenIonModel.hh"
#include "G4EmStandUtil.hh"

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
	// Read configuration parameters
	// ========================================================================
	G4String baseList = "opt4";
	if (fPm && fPm->ParameterExists("Ph/MicroElec/BaseList"))
		baseList = fPm->GetStringParameter("Ph/MicroElec/BaseList");
	G4StrUtil::to_lower(baseList);

	G4double eThreshold = 1.0 * keV;
	if (fPm && fPm->ParameterExists("Ph/MicroElec/ElectronThreshold"))
		eThreshold = fPm->GetDoubleParameter("Ph/MicroElec/ElectronThreshold", "Energy");

	G4double pThreshold = 2.0 * MeV;
	if (fPm && fPm->ParameterExists("Ph/MicroElec/ProtonThreshold"))
		pThreshold = fPm->GetDoubleParameter("Ph/MicroElec/ProtonThreshold", "Energy");

	// Low-energy (< 100 keV) electron ionisation model used above the MicroElec
	// threshold. The default follows the base list; the deprecated UsePenelope
	// = "False" is kept as an alias for "none".
	G4String lowEnergyIoni = (baseList == "opt4") ? "penelope"
	                       : (baseList == "opt4_liv") ? "livermore"
	                       : "none";

	if (fPm && fPm->ParameterExists("Ph/MicroElec/UsePenelope"))
	{
		if (!fPm->GetBooleanParameter("Ph/MicroElec/UsePenelope"))
			lowEnergyIoni = "none";
	}

	if (fPm && fPm->ParameterExists("Ph/MicroElec/LowEnergyIonisation"))
	{
		lowEnergyIoni = fPm->GetStringParameter("Ph/MicroElec/LowEnergyIonisation");
		G4StrUtil::to_lower(lowEnergyIoni);
	}

	// Surface work-function overrides: pair each material name with a WF value.
	// The surface process treats any material absent from the MicroElec database
	// as vacuum (WF = 0), which is correct for air/vacuum but wrong at an
	// interface with a non-database bulk.
	std::vector<G4String> wfMaterials;
	std::vector<G4double> wfValues;
	if (fPm && fPm->ParameterExists("Ph/MicroElec/SurfaceWorkFunctionMaterials"))
	{
		G4int nWf = fPm->GetVectorLength("Ph/MicroElec/SurfaceWorkFunctionMaterials");
		if (!fPm->ParameterExists("Ph/MicroElec/SurfaceWorkFunctionValues") ||
		    fPm->GetVectorLength("Ph/MicroElec/SurfaceWorkFunctionValues") != nWf)
		{
			G4cerr << "TsEmMicroElecPhysics: Ph/MicroElec/SurfaceWorkFunctionMaterials and "
			          "Ph/MicroElec/SurfaceWorkFunctionValues must have the same length." << G4endl;
			if (fPm)
				fPm->AbortSession(1);
		}
		G4String* mats = fPm->GetStringVector("Ph/MicroElec/SurfaceWorkFunctionMaterials");
		G4double* vals = fPm->GetDoubleVector("Ph/MicroElec/SurfaceWorkFunctionValues", "Energy");
		for (G4int i = 0; i < nWf; ++i)
		{
			wfMaterials.push_back(mats[i]);
			wfValues.push_back(vals[i]);
		}
	}

	if (baseList != "opt3" && baseList != "opt4" && baseList != "opt4_liv")
	{
		G4cerr << "TsEmMicroElecPhysics: unknown Ph/MicroElec/BaseList = \"" << baseList
		       << "\". Allowed values are \"opt3\", \"opt4\" and \"opt4_liv\"." << G4endl;
		if (fPm)
			fPm->AbortSession(1);
		baseList = "opt4";
	}

	if (lowEnergyIoni != "penelope" && lowEnergyIoni != "livermore" && lowEnergyIoni != "none")
	{
		G4cerr << "TsEmMicroElecPhysics: unknown Ph/MicroElec/LowEnergyIonisation = \"" << lowEnergyIoni
		       << "\". Allowed values are \"Penelope\", \"Livermore\" and \"None\"." << G4endl;
		if (fPm)
			fPm->AbortSession(1);
		lowEnergyIoni = "none";
	}

	// em_opt3 has no low-energy electron ionisation model, so any choice other
	// than "none" would not reproduce the base list inside the region.
	if (baseList == "opt3" && lowEnergyIoni != "none")
	{
		G4cerr << "TsEmMicroElecPhysics: Ph/MicroElec/LowEnergyIonisation = \"" << lowEnergyIoni
		       << "\" is not consistent with Ph/MicroElec/BaseList = \"opt3\", which uses "
		          "MollerBhabha over the whole range." << G4endl;
		if (fPm)
			fPm->AbortSession(1);
		lowEnergyIoni = "none";
	}

	if (fVerbose > 0)
		G4cout << "TsEmMicroElecPhysics: base list = " << baseList
		       << ", electron threshold = " << eThreshold / keV << " keV"
		       << ", proton threshold = " << pThreshold / MeV << " MeV"
		       << ", low-energy ionisation = " << lowEnergyIoni << G4endl;

	// ========================================================================
	// Configure EM parameters
	// ========================================================================
	G4EmParameters* param = G4EmParameters::Instance();
	param->SetBuildCSDARange(true);

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

			// MicroElec surface process (work-function barrier at material
			// interfaces). Apply any user work-function overrides so a
			// non-database bulk (e.g. Brass under a G4_Cu skin) does not get a
			// spurious barrier; see Ph/MicroElec/SurfaceWorkFunctionMaterials.
			G4MicroElecSurface* microElecSurface = new G4MicroElecSurface("e-_G4MicroElecSurface");
			microElecSurface->SetProcessManager(pmanager);
			for (size_t i = 0; i < wfMaterials.size(); ++i)
				microElecSurface->SetWorkFunction(wfMaterials[i], wfValues[i]);
			pmanager->AddDiscreteProcess(microElecSurface);
		}

		// ====================================================================
		// PROTONS
		// ====================================================================
		else if (particleName == "proton")
		{
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

	// Fixed crossover energies, matching the base lists:
	//   opt4 msc:  GoudsmitSaunderson <= 100 MeV, WentzelVI above (MscEnergyLimit)
	//   opt4 ioni: Penelope (opt4) or Livermore (opt4_liv) <= 100 keV,
	//              MollerBhabha above
	const G4double mscHighLimit = 100.0 * MeV;   // GS / WentzelVI crossover (opt4)
	const G4double lowEIoniHighLimit = 100.0 * keV;  // low-energy ioni / MollerBhabha crossover (opt4)
	const G4double topLimit = 600.0 * MeV;       // upper bound of region overrides

	// ------------------------------------------------------------------------
	// Electrons in microelec region: standard models above eThreshold reproduce
	// the chosen base list; MicroElec track structure below eThreshold.
	// ------------------------------------------------------------------------
	if (baseList == "opt3")
	{
		// msc: Urban over the whole range (matches em_opt3)
		G4UrbanMscModel* msc_urban = new G4UrbanMscModel();
		msc_urban->SetActivationLowEnergyLimit(eThreshold);
		em_config->SetExtraEmModel("e-", "msc", msc_urban, "microelec", eThreshold, topLimit);

		// eIoni: MollerBhabha over the whole range (matches em_opt3)
		mod = new G4MollerBhabhaModel();
		mod->SetActivationLowEnergyLimit(eThreshold);
		em_config->SetExtraEmModel("e-", "eIoni", mod, "microelec", eThreshold, topLimit, G4EmStandUtil::ModelOfFluctuations());
	}
	else // opt4 / opt4_liv
	{
		// msc: GoudsmitSaunderson (<= 100 MeV) + WentzelVI (> 100 MeV)
		// (identical in em_opt4 and its Livermore variant)
		G4GoudsmitSaundersonMscModel* msc_gs = new G4GoudsmitSaundersonMscModel();
		msc_gs->SetActivationLowEnergyLimit(eThreshold);
		em_config->SetExtraEmModel("e-", "msc", msc_gs, "microelec", eThreshold, mscHighLimit);

		G4WentzelVIModel* msc_wentzel = new G4WentzelVIModel();
		msc_wentzel->SetActivationLowEnergyLimit(mscHighLimit);
		em_config->SetExtraEmModel("e-", "msc", msc_wentzel, "microelec", mscHighLimit, topLimit);

		// eIoni: low-energy model (<= 100 keV) + MollerBhabha (> 100 keV).
		// Penelope matches em_opt4, Livermore matches g4em-standard_opt4_liv;
		// "none" drops the low-energy model (NOT consistent with either).
		if (lowEnergyIoni != "none")
		{
			if (lowEnergyIoni == "livermore")
				mod = new G4LivermoreIonisationModel();
			else
				mod = new G4PenelopeIonisationModel();
			mod->SetActivationLowEnergyLimit(eThreshold);
			em_config->SetExtraEmModel("e-", "eIoni", mod, "microelec", eThreshold, lowEIoniHighLimit, G4EmStandUtil::ModelOfFluctuations());

			mod = new G4MollerBhabhaModel();
			mod->SetActivationLowEnergyLimit(lowEIoniHighLimit);
			em_config->SetExtraEmModel("e-", "eIoni", mod, "microelec", lowEIoniHighLimit, topLimit, G4EmStandUtil::ModelOfFluctuations());
		}
		else
		{
			mod = new G4MollerBhabhaModel();
			mod->SetActivationLowEnergyLimit(eThreshold);
			em_config->SetExtraEmModel("e-", "eIoni", mod, "microelec", eThreshold, topLimit, G4EmStandUtil::ModelOfFluctuations());
		}
	}

	// MicroElec elastic (track structure below threshold)
	mod = new G4MicroElecElasticModel_new();
	em_config->SetExtraEmModel("e-", "e-_G4MicroElecElastic", mod, "microelec", 0.0, eThreshold);

	// MicroElec inelastic (track structure below threshold)
	mod = new G4MicroElecInelasticModel_new();
	em_config->SetExtraEmModel("e-", "e-_G4MicroElecInelastic", mod, "microelec", 0.0, eThreshold);

	// LO phonon (polar dielectrics only)
	mod = new G4MicroElecLOPhononModel();
	em_config->SetExtraEmModel("e-", "e-_G4MicroElecLOPhonon", mod, "microelec", 0.0, 10*MeV);

	// ------------------------------------------------------------------------
	// Protons in microelec region
	// ------------------------------------------------------------------------

	// hIoni: Bragg (<= 2 MeV) + BetheBloch (> 2 MeV), matching the base lists.
	// If pThreshold < 2 MeV, Bragg fills the gap between the MicroElec threshold
	// and the Bragg/BetheBloch crossover.
	const G4double braggHighLimit = 2.0 * MeV;  // Bragg / BetheBloch crossover (proton)

	if (pThreshold < braggHighLimit)
	{
		mod = new G4BraggModel();
		mod->SetActivationLowEnergyLimit(pThreshold);
		em_config->SetExtraEmModel("proton", "hIoni", mod, "microelec", pThreshold, braggHighLimit, G4EmStandUtil::ModelOfFluctuations());

		mod = new G4BetheBlochModel();
		mod->SetActivationLowEnergyLimit(braggHighLimit);
		em_config->SetExtraEmModel("proton", "hIoni", mod, "microelec", braggHighLimit, 10*TeV, G4EmStandUtil::ModelOfFluctuations());
	}
	else
	{
		mod = new G4BetheBlochModel();
		mod->SetActivationLowEnergyLimit(pThreshold);
		em_config->SetExtraEmModel("proton", "hIoni", mod, "microelec", pThreshold, 10*TeV, G4EmStandUtil::ModelOfFluctuations());
	}

	mod = new G4MicroElecInelasticModel_new();
	mod->SetActivationLowEnergyLimit(100*eV);
	em_config->SetExtraEmModel("proton", "p_G4MicroElecInelastic", mod, "microelec", 100*eV, pThreshold);

	// ------------------------------------------------------------------------
	// Alpha particles in microelec region
	// ------------------------------------------------------------------------

	// ionIoni
	mod = new G4BetheBlochModel();
	mod->SetActivationLowEnergyLimit(7.9452*MeV);
	em_config->SetExtraEmModel("alpha", "ionIoni", mod, "microelec", 7.9452*MeV, 10*TeV, G4EmStandUtil::ModelOfFluctuations(true));

	mod = new G4MicroElecInelasticModel_new();
	mod->SetActivationLowEnergyLimit(100*eV);
	em_config->SetExtraEmModel("alpha", "alpha_G4MicroElecInelastic", mod, "microelec", 0.0, 7.9452*MeV);

	// ------------------------------------------------------------------------
	// Generic ions in microelec region
	// ------------------------------------------------------------------------

	// ionIoni
	mod = new G4LindhardSorensenIonModel();
	mod->SetActivationLowEnergyLimit(10*MeV);
	em_config->SetExtraEmModel("GenericIon", "ionIoni", mod, "microelec", 10*MeV, 10*TeV, G4EmStandUtil::ModelOfFluctuations(true));

	mod = new G4MicroElecInelasticModel_new();
	mod->SetActivationLowEnergyLimit(100*eV);
	em_config->SetExtraEmModel("GenericIon", "ion_G4MicroElecInelastic", mod, "microelec", 100*eV, 10*MeV);

	if (fVerbose > 0)
		G4cout << "TsEmMicroElecPhysics: MicroElec processes constructed successfully" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
