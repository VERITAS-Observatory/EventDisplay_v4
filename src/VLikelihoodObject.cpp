/*! class VLikelihoodObject
Calculate the likelihood of a model for a single run
based on Piron et al 2001 astro-ph/0106196
*/
#include "VLikelihoodObject.h"


// From a combined file
VLikelihoodObject::VLikelihoodObject( string filename, int run_number ){
	// quickly open the file and find the correct index
	int i_runnum;
	int indx = -1;

	TFile *i_file =  TFile::Open(filename.c_str(), "read");
	TTree* i_run_summary = (TTree*)i_file->Get("total_1/stereo/tRunSummary");
	i_run_summary->SetBranchAddress("runOn", &i_runnum);
	for (unsigned int i = 0; i < i_run_summary->GetEntries(); i ++){
		i_run_summary->GetEntry(i);
		if (i_runnum ==  run_number){
			indx = i; 
			break;
		}
	}
	if (indx < 0){
		cout << "VLikelihoodObject::VLikelihoodObject Error run not found in " << filename;
		return;
	}

	// delete TTree, then close the file, then delete the TFile
	delete i_run_summary;
	i_file->Close();
	delete i_file;

	initialize( filename, indx );
}

// From a single file
VLikelihoodObject::VLikelihoodObject( string filename ){
	int indx = 0;
	initialize(filename, indx);
}


void VLikelihoodObject::initialize(string filename, int indx){
	

	fOnHistogramRebinned = 0;
	fOffHistogramRebinned = 0;
	fAnaFile =  TFile::Open(filename.c_str(), "read");
	TTree* run_summary = (TTree*)fAnaFile->Get("total_1/stereo/tRunSummary");


	if (fAnaFile->IsZombie()){
		cout << "VLikelihoodObject::VLikelihoodObject Error opening TFile: " << filename << endl;
		return;
	}

	int i_runnum;
	double i_MJD, i_Duration, i_DeadTimeFraction, i_Elevation, i_Azimuth, i_Alpha;
	run_summary->SetBranchAddress("runOn", &i_runnum);
	run_summary->SetBranchAddress("MJDOn", &i_MJD);
	run_summary->SetBranchAddress("tOn", &i_Duration);
	run_summary->SetBranchAddress("DeadTimeFracOn", &i_DeadTimeFraction);
	run_summary->SetBranchAddress("elevationOn", &i_Elevation);
	run_summary->SetBranchAddress("azimuthOn", &i_Azimuth);
	run_summary->SetBranchAddress("OffNorm", &i_Alpha);

	run_summary->GetEntry(indx);

	fRunNumber = i_runnum;
	fMJD = i_MJD;
	fDuration = i_Duration;
	fDeadTimeFraction = i_DeadTimeFraction;
	fElevation = i_Elevation;
	fAzimuth = i_Azimuth;
	fAlpha = i_Alpha;
	fLiveTime = fDuration * (1. - fDeadTimeFraction);


	// Reset
	// cout <<"Clearing Pointers" << endl;
	// clearPointers();

	// Get Raw Histograms
	fOnHistogram = getCountingHistogramRaw( "on" );
	fOffHistogram = getCountingHistogramRaw( "off" );

	// Checking that both sets of histograms returned ok
	if( !fOnHistogram || !fOffHistogram )
	{
		cout << "VLikelihoodObject::VLikelihoodObject error getting On/Off histograms" << endl;
		return;
	}

	fMeanEffectiveAreaMC = getEffectiveAreasMCFromFile();
	// Checking if Effective Areas returned ok
	if( !fMeanEffectiveAreaMC )
	{
		cout << "VLikelihoodObject::VLikelihoodObject error getting effective areas" << endl;
		return;
	}


	fResponseMatrix = getResponseMatrixFromFile();
	// Checking if response matrix returned ok
	if( !fResponseMatrix )
	{
		cout << "VLikelihoodObject::VLikelihoodObject error getting response matrix" << endl;
		return;
	}

	// Decouple from root file
	fOnHistogram->SetDirectory(0);
	fOffHistogram->SetDirectory(0);
	// fMeanEffectiveAreaMC->SetDirectory(0);
	fResponseMatrix->SetDirectory(0);

	if( !fMeanEffectiveAreaMC )
	{
		cout << "2VLikelihoodObject::VLikelihoodObject error getting effective areas" << endl;
		return;
	}
	setBinning(0.15);
	setEnergyThreshold(1, 0.2);
	setEnergyRange(-1,2, false);
	fThresholdBias = 0.15;
	fAnaFile->Close();
	fAnaFile = 0;
	// delete fAnaFile;

}

VLikelihoodObject::~VLikelihoodObject(){
	clearPointers();
	
}


