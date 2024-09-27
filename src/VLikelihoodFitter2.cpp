/*! class VLikelihoodFitter2
Calculate the likelihood of a model for a set of runs
Model fitting, flux calculations and variability included
based on Piron et al 2001 astro-ph/0106196
*/
#include "VLikelihoodFitter2.h"


// Default constructor
VLikelihoodFitter2::VLikelihoodFitter2(){
    
    // Set default values
    fModelID = 0;
    fENorm = 1;
    fMJDMin = -1;
    fMJDMax = 999999;
    fEBLAnalysis = false;
	
	// Number of parallel threads
	fNumThreads = 1;

    // Defaulting Energy threshold to 15% of max effective area 
    fEnergyThresholdMethod = 1;
    fEnergyThresholdValue = 0.15;
    fEnergyThresholdBool = false;
    
    setEnergyBinning(0.2,-1,1);

    // Defaulting the energy binning to 0.2 log spaced bins between -1.5 - 2 (log space)
    setEnergyRange(-1,1,false);
    

	// initializing pointers to 0
	fMinimizer = 0;
	fFitfunction = 0;
    fModel = 0;
    fModel_linear = 0;
    fModel_intrinsic = 0;
	fGlobalBestFitParameters = 0;
	fEnergySpectrum = 0;
	fConfidenceInterval = 0;
	setNumThreads(1);
    setModel( fModelID,  fENorm);

	fLiteratureSpectra = 0;
	loadSpectraFromLiterature();
	fCrabID = 1;  // Default to Whipple 1998 (same as VFluxCalculator)
}


// Destructor
VLikelihoodFitter2::~VLikelihoodFitter2(){
	
	// cout << "Deleting VLikelihoodFitter2" << endl;
	// Deleting the likelihood objects
	// cout << "Deleting likelihood objects" << endl;
	for (unsigned int i = 0; i < fLikelihoodObjects.size(); i++){
		delete fLikelihoodObjects[i];
	}
	// cout << "Deleting likelihood objects...done!" << endl;
	// Deleting the minimizer
	// cout << "Deleting minimizer" << endl;
	if (fMinimizer) {delete fMinimizer;}
	// Deleting the fit function
	// cout << "Deleting fit function" << endl;
	if (fFitfunction) {delete fFitfunction;}
	// cout << "Deleting fit function...done!" << endl;
	// Deleting the model
	// cout << "Deleting model" << endl;
	if (fModel) {delete fModel;}
	// cout << "Deleting model...done!" << endl;
	// cout << "Deleting fModel_linear" << endl;
	if (fModel_linear) {delete fModel_linear;}
	// cout << "Deleting fModel_linear...done!" << endl;
	// cout << "Deleting fModel_intrinsic" << endl;
	if (fModel_intrinsic) {delete fModel_intrinsic;}
	// cout << "Deleting fModel_intrinsic...done!" << endl;
	// cout << "Deleting fGlobalBestFitParameters" << endl;
	if (fGlobalBestFitParameters) {delete fGlobalBestFitParameters;}
	// cout << "Deleting fGlobalBestFitParameters...done!" << endl;
	// cout << "Deleting fEnergySpectrum" << endl;
	if (fEnergySpectrum) {delete fEnergySpectrum;}
	// cout << "Deleting fEnergySpectrum...done!" << endl;
	// cout << "Deleting VLikelihoodFitter2...done!" << endl;
	if (fLiteratureSpectra) {delete fLiteratureSpectra;}
}

void VLikelihoodFitter2::setNormalisationEnergyLinear( double iNormEnergy )
{
    if( iNormEnergy <= 0 )
    {
        cout << "VLikelihoodFitter2::setNormalisationEnergyLinear Warning invalid normalization energy (" << iNormEnergy << " TeV)!"
                << "\t\tAllowed range: > 0."
                << "\t\tDefaulting to 1.0 TeV" << endl;
    }
    fENorm = iNormEnergy;
}

bool VLikelihoodFitter2::setEnergyRange(double i_min, double i_max, bool is_linear){
    if (is_linear && (i_min > 0) && (i_max > 0)){
        fEnergyMin = TMath::Log10(i_min);
        fEnergyMax = TMath::Log10(i_max);
        fEnergyMin_linear = i_min;
        fEnergyMax_linear = i_max;
    }
    else if (!is_linear){
        fEnergyMin = i_min;
        fEnergyMax = i_max;
        fEnergyMin_linear = TMath::Power(10, i_min);
        fEnergyMax_linear = TMath::Power(10, i_max);
    }
    else{
        cout << "VLikelihoodFitter2::setEnergyRange Error linear energies must be > 0\n"
             << "Defaulting to 0.1 - 10 TeV\n";
        return setEnergyRange(-1,1,false);

    }

    updateEnergyRange();
    return true;
}



