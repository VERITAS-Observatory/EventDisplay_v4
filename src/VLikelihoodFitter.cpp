/*! class VLikelihoodFitter
    fit and plot spectral data using likelihood methods
    based on Piron et al 2001 astro-ph/0106196
*/


#include "VLikelihoodFitter.h"


// Initialization function
// Extract the raw data from anasum file
bool VLikelihoodFitter::initialize()
{

    // Getting the Effective Areas

    cout << "Getting MC Effective Areas" << endl;
    fMeanEffectiveAreaMC = getEffectiveAreasMCFromFile();

    // Checking effective areas returned ok
    if ( !fMeanEffectiveAreaMC[0] )
    {
        cout << "VLikelihoodFitter::initialize Error getting effective areas" << endl;
        return false;
    }

    // Energy binning
    // Setting bins over 3 decades
    int i_fNEnergyBins = int(3/fEnergyBinWidth);

    // Obtaining the energy bins
    vector <double> i_fEnergyBins;
    for (int i =0; i <= i_fNEnergyBins; i ++)
    {
        i_fEnergyBins.push_back(-1.1 + i*fEnergyBinWidth);
    }

    // Setting energy binning
    if ( !setAnalysisBinning(i_fNEnergyBins, i_fEnergyBins) )
    {
        cout << "VLikelihoodFitter::initialize errors setting analysis bins, see above." << endl;
        return false;
    }

    return true;

}


// Setting the log width of the bin
void VLikelihoodFitter::setBinWidth( double i_BinWidth )
{

    // Catching negative bins
    if (i_BinWidth <= 0 )
    {
        cout << "VLikelihoodFitter::setBinWidth Negative bin width not physics...\n"
             << "\t\tDefaulting to 0.2" << endl;
        fEnergyBinWidth = 0.2;
    }

    else
    {
        fEnergyBinWidth = i_BinWidth;
    }
}


// Setting the binning for the energy bins
bool VLikelihoodFitter::setAnalysisBinning(int i_fNEnergyBins, vector <double> i_fEnergyBins)
{

    string hname;
    // Number of bins
    fNEnergyBins = i_fNEnergyBins;
    // Bin edges
    fEnergyBins = i_fEnergyBins;
    // Bin Centres
    fEnergyBinCentres.clear();

    // Bias of an energy bin
    fEnergyBias.clear();

    // Getting bin centres
    for (int i = 0; i < fNEnergyBins; i++)
    {
        fEnergyBinCentres.push_back( fEnergyBins[i] + 0.5*(fEnergyBins[i+1] - fEnergyBins[i] ) );
    }


    // Reset counting data vectors
    resetCountingHistograms();

    // Get Raw Histograms
    vector <TH1D*> i_hOnRaw = getCountingHistogramRaw("on");
    vector <TH1D*> i_hOffRaw = getCountingHistogramRaw("off");

    // Checking that both sets of histograms returned ok
    if ( !i_hOnRaw[0] || !i_hOffRaw[0] )
    {
        cout << "VLikelihoodFitter::setAnalysisBinning error getting On/Off histograms" << endl;
        return false;
    }

    vector <TH2F*> i_hResponseMatrixRaw = getResponseMatrixRaw();
    // Checking if response matrix returned ok
    if ( !i_hResponseMatrixRaw[0] )
    {
        cout << "VLikelihoodFitter::setAnalysisBinning error getting response matrix" << endl;
        return false;
    }

    // Bias vector
    vector <double> i_RunBias;
    std::ostringstream ss;

    // Looping over each run
    for( unsigned int i = 0; i < i_hOnRaw.size(); i++ )
    {

        // Rebinning Off counts
        ss.str(std::string());
        ss << "Rebinned On Counts " << fRunList[i].runnumber;
        fOnRebinnedHistograms.push_back( (TH1D*) i_hOnRaw[i]->Rebin(fNEnergyBins, ss.str().c_str(), &(fEnergyBins)[0]));

        // Rebinning Off counts
        ss.str(std::string());
        ss << "Rebinned Off Counts " << fRunList[i].runnumber;
        fOffRebinnedHistograms.push_back((TH1D*) i_hOffRaw[i]->Rebin(fNEnergyBins, ss.str().c_str(),&(fEnergyBins)[0]));

        // Calcualating totals
        fTotalOn.push_back(fOnRebinnedHistograms[i]->Integral(1, fNEnergyBins));
        fTotalOff.push_back(fOffRebinnedHistograms[i]->Integral(1, fNEnergyBins));

        // Getting final data bin
        fLastOn.push_back(fOnRebinnedHistograms[i]->GetBinCenter(fOnRebinnedHistograms[i]->FindLastBinAbove(0.9)));
        fLastOff.push_back(fOffRebinnedHistograms[i]->GetBinCenter(fOffRebinnedHistograms[i]->FindLastBinAbove(0.9)));

        // Response Matrix
        // Define empty TH2F with desired analysis binning
        // Histogram is then filled from raw histograms
        ss.str(std::string());
        ss << "RebinnedResponseMatrix_" << fRunList[i].runnumber;
        TH2F* i_htmp2D = new TH2F( ss.str().c_str(), ss.str().c_str(), fNEnergyBins, &(fEnergyBins)[0], fNEnergyBins, &(fEnergyBins)[0] );

        TAxis *i_xAxis = i_hResponseMatrixRaw[i]->GetXaxis();
        TAxis *i_yAxis = i_hResponseMatrixRaw[i]->GetYaxis();

        // Looping over and filling histogram
        for (int j = 1; j <= i_xAxis->GetNbins(); j++)
        {
            for (int k = 1; k <= i_yAxis->GetNbins() ; k++)
            {
                i_htmp2D->Fill(i_xAxis->GetBinCenter(j), i_yAxis->GetBinCenter(k),  i_hResponseMatrixRaw[i]->GetBinContent(j,k) );
            }
        }

        // Normalizing in Y direction (sum MC = 1)
        VHistogramUtilities::normalizeTH2D_y(i_htmp2D);


        // Determining Bias
        // Bias is obtained by applying a gaussian fit to each y-slice

        for (int j = 1; j <= i_htmp2D->GetYaxis()->GetNbins(); j++)
        {
            TF1 *i_fFit = new TF1("i_fFit", "gaus", -2,2.5);
            i_fFit->SetParameter(0, 0.6 );
            i_fFit->SetParameter(1, i_htmp2D->GetYaxis()->GetBinCenter(j) );
            i_fFit->SetParameter(2, 0.2 );

            TH1D *i_slice = (TH1D*)i_htmp2D->ProjectionX("i_slice", j,j);

            // Skipping empty bins
            if(i_slice->GetEntries() == 0)
            {
                i_RunBias.push_back(1.0);
                continue;
            }

            // Fitting a gaussian quietly
            // ROOT 6 seems to fail unless likelihood (L) fit is applied...
            i_slice->Fit("i_fFit","0Lq");

            // Bias defined as:
            // mu = Gaussian_mean (from fit)
            // E_true = Expected Energy (E_MC)
            // Bias = abs( ( mu - E_true ) / E_true )
            double tmp = TMath::Power( 10.,i_fFit->GetParameter(1) ) - TMath::Power( 10.,i_htmp2D->GetYaxis()->GetBinCenter(j) ) ;
            tmp /= TMath::Power(10.,i_htmp2D->GetYaxis()->GetBinCenter(j));

            // Saving to a vector
            i_RunBias.push_back(abs(tmp));

        }

        // Setting histogram titles
        i_htmp2D->GetXaxis()->SetTitle("Energy_{rec} [TeV]");
        i_htmp2D->GetYaxis()->SetTitle("Energy_{mc} [TeV]");

        // Storing the response matrix for this run
        fResponseMatrixRebinned.push_back(i_htmp2D);
        // Storing the bias of each energy bin for this run
        fEnergyBias.push_back(i_RunBias);

    }

    // Getting counts from histograms
    fOffCounts = getCounts(fOffRebinnedHistograms);
    fOnCounts = getCounts(fOnRebinnedHistograms);


    return true;
}



// Getting Vector of effective areas
// returns:
// 	  vector of MC energy effective areas
vector <TGraphAsymmErrors*> VLikelihoodFitter::getEffectiveAreasMCFromFile()
{

    string hname;
    vector <TGraphAsymmErrors*> iVtemp ;
    std::ostringstream ss;

    // Making sure we have a runlist
    if (fRunList.size() == 0)
    {
        cout << "VLikelihoodFitter::getEffectiveAreasMCFromFile Error getting runlist " << endl;
        iVtemp.clear();
        iVtemp.push_back(0);
        return iVtemp;
    }

    for( unsigned int i = 0; i < fRunList.size(); i++ )
    {

        TGraphAsymmErrors* i_gMeanEffectiveArea_on = 0;
        TGraphAsymmErrors* i_gMeanEffectiveArea_off = 0;
        TGraphAsymmErrors* i_gMeanEffectiveArea = 0;

        hname = "gMeanEffectiveAreaMC_on";
        // Using VEnergySpectrum's code to get effective areas
        i_gMeanEffectiveArea_on = ( TGraphAsymmErrors* )getHistogram( hname.c_str(), fRunList[i].runnumber, "EffectiveAreas", -9999 )->Clone();
        bool i_onValid = isPointerValid(i_gMeanEffectiveArea_on);


        hname = "gMeanEffectiveAreaMC_off";
        i_gMeanEffectiveArea_off = ( TGraphAsymmErrors* )getHistogram( hname.c_str(), fRunList[i].runnumber, "EffectiveAreas", -9999 )->Clone();
        bool i_offValid = isPointerValid(i_gMeanEffectiveArea_off);

        // Trying to get On effective area MC
        if (i_onValid)
        {
          i_gMeanEffectiveArea = (TGraphAsymmErrors*)i_gMeanEffectiveArea_on->Clone();
        }

        // Trying to get Off effective area MC
        else if (i_offValid)
        {

          cout << "On MC Effective Area failed. Using Off MC effective Area (Run: " <<  fRunList[i].runnumber << ")" << endl;
          i_gMeanEffectiveArea = (TGraphAsymmErrors*)i_gMeanEffectiveArea_off->Clone();
        }

        // Cleaning up
        delete i_gMeanEffectiveArea_on;
        delete i_gMeanEffectiveArea_off;




        // Checking if there are any issues
        if (!i_gMeanEffectiveArea)
        {
            cout << "VLikelihoodFitter::getEffectiveAreasMCFromFile Error getting Effective Area ( MC ) ";

            cout << " for run " << fRunList[i].runnumber << endl;
            iVtemp.clear();
            iVtemp.push_back(0);
            return iVtemp;
            // return 0;
        }

        // Renaming
        ss.str(std::string());
        ss << "gMeanEffectiveAreaMC_" << fRunList[i].runnumber;


        i_gMeanEffectiveArea->SetTitle( ss.str().c_str());

        // Save the runwise effective area to a vector
        iVtemp.push_back((TGraphAsymmErrors*)i_gMeanEffectiveArea->Clone());
    }

    return iVtemp;

}



// Getting Vector of counts from anasum file
// returns:
// onoff = "on", on events
// onoff = "off", off events

vector <TH1D*> VLikelihoodFitter::getCountingHistogramRaw(string onoff)
{
    std::ostringstream ss;
    vector <TH1D*> iVTemp;

    // Looping over runs
    for( unsigned int i = 0; i < fRunList.size(); i++ )
    {

        // Getting histograms from anasum file
        ss.str(std::string());
        ss << "herecCounts_" << onoff;
        TH1D* i_hErecCounts = ( TH1D* )getHistogram( ss.str().c_str(), fRunList[i].runnumber, "energyHistograms", -9999 );

        // Checking if histograms exist
        if (!i_hErecCounts)
        {
            cout << "VLikelihoodFitter::getCountingHistogramRaw Error getting " << onoff <<" counts histrogram for run " << fRunList[i].runnumber ;
            iVTemp.clear();
            iVTemp.push_back(0);
            return iVTemp;

        }
        // Renaming
        ss.str(std::string());
        ss << "herecCounts_" << onoff << "_" << fRunList[i].runnumber << endl;
        i_hErecCounts->SetTitle( ss.str().c_str());

        iVTemp.push_back(i_hErecCounts);


    }
    return iVTemp;
}




/*
  Getting Vectors of raw response matrix
  Require^1 histograms binned to minimum expected bin size.
  By default the parameter files request 0.05.

  Use TGraph2D::Interpolate to go from:
  E_mc vs E_mc/E_rec -> E_rec vs E_mc

  ^1 only require if we want to use histograms rather than another interpolator
*/
vector <TH2F*> VLikelihoodFitter::getResponseMatrixRaw()
{

    vector <TH2F*>iVtemp ;
    string hname;
    std::ostringstream ss;

    // Looping over runs
    for( unsigned int i = 0; i < fRunList.size(); i++ )
    {
        // Getting response matrix from .anasum.root file
        hname =  "hResponseMatrix_on";
        TH2F* i_hResponseMatrix_on = ( TH2F* )getHistogram( hname.c_str(), fRunList[i].runnumber, "EffectiveAreas", -9999 )->Clone();
        hname =  "hResponseMatrix_off";
        TH2F* i_hResponseMatrix_off = ( TH2F* )getHistogram( hname.c_str(), fRunList[i].runnumber, "EffectiveAreas", -9999 )->Clone();


        // Renaming
        ss.str(std::string());
        ss << "hResponseMatrix_Interpolated_" << fRunList[i].runnumber;

        bool i_onValid = isPointerValid(i_hResponseMatrix_on);
        bool i_offValid = isPointerValid(i_hResponseMatrix_off);


        // Reshape to required dimensions
        // Bins should span -1, 2
        int i_fNEnergyBins = int(3/0.05);
        int i_fNEnergyBinsMC = int(3/0.05);

        // Obtaining the energy bins
        vector <double> i_fEnergyBins;

        for (int i =0; i <= i_fNEnergyBins; i ++)
        {
            i_fEnergyBins.push_back(-1.1 + i*0.05);
        }


        vector <double> i_fEnergyBinsMC;

        for (int i =0; i <= i_fNEnergyBinsMC; i ++)
        {
            i_fEnergyBinsMC.push_back(-1.1 + i*0.05);
        }

        TH2F* ihres_tmp = new TH2F ("ihres_tmp", "Response Matrix; E_{Rec}; E_{MC}", i_fNEnergyBins, &(i_fEnergyBins[0]), i_fNEnergyBinsMC, &(i_fEnergyBinsMC[0]) );
        float ieng_rc = 0;
        float ieng_mc = 0;
	float ebias = 0;

	TGraph2D *igSys2d = 0;

	/*
	  On/Off check is required
	  when 0 on/off counts the on/off effective areas/response matrices aren't filled.
	*/
	if ( i_onValid )
        {
	  // For interpolating
          igSys2d = new TGraph2D (i_hResponseMatrix_on);
        }
        else if ( i_offValid )
        {
	  // For interpolating                                                                                                                                                                                    
          igSys2d = new TGraph2D (i_hResponseMatrix_off); 
        }
        else
        {
          cout << "VLikelihoodFitter::getResponseMatrixRaw error getting response matrix for run " << fRunList[i].runnumber << " On:" << i_onValid << " Off:" << i_offValid <<  endl;
          iVtemp.clear();
          iVtemp.push_back(0);
	  continue;
        }
	
	for (int ierec = 0; ierec < ihres_tmp->GetXaxis()->GetNbins(); ierec++)
          {
            ieng_rc = ihres_tmp->GetXaxis()->GetBinCenter(ierec+1);
            for (int jemc = 0; jemc < ihres_tmp->GetYaxis()->GetNbins(); jemc++)
            {
              ieng_mc = ihres_tmp->GetYaxis()->GetBinCenter(jemc+1);
              ebias = TMath::Power(10, ieng_rc) / TMath::Power(10, ieng_mc);

              float intep = igSys2d->Interpolate(ieng_mc, ebias);
              ihres_tmp->SetBinContent(ierec+1, jemc+1, intep);
            }
          }
	  // expect intergral over E_mc (Y) to be 1
          VHistogramUtilities::normalizeTH2D_y(ihres_tmp);
          ihres_tmp->SetTitle( ss.str().c_str());
          iVtemp.push_back((TH2F*)ihres_tmp->Clone());
	
    }

    return iVtemp;

}