// Model Excess/off counts are always useful to have 
vector <double> VLikelihoodObject::getModelPredictedExcess(){

	vector <double> i_vModel;
	vector <double> iSpectralWeightedCentres;



	// Getting Spectrally weighted bin centres
	for( int i = 0; i < fNEnergyBins; i++ )
	{
		// Spectral weighted bin centres assuming fModel->GetParameter(1) is spectral index or a model equivilent
		iSpectralWeightedCentres.push_back( VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyBins[i], fEnergyBins[i + 1], fModel->GetParameter(1) ) );
		// cout << "i: " << fEnergyBinCentres[i] << " " << iSpectralWeightedCentres[i] << endl;
	}


	double i_ModelElement = 0;  // dNdE (E)
	double i_EffectiveAreaElement = 0; // A_eff(E)
	double i_ReconstructionMatrixElement = 0; // Gamma(E_true -> E_rec)
	double i_ConversionElement = 1.e4; // m^-2 -> cm^-2
	double i_EnergyElement = 0; // dE
	double i_TOn = 0; // T_live

	// Current Bin
	int i_CurrentRecBin = 0;
	int i_CurrentMCBin = 0;

	TAxis* xpoints = fResponseMatrixRebinned->GetXaxis();
	TAxis* ypoints = fResponseMatrixRebinned->GetYaxis();
	vector <double> i_modelCounts(fNEnergyBins, 0);


	// Looping over each Rec Energy bin
	for( int i = 0; i < fNEnergyBins; i++ )
	{

		// i_modelCounts.push_back( 0 );
		// Fining bin index of current energy bin
		i_CurrentRecBin = xpoints->FindBin( iSpectralWeightedCentres[i] );

		// Looping over each MC Energy bin
		for( int j = 0; j < fNEnergyBins; j++ )
		{
			// Fining bin index of current energy bin
			i_CurrentMCBin = ypoints->FindBin( fEnergyBinCentres[j] );

			// Only including points with a sensible bias
			if( fEnergyBias[j] > fThresholdBias )
			{	
				// cout << fEnergyBias[j] << " "  << fThresholdBias << endl;
				continue;
			}

			// dN/dE at E_MC
			i_ModelElement = fModel->Eval( iSpectralWeightedCentres[j] );

			// Eff at E_MC
			i_EffectiveAreaElement = fMeanEffectiveAreaMC->Eval( iSpectralWeightedCentres[j] );

			// Response Matrix at (E_Rec,E_MC)
			i_ReconstructionMatrixElement = fResponseMatrixRebinned->GetBinContent( i_CurrentRecBin, i_CurrentMCBin );
			if( i_ReconstructionMatrixElement < 1.e-5 )
			{
				i_ReconstructionMatrixElement = 0;
			}

			// Dead time corrected exposure
			i_TOn = fLiveTime  ;
			// i_TOn = fRunList[i].tOn ;

			// dE
			i_EnergyElement = pow( 10.0, fEnergyBins[j + 1] ) - pow( 10.0, fEnergyBins[j] );

			// Summing over MC
			i_modelCounts[i] +=   i_ModelElement * i_EffectiveAreaElement * i_ReconstructionMatrixElement * i_ConversionElement *  i_TOn  * i_EnergyElement ;
			// cout << j << " " <<   i_ModelElement << " " <<  i_EffectiveAreaElement << " " <<  i_ReconstructionMatrixElement << " "
			// <<  i_ConversionElement << " " <<   i_TOn  << " " <<  i_EnergyElement  << endl	;
		}
	}

	xpoints = 0;
	ypoints = 0;

	delete xpoints;
	delete ypoints;
	return i_modelCounts;
}


vector <double> VLikelihoodObject::getModelPredictedOff(){

	vector <double> i_myModel = getModelPredictedExcess();
	// Size known at runtime
	vector <double> i_OffMLE(i_myModel.size(), 0);


	vector <double> i_vTmp;
	// Looping over each bin
	for( unsigned int i = 0; i < i_myModel.size(); i++ )
	{
		double i_a = 0;
		double i_b = 0;

		// alpha * (on + off) - (alpha + 1)*S_predicted
		i_a = fAlpha * ( fOnCounts[i] + fOffCounts[i] ) - ( fAlpha  + 1 ) * i_myModel[i];

		// 1/(2 * alpha *( alpha +1 ) ) * i_a *sqrt(i_a^2 + 4 * alpha *(alpha +1)*off*S_predicted)
		i_b = 1.0 / ( 2.0 * fAlpha * ( fAlpha  + 1.0 ) );
		i_b *= ( i_a + TMath::Sqrt( TMath::Power( i_a, 2.0 ) + 4.0 * fAlpha * ( fAlpha + 1.0 ) * fOffCounts[i] * i_myModel[i] ) );

		// Energy bin
		i_OffMLE[i] = i_b;
	}

	return i_OffMLE;
}

