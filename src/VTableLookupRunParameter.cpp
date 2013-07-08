/*! \class VTableLookupRunParameter
    \brief parameter storage class

    Revision $Id: VTableLookupRunParameter.cpp,v 1.1.2.6.4.2.2.4.2.1.6.1.2.6.2.12.2.3 2010/11/18 10:33:51 gmaier Exp $

    \author
    Gernot Maier
*/

#include "VTableLookupRunParameter.h"

ClassImp(VTableLookupRunParameter)

VTableLookupRunParameter::VTableLookupRunParameter()
{
    fDebug = 0;

    outputfile = "";
    tablefile = "";
    ze = 0.;
    isMC = false;
    fInterpolate = 0;
    fUseMedianEnergy = true;
    fPE = false;
    fInterpolateString = "";
    readwrite = 'R';
    writeoption = "recreate";
    fMinRequiredShowerPerBin = 5.;
    bNoNoTrigger = true;
    fUseSelectedImagesOnly = true;
    bWriteReconstructedEventsOnly = 1;
    bShortTree = false;
    bWriteMCPars = true;
    rec_method = 0;
    point_source = false;
    esysfile = "";
    fWrite1DHistograms = false;
    fSpectralIndex = 2.0;
    fWobbleOffset = 500;
    fNoiseLevel = 250;
    fTableFillingCut_CoreError_max = 1.e6;
    fTableFillingCut_NImages_min = 2;
    fTableFillingCut_WobbleCut_max = 15.;
    fmaxlocaldistance = 99.;
    fmaxdist = 50000.;
    fminsize = 0.;
    fSelectRandom = -1.;
    fSelectRandomSeed = 17;
    fMSCWSizecorrection = 1.;
    fMSCLSizecorrection = 1.;
    fEnergySizecorrection = 1.;

    fMC_distance_to_cameracenter_min =  0.;
    fMC_distance_to_cameracenter_max =  1.e10;

    fNentries = 0;
    fMaxRunTime = 1.e9;

    fDeadTimeFraction = 0.5;

    printpara = "";

    meanpedvars = 0.;
}


