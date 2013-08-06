/*! \class VTableEnergyCalculator
    \brief fill or lookup energy tables


    \author Henrik Krawczynski (WashU), Gernot Maier 

*/

#include "VTableEnergyCalculator.h"

VTableEnergyCalculator::VTableEnergyCalculator( int intel )
{
    setConstants();
    fDebug = 0;

    if( intel == 0 ) return;

    for( int i = 0; i < intel; i++ )
    {
        hVMedian.push_back( 0 );
    }
    hMedian = 0;
    hSigma = 0;
    hNevents = 0;
    hMean = 0;

// minimum number per bin required for reconstruction
    fMinShowerPerBin = 5.;

    Omode = 'r';

    initialize();

    fReadHistogramsFromFile = false;
}


VTableEnergyCalculator::VTableEnergyCalculator(const char* hname_add,char m, TDirectory *iDir, bool iMedian, string iInterpolate )
{
    setConstants();
    fDebug = 0;
    
    fUseMedianEnergy = iMedian;
    fHName_add = hname_add;

    fWrite1DHistograms = true;

    fInterPolWidth = 1;
    fInterPolIter = 3;

// minimum number per bin required for reconstruction
    fMinShowerPerBin = 5.;

    Omode  = m;

    fOutDir = iDir;
    if( !fOutDir || !fOutDir->cd() )
    {
        cout << "Energy: error accessing data directory in root file" << endl;
        exit( -1 );
    }
    hMedian = 0;
    hSigma = 0;
    hNevents = 0;
    hMean = 0;

    initialize();

    char htitle[800];
    char hname[800];

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// table making
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

    if ((Omode=='w')||(Omode=='W'))
    {
        if( !fOutDir->IsWritable() )
        {
            cout << "Energy: error data directory in root file not writable" << endl;
            exit( -1 );
        }

/* HSTOGRAM BOOKING */

// median size
        sprintf( hname, "hMedian_energy_%s", hname_add );
        if( fUseMedianEnergy ) sprintf( htitle, "energy vs. distance vs. log10 size (median), %s", hname_add );
        else                   sprintf( htitle, "energy vs. distance vs. log10 size (mean), %s", hname_add );
        hMedian = new TH2F( hname, htitle, eNumEne, log10(eE0Min),log10(eE0Max),eNumDist, 0., eNumDist*edist_delta);
        hMedian->SetXTitle( "log_{10} energy [TeV]" );
        hMedian->SetYTitle( "distance to shower core [m]" );
        hMedian->SetZTitle( "log_{10} size (median)" );

        sprintf( hname, "hMean_energy_%s", hname_add );
        sprintf( htitle, "energy vs. distance vs. log10 size (mean), %s", hname_add );
        hMean = new TProfile2D( hname, htitle, eNumEne, log10(eE0Min),log10(eE0Max),eNumDist, 0., eNumDist*edist_delta, exlow, exhigh );
        hMean->SetXTitle( "log_{10} energy [TeV]" );
        hMean->SetYTitle( "distance to shower core [m]" );
        hMean->SetZTitle( "log_{10} size (mean)" );

/* HSTOGRAM BOOKING */
        for( int i = 0;i < eNumEne; i++ )
        {
            vector< TH1F* > iH1;
            for( int j = 0; j < eNumDist; j++ ) iH1.push_back( 0 );
            Oh.push_back( iH1 );
        }
    }
    else
    {
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// table reading
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
        sprintf( hname, "hMedian_energy_%s", hname_add );
        hMedianName = hname;

        sprintf( hname, "hSigma_energy_%s", hname_add );
        hSigmaName = hname;

        sprintf( hname, "hNevents_energy_%s", hname_add );
        hNeventsName = hname;

        sprintf( hname, "hMean_energy_%s", hname_add );
        hMeanName = hname;

        fInterpolationString = iInterpolate;

        fReadHistogramsFromFile = false;
    }

}

void VTableEnergyCalculator::setConstants()
{
// energy axis: range from 1 GeV to 1000 TeV
/*   eNumEne = 120;
   eE0Min  = 0.001;
   eE0Max  = 1000; */
// energy axis: range from 10 GeV to 316 TeV
   eNumEne = 90;
   eE0Min  = 0.01;
   eE0Max  = TMath::Power( 10., 2.5 );

// core axis: 100 x 15m = 1.5 km
   eNumDist   = 100;
   edist_delta = 15;

// size bins (log10 axis)
   eHistBins = 1000;
   exlow     = 0.;
   exhigh    = 10.;
}