// LogL
double VLikelihoodObject::getLogL(){


	double LogLi = 0;



	vector <double> i_model = getModelPredictedExcess();
	vector <double> i_modelOff = getModelPredictedOff();


	// Make things easier to read
	double a, b, c, d;

	// Looping over data bins
	for( int i = 0; i < fNEnergyBins; i ++ )
	{
		// Fit min/max
		if( (fEnergyBinCentres[i] < fEnergyMin) || (fEnergyBinCentres[i] > fEnergyMax))  
		{
			continue;
		}


		if( fOnCounts[i] >= 1 )
		{
			a = fOnCounts[i] * TMath::Log( i_model[i] + fAlpha * i_modelOff[i] );
		}
		// 0*log(0)
		else
		{
			a = 0;
		}

		if( fOffCounts[i] >= 1 && i_modelOff[i] >= 1 )
		{
			b = fOffCounts[i] * TMath::Log( i_modelOff[i] );
		}
		// 0*log(0)
		else
		{
			b = 0;

		}
		// Getting c term
		c = -1.0 * ( fAlpha + 1.0 ) * i_modelOff[i];

		// Getting d term
		d = -i_model[i];

		LogLi +=  a + b + c + d;
	}

	// Returning - logl for minimization
	return -1 * LogLi;
}

// LogL0 is obtainer by setting the model predicted on/off
// to be equal to the observed
double VLikelihoodObject::getLogL0(){
	double LogLi = 0;

	double a, b, c, d;

	// Looping over data bins
	for( int i = 0; i < fNEnergyBins; i ++ )
	{

		// Fit min/max
		if( (fEnergyBinCentres[i] < fEnergyMin) || (fEnergyBinCentres[i] > fEnergyMax))  
		{
			continue;
		}


		// Avoiding 0*log(0)
		if( fOnCounts[i] >= 1 )
		{
			a = fOnCounts[i] * TMath::Log( fOnCounts[i] );
		}
		else
		{
			a = 0;
		}


		// Avoiding 0*log(0)
		if( fOffCounts[i] >= 1 )
		{
			b = fOffCounts[i] * TMath::Log( fOffCounts[i] );
		}
		else
		{
			b = 0;

		}

		c = -1 * int( fOnCounts[i] );
		d = -1 * int( fOffCounts[i] );

		LogLi += a + b + c + d;
	}

	return -1 * LogLi;
}


// Number of degrees of freedom
int VLikelihoodObject::getNDF(){
	int i_NDF = 0;
	for (unsigned int i = 0; i < fEnergyBinCentres.size(); i++){
		// Fit min/max
		if( (fEnergyBinCentres[i] < fEnergyMin) || (fEnergyBinCentres[i] > fEnergyMax))  
		{
			continue;
		}
		i_NDF += 1;
	}
	// Subtract off model parameters
	return i_NDF - fModel->GetNumberFreeParameters();
}



// Analysis energy range
void VLikelihoodObject::setEnergyRange(double i_min, double i_max, bool is_linear){
	if (is_linear && (i_min > 0) && (i_max > 0)){
		fEnergyMin = TMath::Log10(i_min);
		fEnergyMax = TMath::Log10(i_max);
	}
	else if (!is_linear) {
		fEnergyMin = i_min;
		fEnergyMax = i_max;	
	}
	else{
		cout << "VLikelihoodObject::setEnergyRange failed: "
			 << "\n\tIf is_linear == True then i_min, i_max must be > 0"
			 << "Setting energy range to 0.1-100 TeV\n";
		fEnergyMin = -1;
		fEnergyMax = 2;
	}
}



// Energy threshold
// i_method = 0 // fixed energy at i_value with is_linear defining if TeV or log10(TeV) 
// i_method = 1 // fraction of the max effective area. Fraction set at i_value
// i_method = 2 // set as at a fixed value of the energy bias (bins > bias are excluded)
void VLikelihoodObject::setEnergyThreshold(int i_method, double i_value, bool is_linear){
	if (i_method == 0){
		if (is_linear && (i_value > 0)){
			fEnergyThreshold = TMath::Log10(i_value);
		}
		else if (!is_linear){
			fEnergyThreshold = i_value;
		}
		else{
			cout << "VLikelihoodObject::setEnergyThreshold failed: "
			 << "\n\tIf is_linear == True ti_value must be > 0"
			 << "Defaulting to 15% of the max effective area\n";
			setEnergyThreshold(1, 0.15);
		}
	}
	else if (i_method == 1){
		if ((i_value > 0) && (i_value < 1)){
			vector <double> ieff(fEnergyBinCentres.size());
			double i_max = -999.;
			for (unsigned int i = 0; i < fEnergyBinCentres.size(); i++){
				ieff[i] = fMeanEffectiveAreaMC->Eval(fEnergyBinCentres[i]);
				if (ieff[i] > i_max) {
					i_max = ieff[i];
				}
			}
			for (unsigned int i = 0; i < fEnergyBinCentres.size(); i++){
				ieff[i] /= i_max;
				// Break on the first instance of a good bin
				if (ieff[i] > i_value) {
					fEnergyThreshold = fEnergyBinCentres[i-1];
					break;
				}
			}
		}
		else{
			cout << "VLikelihoodObject::setEnergyThreshold failed: "
			 << "\n\ti_value must be in the range (0-1) when using method 1 (fraction of max effective area)"
			 << "Defaulting to 15% of the max effective area\n";
			setEnergyThreshold(1, 0.15);
		}
	}
	else if (i_method == 2){
		if (i_value > 0){
			for (unsigned int i = 0; i < fEnergyBinCentres.size(); i++){
				// Break on the first instance of a good bin
				if (fEnergyBias[i] < i_value) {
					fEnergyThreshold = fEnergyBinCentres[i-1];
					break;
				}
			}
		}
		else {
				cout << "VLikelihoodObject::setEnergyThreshold failed: "
			 << "\n\ti_value must be > 0 when using method 2 (energy bias)"
			 << "Defaulting to 15% of the max effective area\n";
			setEnergyThreshold(1, 0.15);
		}

	}
}