// Printing some info about the runs
void VLikelihoodFitter::printRunInfo()
{

    double i_total_tOn = 0;
    double i_total_On = 0;
    double i_total_Off = 0;

    cout << "Entry #\tRun\tMJD\tLivetime\tTotal ON\tTotal Off\tAlpha\tLast On Count\tLast Off Count\n";
    for( unsigned int i = 0; i < fRunList.size(); i++ )
    {
        cout << i << "\t" << fRunList[i].runnumber << "\t" << fRunList[i].MJD << "\t" << fRunList[i].tOn * fRunList[i].deadTimeFraction <<
             "\t\t" << fTotalOn[i] << "\t\t" << fTotalOff[i] << "\t\t" << fRunList[i].alpha  <<
             "\t\t" << TMath::Power(10,fLastOn[i]) << "\t\t" << TMath::Power(10,fLastOff[i]) <<  endl;

        // Adding totals
        i_total_tOn += fRunList[i].tOn * fRunList[i].deadTimeFraction;
        i_total_On += fTotalOn[i];
        i_total_Off += fTotalOff[i];
    }
    cout << "Total\t\t" << i_total_tOn << "\t\t" << i_total_On <<"\t\t" << i_total_Off << endl;
}



// Generic function to get counts from the histograms
vector < vector <double> > VLikelihoodFitter::getCounts(vector <TH1D*> i_hTemp)
{
    vector < vector <double> > i_vTemp;

    // Looping over the number of runs
    for (unsigned int i = 0; i < i_hTemp.size(); i++)
    {
        vector <double> i_vRunCounts;
        // Looping over the number of bins in each run
        for (int j = 0; j < i_hTemp[i]->GetXaxis()->GetNbins(); j++ )
        {
            // Checking for any funky stuff going on
            // Shouldn't happen....
            if (i_hTemp[i]->GetBinContent(j+1) > 1e9)
            {
                cout << "VLikelihoodFitter::getCounts Error in Run " << fRunList[i].runnumber << ", Bin " << j << endl;
                i_vRunCounts.push_back( 1.e-9 );
                continue;
            }

            // Adding a small number to advoid log(0) errors
            i_vRunCounts.push_back( i_hTemp[i]->GetBinContent(j+1) + 1.e-9 );
        }

        i_vTemp.push_back( i_vRunCounts );
    }

    return i_vTemp;
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
// to what was set by VLikelihoodFitter::setNormalisationEnergyLinear

void VLikelihoodFitter::setModel(int i_ID, double ifENorm )
{
    std::ostringstream ss;

    // Defaulting to what ever was set by VLikelihoodFitter::setNormalisationEnergyLinear
    if (ifENorm == 0)
    {
        ifENorm = fENorm;
    }

    // Note for spectral weighting of bins, the spectral index must be the 2nd parameter
    // i.e. [1] = Spectral index
    if (i_ID == 0)
    {
        ss.str(std::string());
        ss << "[0]*TMath::Power(TMath::Power(10.0,x) /" << ifENorm << ", [1])";
        fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
        fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

        ss.str(std::string());
        ss << "[0]*TMath::Power(x / " << ifENorm << ", [1])";
        fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
        fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));


        fModelID = i_ID;
        fNParms = 2;
    }

    // Power Law with Exponential Cut off
    else  if (i_ID == 1)
    {
        ss.str(std::string());
        ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( 10, x )  / [2] )";
        fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
        fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

        ss.str(std::string());
        ss << "[0] * TMath::Power( x  / " << ifENorm << ", [1] ) * TMath::Exp( -1. *  x / [2] )";
        fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
        fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));

        fModelID = i_ID;
        fNParms = 3;
    }


    // Curved Spectrum
    else if (i_ID == 2)
    {
        ss.str(std::string());
        ss << "[0] * TMath::Power( TMath::Power( 10, x )/ " << ifENorm << ", [1]+[2]*TMath::Power( 10, x ) )";
        fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
        fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

        ss.str(std::string());
        ss << "[0] * TMath::Power(  x / " << ifENorm << ", [1]+[2]* x  )";
        fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
        fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
        fModelID = i_ID;
        fNParms = 3;

    }

    // Log-parabola
    else if (i_ID == 3)
    {
        ss.str(std::string());
        ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << " , [1] + [2]*TMath::Log( TMath::Power( 10, x ) / " << ifENorm <<" ) )";
        fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
        fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

        ss.str(std::string());
        ss <<  "[0] * TMath::Power(  x  / " << ifENorm << " , [1]+[2]*TMath::Log( x / " << ifENorm << " ) )";
        fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
        fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));

        fModelID = i_ID;
        fNParms = 3;

    }


    // Power Law with Exponential Cut off
    // Energy-bin wise fit function
    else  if (i_ID == 11)
    {
        ss.str(std::string());
        ss << "[0] * TMath::Exp( " << ifENorm << "  / [2] ) * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( 10, x )  / [2] )";
        fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
        fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

        ss.str(std::string());
        ss << "[0] * TMath::Exp( " << ifENorm << "  / [2] ) * TMath::Power( x  / " << ifENorm << ", [1] ) * TMath::Exp( -1. *  x / [2] )";
        fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
        fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));

        // fModelID = i_ID;
        fNParms = 3;
    }

    // Log-parabola with exp cutoff
    else if (i_ID == 4)
    {
       ss.str(std::string());
       ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << " , [1] + [2]*TMath::Log( TMath::Power( 10, x ) / " << ifENorm <<" ) ) * TMath::Exp( -1. * TMath::Power( 10, x )  / [3] )";
       fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
       fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

       ss.str(std::string());
       ss <<  "[0] * TMath::Power(  x  / " << ifENorm << " , [1]+[2]*TMath::Log( x / " << ifENorm << " ) )* TMath::Exp( -1. *  x   / [3] )";
       fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
       fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));

       fModelID = i_ID;
       fNParms = 4;

   }

   // Super Exponentialy cut-off power law
   else if (i_ID == 5)
   {
       ss.str(std::string());
       ss << "[0] * TMath::Power( TMath::Power( 10, x ) / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( TMath::Power( 10, x )  / [2] , [3] ))";
       fModel = new TF1("fModel", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);
       fModel_intrinsic = new TF1("fModel_intrinsic", ss.str().c_str(), fFitMin_logTeV, fFitMax_logTeV);

       ss.str(std::string());
       ss << "[0] * TMath::Power( x  / " << ifENorm << ", [1] ) * TMath::Exp( -1. * TMath::Power( x / [2], [3] ) )";
       fModel_linear = new TF1("fModel", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));
       fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", ss.str().c_str(), TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));

       fModelID = i_ID;
       fNParms = 4;

   }


   // Broken Power Law
   // Need something for energy bin wise....
   // Default to a power law?
   // set gamma1/2 based on energy!
   else if (i_ID == 6)
   {

       fNParms = 4;
       fModel = new TF1("fModel",this, &VLikelihoodFitter::brokenPowerLaw, fFitMin_logTeV, fFitMax_logTeV, fNParms+1);
       fModel_intrinsic = new TF1("fModel_intrinsic", this, &VLikelihoodFitter::brokenPowerLaw, fFitMin_logTeV, fFitMax_logTeV, fNParms+1);



       fModel_linear = new TF1("fModel_linear",this, &VLikelihoodFitter::brokenPowerLaw, TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV), fNParms+1);
       fModel_intrinsic_linear = new TF1("fModel_intrinsic_linear", this, &VLikelihoodFitter::brokenPowerLaw, TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV), fNParms+1);


       // Setting the break energy
       fModel->SetParameter(3, ifENorm);
       fModel_intrinsic->SetParameter(3, ifENorm);
       fModel_linear->SetParameter(3, ifENorm);
       fModel_intrinsic_linear->SetParameter(3, ifENorm);

       // Fixing Linear/Log
       fModel->FixParameter(4,0.);
       fModel_intrinsic->FixParameter(4,0.);
       fModel_linear->FixParameter(4, 1.);
       fModel_intrinsic_linear->FixParameter(4, 1.);

       fModelID = i_ID;

    }


    // Catching all others and defaulting to a power law
    else
    {
        cout << "VLikelihoodFitter::setModel: Model " << i_ID << " not found...Add it to the collection!\n"
             << "\t\tDefaulting to a Power Law Model (0) " << endl;

        setModel(0, ifENorm );
    }

    // EBL Corrected Model
    // This returns fModel_intrinsic * exp(-tau * E)
    if (fEBLAnalysis == true)
    {

      fModel = new TF1("fModel",this, &VLikelihoodFitter::calculateIntrinsicSpectrum, fFitMin_logTeV, fFitMax_logTeV, fNParms);
      // fModel_linear = new TF1("fModel", "fModel.Eval(TMath::Log10(x))", TMath::Power(10, fFitMin_logTeV), TMath::Power(10, fFitMax_logTeV));

      // fModel_linear = new TF1("fModel",this, &VLikelihoodFitter::calculateIntrinsicSpectrum, fFitMin_logTeV, fFitMax_logTeV, fNParms);

    }

}

/*
* Getting model predicted excess counts
* Defined as:
* S_predicted = T_live \int_(rec energy bin) dE_rec \int dNdE * A_eff * Gamma(E_MC->E_Rec) dE_MC
* This gets the number of counts predicted in each energy bin for each run based the input model.
*/
vector < vector <double> >  VLikelihoodFitter::getModelPredictedExcess( vector <double> iParms)
{


    vector < vector <double> > i_vModel;
    vector <double> iSpectralWeightedCentres;

    // This should only happen upon using inputted parameters
    if ( iParms.size() != fNParms )
    {
        cout << "VLikelihoodFitter::getModelPredictedExcess Error Insufficient number of parameter!"
             << "\n\t\tRequested: " << iParms.size()
             << "\n\t\tExpected: " << fNParms << endl;

        iSpectralWeightedCentres.push_back(0);
        i_vModel.push_back(iSpectralWeightedCentres);
        return i_vModel;
    }

    // setting parameters
    for (unsigned int i = 0; i < fNParms; i++)
    {
        fModel->SetParameter(i, iParms[i]);
    }

    // Getting Spectrally weighted bin centres
    for (int i = 0; i < fNEnergyBins; i++)
    {
        // Spectral weighted bin centres assuming iParms[1] is spectral index
        iSpectralWeightedCentres.push_back( VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyBins[i], fEnergyBins[i+1], iParms[1]) );
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

    // Looping over each run
    for (unsigned int i = 0; i < fRunList.size(); i++)
    {

        TAxis *xpoints = fResponseMatrixRebinned[i]->GetXaxis();
        TAxis *ypoints = fResponseMatrixRebinned[i]->GetYaxis();
        vector <double> i_vTmp;


        // Looping over each Rec Energy bin
        for (int j = 0; j < fNEnergyBins; j++)
        {

            i_vTmp.push_back(0);
            // Fining bin index of current energy bin
            i_CurrentRecBin = xpoints->FindBin( iSpectralWeightedCentres[j] );
            // i_CurrentRecBin = xpoints->FindBin( fEnergyBinCentres[j] );


            // Looping over each MC Energy bin
            for (int l = 0; l < fNEnergyBins; l++)
            {
                // Fining bin index of current energy bin
                i_CurrentMCBin = ypoints->FindBin(fEnergyBinCentres[l]);

                // Only including points with a sensible bias
                if (fEnergyBias[i][l] > fThresholdBias) {
                    continue;
                }

                // dN/dE at E_MC
                i_ModelElement = fModel->Eval( iSpectralWeightedCentres[l] );

                // Eff at E_MC
                i_EffectiveAreaElement = fMeanEffectiveAreaMC[i]->Eval( iSpectralWeightedCentres[l] );

                // Response Matrix at (E_Rec,E_MC)
                i_ReconstructionMatrixElement = fResponseMatrixRebinned[i]->GetBinContent(i_CurrentRecBin, i_CurrentMCBin);
                if (i_ReconstructionMatrixElement < 1.e-5 )
                {
                    i_ReconstructionMatrixElement = 0;
                }

                // Dead time corrected exposure
                i_TOn = fRunList[i].tOn * fRunList[i].deadTimeFraction  ;
                // i_TOn = fRunList[i].tOn ;

                // dE
                i_EnergyElement = pow(10.0,fEnergyBins[l+1]) - pow(10.0,fEnergyBins[l]);

                // Summing over MC
                i_vTmp[j] +=   i_ModelElement * i_EffectiveAreaElement * i_ReconstructionMatrixElement * i_ConversionElement *  i_TOn  * i_EnergyElement ;
                // cout << j << " " <<   i_ModelElement << " " <<  i_EffectiveAreaElement << " " <<  i_ReconstructionMatrixElement << " "
                // <<  i_ConversionElement << " " <<   i_TOn  << " " <<  i_EnergyElement  << endl	;
            }
        }

        // Obtaining runwise counts
        i_vModel.push_back(i_vTmp);

    }
    return i_vModel;
}



/*
* Getting model predicted off counts
* Model predicted off counts are obtained by optimizing the likelihood equation
* with respect to the (poissonian) mean off counts.
* This is based on the input source spectral model and the observed counts.
*/
vector < vector <double> >  VLikelihoodFitter::getModelPredictedOff(vector <double> iParms)
{

    vector < vector <double> > i_OffMLE;

    // This should only happen upon using inputted parameters
    if ( iParms.size() != fNParms )
    {
        cout << "VLikelihoodFitter::getModelPredictedOff Error Insufficient number of parameter!"
             << "\n\t\tRequested: " << iParms.size()
             << "\n\t\tExpected: " << fNParms << endl;

        vector <double> i_vTmp(1);
        i_OffMLE.push_back(i_vTmp);
        return i_OffMLE;
    }


    vector < vector <double> > i_myModel = getModelPredictedExcess(iParms);

    // looping over each run
    for (unsigned int i = 0; i < fRunList.size(); i++)
    {
        vector <double> i_vTmp;
        // Looping over each bin
        for (unsigned int j = 0; j < i_myModel[i].size(); j++)
        {

            double i_a = 0;
            double i_b = 0;

            // alpha * (on + off) - (alpha + 1)*S_predicted

            i_a = fRunList[i].alpha *( fOnCounts[i][j] + fOffCounts[i][j] ) - (fRunList[i].alpha  + 1)*i_myModel[i][j];

            // 1/(2 * alpha *( alpha +1 ) ) * i_a *sqrt(i_a^2 + 4 * alpha *(alpha +1)*off*S_predicted)
            i_b = 1.0/(2.0*fRunList[i].alpha *(fRunList[i].alpha  + 1.0));
            i_b *= (i_a + TMath::Sqrt(TMath::Power(i_a,2.0) +4.0 * fRunList[i].alpha * (fRunList[i].alpha + 1.0) * fOffCounts[i][j]*i_myModel[i][j] ));

            // Energy bin
            i_vTmp.push_back(i_b);
        }
        // runwise
        i_OffMLE.push_back(i_vTmp);
    }
    return i_OffMLE;
}