// Setting the model
// ID:
// 0 - Power Law
// 1 - Power Law with Exponential Cut off
// 2 - Curved Power Law
// 3 - Log Parabola
// 4 - Log Parabola with Exponential Cut off
// 5 - Supper Exponential Cut off Log Parabola
// 6 - Broken Power Law
// ifENorm - normalization energy, if not set the defaults
// to what was set by VLikelihoodFitter2::setNormalisationEnergyLinear
void VLikelihoodFitter2::setModel( int i_ID, double ifENorm )
{
	std::ostringstream ss;

    if (fModel) {delete fModel;}
    if (fModel_linear) {delete fModel_linear;}
    if (fModel_intrinsic) {delete fModel_intrinsic;}
	// Defaulting to what ever was set by VLikelihoodFitter2::setNormalisationEnergyLinear
	if( ifENorm == 0 )
	{
		ifENorm = fENorm;
	}

	// Note for spectral weighting of bins, the spectral index must be the 2nd parameter
	// i.e. [1] = Spectral index
	if( i_ID == 0 )
	{
		ss.str( std::string() );
		ss << "[0]*TMath::Power(TMath::Power(10.0,x) /" << ifENorm << ", [1])";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss << "[0]*TMath::Power(x / " << ifENorm << ", [1])";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);

        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        

		fModelID = i_ID;
		fNParms = 2;
	}

	// Power Law with Exponential Cut off
	else  if( i_ID == 1 )
	{
		ss.str( std::string() );
		ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( 10, x )  / [2] )";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss << "[0] * TMath::Power( x  / " << ifENorm << ", [1] ) * TMath::Exp( -1. *  x / [2] )";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);
        fModel->SetParameter(2, 10);


        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);
        fModel_linear->SetParameter(2, 10);

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        fModel_intrinsic->SetParameter(2, 10);

		fModelID = i_ID;
		fNParms = 3;
	}


	// Curved Spectrum
	else if( i_ID == 2 )
	{
		ss.str( std::string() );
		ss << "[0] * TMath::Power( TMath::Power( 10, x )/ " << ifENorm << ", [1]+[2]*TMath::Power( 10, x ) )";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss << "[0] * TMath::Power(  x / " << ifENorm << ", [1]+[2]* x  )";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear);
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);
        fModel->SetParameter(2, 0.1);


        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);
        fModel_linear->SetParameter(2, 0.1);

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        fModel_intrinsic->SetParameter(2, 0.1);


		fModelID = i_ID;
		fNParms = 3;


	}

	// Log-parabola
	else if( i_ID == 3 )
	{
		ss.str( std::string() );
		ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << " , [1] + [2]*TMath::Log( TMath::Power( 10, x ) / " << ifENorm << " ) )";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss <<  "[0] * TMath::Power(  x  / " << ifENorm << " , [1]+[2]*TMath::Log( x / " << ifENorm << " ) )";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);
        fModel->SetParameter(2, 0.1);


        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);
        fModel_linear->SetParameter(2, 0.1);

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        fModel_intrinsic->SetParameter(2, 0.1);



		fModelID = i_ID;
		fNParms = 3;

	}


	// Power Law with Exponential Cut off
	// Energy-bin wise fit function
	else  if( i_ID == 11 )
	{
		ss.str( std::string() );
		ss << "[0] * TMath::Exp( " << ifENorm << "  / [2] ) * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( 10, x )  / [2] )";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss << "[0] * TMath::Exp( " << ifENorm << "  / [2] ) * TMath::Power( x  / " << ifENorm << ", [1] ) * TMath::Exp( -1. *  x / [2] )";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);
        fModel->SetParameter(2, 10);


        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);
        fModel_linear->SetParameter(2, 10);

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        fModel_intrinsic->SetParameter(2, 10);


		// fModelID = i_ID;
		fNParms = 3;
	}

	// Log-parabola with exp cutoff
	else if( i_ID == 4 )
	{
		ss.str( std::string() );
		ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << " , [1] + [2]*TMath::Log( TMath::Power( 10, x ) / " << ifENorm << " ) ) * TMath::Exp( -1. * TMath::Power( 10, x )  / [3] )";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss <<  "[0] * TMath::Power(  x  / " << ifENorm << " , [1]+[2]*TMath::Log( x / " << ifENorm << " ) )* TMath::Exp( -1. *  x   / [3] )";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		fModelID = i_ID;
		fNParms = 4;

        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);
        fModel->SetParameter(2, 0.1);
        fModel->SetParameter(3, 10);
        


        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);
        fModel_linear->SetParameter(2, 0.1);
        fModel_linear->SetParameter(3, 10);
        

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        fModel_intrinsic->SetParameter(2, 0.1);
        fModel_intrinsic->SetParameter(3, 10);
        


	}

	// Super Exponentially cut-off power law
	else if( i_ID == 5 )
	{
		ss.str( std::string() );
		ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( TMath::Power( 10, x )  / [2] , [3] ))";
		fModel = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );

		ss.str( std::string() );
		ss << "[0] * TMath::Power( x  / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( x / [2], [3] ) )";
		fModel_linear = new TF1( "fModel", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), fEnergyMin_linear, fEnergyMax_linear );


        fModel->SetParameter(0, 1e-12);
        fModel->SetParameter(1, -2.5);
        fModel->SetParameter(2, 10);
        fModel->SetParameter(3, 0.1);
        


        fModel_linear->SetParameter(0, 1e-12);
        fModel_linear->SetParameter(1, -2.5);
        fModel_linear->SetParameter(2, 10);
        fModel_linear->SetParameter(3, 0.1);
        

        fModel_intrinsic->SetParameter(0, 1e-12);
        fModel_intrinsic->SetParameter(1, -2.5);
        fModel_intrinsic->SetParameter(2, 10);
        fModel_intrinsic->SetParameter(3, 0.1);

		fModelID = i_ID;
		fNParms = 4;

	}


	// Broken Power Law
	// Need something for energy bin wise....
	// Default to a power law?
	// set gamma1/2 based on energy!
	else if( i_ID == 6 )
	{

		fNParms = 4;
		fModel = new TF1( "fModel", this, &VLikelihoodFitter2::brokenPowerLaw, fEnergyMin_linear, fEnergyMax_linear, fNParms + 1 );
		fModel_intrinsic = new TF1( "fModel_intrinsic", this, &VLikelihoodFitter2::brokenPowerLaw, fEnergyMin_linear, fEnergyMax_linear, fNParms + 1 );



		fModel_linear = new TF1( "fModel_linear", this, &VLikelihoodFitter2::brokenPowerLaw, fEnergyMin_linear, fEnergyMax_linear, fNParms + 1 );
		fModel_intrinsic = new TF1( "fModel_intrinsic", this, &VLikelihoodFitter2::brokenPowerLaw, fEnergyMin_linear, fEnergyMax_linear, fNParms + 1 );


		// Setting the break energy
		fModel->SetParameter( 3, ifENorm );
		fModel_intrinsic->SetParameter( 3, ifENorm );
		fModel_linear->SetParameter( 3, ifENorm );
		fModel_intrinsic->SetParameter( 3, ifENorm );

		// Fixing Linear/Log
		fModel->FixParameter( 4, 0. );
		fModel_intrinsic->FixParameter( 4, 0. );
		fModel_linear->FixParameter( 4, 1. );
		fModel_intrinsic->FixParameter( 4, 1. );

		fModelID = i_ID;

	}


	// Catching all others and defaulting to a power law
	else
	{
		cout << "VLikelihoodFitter2::setModel: Model " << i_ID << " not found...Add it to the collection!\n"
			 << "\t\tDefaulting to a Power Law Model (0) " << endl;

		setModel( 0, ifENorm );
	}

	// EBL Corrected Model
	// This returns fModel_intrinsic * exp(-tau * E)
	if( fEBLAnalysis == true )
	{

		fModel = new TF1( "fModel", this, &VLikelihoodFitter2::calculateIntrinsicSpectrum, fEnergyMin_linear, fEnergyMax_linear, fNParms );
		// fModel_linear = new TF1("fModel", "fModel.Eval(TMath::Log10(x))", TMath::Power(10, fEnergyMin_linear), TMath::Power(10, fEnergyMax_linear));

		// fModel_linear = new TF1("fModel",this, &VLikelihoodFitter2::calculateIntrinsicSpectrum, fEnergyMin_linear, fEnergyMax_linear, fNParms);
	}


	fFixedParameters = vector <double> (fNParms, -999);
    updateModel();
}


void VLikelihoodFitter2::setModel( TF1 *i_model){
    if (fModel) {delete fModel;}
    fModel = (TF1*)i_model->Clone();
    updateModel();
}


void VLikelihoodFitter2::updateModel(){
    for (unsigned int i = 0; i < fLikelihoodObjects.size() ; i++){
        fLikelihoodObjects[i]->setModel(fModel);
    }
}

void VLikelihoodFitter2::updateEnergyRange(){
    for (unsigned int i = 0; i < fLikelihoodObjects.size() ; i++){
        fLikelihoodObjects[i]->setEnergyRange(fEnergyMin, fEnergyMax, false);
    }
}


void VLikelihoodFitter2::updateEnergyBinning(){
    for (unsigned int i = 0; i < fLikelihoodObjects.size() ; i++){
        fLikelihoodObjects[i]->setBinning(fEnergyBinWidth, fEnergyBinMin, fEnergyBinMax);
    }
}

void VLikelihoodFitter2::setEnergyBinning(double i_binw, double i_binMin, double i_binMax){

    bool i_success = true;
    if (i_binw <= 0) {
        cout << "VLikelihoodFitter2::setEenrgyBinning Error bin width must be > 0 \n";
        i_success = false;
    } 
    else if (i_binMin >= i_binMax ) {
        cout << "VLikelihoodFitter2::setEenrgyBinning Error bin min must be  smaller than bin max \n";
        i_success = false;
    }
    else {
        fEnergyBinWidth = i_binw;
        fEnergyBinMin = i_binMin;
        fEnergyBinMax = i_binMax;
    }

    if (!i_success){
        cout << "VLikelihoodFitter2::setEenrgyBinning Error setting binning \n"
             << "Defaulting to 0.2 log spaced bins between 0.1-10 (-1, 1) TeV (log10 TeV)\n";
        setEnergyBinning(0.2,-1,1);
    }

}


void VLikelihoodFitter2::setEnergyThreshold(int i_method, double i_value, bool is_linear ){
	fEnergyThresholdMethod = i_method;
	fEnergyThresholdValue = i_value;
	fEnergyThresholdBool = is_linear;
	updateEnergyThreshold();
}


void VLikelihoodFitter2::updateEnergyThreshold(){
    for (unsigned int i = 0; i < fLikelihoodObjects.size() ; i++){
        fLikelihoodObjects[i]->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThresholdBool);
    }
}



double VLikelihoodFitter2::brokenPowerLaw( Double_t* x, Double_t* parm )
{
	// Define parameters
	double i_N0 = parm[0];
	double i_gamma_1 = parm[1];
	double i_gamma_2 = parm[2];
	double i_eBreak = parm[3];
	double linear = parm[4];
	double i_e = 0.;

	// Linear/log
	if( linear == 0. )
	{
		i_e = TMath::Power( 10., x[0] );
	}
	else
	{
		i_e = x[0];
	}

	if( i_e < i_eBreak )
	{
		return i_N0 * TMath::Power( i_e  / i_eBreak, i_gamma_1 ) ;
	}
	else
	{
		return i_N0 * TMath::Power( i_e / i_eBreak, i_gamma_2 ) ;
	}

}



// Fit function that takes into account EBL attenuation
double VLikelihoodFitter2::calculateIntrinsicSpectrum( Double_t* x, Double_t* parm )
{
	// Setting parameters
	for( unsigned int i = 0; i < fNParms; i++ )
	{
		fModel_intrinsic->SetParameter( i, parm[i] );
	}
	// Returning dNdE * exp (- tau)
	return fModel_intrinsic->Eval( x[0] ) * TMath::Exp( -1 * fEBLOpacityGraph->Eval( TMath::Power( 10, x[0] ) ) );

}