// // Counts histograms
// //////////////////////////////////////////////////////////
// // Stored for easy rebinning 
// TH1D* fOnRebinnedHistogram;
// TH1D* fOffRebinnedHistogram;

// // Model
// // This should be a strictly private pointer to the model
// // Keep private to make sure the model is thread safe
// // Model interface should be handled by the likelihood fitting class
// TF1 *fModel;				

// // IRFs
// //////////////////////////////////////////////////////////
// // Respose matrix
// TH2F* fResponseMatrixRebinned;
// // Effective Areas (MC, True energy)
// TGraphAsymmErrors* fMeanEffectiveAreaMC;
// // Energy bias
// vector <double> fEnergyBias;



TH1D* VLikelihoodObject::getCountingHistogramRaw( string onoff )
{

	if (!fAnaFile) {
		cout << "VLikelihoodObject::getCountingHistogramRaw Error getting counts histrogram for run " << fRunNumber 
			 << "\n\tAnasum file is not open!\n";
	}

	std::ostringstream ss;
	// Getting histograms from anasum file
	ss.str( std::string() );
	// ss << "herecCounts_" << onoff;
	ss << "run_" << fRunNumber << "/stereo/energyHistograms/herecCounts_" << onoff;
	TH1D* i_hErecCounts = ( TH1D* )fAnaFile->Get(ss.str().c_str())->Clone();

	// Checking if histograms exist
	if( !i_hErecCounts )
	{
		cout << "VLikelihoodObject::getCountingHistogramRaw Error getting " << onoff << " counts histrogram for run " << fRunNumber << endl;
		return 0;

	}

	// Renaming
	ss.str( std::string() );
	ss << "herecCounts_" << onoff << "_" << fRunNumber << endl;
	i_hErecCounts->SetTitle( ss.str().c_str() );
	return i_hErecCounts;
}


// Getting Vector of effective areas
// returns:
// 	  MC energy effective areas
TGraphAsymmErrors* VLikelihoodObject::getEffectiveAreasMCFromFile()
{

	string hname;
	// TGraphAsymmErrors*> iVtemp ;
	std::ostringstream ss;



	TGraphAsymmErrors* i_gMeanEffectiveArea_on = 0;
	TGraphAsymmErrors* i_gMeanEffectiveArea_off = 0;
	TGraphAsymmErrors* i_gMeanEffectiveArea = 0;

	// hname = "gMeanEffectiveAreaMC_on";
	ss.str( std::string() );
	ss << "run_" << fRunNumber << "/stereo/EffectiveAreas/gMeanEffectiveAreaMC_on";

	// Using VEnergySpectrum's code to get effective areas
	i_gMeanEffectiveArea_on = ( TGraphAsymmErrors* )fAnaFile->Get( ss.str().c_str());
	bool i_onValid = isPointerValid( i_gMeanEffectiveArea_on );


	ss.str( std::string() );
	ss << "run_" << fRunNumber << "/stereo/EffectiveAreas/gMeanEffectiveAreaMC_off";

	i_gMeanEffectiveArea_off = ( TGraphAsymmErrors* )fAnaFile->Get( ss.str().c_str());
	bool i_offValid = isPointerValid( i_gMeanEffectiveArea_off );

	// Trying to get On effective area MC
	if( i_onValid )
	{
		i_gMeanEffectiveArea = ( TGraphAsymmErrors* )i_gMeanEffectiveArea_on->Clone();
	}

	// Trying to get Off effective area MC
	else if( i_offValid )
	{

		cout << "VLikelihoodObject::getEffectiveAreasMCFromFile On MC Effective Area failed. Using Off MC effective Area (Run: " <<  fRunNumber << ")" << endl;
		i_gMeanEffectiveArea = ( TGraphAsymmErrors* )i_gMeanEffectiveArea_off->Clone();
	}

	// Cleaning up
	delete i_gMeanEffectiveArea_on;
	delete i_gMeanEffectiveArea_off;

	// Checking if there are any issues
	if( !i_gMeanEffectiveArea )
	{
		cout << "1VLikelihoodObject::getEffectiveAreasMCFromFile On MC Effective Area failed. Using Off MC effective Area (Run: " <<  fRunNumber << ")" << endl;
		return 0;
	}

	// Renaming
	ss.str( std::string() );
	ss << "gMeanEffectiveAreaMC_" << fRunNumber;


	i_gMeanEffectiveArea->SetTitle( ss.str().c_str() );

	// Save the runwise effective area to a vector
	return ( TGraphAsymmErrors* )i_gMeanEffectiveArea->Clone();


}