/*
* Function to optimise the likelihood function
* Returns a TF1* with the best fit parameters
* bContours decides whether 1 sigma contours are obtained (this is a slow process)
*/
TF1* VLikelihoodFitter::getLikelihoodFit( bool bContours )
{

    // initialize the minimizer
    initializeMinimizer();

    // Keeping a global copy of the best fit parameters
    if (fGlobalBestFitParameters)
    {
        delete fGlobalBestFitParameters;
    }

    // do the minimization
    fMinimizer->Minimize();
    // Get Best fit values
    const double *xs = fMinimizer->X();
    // Not needed when calling MINOS
    // fMinimizer->Hesse();

    fFitStatus =  fMinimizer->Status();
    if ( fMinimizer->Status() != 0 )
    {
        cout << "VLikelihoodFitter::getLikelihoodFit Warning fit status is not valid. Proceed with caution!" << endl;
    }
    // Getting the symmetric errors
    const double *i_Errors = fMinimizer->Errors();


    // Setting Global best fit parameters
    fGlobalBestFitParameters = new double[fNParms];
    for (unsigned int i = 0; i < fNParms; i++)
    {
        fGlobalBestFitParameters[i] = xs[i];
    }


    // Getting 1 sigma contour
    // This is slow. Only done if requested
    if (bContours)
    {
        std::ostringstream ss;
        for (unsigned int i = 0 ; i < fNParms; i++ )
        {


            for (unsigned int j = 0 ; j < fNParms; j++)
            {
                if (i == j) {
                    continue;
                }

                // Clearing old data
                fIContours.clear();
                fJContours.clear();
                fIContours.assign(fNContours, 0);
                fJContours.assign(fNContours, 0);

                // Getting contours of i,j parameters
                fMinimizer->Contour(i, j, fNContours, &(fIContours)[0], &(fJContours)[0] );

                // Storing contours in a 2 X fNConours vector
                vector < vector <double> > contours;

                contours.push_back(fIContours);
                contours.push_back(fJContours);

                // Storing pair in a map with key "i,j"
                // This can be accessed later
                ss.str(std::string());
                ss << i << "," << j;
                fContourMap[ss.str()] = contours;

            }
        }
    }


    // Saving best fit parameters (linear scale)
    fModel_linear->SetParameters(xs);
    fModel_linear->SetParErrors(i_Errors);

    // Cloning a copy to be returned
    TF1 *i_BestFit = 0;
    if (fEBLAnalysis)
    {
      fModel_intrinsic_linear->SetParameters(xs);
      fModel_intrinsic_linear->SetParErrors(i_Errors);
      i_BestFit = (TF1*)fModel_intrinsic_linear->Clone();
    }
    else
    {
      i_BestFit = (TF1*)fModel_linear->Clone();
    }

    // Upper and lower error values
    vector <double> parm_errorUp;
    vector <double> parm_errorLow;
    vector <bool> parm_status;

    // Getting Minos Errors
    vector <double> i_vec(fNParms);

    for (unsigned int i = 0; i < fNParms; i++)
    {
        i_vec[i] = xs[i] ;
        double i_err_low = 0;
        double i_err_up = 0;

        // Getting minos (asymmetric) errors (true if error is valid)
        bool bErrors = fMinimizer->GetMinosError(i, i_err_low, i_err_up);

        parm_errorLow.push_back(i_err_low);
        parm_errorUp.push_back(i_err_up);
        parm_status.push_back(bErrors);


        cout << scientific << "Variabile " << i << " Error Status: " << bErrors << ", E_Low = " << i_err_low << ", E_Up = " << i_err_up << endl;
    }


    // Saving best fit parameters to fModel

    fModel->SetParameters(xs);
    fModel->SetParErrors(i_Errors);


    // Printing the fit details
    cout << "Binned Likelihood Fit: \n" ;
    cout << "Parameter \t Best Fit \t ErrorL \t ErrorU \t IsMin\n";
    for ( unsigned int i = 0; i < fNParms; i++)
    {
        cout << scientific << "[" << i << "]\t\t" << " " << fMinimizer->VariableName(i) << " \t\t" << fModel->GetParameter(i)
             << "\t\t" << parm_errorLow[i] << "\t\t"  << parm_errorUp[i] << "\t\t" <<  parm_status[i] << endl;
    }
    cout << "E_Norm: " << fENorm << endl << endl;


    // Getting covarance matrix
    cout << "\n\nPrinting Covariance Matrix\n";
    double *i_covmat = new double [fNParms*fNParms];

    for ( unsigned int i =0; i < fNParms; i++)
    {
        for ( unsigned int j = 0; j < fNParms; j++)
        {
            i_covmat[ i * fNParms + j ] = fMinimizer->CovMatrix(i,j);
            cout << scientific << fMinimizer->CovMatrix(i,j) << "\t" ;
        }
        cout << endl;
    }

    cout << "\n";

    // Getting Chi^2
    cout << "Calculating Total Chi^2\n";
    double i_chi2 = getChi2 ( i_vec );
    double i_ndf =  fNBinsFit_Total - fNParms;
    cout << "\n";

    // Saving Chi2 and NDF
    fModel_linear->SetChisquare(i_chi2);
    fModel_linear->SetNDF(i_ndf);
    i_BestFit->SetChisquare( i_chi2 );
    i_BestFit->SetNDF( i_ndf );

    // Getting the 1 sigma confidence interval
    if (fConfidenceInterval)
    {
        delete fConfidenceInterval;
    }

    fConfidenceInterval = calculateConfidenceInterval(i_covmat, fModel, fModelID, fNParms);
    

    // Calculating the model integrated flux
    // i_flux[0] = flux [photons/cm^2/s^1]
    // i_flux[1] = flux error [photons/cm^2/s^1]
    // i_flux[2] = flux [Crab]
    // i_flux[3] = flux error [Crab]

    if (fModelID != 4)
    {

      float *i_flux = getIntegralFlux(fFitMin_logTeV, fFitMax_logTeV, fModel, true);

      cout << "Integral Flux:\n";
      cout << "F (" << TMath::Power(10,fFitMin_logTeV) << " TeV < E < " << TMath::Power(10,fFitMax_logTeV) << ") = " << i_flux[0] << "+/-" << i_flux[1] << " [Photons/cm^2/s] \n";
      cout << "F (" << TMath::Power(10,fFitMin_logTeV) << " TeV < E < " << TMath::Power(10,fFitMax_logTeV) << ") = " << i_flux[2] << "+/-" << i_flux[3] << " [Crab] \n";
    }

    else
    {


      cout << "Integral Flux:\n";
      cout << "F (" << TMath::Power(10,fFitMin_logTeV) << " TeV < E < " << TMath::Power(10,fFitMax_logTeV) << ") = " << i_BestFit->GetParameter(0) << "+/-" << i_BestFit->GetParError(0) << " [Photons/cm^2/s] \n";
    }


    // Getting Decorrelation Energy
    double E_d = fENorm * TMath::Exp( fMinimizer->CovMatrix(0,1) / xs[0] / i_Errors[1] / i_Errors[1] );
    cout << "Printing Decorrelation Energy (Assuming a Power Law Model, consider reapplying the fit.):\nE_d : " << E_d << endl;


    return i_BestFit;
}





/*
 *  initalizing the minimizer to be used
*  iNormGuess - inital guess of the normalization.
 *  iPrintStatue - level of printing output
 *  iFixShape - Fix the spectral shape (used for getting spectral points)
 */
bool VLikelihoodFitter::initializeMinimizer( double iNormGuess, int iPrintStatus, bool iFixShape )
{

    // Create minimizer if it hasn't already been initialized
    if ( fMinimizer )
    {
        delete fMinimizer;
    }
    // Using Minuit2 and Minos
    // Use Minuit not Minuit2
    fMinimizer = ROOT::Math::Factory::CreateMinimizer("Minuit", "Minos");

    // // set tolerance , etc...
    fMinimizer->SetMaxFunctionCalls(10000); // for Minuit/Minuit2
    fMinimizer->SetMaxIterations(10000);  // for GSL
    fMinimizer->SetTolerance(0.01); // default talorance

    // Likelihood ratio test suggests
    // 2(log(l) - log(lmax)) ~ chi^2
    // therefore the erorrs are defined as 0.5
    fMinimizer->SetErrorDef(0.5);

    fMinimizer->SetPrintLevel(iPrintStatus);


    // Checking the a global fit has been applied before tring to fix the shape
    if (!fGlobalBestFitParameters && iFixShape )
    {
        cout << "VLikelihoodFitter::initializeMinimizer Global fit must be applied before getting binwise fit "
             << endl << "\t Call VLikelihoodFitter::getLikelihoodFit() first!" << endl;

        return false;
    }


    // Inital step size and variable estimate
    double *step = new double[fNParms];
    double *variable = new double[fNParms];

    // Vector of parameter names
    fParmName.clear();
    fParmName.assign(fNParms, "");

    // Deleting previous fit functions (if they exist)
    if (fFitfunction)
    {
        delete fFitfunction;
    }

    // Wrapping getLogL_internal and passing it to the Minimizer
    // Catching case of broken power law... Might not be needed?
    if (fModelID == 6)
    {
        fFitfunction = new ROOT::Math::Functor (this,&VLikelihoodFitter::getLogL_internal,fNParms+1);
        // Vector of parameter names
        fParmName.clear();
        fParmName.assign(fNParms+1, "");
    }
    else
    {
        fFitfunction = new ROOT::Math::Functor (this,&VLikelihoodFitter::getLogL_internal,fNParms);
        fParmName.clear();
        fParmName.assign(fNParms, "");
    }
    fMinimizer->SetFunction(*fFitfunction);


    // Setting inital parameters for each model

    // 0 - Power Law
    // 1 - Power Law with Exponential Cut off
    // 2 - Curved Power Law
    // 3 - Log Parabola
    // 4 - Log Parabola with Exponential Cut off
    // 5 - Supper Exponential Cut off Log Parabola
    // 6 - Broken Power Law
    if (fModelID == 0)
    {

        step[0] = 0.01*iNormGuess;
        step[1] = 0.01;
        variable[0] = iNormGuess;
        variable[1] = -2.5;
        fParmName[0] = "Norm";
        fParmName[1] = "Index";

        // Set the free variables to be minimized!
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(),fGlobalBestFitParameters[1], step[1], -10, 0);
        }
        else
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(), variable[1], step[1], -10, 0);
        }

    }

    // Power Law with exp cut off
    if (fModelID == 1)
    {
        step[0] = 0.01*iNormGuess;
        step[1] = 0.01;
        step[2] = 0.01;
        variable[0] = iNormGuess;
        variable[1] = -1.5;
        variable[2] = 1.0;
        fParmName[0] = "Norm";
        fParmName[1] = "Index";
        fParmName[2] = "E_CutOff";



        // Set the free variables to be minimized!
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
            fMinimizer->SetVariable(1, fParmName[1].c_str(),fGlobalBestFitParameters[1],  step[1]);
            fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
        }
        else
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1,fParmName[1].c_str(), variable[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(), variable[1], step[1], -10, 0);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(), variable[2], step[2], 1.e-9, 100);
            // fMinimizer->SetVariable(2,fParmName[2].c_str(), variable[2], step[2]);
        }

    }


    // Curved Power Law
    if (fModelID == 2)
    {
        step[0] = 0.01*iNormGuess;
        step[1] = 0.01;
        step[2] = 0.01;
        variable[0] = iNormGuess;
        variable[1] = -1.5;
        variable[2] = -0.01;
        fParmName[0] = "Norm";
        fParmName[1] = "Index";
        fParmName[2] = "Beta";

        // Set the free variables to be minimized!
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(),fGlobalBestFitParameters[2], step[2], -10, 0);

        }
        else
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
            // (unsigned int ivar, const string& name, double val, double step, double lower, double upper)
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(), variable[2], step[2], -10, 0);
        }
    }


    // Log Parabola
    if (fModelID == 3)
    {
        step[0] = 0.01*iNormGuess;
        step[1] = 0.001;
        step[2] = 0.001;
        variable[0] = iNormGuess;
        variable[1] = -1.5;
        variable[2] = -0.1;

        fParmName[0] = "Norm";
        fParmName[1] = "Alpha";
        fParmName[2] = "Beta";

        // Set the free variables to be minimized!
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(),fGlobalBestFitParameters[1], step[1], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(),fGlobalBestFitParameters[2], step[2], -10, 0);

        }
        else
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(), variable[1], step[1], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(), variable[2], step[2], -10, 0);

        }
    }


    // Log Parabola with exp cutoff
    else if (fModelID == 4)
    {
        step[0] = 0.01*iNormGuess;
        step[1] = 0.001;
        step[2] = 0.001;
        step[3] = 0.01;
        variable[0] = iNormGuess;
        variable[1] = -1.5;
        variable[2] = -0.1;
        variable[3] = 2.0;

        fParmName[0] = "Norm";
        fParmName[1] = "Alpha";
        fParmName[2] = "Beta";
        fParmName[3] = "E_Cutoff";

        // Set the free variables to be minimized!
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(),fGlobalBestFitParameters[1], step[1], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(),fGlobalBestFitParameters[2], step[2], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
            fMinimizer->SetLimitedVariable(3, fParmName[3].c_str(),fGlobalBestFitParameters[3], step[3], 0., 100.);

        }
        else
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(), variable[1], step[1], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(), variable[2], step[2], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
            fMinimizer->SetLimitedVariable(3, fParmName[3].c_str(), variable[3], step[3], 0., 100.);


        }
    }

    // Log Parabola with exp cutoff
    else if (fModelID == 5)
    {
        step[0] = 0.01*iNormGuess;
        step[1] = 0.01;
        step[2] = 0.01;
        step[3] = 0.01;
        variable[0] = iNormGuess;
        variable[1] = -1.5;
        variable[2] = 1.0;
        variable[3] = 0.0;

        fParmName[0] = "Norm";
        fParmName[1] = "Index";
        fParmName[2] = "E_CutOff";
        fParmName[3] = "Delta";

        // Set the free variables to be minimized!
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(),fGlobalBestFitParameters[1], step[1], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(),fGlobalBestFitParameters[2], step[2], 0, 100);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), fGlobalBestFitParameters[2], step[2]);
            fMinimizer->SetLimitedVariable(3, fParmName[3].c_str(),fGlobalBestFitParameters[3], step[3], -5., 5.);


        }
        else
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), variable[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(), variable[1], step[1], -10, 0);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(), variable[2], step[2], 0., 100.);
            // fMinimizer->SetVariable(2, fParmName[2].c_str(), variable[2], step[2]);
            fMinimizer->SetLimitedVariable(3, fParmName[3].c_str(), variable[3], step[3], -5., 5. );



        }
    }



    // Broken Power Law
    else if (fModelID == 6)
    {
        delete[] step;
        double *step = new double[fNParms+1];
        delete[] variable;
        double *variable = new double[fNParms+1];

        step[0] = 0.01*iNormGuess;
        step[1] = 0.01;
        step[2] = 0.01;
        step[3] = 0.01;
        step[4] = 0.01;

        variable[0] = iNormGuess;
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
        if (iFixShape)
        {
            fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
            // fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
            fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(),fGlobalBestFitParameters[1], step[1], -10, 0);
            fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(),fGlobalBestFitParameters[2], step[2], -10, 0);
            fMinimizer->SetLimitedVariable(3, fParmName[3].c_str(),fGlobalBestFitParameters[3], step[3], 0.1, 100);
            fMinimizer->SetLimitedVariable(4, fParmName[4].c_str(),0., step[4], -5., 5.);
            // fMinimizer->FixVariable(3);
            fMinimizer->FixVariable(4);

        }
        else
        {
          fMinimizer->SetLimitedVariable(0, fParmName[0].c_str(),variable[0], step[0], 0, 1.E-5);
          // fMinimizer->SetVariable(1, fParmName[1].c_str(), fGlobalBestFitParameters[1], step[1]);
          fMinimizer->SetLimitedVariable(1, fParmName[1].c_str(),variable[1], step[1], -10, 0);
          fMinimizer->SetLimitedVariable(2, fParmName[2].c_str(),variable[2], step[2], -10, 0);
          fMinimizer->SetLimitedVariable(3, fParmName[3].c_str(),variable[3], step[3], 0, 100);
          fMinimizer->SetLimitedVariable(4, fParmName[4].c_str(),0., step[4], -5., 5.);

          // // Always fix E_break? no!
          // fMinimizer->FixVariable(3);
          fMinimizer->FixVariable(4);
        }
    }


    // Fixing the shape Parameters
    // This is used for getting spectral points
    // Only the norm is free
    if ( iFixShape )
    {
        for (unsigned int i = 1; i < fNParms; i++)
        {
            fMinimizer->FixVariable(i);
        }
    }


    // User Defined Fixed variables
    for ( unsigned int i = 0; i < fFixedParameters.size(); i ++ )
    {
      if (fFixedParameters[i] != -999)
      {
        fMinimizer->SetVariable(i, fParmName[i].c_str(), fFixedParameters[i], 0 );
        fMinimizer->FixVariable(i);
      }
    }


    return true;

}