bool VLikelihoodFitter2::addFromCombined(string filename){
	TFile *i_file = new TFile(filename.c_str(), "READ");
	TTree* i_run_summary = (TTree*)i_file->Get("total_1/stereo/tRunSummary");
	int i_runnum;
	vector <int> i_runlist;

	i_run_summary->SetBranchAddress("runOn", &i_runnum);
	for (unsigned int i = 0; i < i_run_summary->GetEntries(); i ++){
		i_run_summary->GetEntry(i);
		if (i_runnum > 0){
			i_runlist.push_back(i_runnum);
		}
	}
	delete i_run_summary;
	i_file->Close();
	delete i_file;

	for (unsigned int i = 0; i < i_runlist.size(); i++){
		bool i_success = addRun(filename, i_runlist[i]);
		if (!i_success){
			cout << "VLikelihoodFitter2::addFromCombined Error adding run: " << i_runlist[i] << endl;
		}
		return i_success;
	}
	return true;
	
}

bool VLikelihoodFitter2::addRun(string filename){

    VLikelihoodObject *i_obj = new VLikelihoodObject(filename);

    if (!i_obj) {
        cout << "VLikelihoodFitter2::addRun Error adding from file: " << filename << endl; 
        return false;
    }
    // i_obj->setBinning(fEnergyBinWidth, fEnergyBinMin, fEnergyBinMax);
    i_obj->setEnergyRange(fEnergyMin, fEnergyMax, false);
    i_obj->setModel(fModel);
    i_obj->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThresholdBool);

    fLikelihoodObjects.push_back(i_obj);
    return true;
}


bool VLikelihoodFitter2::addRun(string filename, int runnum){

    VLikelihoodObject *i_obj = new VLikelihoodObject(filename, runnum);

    if (!i_obj) {
        cout << "VLikelihoodFitter2::addRun Error adding from file: " << filename << endl; 
        return false;
    }
    // i_obj->setBinning(fEnergyBinWidth, fEnergyBinMin, fEnergyBinMax);
    i_obj->setEnergyRange(fEnergyMin, fEnergyMax, false);
    i_obj->setModel(fModel);
    i_obj->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThresholdBool);

    fLikelihoodObjects.push_back(i_obj);
    return true;
}


void VLikelihoodFitter2::loadFromFile(string filename){
	// Open the file and get the runlist
	TFile *i_file = new TFile(filename.c_str(), "READ");
	TTree* i_run_summary = (TTree*)i_file->Get("total_1/stereo/tRunSummary");
	int i_runnum;
	vector <int> i_runlist;

	i_run_summary->SetBranchAddress("runOn", &i_runnum);
	for (unsigned int i = 0; i < i_run_summary->GetEntries(); i ++){
		i_run_summary->GetEntry(i);
		if (i_runnum > 0){
			i_runlist.push_back(i_runnum);
		}
	}
	delete i_run_summary;
	i_file->Close();
	delete i_file;

	// Add the runs one by one
	for (unsigned int i = 0; i < i_runlist.size(); i++){
		bool i_success = addRun(filename, i_runlist[i]);
		if (!i_success){
			cout << "VLikelihoodFitter2::loadFromFile Error adding run: " << i_runlist[i] << endl;
		}
	}
}


bool VLikelihoodFitter2::addObject(VLikelihoodObject *i_obj){
	if (!i_obj) {
		cout << "VLikelihoodFitter2::addObject Error adding object\n"; 
		return false;
	}
	i_obj->setEnergyRange(fEnergyMin, fEnergyMax, false);
	i_obj->setModel(fModel);
	i_obj->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThresholdBool);

	fLikelihoodObjects.push_back(i_obj);
	return true;
}



double VLikelihoodFitter2::getChi2( vector <double> i_parms, bool i_print ){
	// check if the length of parms matches the expected
	if( i_parms.size() != fNParms )
	{
		cout << "VLikelihoodFitter2::getChi2 Error invalid number of parameters!"
			 << "\n\t\tExpected: " << fNParms
			 << "\n\t\tReceived: " << i_parms.size() << endl;
		return 0;
	}

	double i_logl = -getLogL( i_parms );
	double i_logl0 = -getLogL0();
	// reset the model parameters to the best fit
	for (unsigned int i = 0; i < fNParms; i++ ){
		fModel->SetParameter(i, fGlobalBestFitParameters[i]);
	}
	int i_ndf = getNDF();

	if (i_print){
		cout << "LogL = " << i_logl << endl;
		cout << "LogL0 = " << i_logl0 << endl;
		cout << "NDF = " << i_ndf << endl;
		cout << "Chi2 = " << -2 * ( i_logl - i_logl0 ) << endl;
		cout << "Reduced Chi2 = " << -2 * ( i_logl - i_logl0 ) / i_ndf << endl;
	}

	return -2 * ( i_logl - i_logl0 );
}

int VLikelihoodFitter2::getNDF(){
	int i_max_bins = -999;
	for (unsigned int i = 0; i < fLikelihoodObjects.size(); i++){
		if (fLikelihoodObjects[i]->getNBins() > i_max_bins){
			i_max_bins = fLikelihoodObjects[i]->getNBins();
		}
	}
	fNBinsFit_Total = i_max_bins;
	return i_max_bins - fNParms;
}

// Making getting the profile likelihood
// Leaving minimization/fitting/optimization to the user
// Returning a TGraph of the profile likelihood makes it easier to plot while also saving the x data
TGraph *VLikelihoodFitter2::getProfileLikelihood( int i_parm, double i_min, double i_max, int i_nsteps, bool is_linear){
	
	// vector <double> i_profileLikelihood(i_nsteps);
	TGraph *i_profileLikelihood = new TGraph(i_nsteps);
	if (!is_linear){
		i_min = TMath::Log10(i_min);
		i_max = TMath::Log10(i_max);
	}

	double i_step = (i_max - i_min) / i_nsteps;
	vector <double> i_parms(fNParms);
	
	for (unsigned int i = 0; i < fNParms; i++ ){
		i_parms[i] = fGlobalBestFitParameters[i];
	}

	for (int i = 0; i < i_nsteps; i++){
		if (is_linear){
			i_parms[i_parm] = i_min + i*i_step;
		}
		else{
			i_parms[i_parm] = TMath::Power(10, i_min + i*i_step);
		}
		i_profileLikelihood->SetPoint(i, i_parms[i_parm],  -getLogL(i_parms));
	}
	
	return i_profileLikelihood;
}

vector <VLikelihoodFitter2*> VLikelihoodFitter2::getTimeBinnedData( double i_deltaT, double i_mjdMin, double i_mjdMax ){
	// first check the min/max are valid
	double i_tmpMin = 999999;
	double i_tmpMax = -999999;
	for (unsigned int i = 0; i < fLikelihoodObjects.size(); i++){
		if (i_tmpMin > fLikelihoodObjects[i]->getMJD()){
			i_tmpMin = fLikelihoodObjects[i]->getMJD();
		}
		if (i_tmpMax < fLikelihoodObjects[i]->getMJD()){
			i_tmpMax = fLikelihoodObjects[i]->getMJD();
		}
	}

	// Set the defaults
	if (i_mjdMin == -1){
		i_mjdMin = i_tmpMin;
	}
	if (i_mjdMax == -1){
		i_mjdMax = i_tmpMax;
	}


	// Cast to an int and round up
	int i_nbins = ceil((i_mjdMax - i_mjdMin) / i_deltaT);
	vector <double> i_timeBins(i_nbins);
	for (int i = 0; i < i_nbins; i++ ){
		i_timeBins[i] = i_mjdMin + i*i_deltaT;
	}

	return getTimeBinnedData(i_timeBins);
}


vector <VLikelihoodFitter2*> VLikelihoodFitter2::getTimeBinnedData( vector <double> i_timeBins ){
	
	vector <VLikelihoodFitter2*> i_likeObjs;
	// Loop over the time bins
	for (unsigned int i = 0; i < i_timeBins.size() -1; i++){
		VLikelihoodFitter2 *i_inBin = new VLikelihoodFitter2();

		for (unsigned int j = 0; j < fLikelihoodObjects.size(); j++){
			// Check if the MJD is in the bin
			if ( 
				(fLikelihoodObjects[j]->getMJD() > i_timeBins[i]) && 
				(fLikelihoodObjects[j]->getMJD() <= i_timeBins[i+1]) 
				){
				i_inBin->addObject(fLikelihoodObjects[j]->clone());
			}
		}

		if (i_inBin->fLikelihoodObjects.size() > 0){		
			// Update the energy range
			i_inBin->setEnergyRange(fEnergyMin, fEnergyMax, false);
			// Update the model
			i_inBin->setModel(fModel);
			// Update the energy threshold
			i_inBin->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThresholdBool);
			// Update the energy binning
			i_inBin->setEnergyBinning(fEnergyBinWidth, fEnergyBinMin, fEnergyBinMax);
			i_likeObjs.push_back(i_inBin);
		}
		else {
			delete i_inBin;
		}
	}

	return i_likeObjs;
}