bool VTableLookupRunParameter::fillParameters( int argc, char *argv[] )
{
// check number of command line parameters
    if( argc < 2 )
    {
        printHelp();
        return false;
    }
// =============================================
// reading command line parameters
// =============================================
// read command line parameters
    int i = 1;
    while( i++ < argc )
    {
        string iTemp = argv[i-1];
        string iTemp2 = "";
        if( i < argc ) iTemp2 = argv[i];
        if( iTemp.find( "-help" ) < iTemp.size() )
        {
            printHelp();
            return false;
        }
        if( ( iTemp.find( "-input" ) < iTemp.size() || iTemp.find( "-sourcefile" ) < iTemp.size() ) 
	   && !( iTemp.find( "-inputfilelist" ) < iTemp.size() ) )
        {
            if( iTemp2.size() > 0 )
            {
		inputfile.push_back( iTemp2 );
                i++;
            }
        }
	else if( iTemp.find( "-inputfilelist" ) < iTemp.size() )
	{
            if( iTemp2.size() > 0 )
            {
	       fillInputFile_fromList( iTemp2 );
	       i++;
            }
	}
        else if( iTemp.find( "-mctable" ) < iTemp.size() )
        {
            cout << "THIS COMMAND LINE OPTION IS OBSOLET !!" << endl;
            cout << "(observe: changed handling of lookup tables with version 0.3" << endl;
            cout << endl;
            return false;
        }
        else if( iTemp.find( "-o" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                outputfile = iTemp2;
                i++;
            }
        }
        else if( iTemp.find( "printrunparameters" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                printpara = iTemp2;
                i++;
            }
            return true;
        }
        else if( iTemp.find( "useMedian" ) < iTemp.size() )
        {
            fUseMedianEnergy = (bool)atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "noise" ) < iTemp.size() )
        {
            fNoiseLevel = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
	else if( iTemp.find( "minshowerperbin" ) < iTemp.size() )
	{
	    fMinRequiredShowerPerBin = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "woff" ) < iTemp.size() )
        {
            fWobbleOffset = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
// wobble offset is given in float
            if( fWobbleOffset < 10 )
            {
                fWobbleOffset = (int)( atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() )* 1000+0.5 );
            }
        }
        else if( iTemp.find( "-table" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                tablefile = iTemp2;
                i++;
            }
        }
        else if( iTemp.find( "-esysfi" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                esysfile = iTemp2;
                i++;
            }
        }
        else if( iTemp.find( "-interpolate" ) < iTemp.size() )
        {
            fInterpolate = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-fill" ) < iTemp.size() )
        {
            int iT = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( iT == 1 )      readwrite = 'W';
            else if( iT == 0 ) readwrite = 'R';
            else
            {
                cout << "unknown parameter, choose 1=fill or 2=read lookup tables" << endl;
                return false;
            }
        }
        else if( iTemp.find( "-ze" ) < iTemp.size() )
        {
            ze = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-selectRandom" ) < iTemp.size() && !(iTemp.find( "-selectRandomSeed" ) < iTemp.size()) )
        {
            fSelectRandom = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( fSelectRandom > 1. || fSelectRandom < 0. )
            {
                cout << "Error: probability has to be in [0,1]: " << fSelectRandom << endl;
                return false;
            }
        }
        else if( iTemp.find( "-selectRandomSeed" ) < iTemp.size() )
        {
            fSelectRandomSeed = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-maxdist" ) < iTemp.size() && !(iTemp.find( "-maxdistancetocameracenter" ) < iTemp.size()) )
        {
            fmaxdist = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-maxCoreError" ) < iTemp.size() )
        {
            fTableFillingCut_CoreError_max = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-minImages" ) < iTemp.size() )
        {
            fTableFillingCut_NImages_min = (unsigned int)atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-spectralIndex" ) < iTemp.size() )
        {
            fSpectralIndex = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-maxlocaldist" ) < iTemp.size() )
        {
            fmaxlocaldistance = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
	else if( iTemp.find( "-maxdistancetocameracenter" ) < iTemp.size() )
	{
	    fMC_distance_to_cameracenter_max  = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
	else if( iTemp.find( "-mindistancetocameracenter" ) < iTemp.size() )
	{
	    fMC_distance_to_cameracenter_min  = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
// looking at squared differences!!
	    if( fMC_distance_to_cameracenter_min < 0. )
	    {
	       fMC_distance_to_cameracenter_min = 0.;
            }
        }
        else if( iTemp.find( "-minsize" ) < iTemp.size() )
        {
            fminsize = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-debug" ) < iTemp.size() )
        {
            fDebug = (unsigned int)atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-arrayrecid" ) < iTemp.size() || iTemp.find( "-recid" ) < iTemp.size() )
        {
            rec_method = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-update" ) < iTemp.size() )
        {
            bool iT = (bool)atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( iT ) writeoption = "update";
            else     writeoption = "recreate";
        }
        else if( iTemp.find( "-sizemscwcorrect" ) < iTemp.size() )
        {
            fMSCWSizecorrection = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-sizemsclcorrect" ) < iTemp.size() )
        {
            fMSCLSizecorrection = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-sizeenergycorrect" ) < iTemp.size() )
        {
            fEnergySizecorrection = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "-noNo" ) < iTemp.size() )
        {
            bNoNoTrigger = false;
        }
        else if( iTemp.find( "-writeReconstructedEventsOnly" ) < iTemp.size() )
        {
	    if( iTemp.rfind( "=" ) != string::npos )
	    {
	       bWriteReconstructedEventsOnly = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            }
	    else bWriteReconstructedEventsOnly = 0;
        }
        else if( iTemp.find( "-short" ) < iTemp.size() )
        {
            bShortTree = true;
        }
        else if( iTemp.find( "-pe" ) < iTemp.size() )
        {
            fPE = true;
        }
        else if( iTemp.find( "-nomctree" ) < iTemp.size() )
        {
            bWriteMCPars = false;
        }
        else if( iTemp.find( "-write1DHistograms" ) < iTemp.size() )
        {
            fWrite1DHistograms = true;
        }
        else if( iTemp.find( "maxnevents" ) < iTemp.size() )
        {
            fNentries = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "maxruntime" ) < iTemp.size() )
        {
            fMaxRunTime = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
    }
// filling of tables requires Monte Carlo
    if( readwrite == 'W' )
    {
        isMC = true;
    }
// =============================================
// end of reading command line parameters
// =============================================

// require inputfile name
    if( inputfile.size() == 0 )
    {
        cout << "error: no input file" << endl;
        cout << "...exiting" << endl;
        return false;
    }
// require table file
    if( tablefile.size() == 0 )
    {
        cout << "error: no lookup table file" << endl;
        cout << "...exiting" << endl;
        return false;
    }

// set output file name (mainly for VTS analysis with a single inputfile)
    if( outputfile.size() == 0 && inputfile.size() == 1 )
    {
        if( inputfile[0].find( "*" ) < inputfile[0].size() )
        {
            outputfile = "mscw.root";
        }
        else
        {
            outputfile = inputfile[0].substr( 0, inputfile[0].rfind( "." ) );
            outputfile += ".mscw.root";
        }
    }

    return true;
}


void VTableLookupRunParameter::printHelp()
{
    if( gSystem->Getenv( "EVNDISPSYS" ) )
    {
       system( "cat $EVNDISPSYS/README/README.MSCW_ENERGY" );
    }
    else
    {
       cout << "VTableLookupRunParameter::printHelp() no help available (environmental variable EVNDISPSYS not set)" << endl;
    }
    return;
}


void VTableLookupRunParameter::print( int iP )
{
    cout << "mscw_energy VERSION " << getEVNDISP_VERSION() << endl;
    cout << endl;
    cout << "debug level " << fDebug << endl;
    cout << "lookuptable: " << tablefile << endl;
    cout << endl;
    cout << "evndisp reconstruction parameter ID: " << rec_method << endl;
    cout << endl;
    cout << "input file(s): ";
    for( unsigned int i = 0; i < inputfile.size(); i++ )
    {
       cout << "\t" << inputfile[i] << endl;
    }
    if( isMC ) cout << " (input data is MC)";
    if( fPE ) cout << " (input data is PE)";
    cout << endl;
    if( readwrite != 'W' && readwrite != 'w' ) cout << "output file: " << outputfile << endl;
    if( bWriteReconstructedEventsOnly >=0  ) cout << "(writing reconstructed events only (" << bWriteReconstructedEventsOnly << "))" << endl;
    if( readwrite == 'W' || readwrite == 'w' )
    {
        cout << "filling lookup tables for: ";
        cout << " zenith " << ze << ", direction offset " << fWobbleOffset << " [deg], ";
        cout << "noise level " << fNoiseLevel << ", spectral index " << fSpectralIndex << endl;
        if( fWrite1DHistograms ) cout << "write 1D histograms to disk" << endl;
	cout << "\t minimum telescope multiplicity: " << fTableFillingCut_NImages_min << endl;
        cout << "\t maximum allowed uncertainty in core reconstruction [m]: " << fTableFillingCut_CoreError_max << endl;
	cout << "\t distance to camera: > " << fMC_distance_to_cameracenter_min << " [deg], <" << fMC_distance_to_cameracenter_max << " [deg]" << endl;
    }
    if( iP == 2 ) cout << "zenith angle " << ze << " [deg], wobble offset " << fWobbleOffset/100. << " [deg], noise level " << fNoiseLevel << endl;
//    cout << "cuts: " << endl;
//    cout << "\t maximum impact parameter distance [m]: " << fmaxdist << endl;
//    cout << "\t minimum size per telescope: " << fminsize << endl;
//    cout << "\t maximum local distance of image [deg]: " << fmaxlocaldistance << endl;
    if( fSelectRandom > 0. ) cout << "random event selection: " << fSelectRandom << ", seed:" << fSelectRandomSeed << endl;
    if( fUseSelectedImagesOnly ) cout << "use evndisp image selection" << endl;
    else                         cout << "use all images" << endl;
    if( !(readwrite == 'W' || readwrite == 'w' ) || iP == 2 )
    {
        if( esysfile.size() > 0 ) cout << "correct for systematic errors with " << esysfile << endl;
    }
    else
    {
	cout << "minimum number of showers required per bin: " << fMinRequiredShowerPerBin << endl;
    }
    if( fUseMedianEnergy ) cout << "use median of energy distributions" << endl;
    else                   cout << "use mean of energy distributions" << endl;
    if( fInterpolate > 0 )
    {
        if( fInterpolate == 1 ) fInterpolateString = "simple";
        else if( fInterpolate == 2 ) fInterpolateString = "gaussian";
        cout << "interpolate lookup tables: " << fInterpolateString << endl;
    }
    if( TMath::Abs( fMSCWSizecorrection -1. ) > 1.e-2 )    cout << "size correction for mscw tables: " << fMSCWSizecorrection << endl;
    if( TMath::Abs( fMSCLSizecorrection -1. ) > 1.e-2 )    cout << "size correction for mscl tables: " << fMSCLSizecorrection << endl;
    if( TMath::Abs( fEnergySizecorrection - 1. ) > 1.e-2 ) cout << "size correction for energy tables: " << fEnergySizecorrection << endl;

    if( iP >= 1 )
    {
        cout << endl;
        if( meanpedvars > 0. )
        {
            cout << "mean pedvars: " << meanpedvars << endl;
            cout << "mean pedvars per telescope: ";
            for( unsigned int i = 0; i < pedvars.size(); i++ ) cout << pedvars[i] << "/";
            cout << endl;
        }
        else cout << "no pedvar information available" << endl;
    }
    if( TMath::Abs( fDeadTimeFraction - 0.5 ) > 1.e-3 )
    {
        cout << "calculated dead time fraction: " << fDeadTimeFraction << endl;
    }
}

bool VTableLookupRunParameter::fillInputFile_fromList( string iList )
{
   ifstream is;
   is.open( iList.c_str(), ifstream::in );
   if( !is )
   {
      cout << "VTableLookupRunParameter::fillInputFile_fromList() error reading list of input files: " << endl;
      cout << iList << endl;
      cout << "exiting..." << endl;
      exit( -1 );
   }
   cout << "VTableLookupRunParameter::fillInputFile_fromList() reading input file list: " << endl;
   cout << iList << endl;
   string iLine;
   while( getline( is, iLine ) )
   {
      if( iLine.size() > 0 )
      {
         inputfile.push_back( iLine );
      }
   }
   is.close();

   cout << "total number of input files " << inputfile.size() << endl;

   return true;
}
