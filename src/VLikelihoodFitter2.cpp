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


    // Defaulting Energy threshold to 15% of max effective area 
    fEnergyThresholdMethod = 1;
    fEnergyThresholdValue = 0.15;
    fEnergyThreaholdBool = false;
    
    setEnergyBinning(0.2,-1,1);

    // Defaulting the energy binning to 0.2 log spaced bins between -1.5 - 2 (log space)
    setEnergyRange(-1,1,false);
    
    fModel = 0;
    fModel_linear = 0;
    fModel_intrinsic = 0;
    setModel( fModelID,  fENorm);

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
    // parallel? ToDo
    for (auto i_obj : fLikelihoodObjects ){
        val += i_obj->getLogL();
    }

    return val;
}