// A more user safe implementation
double VLikelihoodFitter2::getLogL( vector <double> i_parms )
{
	// check if the length of parms matches the expected
	if( i_parms.size() != fNParms )
	{
		cout << "VLikelihoodFitter2::getLogL Error invalid number of parameters!"
			 << "\n\t\tExpected: " << fNParms
			 << "\n\t\tReceived: " << i_parms.size() << endl;
		return 0;
	}

	double* i_parmsConst = new double[fNParms];
	for( unsigned int i = 0; i < fNParms; i++ )
	{
		i_parmsConst[i] = i_parms[i];
	}

    double val = getLogL_internal( i_parmsConst );
    delete []i_parmsConst;
	return val;
}


// Internal to be used by the minimizer
double VLikelihoodFitter2::getLogL_internal( const double* i_parms ){

    
    // Update model parameters
    // ToDo abstract out
    for (unsigned int i = 0; i < fNParms; i++ ){
        fModel->SetParameter(i, i_parms[i]);
    }
    double val = 0;

	vector <VLikelihoodObject*> i_objs = getActiveRuns();
	omp_set_num_threads(fNumThreads);

    #pragma omp parallel for reduction (+:val)
	for (unsigned int i = 0; i < i_objs.size(); i++){
		val += i_objs[i]->getLogL();
	}

	return val;
}


// Internal to be used by the minimizer
double VLikelihoodFitter2::getLogL0 (){
    double val = 0;
	omp_set_num_threads(fNumThreads);
	vector <VLikelihoodObject*> i_objs = getActiveRuns();

    #pragma omp parallel for reduction (+:val)
	for (unsigned int i = 0; i < i_objs.size(); i++){
		val += i_objs[i]->getLogL0();
	}
	return val;

}


vector <VLikelihoodObject*> VLikelihoodFitter2::getActiveRuns(){
	vector <VLikelihoodObject*> i_likeObjs;
	for (unsigned int i = 0; i < fLikelihoodObjects.size(); i++){
		double i_mjd = fLikelihoodObjects[i]->getMJD();
		int i_run = fLikelihoodObjects[i]->getRunNumber();
		// Check if the run is in the excluded run list
		if ( find(fExcludeRun.begin(), fExcludeRun.end(), i_run) != fExcludeRun.end()) {
			continue;
		}
		// Check if the run's MJD is excluded
		else if (i_mjd > fMJDMax || i_mjd < fMJDMin){
			continue;
		}
		else{
			i_likeObjs.push_back(fLikelihoodObjects[i]);
		}
	}
	return i_likeObjs;
}

/*
 *  initializing the minimizer to be used
*  i_normGuess - initial guess of the normalization.
 *  iPrintStatue - level of printing output
 *  i_fixShape - Fix the spectral shape (used for getting spectral points)
 */
bool VLikelihoodFitter2::initializeMinimizer( double i_normGuess, int i_printStatus, bool i_fixShape )
{

	// Create minimizer if it hasn't already been initialized
	if( fMinimizer )
	{
		delete fMinimizer;
	}
	// Using Minuit2 and Minos
	// Use Minuit not Minuit2
	fMinimizer = ROOT::Math::Factory::CreateMinimizer( "Minuit", "Minos" );

	// // set tolerance , etc...
	fMinimizer->SetMaxFunctionCalls( 10000 ); // for Minuit/Minuit2
	fMinimizer->SetMaxIterations( 10000 ); // for GSL
	fMinimizer->SetTolerance( 0.01 ); // default talorance

	// Likelihood ratio test suggests
	// 2(log(l) - log(lmax)) ~ chi^2
	// therefore the errors are defined as 0.5
	fMinimizer->SetErrorDef( 0.5 );

	fMinimizer->SetPrintLevel( i_printStatus );


	// Checking the a global fit has been applied before trying to fix the shape
	if( !fGlobalBestFitParameters && i_fixShape )
	{
		cout << "VLikelihoodFitter2::initializeMinimizer Global fit must be applied before getting binwise fit "
			 << endl << "\t Call VLikelihoodFitter2::getLikelihoodFit() first!" << endl;

		return false;
	}


	// Initial step size and variable estimate
	double* step = new double[fNParms];
	double* variable = new double[fNParms];

	// Vector of parameter names
	fParmName.clear();
	fParmName.assign( fNParms, "" );

	// Deleting previous fit functions (if they exist)
	if( fFitfunction )
	{
		delete fFitfunction;
	}

	// Wrapping getLogL_internal and passing it to the Minimizer
	// Catching case of broken power law... Might not be needed?
	if( fModelID == 6 )
	{
		fFitfunction = new ROOT::Math::Functor( this, &VLikelihoodFitter2::getLogL_internal, fNParms + 1 );
		// Vector of parameter names
		fParmName.clear();
		fParmName.assign( fNParms + 1, "" );
	}
	else
	{
		fFitfunction = new ROOT::Math::Functor( this, &VLikelihoodFitter2::getLogL_internal, fNParms );
		fParmName.clear();
		fParmName.assign( fNParms, "" );
	}
	fMinimizer->SetFunction( *fFitfunction );


	// Setting initial parameters for each model

	// 0 - Power Law
	// 1 - Power Law with Exponential Cut off
	// 2 - Curved Power Law
	// 3 - Log Parabola
	// 4 - Log Parabola with Exponential Cut off
	// 5 - Supper Exponential Cut off Log Parabola
	// 6 - Broken Power Law
	if( fModelID == 0 )
	{

		step[0] = 0.01 * i_normGuess;
		step[1] = 0.01;
		variable[0] = i_normGuess;
		variable[1] = -2.5;
		fParmName[0] = "Norm";
		fParmName[1] = "Index";

		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1], -10, 0 );
		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), variable[1], step[1], -10, 0 );
		}

	}

	// Power Law with exp cut off
	if( fModelID == 1 )
	{
		step[0] = 0.01 * i_normGuess;
		step[1] = 0.01;
		step[2] = 0.01;
		variable[0] = i_normGuess;
		variable[1] = -1.5;
		variable[2] = 1.0;
		fParmName[0] = "Norm";
		fParmName[1] = "Index";
		fParmName[2] = "E_CutOff";



		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
			fMinimizer->SetVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1],  step[1] );
			fMinimizer->SetVariable( 2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2] );
		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1,fParmName[1].c_str(), variable[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), variable[1], step[1], -10, 0 );
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), variable[2], step[2], 1.e-9, 100 );
			// fMinimizer->SetVariable(2,fParmName[2].c_str(), variable[2], step[2]);
		}

	}


	// Curved Power Law
	if( fModelID == 2 )
	{
		step[0] = 0.01 * i_normGuess;
		step[1] = 0.01;
		step[2] = 0.01;
		variable[0] = i_normGuess;
		variable[1] = -1.5;
		variable[2] = -0.01;
		fParmName[0] = "Norm";
		fParmName[1] = "Index";
		fParmName[2] = "Beta";

		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			fMinimizer->SetVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1] );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2], -10, 0 );

		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			fMinimizer->SetVariable( 1, fParmName[1].c_str(), variable[1], step[1] );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
			// (unsigned int ivar, const string& name, double val, double step, double lower, double upper)
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), variable[2], step[2], -10, 0 );
		}
	}


	// Log Parabola
	if( fModelID == 3 )
	{
		step[0] = 0.01 * i_normGuess;
		step[1] = 0.001;
		step[2] = 0.001;
		variable[0] = i_normGuess;
		variable[1] = -1.5;
		variable[2] = -0.1;

		fParmName[0] = "Norm";
		fParmName[1] = "Alpha";
		fParmName[2] = "Beta";

		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2], -10, 0 );

		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), variable[1], step[1], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), variable[2], step[2], -10, 0 );

		}
	}


	// Log Parabola with exp cutoff
	else if( fModelID == 4 )
	{
		step[0] = 0.01 * i_normGuess;
		step[1] = 0.001;
		step[2] = 0.001;
		step[3] = 0.01;
		variable[0] = i_normGuess;
		variable[1] = -1.5;
		variable[2] = -0.1;
		variable[3] = 2.0;

		fParmName[0] = "Norm";
		fParmName[1] = "Alpha";
		fParmName[2] = "Beta";
		fParmName[3] = "E_Cutoff";

		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
			fMinimizer->SetLimitedVariable( 3, fParmName[3].c_str(), fGlobalBestFitParameters[3], step[3], 0., 100. );

		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), variable[1], step[1], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), variable[2], step[2], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
			fMinimizer->SetLimitedVariable( 3, fParmName[3].c_str(), variable[3], step[3], 0., 100. );


		}
	}

	// Log Parabola with exp cutoff
	else if( fModelID == 5 )
	{
		step[0] = 0.01 * i_normGuess;
		step[1] = 0.01;
		step[2] = 0.01;
		step[3] = 0.01;
		variable[0] = i_normGuess;
		variable[1] = -1.5;
		variable[2] = 1.0;
		variable[3] = 0.0;

		fParmName[0] = "Norm";
		fParmName[1] = "Index";
		fParmName[2] = "E_CutOff";
		fParmName[3] = "Delta";

		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2], 0, 100 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
			fMinimizer->SetLimitedVariable( 3, fParmName[3].c_str(), fGlobalBestFitParameters[3], step[3], -5., 5. );


		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), variable[1], step[1], -10, 0 );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), variable[2], step[2], 0., 100. );
			// fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
			fMinimizer->SetLimitedVariable( 3, fParmName[3].c_str(), variable[3], step[3], -5., 5. );



		}
	}



	// Broken Power Law
	else if( fModelID == 6 )
	{
		delete[] step;
		double* step = new double[fNParms + 1];
		delete[] variable;
		double* variable = new double[fNParms + 1];

		step[0] = 0.01 * i_normGuess;
		step[1] = 0.01;
		step[2] = 0.01;
		step[3] = 0.01;
		step[4] = 0.01;

		variable[0] = i_normGuess;
		variable[1] = -1.5;
		variable[2] = -2.0;
		variable[3] = 1.0;
		variable[4] = 1.0;

		fParmName[0] = "Norm";
		fParmName[1] = "Index1";
		fParmName[2] = "Index2";
		fParmName[3] = "E_Break";
		fParmName[4] = "Log-Linear";


		// Set the free variables to be minimized!
		if( i_fixShape )
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1], -10, 0 );
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2], -10, 0 );
			fMinimizer->SetLimitedVariable( 3, fParmName[3].c_str(), fGlobalBestFitParameters[3], step[3], 0.1, 100 );
			fMinimizer->SetLimitedVariable( 4, fParmName[4].c_str(), 0., step[4], -5., 5. );
			// fMinimizer->FixVariable(3);
			fMinimizer->FixVariable( 4 );

		}
		else
		{
			fMinimizer->SetLimitedVariable( 0, fParmName[0].c_str(), variable[0], step[0], 0, 1.E-5 );
			// fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
			fMinimizer->SetLimitedVariable( 1, fParmName[1].c_str(), variable[1], step[1], -10, 0 );
			fMinimizer->SetLimitedVariable( 2, fParmName[2].c_str(), variable[2], step[2], -10, 0 );
			fMinimizer->SetLimitedVariable( 3, fParmName[3].c_str(), variable[3], step[3], 0, 100 );
			fMinimizer->SetLimitedVariable( 4, fParmName[4].c_str(), 0., step[4], -5., 5. );

			// // Always fix E_break? no!
			// fMinimizer->FixVariable(3);
			fMinimizer->FixVariable( 4 );
		}
	}


	// Fixing the shape Parameters
	// This is used for getting spectral points
	// Only the norm is free
	if( i_fixShape )
	{
		for( unsigned int i = 1; i < fNParms; i++ )
		{
			fMinimizer->FixVariable( i );
		}
	}


	// User Defined Fixed variables
	for( unsigned int i = 0; i < fFixedParameters.size(); i ++ )
	{
		if( fFixedParameters[i] != -999 )
		{
			fMinimizer->SetVariable( i, fParmName[i].c_str(), fFixedParameters[i], 0 );
			fMinimizer->FixVariable( i );
		}
	}


	return true;

}