// Function to get the runwise results
int VLikelihoodFitter::getRunWiseFitInfo(int i_runnum, TF1* i_fit)
{


    // Checking if within MJD range

    if (isMJDExcluded(fRunList[i_runnum].MJD)  ||  isRunExcluded(fRunList[i_runnum].runnumber)  )
    {
        return -1;
    }


    // Using parameters from fit which is passed
    vector <double> iParms(fNParms);
    for (unsigned int i = 0; i < fNParms; i++)
    {
        iParms[i] = i_fit->GetParameter(i);
    }

    double i_lRun = -1*getRunwiseLogL(i_runnum, iParms);
    double i_l0Run = -1*getRunwiseLogL0(i_runnum, iParms);
    double i_RunNBins = fNBinsFit_runwise - fNParms;
    double i_Chi2Run = -2*(i_lRun - i_l0Run);


    cout << i_runnum << " " << fRunList[i_runnum].runnumber <<  " " << i_lRun << " "
         << i_l0Run << " " << i_Chi2Run << " " << i_RunNBins - fNParms << " "
         << i_Chi2Run/(i_RunNBins - fNParms) << endl;

    return 0;

}

// Printing Runwise fit details. Useful for debugging
void VLikelihoodFitter::getRunwiseFitStatus(TF1 *i_fit)
{
    cout << "Run\tlog(L)\tlog(L_0)\tChi^2\tNDF\treduced Chi^2" << endl;
    for (unsigned int i = 0; i < fRunList.size(); i++)
    {
        getRunWiseFitInfo(i, i_fit);
    }
}


// Runwise Log(L)
double VLikelihoodFitter::getRunwiseLogL(int i_run, vector <double> parms)
{



    vector < vector <double> > i_myModel = getModelPredictedExcess( parms );
    vector < vector <double> > i_myvOffMLE = getModelPredictedOff( parms );
    double a, b, c, d;

    fNBinsFit_runwise = 0;
    double LogLi = 0;


    // Getting the last counts
    int iLastOn = getLastCount(fOnCounts[i_run]);
    int iLastOff = getLastCount(fOffCounts[i_run]);
    int iLastModel = getLastCount(i_myModel[i_run]);


    for (int j = 0; j < fNEnergyBins; j ++)
    {

        if ( isMJDExcluded(fRunList[i_run].MJD)  || isRunExcluded(fRunList[i_run].runnumber) ) {
            continue;
        }
        if (fEnergyBinCentres[j] < fFitMin_logTeV) {
            continue;
        }
        if (fEnergyBinCentres[j] > fFitMax_logTeV) {
            continue;
        }


        // Stopping on Last on count
        if (bStopOnLastOn && (j == iLastOn) )
        {
          break;
        }

        // Stopping on Last off count
        if (bStopOnLastOff && (j == iLastOff))
        {
          break;
        }

        // Stopping on Last model count
        if (bStopOnLastModel && (j == iLastModel))
        {
          break;
        }


        fNBinsFit_runwise += 1;

        // Avoiding 0*log(0)
        if (fOnCounts[i_run][j] > 0)
        {
            a = fOnCounts[i_run][j]*log(i_myModel[i_run][j] + fRunList[i_run].alpha*i_myvOffMLE[i_run][j]);
        }

        else
        {
            a = 0;
        }

        // Avoiding 0*log(0)
        if ( fOffCounts[i_run][j] > 0 )
        {
            b = fOffCounts[i_run][j]*log(i_myvOffMLE[i_run][j]);
        }

        else
        {
            b = 0;

        }
        // Getting c term
        c = -1.0*(fRunList[i_run].alpha + 1.0)*i_myvOffMLE[i_run][j];

        // Getting d term
        d = -i_myModel[i_run][j];

        LogLi +=  a + b + c + d;

    }

    return -1 * LogLi;

}

// Getting Runwise Log(L_0)
double VLikelihoodFitter::getRunwiseLogL0(int i_run, vector <double> iParms)
{

    double LogLi = 0;

    double a, b, c, d;

    vector < vector <double> > i_myModel = getModelPredictedExcess( iParms );

    // Getting the last counts
    int iLastOn = getLastCount(fOnCounts[i_run]);
    int iLastOff = getLastCount(fOffCounts[i_run]);
    int iLastModel = getLastCount(i_myModel[i_run]);



    for (int j = 0; j < fNEnergyBins; j ++)
    {

        if ( isMJDExcluded(fRunList[i_run].MJD)  || isRunExcluded(fRunList[i_run].runnumber) ) {
            continue;
        }
        if (fEnergyBinCentres[j] < fFitMin_logTeV) {
            continue;
        }
        if (fEnergyBinCentres[j] > fFitMax_logTeV) {
            continue;
        }

        // Stopping on Last on count
        if (bStopOnLastOn && (j == iLastOn))
        {
          break;
        }

        // Stopping on Last off count
        if (bStopOnLastOff && (j == iLastOff))
        {
          break;
        }

        // Stopping on Last model count
        if (bStopOnLastModel && (j == iLastModel))
        {
          break;
        }


        // Avoiding 0*log(0)
        if (fOnCounts[i_run][j] < 1)
        {
            a = 0;
        }
        else
        {
            a = fOnCounts[i_run][j] * TMath::Log( fOnCounts[i_run][j] );

        }

        // Avoiding 0*log(0)
        if (fOffCounts[i_run][j] < 1)
        {
            b = 0;
        }
        else
        {
            b = fOffCounts[i_run][j] * TMath::Log( fOffCounts[i_run][j] );

        }

        c = -1*fOnCounts[i_run][j];
        d = -1*fOffCounts[i_run][j];

        LogLi += a + b + c + d;
    }


    return -1 * LogLi;
}



// A more user safe implementation
double VLikelihoodFitter::getLogL( vector <double> parms)
{
    // check if the length of parms matches the expected
    if ( parms.size() != fNParms )
    {
        cout << "VLikelihoodFitter::getLogL Error invalid number of parameters!"
             << "\n\t\tExpected: " << fNParms
             << "\n\t\tReceived: " << parms.size() << endl;
        return 0;
    }

    double *iParms = new double[fNParms];
    for (unsigned int i = 0; i < fNParms; i++)
    {
        iParms[i] = parms[i];
    }

    return getLogL_internal(iParms);
}


// Getting log(L) based
double VLikelihoodFitter::getLogL_internal( const double* parms)
{

    // Converting to Vectors
    vector <double> vec_parms(fNParms);
    for (unsigned int i = 0; i < fNParms; i++)
    {
        vec_parms[i] = parms[i];
    }

    double LogLi = 0;


    // Default is 1 time bin with bin edges bins MJD min/max
    // Only one instance of when this is not 1 time bin
    // That is when used to get time-aveged (yet temporally binned) fit
    for( unsigned int ntimebin = 0 ; ntimebin < fNRunsInBin.size(); ntimebin++ )
    {

      // Checking bin has data
      if ( fNRunsInBin[ntimebin] < 1 )
      {
        continue;
      }

      // Setting time constraints
      // Keep binning (false).
      // This effects the fitting procedure when data in temporally
      // binned with getVariabilityIndex
      setMJDMinMax(fVarIndexTimeBins[ntimebin], fVarIndexTimeBins[ntimebin+1], false);

      vector < vector <double> > i_myModel = getModelPredictedExcess( vec_parms );
      vector < vector <double> > i_myModelOff = getModelPredictedOff( vec_parms );


      vector <double> i_total_On = sumCounts( fOnCounts );
      vector <double> i_total_Off = sumCounts( fOffCounts );
      vector <double> i_total_Model = sumCounts( i_myModel );
      vector <double> i_total_ModelOff = sumCounts( i_myModelOff );


      // Getting the last counts
      int iLastOn = getLastCount(i_total_On);
      int iLastOff = getLastCount(i_total_Off);
      int iLastModel = getLastCount(i_total_Model);



      double i_mean_alpha = getMeanAlpha();

      // Make things easier to read
      double a, b, c, d;

      // Counting the number of bins used in the fit (for NDF)
      fNBinsFit_Total = 0;


      // Looping over data bins
      for (int j = 0; j < fNEnergyBins; j ++)
      {
          // Fit min/max
          if (fEnergyBinCentres[j] < fFitMin_logTeV) {continue;}
          if (fEnergyBinCentres[j] > fFitMax_logTeV) {continue;}


          // Stopping on Last on count
          if (bStopOnLastOn && (j >= iLastOn)) {break;}

          // Stopping on Last off count
          if (bStopOnLastOff && (j >= iLastOff)) {break;}

          // Stopping on Last model count
          if (bStopOnLastModel && (j >= iLastModel)) {break;}


          fNBinsFit_Total++;

          if (i_total_On[j] >= 1)
          {
              a = i_total_On[j]*TMath::Log(i_total_Model[j] + i_mean_alpha*i_total_ModelOff[j]);
          }

          // 0*log(0)
          else
          {
              a = 0;
          }



          if ( i_total_Off[j] >= 1 && i_total_ModelOff[j] >= 1)
          {
              b = i_total_Off[j]*TMath::Log(i_total_ModelOff[j]);
          }

          // 0*log(0)
          else
          {
              b = 0;

          }
          // Getting c term
          c = -1.0*(i_mean_alpha + 1.0)*i_total_ModelOff[j];

          // Getting d term
          d = -i_total_Model[j];

          LogLi = LogLi + a + b + c + d;
      }
    }

    return -1 * LogLi;
}


/*
* Getting L0 based on the total dataset
* This the likelihood of a perfect model in which
* prediced_excess = On_observed - Alpha*Off_observed
* Off_predicted = Off_observed
*/
//double VLikelihoodFitter::getLogL0 ( vector <double> iParms)
double VLikelihoodFitter::getLogL0 ()
{

    double LogLi = 0;

    vector <double> i_total_On = sumCounts( fOnCounts );
    vector <double> i_total_Off = sumCounts( fOffCounts );

    double a, b, c, d;

    //vector < vector <double> > i_myModel = getModelPredictedExcess( iParms );
    //vector <double> i_total_Model = sumCounts( i_myModel );

    // Getting the last counts
    int iLastOn = getLastCount(i_total_On);
    int iLastOff = getLastCount(i_total_Off);
    int iLastModel = getLastCount(i_total_Model);


    // Looping over data bins
    for (int j = 0; j < fNEnergyBins; j ++)
    {

        // If excluded
        // Checking if within min and max
        if (fEnergyBinCentres[j] < fFitMin_logTeV) {
            continue;
        }
        if (fEnergyBinCentres[j] > fFitMax_logTeV) {
            continue;
        }


        // Stopping on Last on count
        if (bStopOnLastOn && (j == iLastOn))
        {
          break;
        }

        // Stopping on Last off count
        if (bStopOnLastOff && (j == iLastOff))
        {
          break;
        }

        // Stopping on Last model count
        if (bStopOnLastModel && (j == iLastModel))
        {
          break;
        }



        // Avoiding 0*log(0)
        if (i_total_On[j] >= 1)
        {
            a = i_total_On[j] * TMath::Log( i_total_On[j] );
        }

        else
        {
            a = 0;
        }


        // Avoiding 0*log(0)
        if ( i_total_Off[j] >= 1 )
        {
            b = i_total_Off[j] * TMath::Log( i_total_Off[j] );
        }

        else
        {
            b = 0;

        }

        c = -1*int(i_total_On[j]);
        d = -1*int(i_total_Off[j]);

        LogLi += a + b + c + d;
    }


    return -1 * LogLi;

}

/*
* Getting Chi2 based on the total data set
* Using likelihood ratio:
* 2\delta(log(l)) ~ chi^2
*/
double VLikelihoodFitter::getChi2 ( vector <double> iParms)
{

    // Getting log(l)
    double logL = -1*getLogL(iParms);
    // Getting log(l_0)
    double logL_0 = -1*getLogL0(iParms);
    // Printing the total live time
    double i_TotalTime = 0;
    for (unsigned int i = 0; i < fRunList.size(); i++)
    {
        i_TotalTime += fRunList[i].tOn * fRunList[i].deadTimeFraction;
    }

    // Printing a summary of the fit
    cout << "Total Live time (s): " << i_TotalTime << endl
         << "Number of Bins: " << fNBinsFit_Total << endl
         << "Number of Fitting Parameters: " << fNParms << endl
         << "Number of Degrees of Freedom: "<< fNBinsFit_Total - fNParms << endl
         << "Chi^2 :" << -2*(logL - logL_0) << endl
         << "Chi^2/NDF : " << -2*(logL - logL_0) *(1.0/(fNBinsFit_Total - fNParms)) << endl
         << "log(L) : " << logL << endl
         << "log(L_0) : " << logL_0 << endl;

    return -2*(logL -1 *(logL_0));
}



// Get the number of degrees of freedom of the fit
double VLikelihoodFitter::getNDF()
{
    return (fNBinsFit - fNParms);
}