/*
  Getting Vectors of raw response matrix
  Require^1 histograms binned to minimum expected bin size.
  By default the parameter files request 0.05.

  Use TGraph2D::Interpolate to go from:
  E_mc vs E_mc/E_rec -> E_rec vs E_mc

  ^1 only require if we want to use histograms rather than another interpolator
*/
TH2F* VLikelihoodObject::getResponseMatrixFromFile()
{

	// TH2F* iVtemp ;
	string hname;
	std::ostringstream ss;


	// Getting response matrix from .anasum.root file
	// Getting histograms from anasum file
	ss.str( std::string() );
	ss << "run_" << fRunNumber << "/stereo/EffectiveAreas/hResponseMatrix_on";
	hname =  "hResponseMatrix_on";
	TH2F* i_hResponseMatrix_on = ( TH2F* )fAnaFile->Get(ss.str().c_str())->Clone();
	ss.str( std::string() );
	ss << "run_" << fRunNumber << "/stereo/EffectiveAreas/hResponseMatrix_off";
	hname =  "hResponseMatrix_off";
	TH2F* i_hResponseMatrix_off = ( TH2F* )fAnaFile->Get(ss.str().c_str())->Clone();

	// Renaming
	ss.str( std::string() );
	ss << "hResponseMatrix_Interpolated_" << fRunNumber;

	bool i_onValid = isPointerValid( i_hResponseMatrix_on );
	bool i_offValid = isPointerValid( i_hResponseMatrix_off );


	// Reshape to required dimensions
	// Bins should span -1, 2
	int i_fNEnergyBins = int( 3 / 0.05 );
	int i_fNEnergyBinsMC = int( 3 / 0.05 );

	// Obtaining the energy bins
	vector <double> i_fEnergyBins;

	for( int j = 0; j <= i_fNEnergyBins; j ++ )
	{
		i_fEnergyBins.push_back( -1.1 + j * 0.05 );
	}


	vector <double> i_fEnergyBinsMC;

	for( int j = 0; j <= i_fNEnergyBinsMC; j ++ )
	{
		i_fEnergyBinsMC.push_back( -1.1 + j * 0.05 );
	}

	TH2F* ihres_tmp = new TH2F( "ihres_tmp", "Response Matrix; E_{Rec}; E_{MC}", i_fNEnergyBins, &( i_fEnergyBins[0] ), i_fNEnergyBinsMC, &( i_fEnergyBinsMC[0] ) );
	float ieng_rc = 0;
	float ieng_mc = 0;
	float ebias = 0;

	TGraph2D* igSys2d = 0;

	/*
		On/Off check is required
		when 0 on/off counts the on/off effective areas/response matrices aren't filled.
	*/
	if( i_onValid )
	{
		// For interpolating
		igSys2d = new TGraph2D( i_hResponseMatrix_on );
	}
	else if( i_offValid )
	{
		// For interpolating
		igSys2d = new TGraph2D( i_hResponseMatrix_off );
	}
	else
	{
		cout << "VLikelihoodObject::getResponseMatrixRaw error getting response matrix for run " << fRunNumber << " On:" << i_onValid << " Off:" << i_offValid <<  endl;
		return 0;
	}

	for( int ierec = 0; ierec < ihres_tmp->GetXaxis()->GetNbins(); ierec++ )
	{
		ieng_rc = ihres_tmp->GetXaxis()->GetBinCenter( ierec + 1 );
		for( int jemc = 0; jemc < ihres_tmp->GetYaxis()->GetNbins(); jemc++ )
		{
			ieng_mc = ihres_tmp->GetYaxis()->GetBinCenter( jemc + 1 );
			ebias = TMath::Power( 10, ieng_rc ) / TMath::Power( 10, ieng_mc );

			float intep = igSys2d->Interpolate( ieng_mc, ebias );
			ihres_tmp->SetBinContent( ierec + 1, jemc + 1, intep );
		}
	}
	// expect integral over E_mc (Y) to be 1
	VHistogramUtilities::normalizeTH2D_y( ihres_tmp );
	ihres_tmp->SetTitle( ss.str().c_str() );

	delete i_hResponseMatrix_on;
	delete i_hResponseMatrix_off;
	delete igSys2d;

	return ihres_tmp;

}