void VLikelihoodFitter2:: fixParameter( int i_parm, double i_value ){
	if (i_parm < fNParms){
		fFixedParameters[i_parm] = i_value;
	}
	else {
		cout << "Parameter " << i_parm << " is out of range" << endl;
	}
}


/*
* Function to optimise the likelihood function
* Returns a TF1* with the best fit parameters
* bContours decides whether 1 sigma contours are obtained (this is a slow process)
*/
TF1* VLikelihoodFitter2::fitEnergySpectrum(int verbosity)
{

	// initialize the minimizer
	initializeMinimizer();

	// Keeping a global copy of the best fit parameters
	if( fGlobalBestFitParameters )
	{
		delete fGlobalBestFitParameters;
	}

	// do the minimization
	fMinimizer->Minimize();
	// Get Best fit values
	const double* xs = fMinimizer->X();
	// Not needed when calling MINOS
	// fMinimizer->Hesse();

	fFitStatus =  fMinimizer->Status();
	if( fMinimizer->Status() != 0 )
	{
		cout << "VLikelihoodFitter2::getLikelihoodFit Warning fit status is not valid. Proceed with caution!" << endl;
	}
	// Getting the symmetric errors
	const double* i_Errors = fMinimizer->Errors();


	// Setting Global best fit parameters
	fGlobalBestFitParameters = new double[fNParms];
	for( unsigned int i = 0; i < fNParms; i++ )
	{
		fGlobalBestFitParameters[i] = xs[i];
	}


	// // Getting 1 sigma contour
	// // This is slow. Only done if requested
	// if( bContours )
	// {
	// 	std::ostringstream ss;
	// 	for( unsigned int i = 0 ; i < fNParms; i++ )
	// 	{


	// 		for( unsigned int j = 0 ; j < fNParms; j++ )
	// 		{
	// 			if( i == j )
	// 			{
	// 				continue;
	// 			}

	// 			// Clearing old data
	// 			fIContours.clear();
	// 			fJContours.clear();
	// 			fIContours.assign( fNContours, 0 );
	// 			fJContours.assign( fNContours, 0 );

	// 			// Getting contours of i,j parameters
	// 			fMinimizer->Contour( i, j, fNContours, &( fIContours )[0], &( fJContours )[0] );

	// 			// Storing contours in a 2 X fNConours vector
	// 			vector < vector <double> > contours;

	// 			contours.push_back( fIContours );
	// 			contours.push_back( fJContours );

	// 			// Storing pair in a map with key "i,j"
	// 			// This can be accessed later
	// 			ss.str( std::string() );
	// 			ss << i << "," << j;
	// 			fContourMap[ss.str()] = contours;

	// 		}
	// 	}
	// }


	// Saving best fit parameters (linear scale)
	fModel_linear->SetParameters( xs );
	fModel_linear->SetParErrors( i_Errors );

	// Cloning a copy to be returned
	TF1* i_BestFit = 0;
	// if( fEBLAnalysis )
	// {
	// 	fModel_intrinsic_linear->SetParameters( xs );
	// 	fModel_intrinsic_linear->SetParErrors( i_Errors );
	// 	i_BestFit = ( TF1* )fModel_intrinsic_linear->Clone();
	// }
	// else
	// {
		i_BestFit = ( TF1* )fModel_linear->Clone();
	// }

	// Upper and lower error values
	vector <double> parm_errorUp;
	vector <double> parm_errorLow;
	vector <bool> parm_status;

	// Getting Minos Errors
	vector <double> i_vec( fNParms );

	for( unsigned int i = 0; i < fNParms; i++ )
	{
		i_vec[i] = xs[i] ;
		double i_err_low = 0;
		double i_err_up = 0;

		// Getting minos (asymmetric) errors (true if error is valid)
		bool bErrors = fMinimizer->GetMinosError( i, i_err_low, i_err_up );

		parm_errorLow.push_back( i_err_low );
		parm_errorUp.push_back( i_err_up );
		parm_status.push_back( bErrors );

		if (verbosity > 0)
		{
			cout << scientific << "Variabile " << i << " Error Status: " << bErrors << ", E_Low = " << i_err_low << ", E_Up = " << i_err_up << endl;
		}
	}


	// Saving best fit parameters to fModel

	fModel->SetParameters( xs );
	fModel->SetParErrors( i_Errors );

	if (verbosity > 0){

		// Printing the fit details
		cout << "Binned Likelihood Fit: \n" ;
		cout << "Parameter \t Best Fit \t ErrorL \t ErrorU \t IsMin\n";
		for( unsigned int i = 0; i < fNParms; i++ )
		{
			cout << scientific << "[" << i << "]\t\t" << " " << fMinimizer->VariableName( i ) << " \t\t" << fModel->GetParameter( i )
				<< "\t\t" << parm_errorLow[i] << "\t\t"  << parm_errorUp[i] << "\t\t" <<  parm_status[i] << endl;
		}
		cout << "E_Norm: " << fENorm << endl << endl;


		// Getting covarance matrix
		cout << "\n\nPrinting Covariance Matrix\n";
		double* i_covmat = new double [fNParms * fNParms];

		for( unsigned int i = 0; i < fNParms; i++ )
		{
			for( unsigned int j = 0; j < fNParms; j++ )
			{
				i_covmat[ i * fNParms + j ] = fMinimizer->CovMatrix( i, j );
				cout << scientific << fMinimizer->CovMatrix( i, j ) << "\t" ;
			}
			cout << endl;
		}

		cout << "\n";

		// Getting Chi^2
		cout << "Calculating Total Chi^2\n";
		double i_chi2 = getChi2( i_vec, true );
		double i_ndf =  fNBinsFit_Total - fNParms;
		cout << "\n";

		// Saving Chi2 and NDF
		fModel_linear->SetChisquare( i_chi2 );
		fModel_linear->SetNDF( i_ndf );
		i_BestFit->SetChisquare( i_chi2 );
		i_BestFit->SetNDF( i_ndf );

		// Getting the 1 sigma confidence interval
		if( fConfidenceInterval )
		{
			delete fConfidenceInterval;
		}

		fConfidenceInterval = calculateConfidenceInterval( i_covmat, fModel, fModelID, fNParms );

	}
	

	// Calculating the model integrated flux
	// i_flux[0] = flux [photons/cm^2/s^1]
	// i_flux[1] = flux error [photons/cm^2/s^1]
	// i_flux[2] = flux [Crab]
	// i_flux[3] = flux error [Crab]

	if( fModelID != 4 )
	{

		float* i_flux = getIntegralFlux( fEnergyMin, fEnergyMax, fModel, true );
		if (verbosity > 0){
			cout << "Integral Flux:\n";
			cout << "F (" << TMath::Power( 10, fEnergyMin ) << " TeV < E < " << TMath::Power( 10, fEnergyMax ) << ") = " << i_flux[0] << "+/-" << i_flux[1] << " [Photons/cm^2/s] \n";
			cout << "F (" << TMath::Power( 10, fEnergyMax ) << " TeV < E < " << TMath::Power( 10, fEnergyMax ) << ") = " << i_flux[2] << "+/-" << i_flux[3] << " [Crab] \n";
		}
	}

	else
	{
		if (verbosity > 0){
			cout << "Integral Flux:\n";
			cout << "F (" << TMath::Power( 10, fEnergyMin ) << " TeV < E < " << TMath::Power( 10, fEnergyMax ) << ") = " << i_BestFit->GetParameter( 0 ) << "+/-" << i_BestFit->GetParError( 0 ) << " [Photons/cm^2/s] \n";
	
		}
	}


	// Getting Decorrelation Energy
	double E_d = fENorm * TMath::Exp( fMinimizer->CovMatrix( 0, 1 ) / xs[0] / i_Errors[1] / i_Errors[1] );
	if (verbosity > 0){
		cout << "Printing Decorrelation Energy (Assuming a Power Law Model, consider reapplying the fit.):\nE_d : " << E_d << endl;
	}

	return i_BestFit;
}