// Plotting Energy Bias for a given run
TCanvas *VLikelihoodFitter::plotEnergyBias(unsigned int i_Entry)
{


    // Checking if run is within range
    if ( i_Entry >= fEnergyBias.size() )
    {
        cout << "VLikelihoodFitter::plotEnergyBias invalided run index. Allowed range [0-"
             << fEnergyBias.size() -1 << "]" << endl;

        return 0;
    }


    TCanvas *cEnergyBias = new TCanvas();

    // Selecting dataset of run to be plotted
    vector <double> i_bias = fEnergyBias[i_Entry];
    vector <double> i_Energy = fEnergyBinCentres;

    // freating new vector
    TGraph *gBias = new TGraph(i_Energy.size(), &(i_Energy[0]), &(i_bias[0]));
    std::ostringstream ss;
    ss << "Energy Bias for " << fRunList[i_Entry].runnumber;

    gBias->SetTitle(ss.str().c_str());
    gBias->GetXaxis()->SetTitle("Log(E_{MC}) (TeV)");
    gBias->GetYaxis()->SetTitle("Energy Bias (E_{MC} - #bar{E_{Rec}})/E_{MC}");
    gBias->SetMarkerSize(1.0);
    gBias->SetMarkerStyle(8);
    gBias->SetMarkerColor(kBlack);
    gBias->Draw("APL");


    // Printing the threshold bias used in the analysis
    TLine *lThresh = new TLine(i_Energy[0], fThresholdBias, i_Energy[i_Energy.size() -1], fThresholdBias);
    lThresh->SetLineColor(kRed);
    lThresh->Draw("SAME");

    return cEnergyBias;
}


// EBL Stuff
// Load in an EBL opacity graph
void VLikelihoodFitter::setEBLOpacity (TGraph* i_EBLOpacity)
{
    fEBLOpacityGraph = (TGraph*)i_EBLOpacity->Clone();
}


// Fit fuction that takes into account EBL attenuation
double VLikelihoodFitter::calculateIntrinsicSpectrum(Double_t *x, Double_t *parm)
{
    // Setting parameters
    for (unsigned int i = 0; i < fNParms; i++)
    {
        fModel_intrinsic->SetParameter(i,parm[i]);
    }
    // Returning dNdE * exp (- tau)
    return fModel_intrinsic->Eval(x[0])*TMath::Exp(-1*fEBLOpacityGraph->Eval(TMath::Power(10,x[0])));

}



// Function to evaluate a broken power law
// Will be accesed via fModel
double VLikelihoodFitter::brokenPowerLaw(Double_t *x, Double_t *parm)
{
  // Define parameters
  double i_N0 = parm[0];
  double i_gamma_1 = parm[1];
  double i_gamma_2 = parm[2];
  double i_eBreak = parm[3];
  double linear = parm[4];
  double i_e = 0.;

  // Linear/log
  if (linear == 0.)
  {
    i_e = TMath::Power( 10., x[0]);
  }
  else
  {
    i_e = x[0];
  }

  if (i_e < i_eBreak )
  {
    return i_N0 * TMath::Power( i_e  / i_eBreak, i_gamma_1 ) ;
  }
  else
  {
    return i_N0 * TMath::Power( i_e / i_eBreak, i_gamma_2 ) ;
  }

}


// Getting the confidence interval
// SOB use TF1::Derivative ?
TGraphAsymmErrors* VLikelihoodFitter::calculateConfidenceInterval( double* i_covmat, TF1 *i_fitfunction, int i_model, int i_fNparms)
{

    int i_nsteps = 1000;
    double i_start = fFitMin_logTeV;
    double i_stop = fFitMax_logTeV;

    double i_step = (i_stop - i_start) / i_nsteps;

    TGraphAsymmErrors *i_ConfidenceInterval = new TGraphAsymmErrors(i_nsteps);


    for ( int i = 0; i < i_nsteps; i++ )
    {
        double i_energy = i_start + i*i_step ;
        double i_flux_err = 0;

        i_flux_err = 0;
        double i_flux = 0;

        if (fEBLAnalysis)
        {
          i_flux = i_fitfunction->Eval(i_energy) * TMath::Exp(fEBLOpacityGraph->Eval(TMath::Power(10,i_energy)));
        }

        else
        {
          i_flux = i_fitfunction->Eval(i_energy);
        }


        // Power Law
        if (i_model == 0 )
        {

          // errN^2/N^2
          i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter(0) / i_fitfunction->GetParameter(0);
          // errGamma^2/log^2(E/E0)
          i_flux_err += TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * i_covmat[ 3 ];
          // 2 log(E/E0) cov_Gamma_N / N
          i_flux_err += -2. * TMath::Log( TMath::Power(10., i_energy) / fENorm ) *i_covmat[ 1 ] /  i_fitfunction->GetParameter(0);
          // F(E) * sqrt(previous)
          i_flux_err = i_flux * TMath::Sqrt(i_flux_err);
        }


        // Exponential Cut Off
        else if (i_model == 1 )
        {
          // errN^2/N^2
          i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter(0) / i_fitfunction->GetParameter(0);
          // log^2(E/E0) * errGamma^2
          i_flux_err += TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * i_covmat[ 4 ];
          // (E/EC^2)^2 * errEC^2
          i_flux_err += TMath::Power( TMath::Power(10., i_energy) / i_fitfunction->GetParameter(2) / i_fitfunction->GetParameter(2) , 2 ) * i_covmat[ 8 ];
          // 2 * log(E/E0) * cov_N_Gamma / N
          i_flux_err += -2. * TMath::Log( TMath::Power(10., i_energy) / fENorm ) *i_covmat[ 1 ] /  i_fitfunction->GetParameter(0);
          // 2 * (E/E_C^2) * cov_N_EC / N
          i_flux_err += 2. * TMath::Power(10., i_energy) / i_fitfunction->GetParameter(2) / i_fitfunction->GetParameter(2) *i_covmat[ 2 ] /  i_fitfunction->GetParameter(0);
          // 2 * log(E/E0) * (E/E_C^2) * cov_Gamma_EC
          i_flux_err += -2. * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Power(10., i_energy) / i_fitfunction->GetParameter(2) / i_fitfunction->GetParameter(2) *i_covmat[ 5 ] ;

          i_flux_err = i_flux * TMath::Sqrt(i_flux_err);
        }



        // Curved Power Law
        else if (i_model == 2 )
        {
          // errN^2/N^2
          i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter(0) / i_fitfunction->GetParameter(0);
          // log^2(E/E0) * errAlpha^2
          i_flux_err += TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * i_covmat[ 4 ];
          // (E/E0) * log^2(E/E0) * errBeta
          i_flux_err += TMath::Power( TMath::Power(10., i_energy) / fENorm * TMath::Log( TMath::Power(10., i_energy) / fENorm ) , 2) * i_covmat[ 8 ];
          // 2 * log(E/E0) * cov_N_Alpha / N
          i_flux_err += -2. * TMath::Log( TMath::Power(10., i_energy) / fENorm ) *i_covmat[ 1 ] /  i_fitfunction->GetParameter(0);
          // 2 * (E/E0) log(E/E0) * cov_N_Beta / N
          i_flux_err += -2. * ( TMath::Power(10., i_energy) / fENorm * TMath::Log( TMath::Power(10., i_energy) / fENorm ) ) *i_covmat[ 2 ] /  i_fitfunction->GetParameter(0);
          // 2 * log^2(E/E0) * (E/E0) * cov_Alpha_Beta
          i_flux_err += 2. * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Power(10., i_energy) / fENorm *i_covmat[ 5 ] ;

          i_flux_err = i_flux * TMath::Sqrt(i_flux_err);
        }



        // Log Parabola
        else if (i_model == 3 )
        {
          // errN^2/N^2
          i_flux_err = i_covmat[ 0 ] / i_fitfunction->GetParameter(0) / i_fitfunction->GetParameter(0);
          // log^2(E/E0) * errAlpha^2
          i_flux_err += TMath::Log( TMath::Power(10., i_energy) / fENorm ) * TMath::Log( TMath::Power(10., i_energy) / fENorm ) * i_covmat[ 4 ];
          // log^4(E/E0) * errBeta^2
          i_flux_err += TMath::Power( TMath::Log( TMath::Power(10., i_energy) / fENorm ), 4 ) * i_covmat[ 8 ];
          // 2 * log(E/E0) * cov_N_alpha / N
          i_flux_err += -2. * TMath::Log( TMath::Power(10., i_energy) / fENorm ) *i_covmat[ 1 ] /  i_fitfunction->GetParameter(0);
          // 2 * log^2(E/E0) * cov_N_beta / N
          i_flux_err += -2. * TMath::Power( TMath::Log( TMath::Power(10., i_energy) / fENorm ), 2 ) *i_covmat[ 2 ] /  i_fitfunction->GetParameter(0);
          // 2 * log^3(E/E0) * cov_alpha_beta / N
          i_flux_err += 2. *  TMath::Power( TMath::Log( TMath::Power(10., i_energy) / fENorm ), 3 )*i_covmat[ 5 ] ;

          i_flux_err = i_flux * TMath::Sqrt(i_flux_err);
        }



        i_ConfidenceInterval->SetPoint(i, TMath::Power(10.,i_energy), i_flux  );
        i_ConfidenceInterval->SetPointError(i, 0, 0, i_flux_err, i_flux_err ) ;// * TMath::Exp(fEBLOpacityGraph->Eval( TMath::Power(10,i_energy) ) ) );


    }

    return i_ConfidenceInterval;
}