void VTableEnergyCalculator::initialize()
{
// minimum event size per telescope
    fMinSize = 0.;
    fMaxDistance = 999999.;
}


void VTableEnergyCalculator::terminate( TDirectory *iOutDir, char *xtitle )
{
    if( iOutDir != 0 ) fOutDir = iOutDir;

    if ((Omode=='w')||(Omode=='W'))
    {
        cout << "\t energy (energy vs distance vs size) tables: evaluating" << endl;
        if( fUseMedianEnergy ) cout << "\t (using median of size distributions)";
        else                   cout << "\t (using mean of size distributions)";
        cout << endl;
        TDirectory *iDir1D = 0;

// number of events
        if( fOutDir )
	{
	   fOutDir->cd();

	   char htitle[800];
	   char hname[800];
	   sprintf( hname, "hNevents_energy_%s", fHName_add.c_str() );
	   sprintf( htitle, "energy vs. distance vs. # events, %s", fHName_add.c_str() );
	   hNevents = new TH2F( hname, htitle, eNumEne, log10(eE0Min),log10(eE0Max),eNumDist, 0., eNumDist*edist_delta);
	   hNevents->SetXTitle( "log_{10} energy [TeV]" );
	   hNevents->SetYTitle( "distance to shower core [m]" );
	   hNevents->SetZTitle( "# of events" );

// sigma
	   sprintf( hname, "hSigma_energy_%s", fHName_add.c_str() );
	   sprintf( htitle, "energy vs. distance vs. log10 size (sigma), %s", fHName_add.c_str() );
	   hSigma = new TH2F( hname, htitle, eNumEne, log10(eE0Min),log10(eE0Max),eNumDist, 0., eNumDist*edist_delta);
	   hSigma->SetXTitle( "log_{10} energy [TeV]" );
	   hSigma->SetYTitle( "distance to shower core [m]" );
	   hSigma->SetZTitle( "log_{10} size (sigma)" );

// make output directory for 1D histograms
           iDir1D = fOutDir->mkdir( "histos1D" );
        }

/* EVALUATION OF HISTOGRAMS */

	float med = 0.;
	float sigma = 0.;

	double i_a[] = { 0.16, 0.5, 0.84 };
	double i_b[] = { 0.0,  0.0, 0.0  };

        for( int i = 0;i<eNumEne; i++ )
        {
            for( int j=0;j<eNumDist;j++)
            {
		if( !Oh[i][j] ) continue;

		if( Oh[i][j]->GetEntries() > fMinShowerPerBin )
		{
		   Oh[i][j]->GetQuantiles( 3, i_b, i_a );
		   med   = i_b[1];
		   sigma = i_b[2] - i_b[0];
                }
                else
                {
                    med   = 0.;
                    sigma = 0.;
                }
                hMedian->SetBinContent( i+1, j+1, med );
		hMedian->SetBinError( i+1, j+1, sigma );
                hSigma->SetBinContent( i+1, j+1, sigma );
                if( Oh[i][j] ) hNevents->SetBinContent( i+1, j+1, Oh[i][j]->GetEntries() );

                if( fOutDir && fWrite1DHistograms )
                {
                    fOutDir->cd();
                    iDir1D->cd();
                    if( Oh[i][j] ) Oh[i][j]->Write();
                }
		if( Oh[i][j] ) delete Oh[i][j];
            }
        }
        if( fOutDir && hNevents && hNevents->GetEntries() > 0 )
        {
            fOutDir->cd();
            if( hMedian && xtitle )
	    {
	       hMedian->SetTitle( xtitle );
	       hMedian->SetEntries( hNevents->GetEntries() );
	       hMedian->Write();
            }
            if( hSigma )
	    {
	       hSigma->SetEntries( hNevents->GetEntries() );
	       hSigma->Write();
            }
            if( hNevents ) hNevents->Write();
            if( hMean )    hMean->Write();
        }
	if( hNevents ) delete hNevents;
	if( hMedian )  delete hMedian;
	if( hSigma )   delete hSigma;
	if( hMean )    delete hMean;
    }
}