// Getting the confidence interval
// SOB use TF1::Derivative ?
TGraphAsymmErrors* VLikelihoodFitter2::calculateConfidenceInterval( double* i_covmat, TF1* i_fitfunction, int i_model, int i_fNparms )
{

	int i_nsteps = 1000;
	double i_start = fEnergyMin;
	double i_stop = fEnergyMax;

	double i_step = ( i_stop - i_start ) / i_nsteps;

	TGraphAsymmErrors* i_ConfidenceInterval = new TGraphAsymmErrors( i_nsteps );


	for( int i = 0; i < i_nsteps; i++ )
	{
		double i_energy = i_start + i * i_step ;
		double i_flux_err = 0;

		i_flux_err = 0;
		double i_flux = 0;

		if( fEBLAnalysis )
		{
			i_flux = i_fitfunction->Eval( i_energy ) * TMath::Exp( fEBLOpacityGraph->Eval( TMath::Power( 10, i_energy ) ) );
		}

		else
		{
			i_flux = i_fitfunction->Eval( i_energy );
		}


		// Power Law
		if( i_model == 0 )
		{

			// errN^2/N^2
			i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter( 0 ) / i_fitfunction->GetParameter( 0 );
			// errGamma^2/log^2(E/E0)
			i_flux_err += TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 3 ];
			// 2 log(E/E0) cov_Gamma_N / N
			i_flux_err += -2. * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 1 ] /  i_fitfunction->GetParameter( 0 );
			// F(E) * sqrt(previous)
			i_flux_err = i_flux * TMath::Sqrt( i_flux_err );
		}


		// Exponential Cut Off
		else if( i_model == 1 )
		{
			// errN^2/N^2
			i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter( 0 ) / i_fitfunction->GetParameter( 0 );
			// log^2(E/E0) * errGamma^2
			i_flux_err += TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 4 ];
			// (E/EC^2)^2 * errEC^2
			i_flux_err += TMath::Power( TMath::Power( 10., i_energy ) / i_fitfunction->GetParameter( 2 ) / i_fitfunction->GetParameter( 2 ), 2 ) * i_covmat[ 8 ];
			// 2 * log(E/E0) * cov_N_Gamma / N
			i_flux_err += -2. * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 1 ] /  i_fitfunction->GetParameter( 0 );
			// 2 * (E/E_C^2) * cov_N_EC / N
			i_flux_err += 2. * TMath::Power( 10., i_energy ) / i_fitfunction->GetParameter( 2 ) / i_fitfunction->GetParameter( 2 ) * i_covmat[ 2 ] /  i_fitfunction->GetParameter( 0 );
			// 2 * log(E/E0) * (E/E_C^2) * cov_Gamma_EC
			i_flux_err += -2. * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Power( 10., i_energy ) / i_fitfunction->GetParameter( 2 ) / i_fitfunction->GetParameter( 2 ) * i_covmat[ 5 ] ;

			i_flux_err = i_flux * TMath::Sqrt( i_flux_err );
		}



		// Curved Power Law
		else if( i_model == 2 )
		{
			// errN^2/N^2
			i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter( 0 ) / i_fitfunction->GetParameter( 0 );
			// log^2(E/E0) * errAlpha^2
			i_flux_err += TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 4 ];
			// (E/E0) * log^2(E/E0) * errBeta
			i_flux_err += TMath::Power( TMath::Power( 10., i_energy ) / fENorm * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ), 2 ) * i_covmat[ 8 ];
			// 2 * log(E/E0) * cov_N_Alpha / N
			i_flux_err += -2. * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 1 ] /  i_fitfunction->GetParameter( 0 );
			// 2 * (E/E0) log(E/E0) * cov_N_Beta / N
			i_flux_err += -2. * ( TMath::Power( 10., i_energy ) / fENorm * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) ) * i_covmat[ 2 ] /  i_fitfunction->GetParameter( 0 );
			// 2 * log^2(E/E0) * (E/E0) * cov_Alpha_Beta
			i_flux_err += 2. * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Power( 10., i_energy ) / fENorm * i_covmat[ 5 ] ;

			i_flux_err = i_flux * TMath::Sqrt( i_flux_err );
		}



		// Log Parabola
		else if( i_model == 3 )
		{
			// errN^2/N^2
			i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter( 0 ) / i_fitfunction->GetParameter( 0 );
			// log^2(E/E0) * errAlpha^2
			i_flux_err += TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 4 ];
			// log^4(E/E0) * errBeta^2
			i_flux_err += TMath::Power( TMath::Log( TMath::Power( 10., i_energy ) / fENorm ), 4 ) * i_covmat[ 8 ];
			// 2 * log(E/E0) * cov_N_alpha / N
			i_flux_err += -2. * TMath::Log( TMath::Power( 10., i_energy ) / fENorm ) * i_covmat[ 1 ] /  i_fitfunction->GetParameter( 0 );
			// 2 * log^2(E/E0) * cov_N_beta / N
			i_flux_err += -2. * TMath::Power( TMath::Log( TMath::Power( 10., i_energy ) / fENorm ), 2 ) * i_covmat[ 2 ] /  i_fitfunction->GetParameter( 0 );
			// 2 * log^3(E/E0) * cov_alpha_beta / N
			i_flux_err += 2. *  TMath::Power( TMath::Log( TMath::Power( 10., i_energy ) / fENorm ), 3 ) * i_covmat[ 5 ] ;

			i_flux_err = i_flux * TMath::Sqrt( i_flux_err );
		}



		i_ConfidenceInterval->SetPoint( i, TMath::Power( 10., i_energy ), i_flux );
		i_ConfidenceInterval->SetPointError( i, 0, 0, i_flux_err, i_flux_err ) ; // * TMath::Exp(fEBLOpacityGraph->Eval( TMath::Power(10,i_energy) ) ) );


	}

	return i_ConfidenceInterval;
}