// Function to print the counts in each bin
void VLikelihoodFitter::printCountsInBins()
{

    // Getting total counts
    vector <double> i_TotalCountsOn = sumCounts( fOnCounts );
    vector <double> i_TotalCountsOff = sumCounts( fOffCounts );

    cout << "Bin#\tEnergy\t#On\t#Off"<<endl;
    for (unsigned int i = 0 ; i < i_TotalCountsOn.size(); i++)
    {
        cout << i << "\t" <<TMath::Power(10.0,fEnergyBinCentres[i])	 << "\t" << i_TotalCountsOn[i] << "\t" << i_TotalCountsOff[i] << endl;
    }

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
float* VLikelihoodFitter::getSpectralPoint( double BinMin, double BinMax, double ifENorm, TF1* iBestFit, bool bPrintAll)
{

    // Setting fit range to that of the enrgy bin
    setEnergyFitMinMaxLog(BinMin, BinMax);

    // Setting the normalization energy to the bin centre
    if ( fModelID == 1 )
    {
      setModel(fModelID +10, ifENorm);
    }
    else if ( fModelID == 6 )
    {
      setModel(0, ifENorm);
      if (ifENorm < iBestFit->GetParameter(3) )
      {
        fModel->SetParameter(1, iBestFit->GetParameter(1));
      }
      else
      {
        fModel->SetParameter(1, iBestFit->GetParameter(2));
      }
    }
    else
    {
      setModel(fModelID, ifENorm);
    }

    // initalizing the minimizing
    // Inital guess is set to the model evaluated at normalization energy
    // Print status of 0 (quiet)
    // Spectral shape parameters are frozen (true)
    if ( !initializeMinimizer( iBestFit->Eval( ifENorm ), 0,  true) )
    {
        cout << "VLikelihoodFitter::getSpectralPoint minimizer initialization failed."
             << endl << "\t Please run VLikelihoodFitter::getLikelihoodFit() " << endl;
        return 0;
    }

    // Checking TS before applying fit
    cout << "Getting Total Counts for TS" << endl;
    float *iFluxPoint = new float[3];


    double i_onTotal = 0;
    double i_offTotal = 0;
    double i_mean_alpha = getMeanAlpha();


    // Getting the total counts
    for (unsigned int i = 0; i < fOnCounts.size(); i++)
    {

        // Making sure run isn't excluded
        if (isMJDExcluded(fRunList[i].MJD) || isRunExcluded(fRunList[i].runnumber))
        {
            continue;
        }

        for (unsigned int j = 0; j < fOnCounts[0].size(); j++ )
        {
            if (fEnergyBinCentres[j] < fFitMin_logTeV) {
                continue;
            }
            if (fEnergyBinCentres[j] > fFitMax_logTeV) {
                continue;
            }
            i_onTotal += fOnCounts[i][j];
            i_offTotal += fOffCounts[i][j];

        }
    }



    // do the minimization
    fMinimizer->Minimize();

    // Fit details are saved if the fit is valid
    // If the fit isn't valid, the details are only outputted if bPrintAll == True
    if ( (fMinimizer->Status() != 0) && !bPrintAll )
    {
        return 0;
    }

    // Best fit and errors
    const double *i_FitValues = fMinimizer->X();
    const double *i_Errors = fMinimizer->Errors();


    iFluxPoint[0] = i_FitValues[0];
    iFluxPoint[1] = i_Errors[0];


    // Getting asymmetric errors
    double i_err_low = 0;
    double i_err_up = 0;

    fMinimizer->GetMinosError(0, i_err_low, i_err_up);


    // Saving fit details to the TGraphAsymmErrors
    // (int) fEnergySpectrum->GetN() is always the next point to be set
    int npoints = fEnergySpectrum->GetN();


    // Saving the point to fEnergySpectrum
    fEnergySpectrum->SetPoint(npoints, ifENorm, i_FitValues[0]);

    // Checking if the errors are valid
    if ( fMinimizer->Status() == 0)
    {
      cout << "Setting Error " << npoints << " " << i_FitValues[0] << " " << abs(i_err_low) << " " << i_err_up << endl;
        fEnergySpectrum->SetPointError(npoints,
                                       ifENorm - TMath::Power(10.,BinMin),  // E Min
                                       TMath::Power(10.,BinMax) - ifENorm,  // E Max
                                       abs(i_err_low), // dNdE err low  SOB why abs?
                                       i_err_up); // dNdE err high
    }

    else
    {
        fEnergySpectrum->SetPointError(npoints,
                                       ifENorm - TMath::Power(10.,BinMin),  // E Min
                                       TMath::Power(10.,BinMax) - ifENorm,  // E Max
                                       0, // dNdE err low
                                       0); // dNdE err high
    }


    // Saving some useful information
    fSpectralPoint_FitStatus.push_back(fMinimizer->Status());
    fSpectralPoint_TS.push_back(getBinTS(i_onTotal, i_offTotal, i_mean_alpha));
    fSpectralPoint_likelihood_max.push_back( -1 * fMinimizer->MinValue());

    return iFluxPoint;
}



// Returning Contour plot
// Normalisation (parameter 0 of model) vs Index (or whatever parameter 1 is )
TGraph *VLikelihoodFitter::getContours(unsigned int i, unsigned int j)
{

    // Checking the parameters are in the acceptable range
    if ( (i >= fNParms) || (j >= fNParms) )
    {
        cout << "VLikelihoodFitter::getContours invalid search parameters: "
             << "\n\ti :" << i << " , j:" << j
             << "\n Allowed range [0-" << fNParms -1 <<  "]" << endl;
        return 0;
    }

    // Rejecting i = j
    if (i == j)
    {
        cout << "VLikelihoodFitter::getContours cannot get contour of i = j" << endl;
        return 0;
    }

    // Getting contour vectors from map
    std::ostringstream ss;
    ss << i << "," << j;

    if( fContourMap.find( ss.str().c_str() ) != fContourMap.end() )
    {
        TGraph *i_contour = new TGraph(fNContours, &(fContourMap[ss.str().c_str()][0])[0], &(fContourMap[ss.str().c_str()][1])[0]);
        i_contour->GetXaxis()->SetTitle( fParmName[i].c_str() );
        i_contour->GetYaxis()->SetTitle( fParmName[j].c_str() );
        i_contour->SetTitle("");

        return i_contour;
    }

    return 0;

}

// Getting Profile Likelihood for best-fit model
// par - spectral parameter
// min - par_min
// max - par_max
// energybin - energy bin to scan (-1 is the total fit)
TF1 *VLikelihoodFitter::getProfileLikelihood( unsigned int par, double min, double max, int energybin, bool bDelta )
{
    // best fit parameters
    double *i_bestfitparms = new double[fNParms];


    if ( par >= fNParms )
    {
        cout << "VLikelihoodFitter::getProfileLikelihood invalid parameter! " << endl;
    }

    // Bin specific energy ranges and ENorm
    double emin  = 0;
    double emax  = 0;
    double enorm = 0;

    // Global fit range
    double globalEMin = fFitMin_logTeV;
    double globalEMax = fFitMax_logTeV;

    // global scan
    if ( energybin == -1 )
    {
        // Setting parameters to the global fit's parametesr
        for (unsigned int i = 0; i < fNParms; i++)
        {
            i_bestfitparms[i] = fGlobalBestFitParameters[i];
        }

    }

    // Binwise scan
    else
    {

        // Checkigng if the energy bin used is with the stored binwise fits
        if ( energybin >= fEnergySpectrum->GetN() )
        {
          cout << "VLikelihoodFitter::getProfileLikelihood bin index out of range!" << endl;
          return 0;
        }

        // Setting parameter 0 to be the normalisation obtained from getEnergySpectrum()
        for (unsigned int i = 1; i < fNParms; i++)
        {
            i_bestfitparms[i] = fGlobalBestFitParameters[i];
        }

        i_bestfitparms[0] = fEnergySpectrum->GetY()[energybin];

        // Binwise energy range and normalization energy
        emin = TMath::Log10(fEnergySpectrum->GetX()[energybin] - fEnergySpectrum->GetErrorXlow(energybin)) ;
        emax = TMath::Log10(fEnergySpectrum->GetX()[energybin] + fEnergySpectrum->GetErrorXhigh(energybin)) ;
        enorm = fEnergySpectrum->GetX()[energybin];

        // Updating the model
        if ( fModelID == 1 )
        {
          setModel(fModelID +10, enorm);

        }
        else
        {
          setModel(fModelID, enorm);
        }
        setEnergyFitMinMaxLog(emin, emax);

    }

    // Defining a TF1 which will calculated the delta log-likelihood
    TF1 *i_profileLike = new TF1("ProfileLikelihood", this, &VLikelihoodFitter::profileLikelihood, min, max, 2  );

    // Parameter 0 is the number of the parameter to scan over
    // e.g. for normalization par = 0
    // This is passed to VLikelihoodFitter::profileLikelihood
    i_profileLike->SetParameter(0, par);
    // Parameter 1 is the maximum likelihood
    if (bDelta)
    {
      i_profileLike->SetParameter(1, -1*getLogL_internal( i_bestfitparms ) );
    }

    else
    {
      i_profileLike->SetParameter(1, 0 );
    }

    // Need to take a clone before fModel is reset
    TF1 *ireturned  = (TF1*)i_profileLike->Clone();

    // Resetting the model details
    setModel(fModelID);
    setEnergyFitMinMaxLog(globalEMin, globalEMax);

    if (bDelta)
    {
      ireturned->GetYaxis()->SetTitle("#Delta log(L)");
    }

    else
    {
      ireturned->GetYaxis()->SetTitle("log(L)");
    }

    ireturned->GetXaxis()->SetTitle(fParmName[par].c_str());

    return ireturned;
}


// Function to calculate profile likelihood
// return log(L) - log(L_max)
double VLikelihoodFitter::profileLikelihood(Double_t *x, Double_t *par)
{

    // Set of parameters to be passed to likelihood equation
    double *parms = new double[fNParms];

    // Setting values equal to best fit values
    for (unsigned int i = 0 ; i < fNParms; i++)
    {
        parms[i] = fGlobalBestFitParameters[i];
    }

    // Changing the parameter we are scanning over to the input value
    parms[ (int)par[0] ] = x[0];

    // Calculating Log(L)
    double l_scan =  -1*getLogL_internal( parms );

    return l_scan - par[1];
}





/*
* Getting the energy spectrum
* This is done by applying a fit each energy bin
* with the spectral shape parameters frozen to the best fit values
* iBestFit - Best fit function
* bPrintAll == True - save results for all energy bins
* bPrintAll == False - save results for energy bins with a valid fit
*/
TGraphAsymmErrors* VLikelihoodFitter::getEnergySpectrum(TF1 *iBestFit, bool bPrintAll)
{

    // Clear previous fit details
    clearSpectralPoints();
    // Checking parameters (note expecting 1 - nPar for broken pwl)
    if ( (iBestFit->GetNpar() != fNParms)  && (fModelID != 6))
    {
        cout << "VLikelihoodFitter::getEnergySpectrum Error invalid number of parameters!\n"
             << "\t\tExpected: " << fNParms << endl
             << "\t\tRecieved: " << iBestFit->GetNpar() << endl;
        return 0;
    }


    // Saving current fit details to reset later
    double iGlobalMin = fFitMin_logTeV;
    double iGlobalMax = fFitMax_logTeV;
    double iGlobalNorm = fENorm;

    float *iFluxPoint;
    double ifENorm;
    double *parms = iBestFit->GetParameters();
    cout << "\n\nGetting Energy Spectrum\n";

    for (unsigned int i = 0; i < fNParms; i++)
    {
        cout << fParmName[i] << " : " << parms[i] << endl;
    }

    // Deleting any previous saved spectra
    if (fEnergySpectrum)
    {
        delete fEnergySpectrum;
    }

    fEnergySpectrum = new TGraphAsymmErrors();

    for (int i = 0 ; i < fNEnergyBins; i++)
    {
        // Excluding bins outside of the minimum and maximum energy range
        // if ( ( fEnergyBins[i] < iGlobalMin ) ||   (fEnergyBins[i] > iGlobalMax || fEnergyBins[i+1] > iGlobalMax)  )
        if ( ( fEnergyBins[i] < iGlobalMin ) ||   (fEnergyBins[i] > iGlobalMax)  )
        {
            continue;
        }


        cout << "Getting energy bin : " << fEnergyBins[i]  << " - " <<  fEnergyBins[i+1] << endl;


        // Applying fit to energy bin
        setEnergyFitMinMaxLog(fEnergyBins[i], fEnergyBins[i+1]);
        ifENorm = TMath::Power(10.,fEnergyBinCentres[i]);

        // Getting fit
        iFluxPoint = getSpectralPoint( fEnergyBins[i], fEnergyBins[i+1], ifENorm, iBestFit, bPrintAll);

    }

    // Resetting range and normalisation
    setEnergyFitMinMaxLog(iGlobalMin, iGlobalMax);
    setNormalisationEnergyLinear(iGlobalNorm);
    setModel(fModelID, iGlobalNorm);

    cout << "Bin No.\tE\t\tEMin\t\tEMax\tdN/dE\terr(dN/dE)_low\terr(dN/dE)_high\tTS\tLog-Likelihood\tFitStatus\n";

    // Printing out energy spectrum
    for (int i = 0 ; i < fEnergySpectrum->GetN(); i++)
    {


        cout << i << "\t" << fEnergySpectrum->GetX()[i] << "\t" << fEnergySpectrum->GetErrorXlow(i) << "\t" << fEnergySpectrum->GetErrorXhigh(i) << "\t"
             << fEnergySpectrum->GetY()[i] << "\t" << fEnergySpectrum->GetErrorYlow(i) << "\t" << fEnergySpectrum->GetErrorYhigh(i) << "\t"
             << fSpectralPoint_TS[i] << "\t" << fSpectralPoint_likelihood_max[i] << "\t" << fSpectralPoint_FitStatus[i] << endl;

    }

    return fEnergySpectrum;
}



// Getting plots with total counts and residuals
TCanvas* VLikelihoodFitter::getTotalCountsPlots()
{
    // Converting to vectors
    vector <double> iParms(fNParms);
    for (unsigned int i = 0 ; i < fNParms ; i++)
    {
        iParms[i] = fModel->GetParameter(i);
    }


    // Getting Counts
    vector < vector <double> > i_vOffMLE = getModelPredictedOff(iParms);
    vector < vector <double> > i_vModel = getModelPredictedExcess(iParms);

    // For counts plots
    vector < double > i_OnTotal = sumCounts( fOnCounts );
    vector < double > i_OffTotal = sumCounts( fOffCounts );
    vector < double > i_vOffMLETotal = sumCounts( i_vOffMLE );
    vector < double > i_vModelTotal = sumCounts( i_vModel );

    vector < double > i_vModelOnCounts( i_vOffMLE[0].size() );


    // For residual plots
    vector < double > i_OnRes( i_vOffMLE[0].size());
    vector < double > i_OffRes( i_vOffMLE[0].size());

    vector < double > i_OnResErr( i_vOffMLE[0].size());
    vector < double > i_OffResErr( i_vOffMLE[0].size());


    // Keeping count of the maximum points
    double max_on = 0;
    double max_off = 0;
    double max_on_pred = 0;
    double max_off_pred = 0;



    double i_mean_alpha = getMeanAlpha();


    // Calculating the residual
    for (unsigned int i = 0 ; i < i_vOffMLE[0].size(); i++)
    {

        i_vModelOnCounts[i] = (i_vModelTotal[i]  + i_mean_alpha * i_vOffMLETotal[i]);

        i_OnRes [i] = (i_OnTotal[i] - i_vModelOnCounts[i]) / i_vModelOnCounts[i];

        i_OffRes [i] = 	( i_OffTotal[i] - i_vOffMLETotal[i] ) / i_vOffMLETotal[i];


        // Geting max counts for plots later
        if (max_on < i_OnTotal[i] )
        {
            max_on = i_OnTotal[i];
        }

        if (max_off < i_OffTotal[i] )
        {
            max_off = i_OffTotal[i];
        }

        if (max_on_pred < i_vModelOnCounts[i] )
        {
            max_on_pred = i_vModelOnCounts[i] ;
        }

        if (max_off_pred <  i_vOffMLETotal[i] )
        {
            max_off_pred =  i_vOffMLETotal[i];
        }


        if (i_OnTotal[i] < 1.e-5 )
        {
            i_OnResErr [i] = 0;
            i_OnRes [i] = 0;
        }
        else
        {
            i_OnResErr [i] = TMath::Sqrt(i_OnTotal[i]) / i_vModelOnCounts[i] ;
        }


        if (i_OffTotal[i] < 1.e-5 )
        {
            i_OffResErr [i] = 0;
            i_OffRes [i] = 0;
        }
        else
        {
            i_OffResErr [i] = TMath::Sqrt(i_OffTotal[i]) / i_vOffMLETotal[i] ;

        }

    }



    TCanvas *i_cTemp = new TCanvas();
    i_cTemp->Divide(2,2);

    // Zero Line
    TF1 *fZero = new TF1("fZero", "0", -10,10);
    fZero->SetLineStyle(7);
    fZero->SetLineColor(kBlack);




    // On
    TPad *p = (TPad*)i_cTemp->cd(1);
    TGraphAsymmErrors *i_gOnCounts = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_OnTotal[0]) );
    TGraphAsymmErrors *i_gOnModel = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_vModelOnCounts[0]) );

    TLine *line1_On = 0;
    TLine *line2_On = 0;
    TLine *line1_Off = 0;
    TLine *line2_Off = 0;

    // Maximum for the on plot
    if (max_on < max_on_pred)
    {
        line1_On = new TLine(fFitMin_logTeV,0.1,fFitMin_logTeV,10*max_on);
        line2_On = new TLine(fFitMax_logTeV,0.1,fFitMax_logTeV,10*max_on);
    }
    else
    {
        line1_On = new TLine(fFitMin_logTeV,0.1,fFitMin_logTeV,10*max_on_pred);
        line2_On = new TLine(fFitMax_logTeV,0.1,fFitMax_logTeV,10*max_on_pred);
    }

    // Maximum of the off plot
    if (max_on < max_on_pred)
    {
        line1_Off = new TLine(fFitMin_logTeV,0.1,fFitMin_logTeV,10*max_off);
        line2_Off = new TLine(fFitMax_logTeV,0.1,fFitMax_logTeV,10*max_off);
    }
    else
    {
        line1_Off = new TLine(fFitMin_logTeV,0.1,fFitMin_logTeV,10*max_off_pred);
        line2_Off = new TLine(fFitMax_logTeV,0.1,fFitMax_logTeV,10*max_off_pred);
    }


    line1_On->SetLineColor(kRed);
    line2_On->SetLineColor(kRed);
    line1_Off->SetLineColor(kRed);
    line2_Off->SetLineColor(kRed);

    i_gOnCounts->SetTitle("On Counts");
    i_gOnCounts->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gOnCounts->GetYaxis()->SetTitle("Counts");
    i_gOnCounts->GetYaxis()->SetRangeUser(0.1,10*max_on);

    i_gOnCounts->SetMarkerStyle(8);
    i_gOnCounts->SetMarkerSize(1.0);
    i_gOnCounts->SetMarkerColor(kBlue);
    i_gOnCounts->Draw("AP");


    i_gOnModel->SetMarkerStyle(8);
    i_gOnModel->SetMarkerSize(1.0);
    i_gOnModel->SetMarkerColor(kGreen);
    i_gOnModel->Draw("SAME,P");


    line1_On->Draw("SAME");
    line2_On->Draw("SAME");

    TLegend *leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->AddEntry(i_gOnCounts, "Observed Counts","p");
    leg->AddEntry(i_gOnModel, "Model Counts","p");
    leg->Draw();
    p->SetLogy();



    // Off
    p = (TPad*)i_cTemp->cd(2);
    TGraphAsymmErrors *i_gOffCounts = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_OffTotal[0]) );
    TGraphAsymmErrors *i_gPredOffCounts = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_vOffMLETotal[0]) );

    i_gOffCounts->SetTitle("Off Counts");
    i_gOffCounts->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gOffCounts->GetYaxis()->SetTitle("Counts");
    i_gOffCounts->GetYaxis()->SetRangeUser(0.1,10*max_off);

    i_gOffCounts->SetMarkerStyle(8);
    i_gOffCounts->SetMarkerSize(1.0);
    i_gOffCounts->SetMarkerColor(kBlue);
    i_gOffCounts->Draw("AP");

    i_gPredOffCounts->SetMarkerStyle(8);
    i_gPredOffCounts->SetMarkerSize(1.0);
    i_gPredOffCounts->SetMarkerColor(kGreen);
    i_gPredOffCounts->Draw("SAME,P");

    line1_Off->Draw("SAME");
    line2_Off->Draw("SAME");
    leg->Draw();

    p->SetLogy();



    TLine *line3= new TLine(fFitMin_logTeV,-2,fFitMin_logTeV,2);
    TLine *line4 = new TLine(fFitMax_logTeV,-2,fFitMax_logTeV,2);
    line3->SetLineColor(kRed);
    line4->SetLineColor(kRed);

    // On Residuals
    p = (TPad*)i_cTemp->cd(3);
    TGraphErrors *i_gOnRes = new TGraphErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_OnRes[0]), 0, &(i_OnResErr[0]));

    i_gOnRes->SetTitle("On Count Residuals");
    i_gOnRes->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gOnRes->GetYaxis()->SetTitle("(Counts - Model) / Model");
    i_gOnRes->GetYaxis()->SetRangeUser(-2,2);

    i_gOnRes->SetMarkerStyle(8);
    i_gOnRes->SetMarkerSize(1.0);
    i_gOnRes->SetMarkerColor(kBlack);
    i_gOnRes->Draw("AP");
    line3->Draw("SAME");
    line4->Draw("SAME");
    fZero->Draw("SAME");



    // Off Residuals
    p = (TPad*)i_cTemp->cd(4);
    TGraphErrors *i_gOffRes = new TGraphErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_OffRes[0]), 0, &(i_OffResErr[0]));

    i_gOffRes->SetTitle("Off Count Residuals");
    i_gOffRes->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gOffRes->GetYaxis()->SetTitle("(Counts - Model) / Model");
    i_gOffRes->GetYaxis()->SetRangeUser(-2,2);

    i_gOffRes->SetMarkerStyle(8);
    i_gOffRes->SetMarkerSize(1.0);
    i_gOffRes->SetMarkerColor(kBlack);
    i_gOffRes->Draw("AP");

    line3->Draw("SAME");
    line4->Draw("SAME");
    fZero->Draw("SAME");


    return i_cTemp;

}