// Generic function to get counts from the histograms
vector <double> VLikelihoodObject::getCounts( TH1D* i_hTemp )
{
	vector <double> i_vTemp(i_hTemp->GetXaxis()->GetNbins(), 0);

	// Looping over the number of bins in each run
	for( int i = 0; i < i_hTemp->GetXaxis()->GetNbins(); i++ )
	{
		// Checking for any funky stuff going on
		// Shouldn't happen....
		if( i_hTemp->GetBinContent( i + 1 ) > 1e9 )
		{
			cout << "VLikelihoodObject::getCounts Error in Run " << fRunNumber <<  endl;
			i_vTemp[i] =  1.e-9;
			continue;
		}

		// Adding a small number to advoid log(0) errors
		i_vTemp[i] =  i_hTemp->GetBinContent( i + 1 ) + 1.e-9 ;
	}

	return i_vTemp;
}


void VLikelihoodObject::setBinning(double i_binWidth, double i_binMin, double i_binMax){
	int i_nBins = (i_binMax - i_binMin) / i_binWidth;
	vector <double>  i_binEdges;
	for (int i = 0; i <= i_nBins; i ++){
		// cout << i_binMin + i * i_binWidth << " ";
		i_binEdges.push_back(i_binMin + i * i_binWidth);
	}
	// cout << endl;
	setAnalysisBinning(i_binEdges.size()-1, i_binEdges );
}


// Set the binning to be used in the analysis
bool VLikelihoodObject::setAnalysisBinning( int i_fNBins, vector <double> i_fBins ){

	string hname;
	// Number of bins
	fNEnergyBins = i_fNBins;
	// Bin edges
	fEnergyBins = i_fBins;
	// Bin Centres
	fEnergyBinCentres.clear();

	// Bias of an energy bin
	fEnergyBias.clear();

	// Getting bin centres
	for( int i = 0; i < fNEnergyBins; i++ )
	{
		fEnergyBinCentres.push_back( fEnergyBins[i] + 0.5 * ( fEnergyBins[i + 1] - fEnergyBins[i] ) );
	}


	// Bias vector
	vector <double> i_RunBias;
	std::ostringstream ss;


	// Rebinning Off counts
	ss.str( std::string() );
	ss << "Rebinned On Counts " << fRunNumber;
	if (fOnHistogramRebinned) {delete fOnHistogramRebinned;}
	if (fOffHistogramRebinned) {delete fOffHistogramRebinned;}
	fOnHistogramRebinned = ( TH1D* ) fOnHistogram->Rebin( fNEnergyBins, ss.str().c_str(), &(fEnergyBins)[0]);

	// Rebinning Off counts
	ss.str( std::string() );
	ss << "Rebinned Off Counts " << fRunNumber;
	fOffHistogramRebinned = ( TH1D* ) fOffHistogram->Rebin( fNEnergyBins, ss.str().c_str(), &( fEnergyBins )[0] );


	// Response Matrix
	// Define empty TH2F with desired analysis binning
	// Histogram is then filled from raw histograms
	ss.str( std::string() );
	ss << "RebinnedResponseMatrix_" << fRunNumber;

	cout << endl;
	TH2F* i_htmp2D = new TH2F( ss.str().c_str(), ss.str().c_str(), fNEnergyBins, &( fEnergyBins )[0], fNEnergyBins, &( fEnergyBins )[0] );
	TAxis* i_xAxis = fResponseMatrix->GetXaxis();
	TAxis* i_yAxis = fResponseMatrix->GetYaxis();

	// Looping over and filling histogram
	for( int j = 1; j <= i_xAxis->GetNbins(); j++ )
	{
		for( int k = 1; k <= i_yAxis->GetNbins() ; k++ )
		{
			i_htmp2D->Fill( i_xAxis->GetBinCenter( j ), i_yAxis->GetBinCenter( k ),  fResponseMatrix->GetBinContent( j, k ) );
		}
	}

	// Normalizing in Y direction (sum MC = 1)
	VHistogramUtilities::normalizeTH2D_y( i_htmp2D );


	// Determining Bias
	// Bias is obtained by applying a gaussian fit to each y-slice
	for( int j = 1; j <= i_htmp2D->GetYaxis()->GetNbins(); j++ )
	{
		TF1* i_fFit = new TF1( "i_fFit", "gaus", -2, 2.5 );
		i_fFit->SetParameter( 0, 0.6 );
		i_fFit->SetParameter( 1, i_htmp2D->GetYaxis()->GetBinCenter( j ) );
		i_fFit->SetParameter( 2, 0.2 );

		TH1D* i_slice = ( TH1D* )i_htmp2D->ProjectionX( "i_slice", j, j );

		// Skipping empty bins
		if( i_slice->GetEntries() == 0 )
		{
			fEnergyBias.push_back( 1.0 );
			continue;
		}

		// Fitting a gaussian quietly
		// ROOT 6 seems to fail unless likelihood (L) fit is applied...
		i_slice->Fit( "i_fFit", "0Lq" );

		// Bias defined as:
		// mu = Gaussian_mean (from fit)
		// E_true = Expected Energy (E_MC)
		// Bias = abs( ( mu - E_true ) / E_true )
		double tmp = TMath::Power( 10., i_fFit->GetParameter( 1 ) ) - TMath::Power( 10., i_htmp2D->GetYaxis()->GetBinCenter( j ) ) ;
		tmp /= TMath::Power( 10., i_htmp2D->GetYaxis()->GetBinCenter( j ) );

		// Saving to a vector
		fEnergyBias.push_back( abs( tmp ) );

	}



	// Setting histogram titles
	i_htmp2D->GetXaxis()->SetTitle( "Energy_{rec} [TeV]" );
	i_htmp2D->GetYaxis()->SetTitle( "Energy_{mc} [TeV]" );


	// Storing the response matrix for this run
	// if (fResponseMatrixRebinned) { delete fResponseMatrixRebinned;}
	fResponseMatrixRebinned = i_htmp2D;


	// Getting counts from histograms
	fOnCounts.clear();
	fOnCounts = getCounts( fOnHistogramRebinned );
	fOffCounts.clear();
	fOffCounts = getCounts( fOffHistogramRebinned );


	fOnHistogramRebinned->SetDirectory(0);
	fOffHistogramRebinned->SetDirectory(0);
	fResponseMatrixRebinned->SetDirectory(0);


	// delete i_xAxis;
	// delete i_yAxis;
	// delete i_htmp2D;
	// delete i_OnRebinnedHistogram;
	// delete i_OffRebinnedHistogram;
	return true;
}


