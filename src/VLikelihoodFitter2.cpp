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
    fEnergyThreaholdBool = false;
    
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
	fPool = 0;
	setNumThreads(1);
    setModel( fModelID,  fENorm);

}


// Destructor
VLikelihoodFitter2::~VLikelihoodFitter2(){
	
	for (unsigned int i = 0; i < fLikelihoodObjects.size(); i++){
		delete fLikelihoodObjects[i];
	}
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2" << endl;
	// Deleting the minimizer
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting minimizer" << endl;
	if (fMinimizer) {delete fMinimizer;}
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting fit function" << endl;
	if (fFitfunction) {delete fFitfunction;}
	
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting model" << endl;
	if (fModel) {delete fModel;}
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting linear model" << endl;
	if (fModel_linear) {delete fModel_linear;}
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting intrinsic model" << endl;
	if (fModel_intrinsic) {delete fModel_intrinsic;}
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting global best fit parameters" << endl;
	if (fGlobalBestFitParameters) {delete fGlobalBestFitParameters;}
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting thread pool" << endl;
	if (fPool) {delete fPool;}
	// cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Deleting likelihood objects" << endl;
	cout << "VLikelihoodFitter2::~VLikelihoodFitter2: Done" << endl;
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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );

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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );


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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );

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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );

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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );

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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );

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
		fModel_linear = new TF1( "fModel", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );
		fModel_intrinsic = new TF1( "fModel_intrinsic", ss.str().c_str(), TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ) );


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



		fModel_linear = new TF1( "fModel_linear", this, &VLikelihoodFitter2::brokenPowerLaw, TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ), fNParms + 1 );
		fModel_intrinsic = new TF1( "fModel_intrinsic", this, &VLikelihoodFitter2::brokenPowerLaw, TMath::Power( 10, fEnergyMin_linear ), TMath::Power( 10, fEnergyMax_linear ), fNParms + 1 );


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


void VLikelihoodFitter2::updateEnergyThreshold(){
    for (unsigned int i = 0; i < fLikelihoodObjects.size() ; i++){
        fLikelihoodObjects[i]->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThreaholdBool = false );
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


bool VLikelihoodFitter2::addRun(string filename){

    VLikelihoodObject *i_obj = new VLikelihoodObject(filename);

    if (!i_obj) {
        cout << "VLikelihoodFitter2::addRun Error adding from file: " << filename << endl; 
        return false;
    }
    // i_obj->setBinning(fEnergyBinWidth, fEnergyBinMin, fEnergyBinMax);
    i_obj->setEnergyRange(fEnergyMin, fEnergyMax, false);
    i_obj->setModel(fModel);
    i_obj->setEnergyThreshold(fEnergyThresholdMethod, fEnergyThresholdValue, fEnergyThreaholdBool);

    fLikelihoodObjects.push_back(i_obj);
    return true;

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

	// Single thread
	if (fNumThreads == 1){
		for (auto i_obj : fLikelihoodObjects ){
			val += i_obj->getLogL();
		}
		return val;
	}
	// Multi-threaded
	else{
		// Get the individual logL values for each object
		auto logl = fPool->Map([](VLikelihoodObject *i_obj) { return i_obj->getLogL(); }, fLikelihoodObjects);
		// combine in serial
		for (auto i_logl : logl){
			val += i_logl;
		}
		return val;
	}

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



/*
* Function to optimise the likelihood function
* Returns a TF1* with the best fit parameters
* bContours decides whether 1 sigma contours are obtained (this is a slow process)
*/
TF1* VLikelihoodFitter2::fitEnergySpectrum()
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


		cout << scientific << "Variabile " << i << " Error Status: " << bErrors << ", E_Low = " << i_err_low << ", E_Up = " << i_err_up << endl;
	}


	// Saving best fit parameters to fModel

	fModel->SetParameters( xs );
	fModel->SetParErrors( i_Errors );


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

	// // Getting Chi^2
	// cout << "Calculating Total Chi^2\n";
	// double i_chi2 = getChi2( i_vec );
	// double i_ndf =  fNBinsFit_Total - fNParms;
	// cout << "\n";

	// // Saving Chi2 and NDF
	// fModel_linear->SetChisquare( i_chi2 );
	// fModel_linear->SetNDF( i_ndf );
	// i_BestFit->SetChisquare( i_chi2 );
	// i_BestFit->SetNDF( i_ndf );

	// // Getting the 1 sigma confidence interval
	// if( fConfidenceInterval )
	// {
	// 	delete fConfidenceInterval;
	// }

	// fConfidenceInterval = calculateConfidenceInterval( i_covmat, fModel, fModelID, fNParms );


	// // Calculating the model integrated flux
	// // i_flux[0] = flux [photons/cm^2/s^1]
	// // i_flux[1] = flux error [photons/cm^2/s^1]
	// // i_flux[2] = flux [Crab]
	// // i_flux[3] = flux error [Crab]

	// if( fModelID != 4 )
	// {

	// 	float* i_flux = getIntegralFlux( fFitMin_logTeV, fFitMax_logTeV, fModel, true );

	// 	cout << "Integral Flux:\n";
	// 	cout << "F (" << TMath::Power( 10, fFitMin_logTeV ) << " TeV < E < " << TMath::Power( 10, fFitMax_logTeV ) << ") = " << i_flux[0] << "+/-" << i_flux[1] << " [Photons/cm^2/s] \n";
	// 	cout << "F (" << TMath::Power( 10, fFitMin_logTeV ) << " TeV < E < " << TMath::Power( 10, fFitMax_logTeV ) << ") = " << i_flux[2] << "+/-" << i_flux[3] << " [Crab] \n";
	// }

	// else
	// {


	// 	cout << "Integral Flux:\n";
	// 	cout << "F (" << TMath::Power( 10, fFitMin_logTeV ) << " TeV < E < " << TMath::Power( 10, fFitMax_logTeV ) << ") = " << i_BestFit->GetParameter( 0 ) << "+/-" << i_BestFit->GetParError( 0 ) << " [Photons/cm^2/s] \n";
	// }


	// // Getting Decorrelation Energy
	// double E_d = fENorm * TMath::Exp( fMinimizer->CovMatrix( 0, 1 ) / xs[0] / i_Errors[1] / i_Errors[1] );
	// cout << "Printing Decorrelation Energy (Assuming a Power Law Model, consider reapplying the fit.):\nE_d : " << E_d << endl;


	return i_BestFit;
}



void VLikelihoodFitter2::setNumThreads( int i_numThreads ){
	if (i_numThreads < 1){
		cout << "VLikelihoodFitter2::setNumThreads Error invalid number of threads: " << i_numThreads << endl;
		// default to 1
		fNumThreads = 1;
		return;
	}
	else if (i_numThreads == 1){
		fNumThreads = 1;
	}
	else {
		if (fPool){delete fPool;}
		fNumThreads = i_numThreads;
		fPool = new ROOT::TProcessExecutor(fNumThreads);
	}
}