// Getting useful plots for runs
TCanvas* VLikelihoodFitter::getRunPlots(unsigned int i_Entry)
{

    // Checking to see if run is in range
    if ( i_Entry >= fRunList.size() )
    {
        cout << "VLikelihoodFitter::getRunPlots Error index out of range [0:" << fRunList.size() -1 << "]" << endl;
        return 0;
    }


    vector <double> iParms(fNParms);
    for (unsigned int i = 0; i < fNParms; i ++)
    {
        iParms[i] = fModel->GetParameter(i);

    }

    vector < vector <double> > i_vOffMLE = getModelPredictedOff(iParms);
    vector < vector <double> > i_vModel = getModelPredictedExcess(iParms);

    TCanvas *i_cTemp = new TCanvas();
    i_cTemp->Divide(3,1);

    TLine *line1 = new TLine(fFitMin_logTeV,0.1,fFitMin_logTeV,5000);
    TLine *line2 = new TLine(fFitMax_logTeV,0.1,fFitMax_logTeV,5000);
    line1->SetLineColor(kRed);
    line2->SetLineColor(kRed);

    // On
    TPad *p = (TPad*)i_cTemp->cd(1);
    TGraphAsymmErrors *i_gOnCounts = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(fOnCounts[i_Entry][0]) );
    // TGraphAsymmErrors *i_gOn

    i_gOnCounts->SetTitle("On Counts");
    i_gOnCounts->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gOnCounts->GetYaxis()->SetTitle("Counts");
    i_gOnCounts->GetYaxis()->SetRangeUser(0.1,1.e5);

    i_gOnCounts->SetMarkerStyle(8);
    i_gOnCounts->SetMarkerSize(1.0);
    i_gOnCounts->SetMarkerColor(kRed);
    i_gOnCounts->Draw("AP");

    line1->Draw("SAME");
    line2->Draw("SAME");

    p->SetLogy();


    // Off
    p = (TPad*)i_cTemp->cd(2);
    TGraphAsymmErrors *i_gOffCounts = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(fOffCounts[i_Entry][0]) );
    TGraphAsymmErrors *i_gPredOffCounts = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_vOffMLE[i_Entry][0]) );

    i_gOffCounts->SetTitle("Off Counts");
    i_gOffCounts->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gOffCounts->GetYaxis()->SetTitle("Counts");
    i_gOffCounts->SetMarkerStyle(8);
    i_gOffCounts->SetMarkerSize(1.0);
    i_gOffCounts->SetMarkerColor(kBlue);
    i_gOffCounts->GetYaxis()->SetRangeUser(0.1,1.e5);

    i_gOffCounts->Draw("AP");

    i_gPredOffCounts->SetMarkerStyle(8);
    i_gPredOffCounts->SetMarkerSize(1.0);
    i_gPredOffCounts->SetMarkerColor(kGreen);
    i_gPredOffCounts->Draw("SAME,P");

    line1->Draw("SAME");
    line2->Draw("SAME");
    p->SetLogy();

    // Excess
    p = (TPad*)i_cTemp->cd(3);
    vector < double > i_vExcess;

    for (unsigned int i = 0; i < fOffCounts[i_Entry].size(); i++)
    {
        i_vExcess.push_back(fOnCounts[i_Entry][i] - fRunList[i_Entry].alpha * fOffCounts[i_Entry][i]);
    }

    TGraphAsymmErrors *i_gExcess = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_vExcess[0]) );
    TGraphAsymmErrors *i_gPredExcess = new TGraphAsymmErrors(fEnergyBinCentres.size(), &(fEnergyBinCentres[0]), &(i_vModel[i_Entry][0]) );

    i_gExcess->SetTitle("Excess Counts");
    i_gExcess->GetXaxis()->SetTitle("log(Energy) [TeV]");
    i_gExcess->GetYaxis()->SetTitle("Counts");
    i_gExcess->GetYaxis()->SetRangeUser(0.1,1.e5);

    i_gExcess->SetMarkerStyle(8);
    i_gExcess->SetMarkerSize(1.0);
    i_gExcess->SetMarkerColor(kBlue);
    i_gExcess->Draw("AP");

    i_gPredExcess->SetMarkerStyle(8);
    i_gPredExcess->SetMarkerSize(1.0);
    i_gPredExcess->SetMarkerColor(kGreen);
    i_gPredExcess->Draw("SAME,P");

    line1->Draw("SAME");
    line2->Draw("SAME");

    p->SetLogy();


    return i_cTemp;

}



// Dates to exclude from Spectral Fit
void VLikelihoodFitter::addExclusionDate(double i_MJDStart, double i_MJDStop)
{
    vector <double> tmp;
    tmp.push_back(i_MJDStart);
    tmp.push_back(i_MJDStop);

    fExcludeMJD.push_back(tmp);
}


/*
 * calculate integral flux from best fit model
 * Model is simply integrated
 */
float* VLikelihoodFitter::getIntegralFlux(double i_EMin, double i_EMax, TF1* i_Model, bool i_log, bool i_ul)
{


    // Getting the ingtegral flux from the best fit model

    if (i_log)
    {
        i_EMin = TMath::Power(10,i_EMin);
        i_EMax = TMath::Power(10,i_EMax);
    }

    float *i_flux = 0;

    if ( i_ul )
    {
      i_flux = new float[2];
      i_flux[0] = i_Model->GetParameter(0) * (TMath::Power(i_EMax, i_Model->GetParameter(1) +1 ) - TMath::Power(i_EMin, i_Model->GetParameter(1) +1 ) ) /( i_Model->GetParameter(1) +1) / (TMath::Power(fENorm,i_Model->GetParameter(1))) ;
      i_flux[1] = getCrabFlux(i_flux[0],i_EMin, i_EMax);
      return i_flux;
    }

    i_flux = new float[4];

    // Calculating flux from best fit model;
    i_flux[0] = i_Model->GetParameter(0) * (TMath::Power(i_EMax, i_Model->GetParameter(1) +1 ) - TMath::Power(i_EMin, i_Model->GetParameter(1) +1 ) ) /( i_Model->GetParameter(1) +1) / (TMath::Power(fENorm,i_Model->GetParameter(1))) ;

    // Calculating the Error
    float a,b,c,d;
    // dN_int/dNo
    a = (TMath::Power(i_EMax, i_Model->GetParameter(1) +1 ) - TMath::Power(i_EMin, i_Model->GetParameter(1) +1 ) )/( i_Model->GetParameter(1) +1) / (TMath::Power(fENorm,i_Model->GetParameter(1))) ;

    // dN_int/dGamma
    b = i_Model->GetParameter(0) * (TMath::Power(i_EMax, i_Model->GetParameter(1) +1 ) - TMath::Power(i_EMin, i_Model->GetParameter(1) +1 ) ) /( i_Model->GetParameter(1) +1) / ( i_Model->GetParameter(1) +1)  / (TMath::Power(fENorm,i_Model->GetParameter(1))) ;

    c = i_Model->GetParameter(0) * (TMath::Power(i_EMax, i_Model->GetParameter(1) +1 ) * TMath::Log(i_EMax) - TMath::Power(i_EMin, i_Model->GetParameter(1) +1 ) * TMath::Log(i_EMin) ) /( i_Model->GetParameter(1) +1)  / (TMath::Power(fENorm,i_Model->GetParameter(1))) ;

    d =  - i_Model->GetParameter(0) * (TMath::Power(fENorm,-1*i_Model->GetParameter(1))) * TMath::Log(fENorm) * (TMath::Power(i_EMax, i_Model->GetParameter(1) +1 ) - TMath::Power(i_EMin, i_Model->GetParameter(1) +1 ) ) /( i_Model->GetParameter(1) +1) ;



    // Calculating Error
    i_flux[1] = sqrt( a* a * i_Model->GetParError(0) * i_Model->GetParError(0) + (b + c + d) * (b + c + d) *  i_Model->GetParError(1) * i_Model->GetParError(1));



    i_flux[2] = getCrabFlux(i_flux[0],i_EMin, i_EMax);
    i_flux[3] = getCrabFlux(i_flux[1],i_EMin, i_EMax);

    return i_flux;
}


/*
 *  calculate flux in Crab units
 *
 *  (GM) which Crab is this? Whipple?
 *  (SOB) Yes Whipple 1998
 *  ToDo: Implement different Crab spectra options
 */
double VLikelihoodFitter::getCrabFlux( double iF, double i_EMin, double i_EMax, double i_Gamma)
{
    double i_N0 = 3.20e-11;
    double i_Crab = i_N0 * (TMath::Power(i_EMax, i_Gamma +1 ) - TMath::Power(i_EMin, i_Gamma +1 ) ) /( i_Gamma +1);

    return (iF/i_Crab);

}

/*
 *  calculate TS for given Non, Noff, and alpha
 *
 */
double VLikelihoodFitter::getBinTS(double i_on, double i_off, double i_alpha)
{
    double i_a = 0;
    double i_b = 0;

    i_a = i_on * TMath::Log( (1. + i_alpha) *i_on / ( i_alpha * (i_on + i_off) ) );
    i_b = i_off * TMath::Log( (1. + i_alpha) *i_off / ( (i_on + i_off) ) );

    return 2.*( i_a + i_b );

}

/*
 * check if a a certain MJD date is exclude
 *
 * return values:
 *    true:  MJD value is excluded
 *    false: MJD value is not excluded
 *
 */
bool VLikelihoodFitter::isMJDExcluded( double iMJD )
{
    for (unsigned int e = 0; e < fExcludeMJD.size(); e++)
    {
        if ( iMJD > fExcludeMJD[e][0] && iMJD < fExcludeMJD[e][1] )
        {
            return true;
        }
    }

    if ( iMJD < fMJD_Min || iMJD > fMJD_Max )
    {
      return true;
    }
    return false;
}

/*
 * check if a a certain run is exclude
 *
 * return values:
 *    true:  run is excluded
 *    false: run value is not excluded
 *
 */
bool VLikelihoodFitter::isRunExcluded( int iRun )
{
    for (unsigned int e = 0; e < fExcludeRun.size(); e++)
    {
        if( iRun == fExcludeRun[e] )
        {
            return true;
        }
    }
    return false;
}


// Function to set the definition of last bin to use
// (SOB) To simulate when to stop the analysis
void VLikelihoodFitter::setLastCountDefinition (string def)
{
  // Stopping on last on count
  if (def == "on")
  {
    bStopOnLastOn = true;
    bStopOnLastOff = false;
    bStopOnLastModel = false;
  }
  // Stopping on last off count
  else if ( def == "off" )
  {
    bStopOnLastOn = false;
    bStopOnLastOff = true;
    bStopOnLastModel = false;
  }

  // Stopping on last model predicted excess
  else if ( def == "model" )
  {
    bStopOnLastOn = false;
    bStopOnLastOff = false;
    bStopOnLastModel = true;
  }

  else
  {
    cout << "VLikelihoodFitter::setLastCountDefinition Invalid option given"
         << "\n\t\tValid options are 'on', 'off' or 'model'" << endl;
  }

}

// Function to get the last bin with a count in it.
int VLikelihoodFitter::getLastCount( vector <double> ivec )
{
  int ilastbin = 0;
  for (unsigned int i = 0; i < ivec.size(); i++ )
  {
    if ( ivec[i] >= 1 )
    {
      ilastbin = i;
    }
  }

  return ilastbin;
}



// Check if response matrix is ok to use
bool VLikelihoodFitter::isPointerValid(TH2F* i_obj)
{
  bool i_notNull = false;
  bool i_hasCounts = false;

  if (i_obj){i_notNull = true;}
  if (i_obj->GetEntries() != 0) {i_hasCounts = true;}


  return i_notNull && i_hasCounts;
}

// Check if effective area is ok to use
bool VLikelihoodFitter::isPointerValid(TGraphAsymmErrors* i_obj)
{
  bool i_notNull = false;
  bool i_hasCounts = false;

  if (i_obj){i_notNull = true;}
  if (i_obj->Integral() >= 1) {i_hasCounts = true;}


  return i_notNull && i_hasCounts;
}


// Return the time weighted mean alpha normalisation
double VLikelihoodFitter::getMeanAlpha()
{

  double i_mean_alpha = 0;
  double i_alpha_weight = 0;

  for (unsigned int i = 0; i < fOnCounts.size(); i++)
  {

    // Making sure run isn't excluded
    if (isMJDExcluded(fRunList[i].MJD) || isRunExcluded(fRunList[i].runnumber))
    {
      continue;
    }

    // Weighted mean alpha
    i_mean_alpha += fRunList[i].alpha * fRunList[i].tOn * fRunList[i].deadTimeFraction  ;
    i_alpha_weight += fRunList[i].tOn * fRunList[i].deadTimeFraction ;

  }

  // Using the mean alpha normalization
  return i_mean_alpha /= i_alpha_weight ;

}