void VLikelihoodObject::clearPointers() {
	// cout << "Deleting VLikelihoodObject pointers" << endl;
	// cout << "Deleting fOnHistogram" << endl;
	if (fOnHistogram) { delete fOnHistogram;}
	// cout << "Deleting fOffHistogram" << endl;
	if (fOffHistogram) { delete fOffHistogram;}
	// cout << "Deleting fOnHistogramRebinned" << endl;
	if (fOnHistogramRebinned) {  delete fOnHistogramRebinned;}
	// cout << "Deleting fOffHistogramRebinned" << endl;
	if (fOffHistogramRebinned) { delete fOffHistogramRebinned;}
	// cout << "Deleting fMeanEffectiveAreaMC" << endl;
	if (fMeanEffectiveAreaMC) { delete fMeanEffectiveAreaMC;}
	// cout << "Deleting fResponseMatrix" << endl;
	if (fResponseMatrix) { delete fResponseMatrix;}
	// cout << "Deleting fResponseMatrixRebinned" << endl;
	if (fResponseMatrixRebinned) { delete fResponseMatrixRebinned;}
	// Model is external to VLikelihoodObject
	fModel = 0;
	
}



// Check if response matrix is ok to use
bool VLikelihoodObject::isPointerValid( TH2F* i_obj )
{
	bool i_notNull = false;
	bool i_hasCounts = false;

	if( i_obj ){
		i_notNull = true;
	}
	if( i_obj->GetEntries() != 0 ){
		i_hasCounts = true;
	}

	return i_notNull && i_hasCounts;
}

// Check if effective area is ok to use
bool VLikelihoodObject::isPointerValid( TGraphAsymmErrors* i_obj )
{
	bool i_notNull = false;
	bool i_hasCounts = false;

	if( i_obj ){
		i_notNull = true;
	}
	if( i_obj->Integral() >= 1 ){
		i_hasCounts = true;
	}

	return i_notNull && i_hasCounts;
}