void VLikelihoodFitter2::clearSpectralPoints(){
	fEnergyBins.clear();
	fEnergyBinCentres.clear();
	fSpectralPoint_likelihood_max.clear();
	fSpectralPoint_TS.clear();
	fSpectralPoint_FitStatus.clear();
}

/*
* Getting the energy spectrum
* This is done by applying a fit each energy bin
* with the spectral shape parameters frozen to the best fit values
* iBestFit - Best fit function
* bPrintAll == True - save results for all energy bins
* bPrintAll == False - save results for energy bins with a valid fit
*/
TGraphAsymmErrors* VLikelihoodFitter2::getEnergySpectrum( TF1* iBestFit, bool bPrintAll )
{

	// Clear previous fit details
	clearSpectralPoints();
	fEnergyBins = fLikelihoodObjects[0]->getEnergyBins();
	fEnergyBinCentres = fLikelihoodObjects[0]->getEnergyBinCentres();
	fNEnergyBins = fEnergyBinCentres.size();

	// Checking parameters (note expecting 1 - nPar for broken pwl)
	if( ( iBestFit->GetNpar() != ( int )fNParms )  && ( fModelID != 6 ) )
	{
		cout << "VLikelihoodFitter2::getEnergySpectrum Error invalid number of parameters!\n"
			 << "\t\tExpected: " << fNParms << endl
			 << "\t\tRecieved: " << iBestFit->GetNpar() << endl;
		return 0;
	}


	// Saving current fit details to reset later
	double iGlobalMin = fEnergyMin;
	double iGlobalMax = fEnergyMax;
	double iGlobalNorm = fENorm;

	double ifENorm;
	double* parms = iBestFit->GetParameters();
	cout << "\n\nGetting Energy Spectrum\n";

	for( unsigned int i = 0; i < fNParms; i++ )
	{
		cout << fParmName[i] << " : " << parms[i] << endl;
	}

	// Deleting any previous saved spectra
	if( fEnergySpectrum )
	{
		delete fEnergySpectrum;
	}

	fEnergySpectrum = new TGraphAsymmErrors();

	for(unsigned int i = 0 ; i < fNEnergyBins; i++ )
	{
		// Excluding bins outside of the minimum and maximum energy range
		// if ( ( fEnergyBins[i] < iGlobalMin ) ||   (fEnergyBins[i] > iGlobalMax || fEnergyBins[i+1] > iGlobalMax)  )
		if( ( fEnergyBins[i] < iGlobalMin ) || ( fEnergyBins[i] > iGlobalMax ) )
		{
			continue;
		}


		cout << "Getting energy bin : " << fEnergyBins[i]  << " - " <<  fEnergyBins[i + 1] << endl;


		// Applying fit to energy bin
		setEnergyRange( fEnergyBins[i], fEnergyBins[i + 1], false );
		ifENorm = TMath::Power( 10., fEnergyBinCentres[i] );

		// Getting fit
		getSpectralPoint( fEnergyBins[i], fEnergyBins[i + 1], ifENorm, iBestFit, bPrintAll );

	}

	// Resetting range and normalisation
	setEnergyRange( iGlobalMin, iGlobalMax, false );
	setNormalisationEnergyLinear( iGlobalNorm );
	setModel( fModelID, iGlobalNorm );

	cout << "Bin No.\tE\t\tEMin\t\tEMax\tdN/dE\terr(dN/dE)_low\terr(dN/dE)_high\tTS\tLog-Likelihood\tFitStatus\n";

	// Printing out energy spectrum
	for( int i = 0 ; i < fEnergySpectrum->GetN(); i++ )
	{


		cout << i << "\t" << fEnergySpectrum->GetX()[i] << "\t" << fEnergySpectrum->GetErrorXlow( i ) << "\t" << fEnergySpectrum->GetErrorXhigh( i ) << "\t"
			 << fEnergySpectrum->GetY()[i] << "\t" << fEnergySpectrum->GetErrorYlow( i ) << "\t" << fEnergySpectrum->GetErrorYhigh( i ) << "\t"
			//  << fSpectralPoint_TS[i] << "\t" << fSpectralPoint_likelihood_max[i] << "\t" << fSpectralPoint_FitStatus[i] << endl;
			 << 0 << "\t" << fSpectralPoint_likelihood_max[i] << "\t" << fSpectralPoint_FitStatus[i] << endl;

	}

	return fEnergySpectrum;
}


/*
* Function to apply fit to only the energy bin of interest
* Details of the best fit parameters are saved to a TGraphAsymmErrors
*	BinMin - Lower edge of energy bin
* BinMin - Upper edge of energy bin
* iBestFit - Best fit function
* bPrintAll == True - save results for all energy bins
* bPrintAll == False - save results for energy bins with a valid fit
*/
float* VLikelihoodFitter2::getSpectralPoint( double BinMin, double BinMax, double ifENorm, TF1* iBestFit, bool bPrintAll )
{

	// Setting fit range to that of the enrgy bin
	setEnergyRange( BinMin, BinMax, false );

	// Setting the normalization energy to the bin centre
	if( fModelID == 1 )
	{
		setModel( fModelID + 10, ifENorm );
	}
	else if( fModelID == 6 )
	{
		setModel( 0, ifENorm );
		if( ifENorm < iBestFit->GetParameter( 3 ) )
		{
			fModel->SetParameter( 1, iBestFit->GetParameter( 1 ) );
		}
		else
		{
			fModel->SetParameter( 1, iBestFit->GetParameter( 2 ) );
		}
	}
	else
	{
		setModel( fModelID, ifENorm );
	}

	// initializing the minimizing
	// Initial guess is set to the model evaluated at normalization energy
	// Print status of 0 (quiet)
	// Spectral shape parameters are frozen (true)
	if( !initializeMinimizer( iBestFit->Eval( ifENorm ), 0,  true ) )
	{
		cout << "VLikelihoodFitter2::getSpectralPoint minimizer initialization failed."
			 << endl << "\t Please run VLikelihoodFitter2::getLikelihoodFit() " << endl;
		return 0;
	}

	// Checking TS before applying fit
	cout << "Getting Total Counts for TS" << endl;
	float* i_fluxPoint = new float[3];


	// double i_onTotal = 0;
	// double i_offTotal = 0;
	// double i_mean_alpha = getMeanAlpha();


	// // Getting the total counts
	// for( unsigned int i = 0; i < fOnCounts.size(); i++ )
	// {

	// 	// Making sure run isn't excluded
	// 	if( isMJDExcluded( fRunList[i].MJD ) || isRunExcluded( fRunList[i].runnumber ) )
	// 	{
	// 		continue;
	// 	}

	// 	for( unsigned int j = 0; j < fOnCounts[0].size(); j++ )
	// 	{
	// 		if( fEnergyBinCentres[j] < fFitMin_logTeV )
	// 		{
	// 			continue;
	// 		}
	// 		if( fEnergyBinCentres[j] > fEnergyMax )
	// 		{
	// 			continue;
	// 		}
	// 		i_onTotal += fOnCounts[i][j];
	// 		i_offTotal += fOffCounts[i][j];

	// 	}
	// }



	// do the minimization
	fMinimizer->Minimize();

	// Fit details are saved if the fit is valid
	// If the fit isn't valid, the details are only outputted if bPrintAll == True
	if( ( fMinimizer->Status() != 0 ) && !bPrintAll )
	{
		return 0;
	}

	// Best fit and errors
	const double* i_FitValues = fMinimizer->X();
	const double* i_Errors = fMinimizer->Errors();


	i_fluxPoint[0] = i_FitValues[0];
	i_fluxPoint[1] = i_Errors[0];


	// Getting asymmetric errors
	double i_err_low = 0;
	double i_err_up = 0;

	fMinimizer->GetMinosError( 0, i_err_low, i_err_up );


	// Saving fit details to the TGraphAsymmErrors
	// (int) fEnergySpectrum->GetN() is always the next point to be set
	int npoints = fEnergySpectrum->GetN();


	// Saving the point to fEnergySpectrum
	fEnergySpectrum->SetPoint( npoints, ifENorm, i_FitValues[0] );

	// Checking if the errors are valid
	if( fMinimizer->Status() == 0 )
	{
		cout << "Setting Error " << npoints << " " << i_FitValues[0] << " " << abs( i_err_low ) << " " << i_err_up << endl;
		fEnergySpectrum->SetPointError( npoints,
										ifENorm - TMath::Power( 10., BinMin ), // E Min
										TMath::Power( 10., BinMax ) - ifENorm, // E Max
										abs( i_err_low ), // dNdE err low  SOB why abs?
										i_err_up ); // dNdE err high
	}

	else
	{
		fEnergySpectrum->SetPointError( npoints,
										ifENorm - TMath::Power( 10., BinMin ), // E Min
										TMath::Power( 10., BinMax ) - ifENorm, // E Max
										0, // dNdE err low
										0 ); // dNdE err high
	}


	// Saving some useful information
	fSpectralPoint_FitStatus.push_back( fMinimizer->Status() );
	// fSpectralPoint_TS.push_back( 0 );
	fSpectralPoint_likelihood_max.push_back( -1 * fMinimizer->MinValue() );

	return i_fluxPoint;
}