/* 
     Histogram filling and Energy calculation

     * TABLE FILLING:
                        chi2 : weight used in histogram filling (can be spectral weight)

     * TABLE READING:
                        esys: correction added to estimated energy (correction of systematic error in energy reconstruction)

*/
double VTableEnergyCalculator::calc(int ntel, double e, double *r, double *s, double *et,double &chi2, double &dE, double esys )
{
    int tel = 0;

    int ie = 0;
    int ir = 0;
    double amp = 0.;
    double bincenter = 0.;
////////////////////////////////
// FILL TABLES
////////////////////////////////
    if ((Omode=='w')||(Omode=='W'))
    {
        for( tel = 0; tel < ntel; tel++ )
        {
            if( e != 0. && s[tel] > fMinSize && r[tel] >= 0. && r[tel] < fMaxDistance )
            {
	        ie = hMean->GetXaxis()->FindBin( log10( e ) ) - 1;
// if energy is smaller than lower limit of energy range: fill into first bin
                if( ie < 0 ) ie = 0;
		bincenter = TMath::Power( 10., hMean->GetXaxis()->GetBinCenter( ie+1 ) );

// scale to bincenter
// (GM) changed: change now to bin center on logarithmic scale
                amp = log10( s[tel] * bincenter / e );

                ir = hMean->GetYaxis()->FindBin( r[tel] ) - 1;
                if( ir >= 0 && isnormal( amp ) && isnormal( chi2 ) )
                {
// check for overflow bins
		    if( (unsigned int)ie < Oh.size() && (unsigned int)ir < Oh[ie].size() )
		    {
		       if( !Oh[ie][ir] ) create1DHistogram( ie, ir );
		       Oh[ie][ir]->Fill( (float)(amp), chi2 );
		       hMean->Fill( log10(e), r[tel], amp * chi2 );
                    }
                }
            }
        }
        return -1.;
    }
/////////////////////////////////////////
// READ TABLES (compute energy values) //
/////////////////////////////////////////
    else
    {
        double med = 0.;
        double sigma = 0.;
        double logEnergy = 0.;
        double weight = 0.;

// reset energy and echi values
        chi2 = -99.;
        for( tel = 0; tel < ntel; tel++ ) et[tel] = -99.;

// energy per telescope
        double a[VDST_MAXTELESCOPES];
        double b[VDST_MAXTELESCOPES];
        int k=0;

        if( fDebug ) cout << "VTableEnergyCalculator::calc(): number of telescopes " << ntel << endl;

/////////////////////////////////////////////////////
// loop over all telescopes
        for( tel=0; tel < ntel; tel++ )
        {
// check for minimum size and distance to telescope values
            if (s[tel] > fMinSize && r[tel] >= 0. && r[tel] < fMaxDistance )
            {
                if( fDebug )
		{
		   cout << "\t VTableEnergyCalculator::calc(): passed size and distance cut for telescope " ;
		   cout << tel << "\t( size " << s[tel] << ", distance " << r[tel] << "m )" << endl;
                }
                {
                    if( fDebug ) cout << "\t\t VTableEnergyCalculator::calc(): passed maximum local distance cut for telescope " << tel << endl;

// get energy from table
//   (med is energy estimator for this telescope)
                    get_logEnergy2D( log10(s[tel]), r[tel], med, sigma, tel );

                    if( fDebug ) cout << "\t\t  VTableEnergyCalculator::calc(): energy estimator: " << med << "\t" << sigma << endl;

                    if( sigma > 0. )
                    {
////////////////////////////////////////////////
// apply correction from VEnergyCorrection
                        med -= esys;

// add energy and weight per telescope to mean energy
                        logEnergy += med/(sigma*sigma);
                        weight    += 1. /(sigma*sigma);

// store energy per telescope
                        if( med > -90. )
                        {
                            et[tel] = pow(10.,med);
                            a[k] = med;
                            b[k] = sigma;
                            k++;
                        }
                        else
                        {
                            et[tel]    = -90.;
                        }
                    }
                }
            }
        }  // end of loop over all telescopes
/////////////////////////////////////////////////////

// calculate mean energy
        if( weight > 0. )
        {
            logEnergy /= weight;

            if( k > 1 )
            {
                chi2 = 0.;
                double z1 = 0.;
//////////////////////////////////////////////////////////////////////
// OLD dE definition
//	 	dE   = 0.;
//                int z2 = 0;
//		double linEnergy = pow(10.,logEnergy);
//////////////////////////////////////////////////////////////////////

// loop over all telescopes with valid energy
                for( int j = 0; j < k; j++ )
                {
                    if( b[j] != 0. )
                    {
                        chi2  += (logEnergy-a[j])*(logEnergy-a[j]) / b[j] / b[j];
                        z1++;
                    }
//////////////////////////////////////////////////////////////////////
// OLD dE definition
//		    if( linEnergy > 0 )
//		    {
//		        dE +=  ( TMath::Power( 10., a[j] ) - linEnergy ) / linEnergy;
//			z2++;
//                    }
//////////////////////////////////////////////////////////////////////
                }
                if( z1 > 1 ) chi2 /= (z1-1.);
                else         chi2 = -99.;
//////////////////////////////////////////////////////////////////////
// OLD dE definition
//		if( z2 > 0 ) dE   /= z2;
//////////////////////////////////////////////////////////////////////
//		else         dE   = -99.;
// NEW dE definition
// use sqrt of weights
                dE = sqrt( 1./weight );
            }
            else
            {
                chi2=-99.;
                logEnergy = -99.;
            }
            if( logEnergy > -90. ) return pow(10.,logEnergy);
            else return logEnergy;
        }
        else return -99.;
    }
}