TCanvas *VLikelihoodObject::peak(){
	TCanvas* i_cTemp = new TCanvas();
	i_cTemp->Divide( 2, 2 );

	// Axis 1 on counts
	TPad* p = ( TPad* )i_cTemp->cd( 1 );
	fOnHistogramRebinned->Draw();
	p->SetGrid();
	p->Update();
	TLine *l1 = new TLine(fEnergyThreshold, 0, fEnergyThreshold, gPad->GetUymax());
	l1->SetLineColor(kBlack);
	l1->Draw("SAME");

	// Axis 2 off counts
	p = ( TPad* )i_cTemp->cd( 2 );
	fOffHistogramRebinned->Draw();
	p->SetGrid();
	p->Update();
	TLine *l2 = new TLine(fEnergyThreshold, 0, fEnergyThreshold, gPad->GetUymax());
	l2->SetLineColor(kBlack);
	l2->Draw("SAME");


	// Axis 3 Effective areas
	p = ( TPad* )i_cTemp->cd( 3 );
	fMeanEffectiveAreaMC->Draw("APL");
	p->SetGrid();
	p->Update();
	TLine *l3 = new TLine(
		fEnergyThreshold, 
		gPad->GetUymin(), 
		fEnergyThreshold, 
		TMath::MaxElement(
			fMeanEffectiveAreaMC->GetN(),
			fMeanEffectiveAreaMC->GetY()
		)
	);
	p->SetLogy();
	l3->SetLineColor(kBlack);
	l3->Draw("SAME");


	// Axis 4 Response Matrix
	p = ( TPad* )i_cTemp->cd( 4 );
	fResponseMatrixRebinned->Draw("COLZ");
	p->SetGrid();
	p->Update();
	TLine *l4 = new TLine(fEnergyThreshold, gPad->GetUymin(), fEnergyThreshold, gPad->GetUymax());
	l4->SetLineColor(kBlack);
	l4->Draw("SAME");

	return i_cTemp;
}

// // Energy Threshold and Limits
// double fEnergyThreshold;
// double fEnergyMin;
// double fEnergyMax;


// // Runwise details that one might want to make cuts on
// //////////////////////////////////////////////////////////
// int fRunNumber				// Run number
// double fMJD;				// MJD of the run
// double fDuration			// Duration of the run
// double fDeadTimeFraction	// Dead time fraction
// double fLiveTime			// Dead time corrected exposure
// double fElevation			// Eelevation of the run


// // File I/O
// //////////////////////////////////////////////////////////
// TGraphAsymmErrors* getEffectiveAreasMCFromFile(TFile *i_anasumFile);
// TH2F* getResponseMatrixFromFile(TFile *i_anasumFile);


// // Msc Helper functions
// //////////////////////////////////////////////////////////

// // Check if Object is valid
// bool isPointerValid( TGraphAsymmErrors* i_obj );
// bool isPointerValid( TH2F* i_obj );



VLikelihoodObject * VLikelihoodObject::fakeIt(TF1 *i_model, int i_run_num, double i_mjd){
	// Get a copy of the current object
	int i_current_run = fRunNumber;
	TF1 *i_current_model = fModel;
	double i_current_mjd = fMJD;
	vector <double> i_current_on = fOnCounts;
	vector <double> i_current_off = fOffCounts;

	// Assign the simulated values
	fModel = i_model;
	fMJD =  (i_mjd > 0)? i_mjd : fMJD;
	fRunNumber =  (i_run_num > 0)? i_run_num : fRunNumber;

	// Get the model predicted counts
	vector <double> i_model_counts = getModelPredictedExcess();
	vector <double> i_model_off = getModelPredictedOff();

	// Assign the model predicted counts
	TRandom3 i_rand(0);
	// i_rand.SetSeed(0);
	for (unsigned int i = 0; i < i_model_counts.size(); i++){
		fOnCounts[i] = i_rand.Poisson(i_model_counts[i]);
		fOnCounts[i] += fAlpha * i_rand.Poisson(i_model_off[i]);
		fOffCounts[i] = i_rand.Poisson(i_model_off[i]);
	}

	// Return the object
	VLikelihoodObject *i_fake = new VLikelihoodObject(*this);
	
	// Reclone the various pointers (otherwise i_fake points towards this->pointer...)
	// i_fake getting deleted will delete the pointers of this object
	fOnHistogram = (TH1D*)fOnHistogram->Clone();
	fOffHistogram = (TH1D*)fOffHistogram->Clone();
	fOnHistogramRebinned = (TH1D*)fOnHistogramRebinned->Clone();
	fOffHistogramRebinned = (TH1D*)fOffHistogramRebinned->Clone();
	fMeanEffectiveAreaMC = (TGraphAsymmErrors*)fMeanEffectiveAreaMC->Clone();
	fResponseMatrix = (TH2F*)fResponseMatrix->Clone();
	fResponseMatrixRebinned = (TH2F*)fResponseMatrixRebinned->Clone();

	// Reset the values
	fModel = i_current_model;
	fRunNumber = i_current_run;
	fMJD = i_current_mjd;
	fOnCounts = i_current_on;
	fOffCounts = i_current_off;

	return i_fake;
}


VLikelihoodObject * VLikelihoodObject::clone(){
	VLikelihoodObject *i_clone = new VLikelihoodObject(*this);
	return i_clone;
}


int VLikelihoodObject::getNBins(){
	int i_nBins = 0;
	for (unsigned int i = 0; i < fEnergyBinCentres.size(); i++){
		// Fit min/max
		if( (fEnergyBinCentres[i] < fEnergyMin) || (fEnergyBinCentres[i] > fEnergyMax))  
		{
			continue;
		}
		i_nBins += 1;
	}
	return i_nBins;
}