// // Returning Contour plot
// // Normalisation (parameter 0 of model) vs Index (or whatever parameter 1 is )
// TGraph* VLikelihoodFitter2::getContours( unsigned int i, unsigned int j )
// {

// 	// Checking the parameters are in the acceptable range
// 	if( ( i >= fNParms ) || ( j >= fNParms ) )
// 	{
// 		cout << "VLikelihoodFitter2::getContours invalid search parameters: "
// 			 << "\n\ti :" << i << " , j:" << j
// 			 << "\n Allowed range [0-" << fNParms - 1 <<  "]" << endl;
// 		return 0;
// 	}

// 	// Rejecting i = j
// 	if( i == j )
// 	{
// 		cout << "VLikelihoodFitter2::getContours cannot get contour of i = j" << endl;
// 		return 0;
// 	}

// 	// Getting contour vectors from map
// 	std::ostringstream ss;
// 	ss << i << "," << j;

// 	if( fContourMap.find( ss.str().c_str() ) != fContourMap.end() )
// 	{
// 		TGraph* i_contour = new TGraph( fNContours, &( fContourMap[ss.str().c_str()][0] )[0], &( fContourMap[ss.str().c_str()][1] )[0] );
// 		i_contour->GetXaxis()->SetTitle( fParmName[i].c_str() );
// 		i_contour->GetYaxis()->SetTitle( fParmName[j].c_str() );
// 		i_contour->SetTitle( "" );

// 		return i_contour;
// 	}

// 	return 0;

// }


void VLikelihoodFitter2::setNumThreads( int i_numThreads ){
	// todo add compiler flag for this
	// cout << "OpenMP is not enabled. Please enable OpenMP in your compiler settings to use multi-threading." << endl;
	if (i_numThreads > 0){
		fNumThreads = i_numThreads;
	}
	else{
		cout << "VLikelihoodFitter2::setNumThreads Error invalid number of threads! \n\t Defaulting to 1" << endl;
		fNumThreads = 1;
	}
	omp_set_num_threads(fNumThreads);
}


/*
 * calculate integral flux from best fit model
 * Model is simply integrated
 */
float* VLikelihoodFitter2::getIntegralFlux( double i_EMin, double i_EMax, TF1* i_Model, bool i_log, bool i_ul )
{


	// Getting the ingtegral flux from the best fit model

	if( i_log )
	{
		i_EMin = TMath::Power( 10, i_EMin );
		i_EMax = TMath::Power( 10, i_EMax );
	}

	float* i_flux = 0;

	if( i_ul )
	{
		i_flux = new float[2];
		i_flux[0] = i_Model->GetParameter( 0 ) * ( TMath::Power( i_EMax, i_Model->GetParameter( 1 ) + 1 ) - TMath::Power( i_EMin, i_Model->GetParameter( 1 ) + 1 ) ) / ( i_Model->GetParameter( 1 ) + 1 ) / ( TMath::Power( fENorm, i_Model->GetParameter( 1 ) ) ) ;
		i_flux[1] = getCrabFlux( i_flux[0], i_EMin, i_EMax );
		return i_flux;
	}

	i_flux = new float[4];

	// Calculating flux from best fit model;
	i_flux[0] = i_Model->GetParameter( 0 ) * ( TMath::Power( i_EMax, i_Model->GetParameter( 1 ) + 1 ) - TMath::Power( i_EMin, i_Model->GetParameter( 1 ) + 1 ) ) / ( i_Model->GetParameter( 1 ) + 1 ) / ( TMath::Power( fENorm, i_Model->GetParameter( 1 ) ) ) ;

	// Calculating the Error
	float a, b, c, d;
	// dN_int/dNo
	a = ( TMath::Power( i_EMax, i_Model->GetParameter( 1 ) + 1 ) - TMath::Power( i_EMin, i_Model->GetParameter( 1 ) + 1 ) ) / ( i_Model->GetParameter( 1 ) + 1 ) / ( TMath::Power( fENorm, i_Model->GetParameter( 1 ) ) ) ;

	// dN_int/dGamma
	b = i_Model->GetParameter( 0 ) * ( TMath::Power( i_EMax, i_Model->GetParameter( 1 ) + 1 ) - TMath::Power( i_EMin, i_Model->GetParameter( 1 ) + 1 ) ) / ( i_Model->GetParameter( 1 ) + 1 ) / ( i_Model->GetParameter( 1 ) + 1 )  / ( TMath::Power( fENorm, i_Model->GetParameter( 1 ) ) ) ;

	c = i_Model->GetParameter( 0 ) * ( TMath::Power( i_EMax, i_Model->GetParameter( 1 ) + 1 ) * TMath::Log( i_EMax ) - TMath::Power( i_EMin, i_Model->GetParameter( 1 ) + 1 ) * TMath::Log( i_EMin ) ) / ( i_Model->GetParameter( 1 ) + 1 )  / ( TMath::Power( fENorm, i_Model->GetParameter( 1 ) ) ) ;

	d =  - i_Model->GetParameter( 0 ) * ( TMath::Power( fENorm, -1 * i_Model->GetParameter( 1 ) ) ) * TMath::Log( fENorm ) * ( TMath::Power( i_EMax, i_Model->GetParameter( 1 ) + 1 ) - TMath::Power( i_EMin, i_Model->GetParameter( 1 ) + 1 ) ) / ( i_Model->GetParameter( 1 ) + 1 ) ;



	// Calculating Error
	i_flux[1] = sqrt( a * a * i_Model->GetParError( 0 ) * i_Model->GetParError( 0 ) + ( b + c + d ) * ( b + c + d ) *  i_Model->GetParError( 1 ) * i_Model->GetParError( 1 ) );



	i_flux[2] = getCrabFlux( i_flux[0], i_EMin, i_EMax );
	i_flux[3] = getCrabFlux( i_flux[1], i_EMin, i_EMax );

	return i_flux;
}


/*
 *  calculate flux in Crab units
 *
 *  (GM) which Crab is this? Whipple?
 *  (SOB) Yes Whipple 1998
 *  ToDo: Implement different Crab spectra options
 */
double VLikelihoodFitter2::getCrabFlux( double iF, double i_EMin, double i_EMax )
{
	// double i_N0 = 3.20e-11;
	// double i_Crab = i_N0 * (TMath::Power(i_EMax, i_Gamma +1 ) - TMath::Power(i_EMin, i_Gamma +1 ) ) /( i_Gamma +1);
	if( bValidLiterature )
	{
		double i_Crab = fLiteratureSpectra->getIntegralFlux( i_EMin, i_EMax, fCrabID );
		return ( iF / i_Crab );
	}
	return 0;
}


/*
  Use a VEnergySpectrumfromLiterature instance to handle Crab Flux
  by default we use the Whipple 1998 Crab Spectrum

*/
void VLikelihoodFitter2::loadSpectraFromLiterature( string filename )

{
	if( fLiteratureSpectra )
	{
		delete fLiteratureSpectra;
	}

	// This requires the specific AstroData to be loaded in.
	if( filename == "" )
	{
		fLiteratureSpectra = new VEnergySpectrumfromLiterature( "$VERITAS_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat" );
	}
	else
	{
		fLiteratureSpectra = new VEnergySpectrumfromLiterature( filename );
	}
	// "not" zombie as zombie suggests file couldn't be opened
	// Invalid files will print a sane error without thowing errors
	// Will use bValidLiterature to check we have a valid
	bValidLiterature = !fLiteratureSpectra->isZombie();
}