/* 
    get energy from table 
    
    Note that this implementation is a bit of a night mare: it requires strict proportionality between size and energy
    (a few tricks are applied to be robust against fluctuations;
     events for which the proportionality cannot be seen are discarded!)

    
*/
void VTableEnergyCalculator::get_logEnergy( double logSize, int ir, double &med, double &sigma, unsigned int tel )
{
    TH2F *hM = hMedian;
    if( hVMedian.size() > 0 && tel < hVMedian.size() ) hM = hVMedian[tel];
    if( !hM ) return;

// weird things happen at the upper end of the histograms
    int i_eNumEneMax = hM->GetNbinsX() - 4;
    int i = 0;
    int j = 0;
    int ij = 0;
////////////////////
// get energy bin number for corresponding size
// this assumes strict proportionality between energy and size for a given distance

// check if size is too small
    for( i = 1; i <= i_eNumEneMax; i++ )
    {
        if(  hM->GetBinContent( i, ir ) > logSize )
	{
// check that the next 4 bins are higher than the current one (be more robust against fluctuations)
   	   bool i_bContinue = false;
           for( int k = 1; k < 5; k++ )
	   {
	      if( i+k < hM->GetNbinsX() && hM->GetBinContent( i+k, ir ) < hM->GetBinContent( i, ir ) )
	      {
		 i_bContinue = true;
	      }
           }
	   if( i_bContinue ) continue;
	   break;
        }
    }
    if( i > 1 ) i--;
    for( j = i_eNumEneMax; j > 1; j-- )
    {
        if( hM->GetBinContent( j, ir ) > 0. && hM->GetBinContent( j, ir ) < logSize ) break;
    }
////////////////////
// check consistency between both methods

// discard event for inconsistent results
    if( i - j != 0 )
    {
	med  =-99.;
	sigma=-99.;
	return;
    }
// consistent results
    else ij = i;

    if( hM->GetBinError( ij, ir ) > 0. && hM->GetBinError( ij+1, ir ) < 1.e-6 && ij > 0 ) ij--;

// interpolation
    double s1 = hM->GetBinContent( ij, ir );
    double s2 = hM->GetBinContent( ij+1, ir );
    double w1 = hM->GetBinError( ij, ir );
    double w2 = hM->GetBinError( ij+1, ir );

// (GM) do not interpolate beyond largest size (s2 > logSize)
    if( (s1>0.) && (s2>0.) && (w1>0.) && (w2>0.) && (s2 > logSize) )
    {
        double e1 = hM->GetXaxis()->GetBinCenter( ij );
        double e2 = hM->GetXaxis()->GetBinCenter( ij+1 );

        double delta = s2-s1;
        if( delta != 0. )
        {
            double rest  = (logSize-s1)/delta;
            med   =  (1.-rest)*e1 + rest*e2;
            sigma = ( (1.-rest)*w1 + rest*w2) / log( 10. );          // observe this has the unit size
        }
        else
        {
            med = e1;
            sigma = 0.;
        }
    }
    else
    {
        med  =-99.;
        sigma=-99.;
    }

    return;
}