/*
  Getting the variabiltiy index as defined in FGL catalogues

  Inputs:
    double i_delT - the width of a time bin
    TF1* i_bestFit - the best fit model from which L0 is obtained
    double i_mjdMin - Lower time bin edge (defaults to first observation)
    double i_mjdMax - Upper time bin edge (defaults to last observation)

  Output:
    double -2 (Log(L) - Log(L0))


  Data is binned into time bins of width i_delT.
  Using these data bins one calculates:

  Log(L) - This is the likelihood obtained when the normalization is allowed to vary
  Log(L0) - This is the likelihood obtained with the model paramters set to the best fit

  Function returns -2 (Log(L) - Log(L0)) ~chi^2 distributed 
  Confidence intervals should be verified from simulations.


*/
double VLikelihoodFitter::getVariabilityIndex(double i_delT, TF1 *i_bestFit, double i_mjdMin, double i_mjdMax, int iPrintStatus, bool i_ul)
{

  // Storing global mjd range
  double i_MJDMin_Global = fMJD_Min;
  double i_MJDMax_Global = fMJD_Max;

  // Saving a copy of the best fit for local use
  TF1 *i_localFit = (TF1*)i_bestFit->Clone();

  // checking for nonsense time binning
  if ( i_delT <= 0 )
  {
    cout << "VLikeLihoodFitter::getVariabilityIndex i_delT <= 0!"
         << "\n\tTime binning must be positive and non-zero" << endl;

    return 0;
  }


  // Checking for default values
  // Assuming ordered data...
  // Double check?
  if ( i_mjdMin == -999 )
  {
    i_mjdMin = fRunList[0].MJD - 0.0001;
  }
  if ( i_mjdMax == -999 )
  {
    i_mjdMax = fRunList[fRunList.size() -1].MJD + 0.0001;
  }


  // Checking upper and lower limits
  if ( i_mjdMin >= i_mjdMax )
  {
    cout << "VLikeLihoodFitter::getVariabilityIndex  i_mjdMin >= i_mjdMax!"
         << "\n\tMinimum time must be strictly less than i_mjdMax" << endl;

    return 0;
  }

  // Checking for null pointers
  if ( !i_bestFit )
  {
    cout << "VLikeLihoodFitter::getVariabilityIndex null pointer passed as i_bestFit"
         << "\n\tExpected TF1* of best fit parameters" << endl;
    return 0;
  }

  // Checking dimensioality of i_bestFit
  if (i_bestFit->GetNpar() != fNParms)
  {
    cout << "VLikeLihoodFitter::getVariabilityIndex NPar != fNParms"
         << "\n\tExpected TF1* with NPar equal to the number of parameters of the model" << endl;
    return 0;
  }

  // Setting Upper and Lower edges
  i_mjdMin -= 0.5 * i_delT;
  i_mjdMax += 0.5 * i_delT;


  // Setting data binning
  int i_nBins = int(round((i_mjdMax - i_mjdMin) / i_delT));
  fVarIndexTimeBins.clear();
  fVarIndexTimeBins.assign(i_nBins+1, 0);
  // Local Copy
  vector <double> i_VarIndexTimeBins(i_nBins+1);
  vector <double> i_VarIndexBinCentres(i_nBins);
  vector <double> i_VarIndexBinDurations(i_nBins);

  // Vector of bin min/max MJD
  vector < vector <double> > i_binMJDMinMax(i_nBins, vector<double>(2));

  for (unsigned int i = 0; i < i_binMJDMinMax.size(); i++ )
  {
    i_binMJDMinMax[i][0] = 999999;
    i_binMJDMinMax[i][1] = -999999;
  }


  // Likelihood Holders
  vector <double> i_LogLVarI;
  i_LogLVarI.assign(i_nBins, 0);
  vector <double> i_LogL0VarI;
  i_LogL0VarI.assign(i_nBins, 0);


  fLCFlux.clear();
  fLCFluxErr.clear();
  fLCMJD.clear();
  fLCMJDErr.clear();
  // fLCTS.clear();


  // Does time bin have data?
  fNRunsInBin.clear();
  fNRunsInBin.assign(i_nBins, 0);
  // Local copy
  vector <int> i_NRunsInBin;
  i_NRunsInBin.assign(i_nBins, 0);

  // Setting bin edges
  for (unsigned int i = 0; i < fVarIndexTimeBins.size() ; i++ )
  {
    fVarIndexTimeBins[i] = i_mjdMin + i * i_delT;
    i_VarIndexTimeBins[i] = i_mjdMin + i * i_delT;
    // Setting bin centres
    if (i < i_VarIndexBinCentres.size() )
    {
      i_VarIndexBinCentres[i] = fVarIndexTimeBins[i] + 0.5 * i_delT;
    }
  }


  // Finding databins with entries
  for ( unsigned int i = 0; i < fRunList.size(); i++ )
  {
    // Checking for excluded data
    if (isMJDExcluded(fRunList[i].MJD)  ||  isRunExcluded(fRunList[i].runnumber)  )
    {
        continue;
    }

    // Checking if date is within range
    if (fRunList[i].MJD < i_mjdMin || fRunList[i].MJD > i_mjdMax )
    {
      continue;
    }

    // Bin index for this MJD
    int ibin = int((fRunList[i].MJD - i_mjdMin) / i_delT );

    fNRunsInBin[ibin]++;
    i_NRunsInBin[ibin]++;
    i_VarIndexBinDurations[ibin] += fRunList[i].tOn;

    // checking if new bin min/max
    if ( fRunList[i].MJD < i_binMJDMinMax[ibin][0] )
    {
      i_binMJDMinMax[ibin][0] = fRunList[i].MJD;
    }
    else if ( fRunList[i].MJD > i_binMJDMinMax[ibin][1] )
    {
      i_binMJDMinMax[ibin][1] = fRunList[i].MJD;
    }

  }

  // Applying Fit to binned data
  // Initializing Minimizer with inital guess and fit status
  initializeMinimizer(i_bestFit->Eval(fENorm),  iPrintStatus);

  // Getting LogL
  // do the minimization
  /* Note: 
     For TSVar to be correctly normalized the best fit is obtained by optimizing
     the likelihood equation across each time bin, not from the total time averaged
     dataset. This is a small but very important consideration.
  */
  cout << "\t\t Checking Time binning: "  << fVarIndexTimeBins.size() << " " <<  fNRunsInBin.size() << endl;
  fMinimizer->Minimize();
  const double *xs_logl = fMinimizer->X();
  const double *i_Errors = fMinimizer->Errors();
  TF1 *i_timeBinnedFit = (TF1*)i_bestFit->Clone();

  // Setting best fit parameters and errors
  for (unsigned int j = 0; j < fNParms; j ++)
  {
    i_timeBinnedFit->SetParameter(j, xs_logl[j]  );
    i_timeBinnedFit->SetParError(j, i_Errors[j]  );
  }


  // Comparing Fits
  cout << "Global Fit:" << endl;
  for (unsigned int j = 0; j < fNParms; j ++)
  {
    cout << j << " " << i_bestFit->GetParameter(j) << " " << i_bestFit->GetParError(j) << endl;
  }

  // Comparing Fits
  cout << "Binned Fit:" << endl;
  for (unsigned int j = 0; j < fNParms; j ++)
  {
    cout << j << " " << i_timeBinnedFit->GetParameter(j) << " " << i_timeBinnedFit->GetParError(j) << endl;
  }
  // Getting fixed parameters for LogLo
  double *i_fixedParms = new double[fNParms];
  for (unsigned int i = 0 ; i < fNParms; i++)
  {
    i_fixedParms[i] = i_timeBinnedFit->GetParameter(i);
  }

  // Looping over time bins
  for (unsigned int i = 0 ; i < i_VarIndexBinCentres.size(); i++ )
  {
    // Setting MJD range
    // Binned fit to total data has been applied...
    // Now okay to overwrite time binning
    setMJDMinMax(i_VarIndexTimeBins[i], i_VarIndexTimeBins[i+1], true);

    // Excluding time bins with no data
    if (i_NRunsInBin[i] < 1)
    {
      continue;
    }

    // initalizing the minimizer
    // Freezing all but the flux normalization
    for (unsigned int j = 1; j < fNParms; j ++)
    {
      fixParameter(j,  i_timeBinnedFit->GetParameter(j) );
    }

    // Initializing Minimizer with inital guess and fit status
    initializeMinimizer(i_timeBinnedFit->Eval(fENorm),  iPrintStatus);

    // Getting LogL
    // do the minimization
    fMinimizer->Minimize();
    const double *xs_logl = fMinimizer->X();
    const double *i_Errors = fMinimizer->Errors();

    i_LogLVarI[i] = -1*getLogL_internal(xs_logl);

    // Setting best fit parameters and errors
    for (unsigned int j = 0; j < fNParms; j ++)
    {
      i_localFit->SetParameter(j, xs_logl[j]  );
      i_localFit->SetParError(j, i_Errors[j]  );
    }

    // Getting integral Flux
    float *i_flux = getIntegralFlux(fFitMin_logTeV, fFitMax_logTeV, i_localFit);

    fLCFlux.push_back( i_flux[0] );
    fLCFluxErr.push_back( i_flux[1] );

    vector <double> i_totalOn_vec = sumCounts(fOnCounts);
    double i_totalOn = sumCounts(i_totalOn_vec);
    vector <double> i_totalOff_vec = sumCounts(fOffCounts);
    double i_totalOff = sumCounts(i_totalOff_vec);

    fLCFluxTS.push_back( getBinTS( i_totalOn, i_totalOff, getMeanAlpha()) );

    // Gettin Upper limits (95%)
    // (SOB) UL code needs work...
    if (i_ul)
    {
      double ul = 0;
      double n_mult = 1;
      // Getting upper limit
      while (ul == 0)
      {
        // Need to scan over norm and see if the UL level is met
        if (n_mult > 100. ){break;}

        TF1 *i_profile = getProfileLikelihood(0, 0, (n_mult+1) * i_localFit->GetParameter(0) );
        // Last two parameters are:
        // epislon (default of 1.e-11 is too coarse)
        // number of iterations (default is 100, playing it a bit safer)
        ul = i_profile->GetX(-2.71/2, i_localFit->GetParameter(0), i_localFit->GetParameter(0) + n_mult * i_localFit->GetParError(0),  1.e-15, 500  );
        n_mult += 1.;

        delete i_profile;
      }

      if ( ul == 0 )
      {
        cout << "UL Not Found. NIterations: " << n_mult << endl;
        fLCFluxUL.push_back( 0 );
      }

      else
      {
        i_localFit->SetParameter(0, ul);
        float *i_flux_ul = getIntegralFlux(fFitMin_logTeV, fFitMax_logTeV, i_localFit, true, true );
        fLCFluxUL.push_back( i_flux_ul[0] );
        cout << "UL Found. NIterations: " << n_mult << " " << ul << " " << i_flux_ul[0] << endl;
      }
    }

    else
    {
      fLCFluxUL.push_back( 0 );
    }

    double iWidth = ( i_binMJDMinMax[i][1] - i_binMJDMinMax[i][0] ) / 2. ;
    // Handling the case with 1 run
    if (i_NRunsInBin[i] == 1)
    {
      fLCMJD.push_back( i_binMJDMinMax[i][0]  );
      // To days / 60 seconds / 60 minutes / 24 hours
      fLCMJDErr.push_back( i_VarIndexBinDurations[i] / 60. / 60. / 24. );
    }
    else
    {
      fLCMJD.push_back( i_binMJDMinMax[i][0] + iWidth );
      fLCMJDErr.push_back( iWidth );
    }

    delete i_flux;

    // Getting LogL0
    // initalizing the minimizer
    // Freezing all parameters
    for (unsigned int j = 0; j < fNParms; j ++)
    {
      fixParameter(j,  i_localFit->GetParameter(j) );
    }

    // initializeMinimizer();

    // Getting LogL0
    i_LogL0VarI[i] = -1*getLogL_internal(i_fixedParms);


    // Resetting Fixed Parameters
    for (unsigned int j = 0; j < fNParms; j ++)
    {
      fFixedParameters[j] = -999;
    }
  }



  // Looping over to give final details
  cout << "Summing Results...."
       << "\n\tBin#\tMJDMin\tMJDMax\tLogL\tLogL0" << endl;

  double i_logL_Total = 0;
  double i_logL0_Total = 0;
  int count = 0;

  for (unsigned int i = 0 ; i < i_VarIndexBinCentres.size(); i++ )
  {
    // Skipping empty bins
    if (i_NRunsInBin[i] < 1)
    {
      continue;
    }

    cout << "\t" << count
         << "\t" << i_VarIndexTimeBins[i]
         << "\t" << i_VarIndexTimeBins[i+1]
         << "\t" << i_LogLVarI[i]
         << "\t" << i_LogL0VarI[i]
         << endl;
         count++;
         i_logL_Total += i_LogLVarI[i];
         i_logL0_Total += i_LogL0VarI[i];
  }

  cout <<"Totals..." << endl;
  cout << "\t\t" << i_VarIndexTimeBins[0]
       << "\t" << i_VarIndexTimeBins[i_VarIndexTimeBins.size() -1]
       << "\t" << i_logL_Total
       << "\t" << i_logL0_Total
       << endl;

  // Resetting min/max mjd
  setMJDMinMax(i_MJDMin_Global, i_MJDMax_Global);
  delete[] i_fixedParms;
  return -2 *( i_logL0_Total - i_logL_Total ) ;

}



// Function to return the light curve info from
TGraphErrors *VLikelihoodFitter::getLightCurve()
{

  // Check if data vectors have been filled
  if ( fLCFlux.size() == 0 )
  {
    cout << "VLikelihoodFitter::getLightCurve Flux data has not been calculated!"
         << "\tCall VLikeLihoodFitter::getVariabilityIndex before obtaining Light curve"
         << endl;
    return 0;
  }
  // For labeling axis
  std::ostringstream ss;

  TGraphErrors *i_lightCurve = new TGraphErrors();
  // Filling the LC on a bin by bin bases to allow cuts on TS
  // ULs could be used instead for those points
  cout << "# MJD\tMJD-err\tFlux\tFlux-err\tFlux-UL\tTS" << endl;
  for (unsigned int i = 0; i < fLCFlux.size() ; i++ )
  {
    i_lightCurve->SetPoint(i, fLCMJD[i] , fLCFlux[i] );
    i_lightCurve->SetPointError(i, fLCMJDErr[i] , fLCFluxErr[i] );

    cout << fLCMJD[i] << "\t"
        << fLCMJDErr[i] << "\t"
        << fLCFlux[i] << "\t"
        << fLCFluxErr[i] << "\t"
        << fLCFluxUL[i] << "\t"
        << fLCFluxTS[i] << endl;
  }


  ss.str(std::string());
  ss << "Flux (" << TMath::Power(10,fFitMin_logTeV) << " TeV < E < " << TMath::Power(10,fFitMax_logTeV) << "TeV) [cm^{-2} s^{-1}] " ;

  i_lightCurve->SetMarkerSize(1);
  i_lightCurve->SetMarkerStyle(21);
  i_lightCurve->SetTitle("");
  i_lightCurve->GetXaxis()->SetTitle("Date [MJD]");
  i_lightCurve->GetYaxis()->SetTitle(ss.str().c_str());

  return (TGraphErrors*)i_lightCurve->Clone();
}


// Generic function to sum the runwise counts
vector <double> VLikelihoodFitter::sumCounts( vector < vector <double> > i_countVector )
{

    if ( i_countVector.size() == 1)
    {
        return i_countVector[0];
    }

    vector <double> i_tmpVec( i_countVector[0].size() );

    // Looping over runs
    for (unsigned int i = 0; i < fRunList.size(); i ++)
    {
        // checking if run is excluded
        if ( isMJDExcluded(fRunList[i].MJD)  || isRunExcluded(fRunList[i].runnumber) ) {
            continue;
        }

        // Looping over energy bin
        for (unsigned int j = 0; j < i_tmpVec.size(); j++)
        {
          // Rejecting counts below threshold
          if ( TMath::Power(10., fEnergyBinCentres[j]) < fRunList[i].energyThreshold)
          {
              continue;
          }

            i_tmpVec[j] += i_countVector[i][j];
        }
    }

    return i_tmpVec;
}


// Generic function to sum the vector of counts
double VLikelihoodFitter::sumCounts( vector < double > i_countVector )
{

    double i_tmpSum = 0;

    // length of vector could be same as energy Binning
    if (i_countVector.size() != fEnergyBinCentres.size() )
    {
      cout << "VLikelihoodFitter::sumCounts Incorrect number of bins" << endl;
      return 0;
    }

    // Looping over counts
    for (unsigned int i = 0; i < i_countVector.size(); i ++)
    {
      // Checking if within allowed range
      if (fEnergyBinCentres[i] < fFitMin_logTeV || fEnergyBinCentres[i] >  fFitMax_logTeV)
      {
        continue;
      }
      i_tmpSum += i_countVector[i];
    }

    return i_tmpSum;
}

// Function to get dead time corrected exposure
double VLikelihoodFitter::getLiveTime()
{
  double i_totalLiveTime = 0;
  // Looping over runs
  for (unsigned int i = 0; i < fRunList.size(); i ++)
  {
      // checking if run is excluded
      if ( isMJDExcluded(fRunList[i].MJD)  || isRunExcluded(fRunList[i].runnumber) ) {
          continue;
      }
      // Adding deadtime correct exposure for each run
      i_totalLiveTime += fRunList[i].tOn * fRunList[i].deadTimeFraction;
  }

  return i_totalLiveTime;
}