/*!
     get energy, interpolate between two distance bins
*/
void VTableEnergyCalculator::get_logEnergy2D( double logSize, double r, double &med, double &sigma, unsigned int itel )
{
// get histogram
    TH2F *hM = hMedian;
    if( hVMedian.size() > 0 && itel < hVMedian.size() ) hM = hVMedian[itel];
    if( !hM ) return;

// get bin number for distance r 
    int ir1 = hM->GetYaxis()->FindBin( r );
// check that r is not in under or overflow of histograms
    if( ir1 < 1 || ir1 >= hM->GetNbinsY() )
    {
        med = -99.;
        sigma = -99.;
	if( fDebug ) cout << "\t\t void VTableEnergyCalculator::get_logEnergy2D overflow bin in r " << r << "\t" << ir1 << "\t" <<  hM->GetNbinsY() << endl;
        return;
    }
// check if distance r is larger or smaller than corresponding bin center (interpolation)
    if( r < hM->GetYaxis()->GetBinCenter( ir1 ) && ir1 > 1 ) ir1--;
    double re = hM->GetYaxis()->GetBinCenter( ir1 );

    double rest  = ( r - re ) / hM->GetYaxis()->GetBinWidth( ir1 );
    double e1 = 0.;
    double e2 = 0.;
    double s1 = 0.;
    double s2 = 0.;

// get energies for these two radius bins
    get_logEnergy( logSize, ir1, e1, s1, itel );
    get_logEnergy( logSize, ir1+1, e2, s2, itel );

// interpolate between distance bins
// (if there is only one estimator, use only that)
    if( e1 > -99 && e2 > -99 ) med   =  (1.-rest)*e1 + rest*e2;
    else if( e1 > -99 ) med = e1;
    else if( e2 > -99 ) med = e2;
    else
    {
       if( fDebug )
       {
          cout << "\t\t void VTableEnergyCalculator::get_logEnergy2D interpolation unsuccessfull: ";
          cout << e1 << "\t" << e2 << "\t" << rest;
	  cout << ", bins: " << ir1 << "\t" << s1 << "\t" << ir1+1 << "\t" << s2 << endl;
       }
       med = -99.;
    }

    if( s1 > -99 && s2 > -99 )  sigma =  (1.-rest)*s1 + rest*s2;
    else if( s1 > -99 ) sigma = s1;
    else if( s2 > -99 ) sigma = s2;
    else sigma = -99.;
}


void VTableEnergyCalculator::setCutValues( double iSize, double iDist )
{
    fMinSize = iSize;
    if( fMinSize < 0. ) fMinSize = 0.;
    fMaxDistance = iDist;
}


void VTableEnergyCalculator::setInterpolationConstants( int iwidth, int iinter )
{
    fInterPolWidth = iwidth;
    fInterPolIter = iinter;
}


void VTableEnergyCalculator::setVHistograms( vector< TH2F* >& hM )
{
    hVMedian = hM;
}


TH2F* VTableEnergyCalculator::getHistoMedian()
{
    if( !fReadHistogramsFromFile ) fReadHistogramsFromFile = readHistograms();

    return hMedian;
}


TH2F* VTableEnergyCalculator::getHistoSigma()
{
    if( !fReadHistogramsFromFile ) fReadHistogramsFromFile = readHistograms();

    return hSigma;
}


bool VTableEnergyCalculator::readHistograms()
{
    if( fOutDir )
    {
        hMedian = (TH2F*)fOutDir->Get( hMedianName.c_str() );

        fReadHistogramsFromFile = true;

// interpolate or smooth 2D histograms
// (switched of for efficiency reasons
/*        if( fInterpolationString.size() > 0 )
        {
            VInterpolate2DHistos iInter;
            if( fInterpolationString == "simple" )
            {
                hMedian = iInter.doSimpleInterpolation( hMedian, "interpol", fInterPolWidth, fInterPolIter, false );
            }
            else if( fInterpolationString == "gaussian" )
            {
                hMedian = iInter.doGaussianInterpolation( hMedian, "interpol", hNevents, 1, 1. );
            }
        } */

    }
    if( !hMedian )
    {
        cout << "\t energy tables info: table histograms not found in " << gDirectory->GetName();
	if( fOutDir ) cout << ", " << fOutDir->GetPath();
	cout << endl;
//        exit( -1 );
    }

    return true;

}

bool VTableEnergyCalculator::create1DHistogram( int i, int j )
{
   if( i >= 0 && j >= 0 && i < (int)Oh.size() && j < (int)Oh[i].size() && !Oh[i][j] )
   {
        if( !fOutDir->cd() ) return false;
        char hname[600];
        char htitle[600];
        int id = 100000 + i * 1000 + j;
        sprintf( hname , "h%d",id);
        double ie1 = hMean->GetXaxis()->GetBinLowEdge( i+1 );
        double ie2 = hMean->GetXaxis()->GetBinUpEdge( i+1 );
        double id1 = hMean->GetYaxis()->GetBinLowEdge( j+1 );
        double id2 = hMean->GetYaxis()->GetBinUpEdge( j+1 );
        sprintf( htitle, "%.2f < log10 E < %.2f, %.1f < r < %.1f (%s)", ie1, ie2, id1, id2, fHName_add.c_str() );
        Oh[i][j] = new TH1F( hname, htitle, eHistBins, exlow, exhigh );
        Oh[i][j]->SetXTitle( "log_{10} size" );
   }
   else
   {
      return false;
   }
   return true;
}

