/*! \class VArrayAnalyzer

     VArrayAnalyzer class for analyzing VERITAS data (full array analysis)

    \author
      Gernot Maier

      Revision $Id: VArrayAnalyzer.cpp,v 1.73.2.9.4.12.4.1.4.6.2.3.4.3.2.1.2.10.2.10.2.4.2.1.2.12.2.22.2.3.2.5.2.1 2011/04/21 10:03:36 gmaier Exp $
*/

#include "VArrayAnalyzer.h"

VArrayAnalyzer::VArrayAnalyzer()
{
    fDebug = getDebugFlag();
    if( fDebug ) cout << "VArrayAnalyzer::VArrayAnalyzer()" << endl;

    fInitialized = false;

// set up data storage class (all analysis results are stored here)
    fShowerParameters = new VShowerParameters( getNTel(), getRunParameter()->fShortTree );
// set up MC data storage class
    fMCParameters = new VMCParameters( fDebug );
// test if number of telescopes exceeds value in fShowerParameters
    if( getNTel() > fShowerParameters->getMaxNTelescopes() )
    {
        cout << "VArrayAnalyzer::VArrayAnalyzer(): error, to many telescopes ";
        cout << getNTel() << "\t" << fShowerParameters->getMaxNTelescopes() << endl;
        cout << "\t adjust fMaxNbrTel in VShowerParameters" << endl;
        exit( -1 );
    }

}


VArrayAnalyzer::~VArrayAnalyzer()
{

}


/*!
     called for every event
*/
void VArrayAnalyzer::doAnalysis()
{
    if( fDebug ) cout << "void VArrayAnalyzer::doAnalysis()" << endl;

// only at first call in the analysis run: initialize data class, set trees
    if( !fInitialized )
    {
        initAnalysis();
        fInitialized = true;
    }

// fill simulation data
    if( isMC() ) fillSimulationEvent();

// return if triggered only events are written to output
    if( getRunParameter()->fWriteTriggerOnly && !fReader->hasArrayTrigger() ) return;

// reset data vectors, etc.
    initEvent();

    if( isMC() )
    {
// fill shower parameter tree
        if( getShowerParameters() )
	{
	   getShowerParameters()->setMC();
	   getShowerParameters()->MCprimary = fReader->getMC_primary();
	   getShowerParameters()->MCenergy  = fReader->getMC_energy();
	   getShowerParameters()->MCxcore   = fReader->getMC_X();
	   getShowerParameters()->MCycore   = fReader->getMC_Y();
	   getShowerParameters()->MCxcos    = fReader->getMC_Xcos();
	   getShowerParameters()->MCycos    = fReader->getMC_Ycos();
	   getShowerParameters()->MCTel_Xoff= fReader->getMC_Xoffset();
	   getShowerParameters()->MCTel_Yoff= fReader->getMC_Yoffset();
	   getShowerParameters()->MCze = fReader->getMC_Ze();
	   getShowerParameters()->MCaz = fReader->getMC_Az();
       }
    }

// added C.Duke 21dec06
// move source location info into ShowerParameter class
    if( getTeltoAna()[0] < getPointing().size() && getPointing()[getTeltoAna()[0]] )
    {
       getShowerParameters()->fTargetElevation = getPointing()[getTeltoAna()[0]]->getTargetElevation();
       getShowerParameters()->fTargetAzimuth = getPointing()[getTeltoAna()[0]]->getTargetAzimuth();
       getShowerParameters()->fTargetDec = getPointing()[getTeltoAna()[0]]->getTargetDec();
       getShowerParameters()->fTargetRA = getPointing()[getTeltoAna()[0]]->getTargetRA();
       getShowerParameters()->fWobbleNorth = getPointing()[getTeltoAna()[0]]->getWobbleNorth();
       getShowerParameters()->fWobbleEast = getPointing()[getTeltoAna()[0]]->getWobbleEast();
    }
    else
    {
       getShowerParameters()->fTargetElevation = 0.;
       getShowerParameters()->fTargetAzimuth = 0.;
       getShowerParameters()->fTargetDec = 0.;
       getShowerParameters()->fTargetRA = 0.;
       getShowerParameters()->fWobbleNorth = 0.;
       getShowerParameters()->fWobbleEast = 0.;
    }

    for( unsigned int i = 0; i < getNTel(); i++ )
    {
// get pointing direction
        if( i < getPointing().size() && getPointing()[i] )
	{
	   getShowerParameters()->fTelElevation[i] = getPointing()[i]->getTelElevation();
	   getShowerParameters()->fTelAzimuth[i] = getPointing()[i]->getTelAzimuth();
	   getShowerParameters()->fTelDec[i] = getPointing()[i]->getTelDec();
	   getShowerParameters()->fTelRA[i] = getPointing()[i]->getTelRA();
        }
	else
	{
	   getShowerParameters()->fTelElevation[i] = 0.;
	   getShowerParameters()->fTelAzimuth[i] = 0.;
	   getShowerParameters()->fTelDec[i] = 0.;
	   getShowerParameters()->fTelRA[i] = 0.;
        }
// compare calculated pointing with vbf pointing
        checkPointing();
    }

// do array analysis only if there is an array trigger
    if( getReader()->hasArrayTrigger() )
    {
// transform telescope positions into shower coordinates (direction is telescope orientation)
        for( unsigned int i = 0; i < getNTel(); i++ )
        {
            if( i < getPointing().size() && getPointing()[i] )
	    {
	       transformTelescopePosition( i, 90.-getPointing()[i]->getTelElevation(), getPointing()[i]->getTelAzimuth(), fReader->isMC() && (i==getTeltoAna()[0]) );
            }
        }
//////////////////////////////////////////////////////////////////////////////////////////////
// calculate shower direction and shower core position
        calcShowerDirection_and_Core();
//////////////////////////////////////////////////////////////////////////////////////////////
    }

// calculate RA and dec
    for( unsigned int i = 0; i < getShowerParameters()->fNMethods; i++ )
    {
// test for successfull reconstruction (zenith angle > 0.)
        if( getShowerParameters()->fShowerZe[i] < 0. ) continue;
// y-axis flip in calculation of yoff
        if( i < getPointing().size() && getPointing()[i] )
	{
	   getShowerParameters()->fDec[i] = getPointing()[getTeltoAna()[0]]->getTelDec() + getShowerParameters()->fShower_YoffsetDeRot[i];
	   if( cos(getShowerParameters()->fDec[i]/TMath::RadToDeg()) != 0. )
	   {
	       getShowerParameters()->fRA[i] = getPointing()[getTeltoAna()[0]]->getTelRA() + getShowerParameters()->fShower_XoffsetDeRot[i]/cos(getShowerParameters()->fDec[i]/TMath::RadToDeg() );
	   }
	   else getShowerParameters()->fRA[i] = getPointing()[getTeltoAna()[0]]->getTelRA();
	}
	else
	{
	   getShowerParameters()->fDec[i] = 0.;
	   getShowerParameters()->fRA[i] = 0.;
        }
    }
    getShowerParameters()->eventStatus = getAnalysisArrayEventStatus();
// fill shower parameter tree with results
    getShowerParameters()->getTree()->Fill();

}


/*!
     this routine is called for each event before the event analysis
*/
void VArrayAnalyzer::initEvent()
{
    if( fDebug ) cout << "void VArrayAnalyzer::initEvent()" << endl;

    setAnalysisArrayEventStatus( 0 );

// reset shower parameters
    getShowerParameters()->reset( getNTel() );

// get basic infos for this event
    getShowerParameters()->runNumber = getRunNumber();
    getShowerParameters()->eventNumber = getEventNumber();
    getShowerParameters()->fsourcetype = fReader->getDataFormatNum();
    getShowerParameters()->fNTelescopes = getNTel();

// get event times
    if( getTeltoAna().size() > 0 && getTeltoAna()[0] < getEventTimeVector().size() ) getShowerParameters()->time = getEventTimeVector()[getTeltoAna()[0]];
    else
    {
       cout << "VArrayAnalyzer::initEvent() warning: no event times" << endl;
    }
    if( getTeltoAna().size() > 0 && getTeltoAna()[0] < getEventMJDVector().size() ) getShowerParameters()->MJD  = getEventMJDVector()[getTeltoAna()[0]];
    else
    {
       cout << "VArrayAnalyzer::initEvent() warning: no event times" << endl;
    }

// fix pointing with time from majority vote
    if( !fReader->isMC() )
    {
        for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
        {
	    if( getTeltoAna()[i]  < getPointing().size() && getPointing()[getTeltoAna()[i]] )
	    {
	       getPointing()[getTeltoAna()[i]]->setTelPointing( getShowerParameters()->MJD, getShowerParameters()->time, true );
            }
        }
    }
/////////////////////////////////////////////////////////////////////

    getShowerParameters()->fTraceFit = ((int)getTraceFit())*100;

// get number of telescopes with local trigger
// (works only for relatively small number of telescopes)
    bitset<8*sizeof(unsigned long)> i_ntrigger;
    if( fNTel < i_ntrigger.size() )
    {
        if( fReader->getLocalTrigger().size() >= fNTel )
        {
            for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
            {
                if( fReader->getLocalTrigger()[getTeltoAna()[i]] )
                {
                    if( getTeltoAna()[i] < i_ntrigger.size() ) i_ntrigger.set( getTeltoAna()[i], 1 );
                }
            }
        }
        getShowerParameters()->fLTrig = i_ntrigger.to_ulong();
    }
    else
    {
        getShowerParameters()->fLTrig = 0;
    }
// list of telescopes with local trigger
    if( fReader->getLocalTrigger().size() >= fNTel )
    {
        unsigned int ztrig = 0;
        for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
        {
            if( fReader->getLocalTrigger()[getTeltoAna()[i]] )
            {
                getShowerParameters()->fTrig_list[ztrig] = getTeltoAna()[i];
                ztrig++;
            }
        }
        getShowerParameters()->fNTrig = ztrig;
    }
}


/*!
    this routine is called once at the beginning of the analysis
*/
void VArrayAnalyzer::initAnalysis()
{
    if( fDebug ) cout << "void VArrayAnalyzer::initAnalysis()" << endl;

    fMeanPointingMismatch.assign( getNTel(), 0. );
    fNMeanPointingMismatch.assign( getNTel(), 0. );

// loop over all methods and read in necessary MLPs, etc....
    for( unsigned int i = 0; i < getShowerParameters()->fNMethods; i++ )
    {
// set reconstruction method
        if( i < getArrayAnalysisCuts()->fMethodID.size() )
        {
            if( getArrayAnalysisCuts()->fMethodID[i] == 5 )
            {
                fMLPAnalyzer.push_back( new VMLPAnalyzer( getArrayAnalysisCuts()->fMLPFileName[i] ) );
                if( fMLPAnalyzer.back()->isZombie() )
                {
                    cout << "VArrayAnalyzer::initAnalysis() error while initializing MLP for ";
                    cout << getArrayAnalysisCuts()->fMLPFileName[i] << endl;
                    exit( -1 );
                }
            }
            else fMLPAnalyzer.push_back( 0 );
            if( getArrayAnalysisCuts()->fMethodID[i] == 6 || getArrayAnalysisCuts()->fMethodID[i] == 7 || getArrayAnalysisCuts()->fMethodID[i] == 8 )
            {
		cout << "init array analysis for analysis cut set " << fDispAnalyzer.size();
		cout << " and reconstruction method " << getArrayAnalysisCuts()->fMethodID[i] << endl;
                fDispAnalyzer.push_back( new VDispTableAnalyzer( getArrayAnalysisCuts()->fDispFileName[i] ) );
                if( fDispAnalyzer.back()->isZombie() )
                {
                    cout << "VArrayAnalyzer::initAnalysis() error while initializing MLP for ";
                    cout << getArrayAnalysisCuts()->fDispFileName[i] << endl;
                    exit( -1 );
                }
            }
            else fDispAnalyzer.push_back( 0 );
        }
    }

// init output file
    initOutput();
// init data trees
    initTree();

}


/*!
   open root outputfile (to be called once before starting the array analysis)
*/
void VArrayAnalyzer::initOutput()
{
    if( fDebug ) cout << "void VArrayAnalyzer::initOuput()" << endl;
// check if root outputfile exist
    if( fOutputfile != 0 ) return;
// otherwise create it
    if( fRunPar->foutputfileName != "-1" )
    {
// tree versioning numbers used in mscw_energy
        char i_textTitle[300];
        sprintf( i_textTitle, "VERSION %d", getRunParameter()->getEVNDISP_TREE_VERSION() );
        if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
        fOutputfile = new TFile( fRunPar->foutputfileName.c_str(), "RECREATE", i_textTitle );
    }
}


/*!
   create a new tree in the top directory of the analysis output file
   (to be called once before starting the array analysis)
*/
void VArrayAnalyzer::initTree()
{
    if( fDebug ) cout << "void VArrayAnalyzer::initTrees() " <<  endl;

    fOutputfile->cd();
// now book the tree (for MC with additional MC block)
    char i_text[300];
    char i_textTitle[300];
    sprintf( i_text, "showerpars" );
// tree versioning numbers used in mscw_energy
    sprintf( i_textTitle, "Shower Parameters (VERSION %d)", getRunParameter()->getEVNDISP_TREE_VERSION() );
    if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
    fShowerParameters->initTree( i_text, i_textTitle, fReader->isMC() );
    if( isMC() && fMCParameters ) fMCParameters->initTree();
}


/*!
    write result tree into ROOT output file
    (called once after finishing the analysis)
*/
void VArrayAnalyzer::terminate()
{
    if( fDebug ) cout << "void VArrayAnalyzer::terminate()" << endl;

    for( unsigned int i = 0; i < fMLPAnalyzer.size(); i++ )
    {
        if( fMLPAnalyzer[i] ) fMLPAnalyzer[i]->terminate();
    }
    for( unsigned int i = 0; i < fDispAnalyzer.size(); i++ )
    {
        if( fDispAnalyzer[i] ) fDispAnalyzer[i]->terminate();
    }

    if( fOutputfile != 0 && fRunPar->foutputfileName != "-1" )
    {

// write pointing mismatch
        if( !getNoPointing() && getTeltoAna()[0] < getPointing().size() && getPointing()[getTeltoAna()[0]]->getTargetName() != "laser" && !fReader->isMC() )
        {
            cout << endl;
            cout << "Pointing check results (using VBF data):" << endl;
            for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
            {
                if( getMeanPointingMismatch( getTeltoAna()[i] ) > 0.1 )
                {
                    cout << endl;
                    cout << "WARNING: LARGE MISMATCH BETWEEN EVENTDISPLAY AND VBF POINTING DATA FOR TELESCOPE " << getTeltoAna()[i]+1;
                    cout << " (mean mismatch is " << getMeanPointingMismatch( getTeltoAna()[i] ) << " deg)" << endl;
                    cout << "\t IS THE WOBBLE OFFSET RIGHT?" << endl;
                }
                else
                {
                    cout << "\t mean pointing mismatch between eventdisplay and VBF pointing data for Telescope " << getTeltoAna()[i]+1 << ": ";
                    cout << getMeanPointingMismatch( getTeltoAna()[i] ) << " deg" << endl;
                }
            }
        }
        fOutputfile->cd();

        if( !getShowerParameters()->getTree() ) return;
        cout << "---------------------------------------------------------------------------------------------------------" << endl;
        cout << "writing analysis tree (";
        cout << getShowerParameters()->getTree()->GetEntries();
        cout << " entries) to : " << fOutputfile->GetName() << endl;
        getShowerParameters()->getTree()->Write();
// MC tree and histograms
        if( isMC() )
        {
// get MC tree
            TTree *i_tMC = 0;
            if( getRunParameter()->fsourcetype == 7 )
            {
                if( getReader()->getMCTree() )
                {
                    if( getRunParameter()->fwriteMCtree )
		    {
		       if( getRunParameter()->fwriteMCtree ) i_tMC = (TTree*)getReader()->getMCTree()->CloneTree( -1, "fast" );
		       if( i_tMC ) i_tMC->SetName( "MCpars" );
                    }
                    else                                  i_tMC = (TTree*)getReader()->getMCTree();
                }
            }
            else
            {
                if( getMCParameters()->getTree() ) i_tMC = getMCParameters()->getTree();
            }
	    if( getRunParameter()->fwriteMCtree && i_tMC )
	    {
		 cout << "writing MC tree " << endl;
	         i_tMC->Write();
		 cout << "(number of MC events in tree " << i_tMC->GetName() << ": " << i_tMC->GetEntries() << ")" << endl;
   	         cout << "...done" << endl;
            }
	    if( getRunParameter()->fFillMCHistos && i_tMC )
	    {
	        cout << "filling MC histograms" << endl;
		VEffectiveAreaCalculatorMCHistograms iMC_histos;
		if( getReader()->getMonteCarloHeader() )
		{
		   iMC_histos.setMonteCarloEnergyRange( getReader()->getMonteCarloHeader()->E_range[0], getReader()->getMonteCarloHeader()->E_range[1],
		                                         TMath::Abs( getReader()->getMonteCarloHeader()->spectral_index ) );
                }
		iMC_histos.setDefaultValues();
		iMC_histos.initializeHistograms();
		iMC_histos.fill( getReader()->getMonteCarloHeader()->getMeanZenithAngle_Deg(), i_tMC, true );
		iMC_histos.print();
		iMC_histos.Write();
            }
        }
        if( getArrayAnalysisCuts() ) getArrayAnalysisCuts()->Write();
	if( fOutputfile ) fOutputfile->Flush();
        cout << "---------------------------------------------------------------------------------------------------------" << endl;
    }
}


/*!
          use GrIsu routines for the actual transformation

      \param iTel telescope number
      \param i_ze zenith angle [deg]
      \param i_az azimuth angle [deg]
      \param i_MC data is MC data (true)
*/
void VArrayAnalyzer::transformTelescopePosition( int iTel, float i_ze, float i_az, bool i_MC )
{
// transform telescope positions from ground into shower coordinates
    float i_xrot, i_yrot, i_zrot;
    float i_xcos = 0.;
    float i_ycos = 0.;
// calculate direction cosinii
    i_xcos = sin(i_ze / TMath::RadToDeg() ) * sin( (i_az-180.)/TMath::RadToDeg() );
    i_ycos = sin(i_ze / TMath::RadToDeg() ) * cos( (i_az-180.)/TMath::RadToDeg() );

    setTelID( iTel );
// call to GrIsu routine
    tel_impact( i_xcos, i_ycos, getDetectorGeo()->getTelXpos()[iTel], getDetectorGeo()->getTelYpos()[iTel], getDetectorGeo()->getTelZpos()[iTel], &i_xrot, &i_yrot, &i_zrot, false );

    getImageParameters()->Tel_x_SC = i_xrot;
    getImageParameters()->Tel_y_SC = i_yrot;
    getImageParameters()->Tel_z_SC = i_zrot;

    if( getRunParameter()->fImageLL )
    {
        getImageParametersLogL()->Tel_x_SC = i_xrot;
        getImageParametersLogL()->Tel_y_SC = i_yrot;
        getImageParametersLogL()->Tel_z_SC = i_zrot;
    }

    getShowerParameters()->fTel_x_SC[iTel]=i_xrot;
    getShowerParameters()->fTel_y_SC[iTel]=i_yrot;
    getShowerParameters()->fTel_z_SC[iTel]=i_zrot;

    if( i_MC )
    {
// transformation of Monte Carlo shower core into shower coordinates
// call to GrIsu routine
        tel_impact( i_xcos, i_ycos, getShowerParameters()->MCxcore, getShowerParameters()->MCycore, getShowerParameters()->MCzcore, &i_xrot, &i_yrot, &i_zrot, false );
        getShowerParameters()->MCxcore_SC = i_xrot;
        getShowerParameters()->MCycore_SC = i_yrot;
        getShowerParameters()->MCzcore_SC = i_zrot;
    }
}


/*!
     select images used in shower reconstruction

     cuts possible on ntubes, dist, alpha (set with in files with arraycuts,
     see example array_analysis_cuts.txt)

     results stored in boolean vector with length of number of telescopes

     \par iMeth shower reconstruction method number
*/
void VArrayAnalyzer::selectShowerImages( unsigned int iMeth )
{
    bool fSelectDebug = false;

    getShowerParameters()->fTelIDImageSelected[iMeth].clear();
    getShowerParameters()->fTelIDImageSelected_bitcode[iMeth] = 0;
    getShowerParameters()->fShowerNumImages[iMeth]= 0;

    for( unsigned int t = 0; t < getNTel(); t++ )
    {
        setTelID( t );

// reset list with selected images
        getShowerParameters()->fTelIDImageSelected_list[iMeth][t] = 0;
        getShowerParameters()->fTelIDImageSelected[iMeth].push_back( true );

        if( fSelectDebug ) cout << "VArrayAnalyzer::selectShowerImages " << getTelID()+1 << "\t" << getEventNumber() << "\t" << iMeth << endl;
        if( fSelectDebug ) cout << "VArrayAnalyzer::selectShowerImages eventstatus " << getImageParameters( getRunParameter()->fImageLL )->eventStatus << endl;

// get telescope type for this telescope
	int iTelType = fArrayAnalysisCuts->getTelescopeType_counter( getDetectorGeometry()->getTelType()[t] );
	if( iTelType < 0 )
	{
	   cout << "VArrayAnalyzer::selectShowerImages error: invalid telescope counter: " << t << "\t" << iTelType << endl;
	   exit( -1 );
	   continue;
        }
// apply array analysis cuts
        getShowerParameters()->fTelIDImageSelected[iMeth].back() = fArrayAnalysisCuts->applyArrayAnalysisCuts( iMeth, iTelType, getImageParameters( getRunParameter()->fImageLL ) );

///////////////////////////

// check if fit was successfull
        if( getRunParameter()->fImageLL && getImageParametersLogL()->Fitstat < 3 )  getShowerParameters()->fTelIDImageSelected[iMeth].back() = false;

// list of selected images
        if( getShowerParameters()->fTelIDImageSelected[iMeth].back() )
        {
            getShowerParameters()->fTelIDImageSelected_list[iMeth][getTelID()] = 1;
            getShowerParameters()->fShowerNumImages[iMeth]++;
        }

// C. Duke 19Oct06  bitmapped list of images
        bitset<8*sizeof(unsigned long)> i_nimage;
        if( fNTel < i_nimage.size() )
        {
            for( unsigned int i = 0; i < getNTel(); i++ )
            {
                if( i < i_nimage.size() )
                {
                    if( getShowerParameters()->fTelIDImageSelected[iMeth][i] ) i_nimage.set(i,1);
                    else i_nimage.set(i,0);
                }
            }
            getShowerParameters()->fTelIDImageSelected_bitcode[iMeth] = i_nimage.to_ulong();
        }
        else getShowerParameters()->fTelIDImageSelected_bitcode[iMeth] = 0;
    }
}


void VArrayAnalyzer::calcShowerDirection_and_Core()
{
    if( fDebug ) cout << "VArrayAnalyzer::calcShowerDirection_and_Core()" << endl;

// loop over all methods
    for( unsigned int i = 0; i < getShowerParameters()->fNMethods; i++ )
    {
// set reconstruction method
        if( i < getArrayAnalysisCuts()->fMethodID.size() )
        {
            getShowerParameters()->fMethodID[i] = getArrayAnalysisCuts()->fMethodID[i];
// select shower images to be used to determinate of shower coordinates
            selectShowerImages( i );

// call reconstruction method
            if( getArrayAnalysisCuts()->fMethodID[i] == 0 )      rcs_method_0( i );
            else if( getArrayAnalysisCuts()->fMethodID[i] == 3 ) rcs_method_3( i );
            else if( getArrayAnalysisCuts()->fMethodID[i] == 4 ) rcs_method_4( i );
            else if( getArrayAnalysisCuts()->fMethodID[i] == 5 ) rcs_method_5( i, 5 );
            else if( getArrayAnalysisCuts()->fMethodID[i] == 6 ) rcs_method_5( i, 6 );
            else if( getArrayAnalysisCuts()->fMethodID[i] == 7 ) rcs_method_7( i );
            else if( getArrayAnalysisCuts()->fMethodID[i] == 8 ) rcs_method_8( i );
            else
            {
                cout << "VArrayAnalyzer::calcShowerDirection_and_Core(): unknown array reconstruction method " << getArrayAnalysisCuts()->fMethodID[i] << endl;
                continue;
            }
        }
        else
        {
            cout << "VArrayAnalyzer::calcShowerDirection_and_Core(): error, analysis vectors with different size" << endl;
            cout << "...fatal error" << endl;
            exit( 0 );
        }
    }
}


/*!
      reconstruction of shower direction and core

      shower direction by intersection of image axes
      shower core by intersection of lines connecting reconstruced shower
      direction and image centroids

      basic code is a copy from Charlie Duke's routines in GrIsu

      adjustments:
      * technical stuff to run in the eventdisplay frame
* weighting
*/
int VArrayAnalyzer::rcs_method_0( unsigned int iMethod )
{
    if( fDebug ) cout << "VArrayAnalyzer::rcs_method_0 " << iMethod << endl;

/* Requires phi, xc, yc, and sum image parameters

   use image parameters, phi, xc, and yc to determine image
   axes. Use rcs_perpendicular_fit to find source point.

   find impact point by rotation through delta to position
   source point at camera center, then find impact point
   using ground lines connecting the reconstructed shower
direction with the image centroids. Impact
   point found using rcs_perpendicular_fit.
*/
    int num_images = 0;                           /* number of images included in the fit */

    float xs = 0.;
    float ys = 0.;
    float stds = 0.;

    float ximp = 0.;                              /* impact point, from perpen. minimization */
    float yimp = 0.;
    float stdp = 0.;

    num_images = getShowerParameters()->fShowerNumImages[iMethod];

// are there enough images the run an array analysis
    if( num_images >= (int)fArrayAnalysisCuts->fNImages_min[iMethod] )
    {
        prepareforDirectionReconstruction( iMethod, 0 );
    }
    else
    {
        fillShowerDirection( iMethod, 0., 0., -1. );
        return 0;
    }

// don't do anything if angle between image axis is too small (for 2 images only)
    if( num_images == 2 )
    {
        float iangdiff = fabs(atan(m[0])-atan(m[1]));
        getShowerParameters()->fiangdiff[iMethod] = iangdiff*TMath::RadToDeg();
        if( iangdiff < fArrayAnalysisCuts->fAxesAngles_min[iMethod]/TMath::RadToDeg() || fabs(180./TMath::RadToDeg()-iangdiff) < fArrayAnalysisCuts->fAxesAngles_min[iMethod]/TMath::RadToDeg() )
        {
            getShowerParameters()->fShower_Chi2[iMethod] = -1.*fabs(atan(m[0])-atan(m[1]))*TMath::RadToDeg();
            return 0;
        }
    }
    else getShowerParameters()->fiangdiff[iMethod] = 0.;

///////////////////////////////
// direction reconstruction
///////////////////////////////
// calculate offset in camera coordinates

    rcs_perpendicular_fit(x, y, w, m, num_images, &xs, &ys, &stds);

// assume all focal lengths of all telescopes are the same
    xs = atan(xs/( 1000. * getDetectorGeo()->getFocalLength()[0] ) ) * TMath::RadToDeg();
    ys = atan(ys/( 1000. * getDetectorGeo()->getFocalLength()[0] ) ) * TMath::RadToDeg();
    stds = atan(stds/( 1000. * getDetectorGeo()->getFocalLength()[0] ) ) * TMath::RadToDeg();

// calculate and fill directions
    if( !fillShowerDirection( iMethod, xs, ys, stds ) ) return 0;

// end of direction reconstruction
////////////////////////////////////////////////
////////////////////////////////////////////////

/* - - - - - now find the impact location in the plane
    perpendicular to the array. Since we know the source location,
    we can rotate the array to point in this direction thus moving
    the source point to the center of the camera.  Source location
    will be at (0, 0).
*/
    prepareforCoreReconstruction( iMethod, xs, ys );

/* Now call perpendicular_distance for the fit, returning ximp and yimp */

    rcs_perpendicular_fit(x, y, w, m, num_images, &ximp, &yimp, &stdp);

    if( isnormal( ximp ) && isnormal( yimp ) )
    {
        fillShowerCore( iMethod, ximp, yimp );
        getShowerParameters()->fShower_stdP[iMethod] = stdp;
                                                  /* indicates fit ok */
        getShowerParameters()->fShower_Chi2[iMethod] = 0.0;
    }
// this happens sometimes for two-telescope events when image axis are exactly parallel
    else
    {
        getShowerParameters()->fShower_stdP[iMethod] = 0.;
        getShowerParameters()->fShower_Chi2[iMethod] = -2.;
    }

    return 0;
}


/***************** end of rcs_method_0 *********************************/

/*!
    \par iMeth reconstruction method number
    \par ximp shower core position in shower coordinates
    \par yimp shower core position in shower coordinates
*/
bool VArrayAnalyzer::fillShowerCore( unsigned int iMeth, float ximp, float yimp )
{
// check validity
    if( !isnormal( ximp ) || !isnormal( ximp ) )
    {
        ximp = -99999.;
        yimp = -99999.;
        return false;
    }
// reconstructed shower core in shower coordinates
    getShowerParameters()->fShowerXcore_SC[iMeth] = ximp;
    getShowerParameters()->fShowerYcore_SC[iMeth] = yimp;

// reconstructed shower core in ground coordinates
    float i_xcos = 0.;
    float i_ycos = 0.;
    float zimp = 0.;
    float igx, igy, igz;
// calculate z in shower coordinates (for z=0 in ground coordinates)
    if( getShowerParameters()->fShowerZe[iMeth] != 0. ) zimp = yimp / tan( (90.-getShowerParameters()->fShowerZe[iMeth])/TMath::RadToDeg() );
    else                                                zimp = 0.;
// calculate direction cosinii
// taking telescope plane as reference plane. This is not exactly correct for method 0
// taking pointing direction of first telescope in teltoana vector
    if( getTeltoAna()[0] < getPointing().size() && getPointing()[getTeltoAna()[0]] )
    {
       i_xcos = sin((90.-getPointing()[getTeltoAna()[0]]->getTelElevation())/ TMath::RadToDeg() ) * sin( (getPointing()[getTeltoAna()[0]]->getTelAzimuth()-180.)/TMath::RadToDeg() );
       if( fabs( i_xcos ) < 1.e-7 ) i_xcos = 0.;
       i_ycos = sin((90.-getPointing()[getTeltoAna()[0]]->getTelElevation())/ TMath::RadToDeg() ) * cos( (getPointing()[getTeltoAna()[0]]->getTelAzimuth()-180.)/TMath::RadToDeg() );
       if( fabs( i_ycos ) < 1.e-7 ) i_ycos = 0.;
    }
    else
    {
       i_xcos = 0.;
       i_ycos = 0.;
    }
    tel_impact( i_xcos, i_ycos, ximp, yimp, zimp, &igx, &igy, &igz, true );
    if( isinf( igx ) || isinf( igy ) )
    {
        igx = -99999;
        igy = -99999;
        getShowerParameters()->fShower_Chi2[iMeth] = -2.;
    }
    getShowerParameters()->fShowerXcore[iMeth] = igx;
    getShowerParameters()->fShowerYcore[iMeth] = igy;

// z coordinate should be zero in ground coordinate system, if not -> problem
    if( fabs( igz ) > 1.e3 ) return false;

    return true;
}


void VArrayAnalyzer::getNumImages()
{
    getShowerParameters()->fNumImages = 0;
    for( unsigned int i = 0; i < getNTel(); i++ )
    {
        setTelID( i );
        if( getImageParameters( getRunParameter()->fImageLL )->ntubes > 0 ) getShowerParameters()->fNumImages++;
    }
}


double VArrayAnalyzer::angDist( double Az, double Ze, double Traz, double Trze )
{
    double value;

    value  = sin( Ze ) * sin( Trze ) * cos( ( Az - Traz ) );
    value += cos( Ze ) * cos( Trze );
// limited accuracy results sometimes in values slightly larger than 1
    if( value > 1. ) value = 1.;
    value = acos( value );
    value *= 45. / atan( 1. );

    return value;
}


void VArrayAnalyzer::checkPointing()
{
    float iPointingDiff = 0.;

// there is no pointing available
    if( getNoPointing() ) return;

// calculate difference between calculated pointing direction and vbf pointing direction
    if( getReader()->getArrayTrigger() )
    {
        for( unsigned int j = 0; j < getTeltoAna().size(); j++ )
        {
            unsigned int i = getTeltoAna()[j];
// get vbf telescope index
            unsigned int ivbf = 9999;
            for( int t = 0; t < (int)getReader()->getArrayTrigger()->getNumSubarrayTelescopes(); t++ )
            {
                if( getTeltoAna()[j] == getReader()->getArrayTrigger()->getSubarrayTelescopeId( t ) )
                {
                    ivbf = t;
                    break;
                }
            }
            if( ivbf == 9999 ) continue;

	    getShowerParameters()->fTelElevationVBF[i] = getReader()->getArrayTrigger()->getAltitude( ivbf );
	    getShowerParameters()->fTelAzimuthVBF[i] = getReader()->getArrayTrigger()->getAzimuth( ivbf );
	    if( i < getPointing().size() && getPointing()[i] )
	    {
	       iPointingDiff = (float)angDist( getPointing()[i]->getTelAzimuth()/TMath::RadToDeg(), (90.-getPointing()[i]->getTelElevation())/TMath::RadToDeg(), getReader()->getArrayTrigger()->getAzimuth( ivbf )/TMath::RadToDeg(), (90. - getReader()->getArrayTrigger()->getAltitude( ivbf ))/TMath::RadToDeg() );

	       getShowerParameters()->fTelPointingMismatch[i] = iPointingDiff;
	       getShowerParameters()->fTelPointingErrorX[i] = getPointing()[i]->getPointingErrorX();
	       getShowerParameters()->fTelPointingErrorY[i] = getPointing()[i]->getPointingErrorY();

	       fMeanPointingMismatch[i] += iPointingDiff;
	       fNMeanPointingMismatch[i]++;

// check pointing difference, abort if too large
	       if( getRunParameter()->fCheckPointing < 900. && iPointingDiff > getRunParameter()->fCheckPointing )
	       {
		   cout << "VArrayAnalyzer::checkPointing() large mismatch between calculated telescope pointing direction and VBF pointing: " << iPointingDiff << " deg" << endl;
		   cout << "\t calculated telescope pointing direction: " << getPointing()[i]->getTelAzimuth() << "\t" << getPointing()[i]->getTelElevation() << endl;
		   cout << "\t VBF telescope pointing direction:        " << getReader()->getArrayTrigger()->getAzimuth( ivbf ) << "\t" << getReader()->getArrayTrigger()->getAltitude( ivbf ) << endl;
		   cout << "ABORT due to large pointing error (>" << getRunParameter()->fCheckPointing << " deg, event " << getEventNumber() << ")" << endl;
		   exit( 0 );
	       }
           }
           else
	   {
	       getShowerParameters()->fTelPointingMismatch[i] = 0.;
	       getShowerParameters()->fTelPointingErrorX[i] = 0.;
	       getShowerParameters()->fTelPointingErrorY[i] = 0.;
           }
        }
    }
}


double VArrayAnalyzer::getMeanPointingMismatch( unsigned int iTel )
{
    if( getNoPointing() ) return -1.;
    if( iTel > fMeanPointingMismatch.size() ) return -2.;

    if( isnan(fMeanPointingMismatch[iTel]) == 0 && isnan(fNMeanPointingMismatch[iTel]) == 0 )
    {
        if( fNMeanPointingMismatch[iTel] > 0. )
        {
            if( TMath::Abs( fMeanPointingMismatch[iTel] / fNMeanPointingMismatch[iTel] ) < 1.e-5 ) return 0.;
            else return fMeanPointingMismatch[iTel] / fNMeanPointingMismatch[iTel];
        }
    }

    return -3.;
}


/*
    add a small pointing error to centroids and recalculate image angle phi
*/
float VArrayAnalyzer::recalculateImagePhi( double iDeltaX, double iDeltaY )
{
    if( getRunParameter()->fImageLL ) return getImageParameters( getRunParameter()->fImageLL )->phi;

// par geo only
    float i_phi = 0.;
    float i_cen_x = getImageParameters( getRunParameter()->fImageLL )->cen_x + iDeltaX;
    float i_cen_y = getImageParameters( getRunParameter()->fImageLL )->cen_y + iDeltaY;
    float i_d = getImageParameters( getRunParameter()->fImageLL )->f_d;
    float i_s = getImageParameters( getRunParameter()->fImageLL )->f_s;
    float i_sdevxy  = getImageParameters( getRunParameter()->fImageLL )->f_sdevxy;

    i_phi = atan2( (i_d+i_s)*i_cen_y + 2.*i_sdevxy*i_cen_x, 2.*i_sdevxy*i_cen_y - (i_d-i_s)*i_cen_x );

    return i_phi;
}


/*!
    Normalize angle into range 0-2 pi
*/
float VArrayAnalyzer::adjustAzimuthToRange( float az )
{
    return slaDranrm( az/TMath::RadToDeg()) * TMath::RadToDeg();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/*!
      reconstruction of shower direction and core

      Hofmann et al 1999, Method 1 (HEGRA method)

      shower direction by intersection of image axes
      shower core by intersection of lines connecting reconstruced shower
      direction and image centroids

      todo: core reconstruction

*/
int VArrayAnalyzer::rcs_method_3( unsigned int iMethod )
{
    if( fDebug ) cout << "VArrayAnalyzer::rcs_method_3 " << iMethod << endl;

    int num_images = 0;                           /* number of images included in the fit */

    float xs = 0.;
    float ys = 0.;
    float stds = 0.;

    float ximp = 0.;                              /* impact point, from perpen. minimization */
    float yimp = 0.;
    float stdp = 0.;

    num_images = getShowerParameters()->fShowerNumImages[iMethod];

// are there enough images the run an array analysis
    if( num_images >= (int)fArrayAnalysisCuts->fNImages_min[iMethod] )
    {
        prepareforDirectionReconstruction( iMethod, 3 );
    }
    else
    {
        fillShowerDirection( iMethod, 0., 0., -1. );
        return 0;
    }
// don't do anything if angle between image axis is too small (for 2 images only)
    if( num_images == 2 )
    {
        float iangdiff = fabs(atan(m[0])-atan(m[1]));
        getShowerParameters()->fiangdiff[iMethod] = iangdiff*TMath::RadToDeg();
        if( iangdiff < fArrayAnalysisCuts->fAxesAngles_min[iMethod]/TMath::RadToDeg() || fabs(180./TMath::RadToDeg()-iangdiff) < fArrayAnalysisCuts->fAxesAngles_min[iMethod]/TMath::RadToDeg() )
        {
            getShowerParameters()->fShower_Chi2[iMethod] = -1.*fabs(atan(m[0])-atan(m[1]))*TMath::RadToDeg();
            return 0;
        }
    }
    else getShowerParameters()->fiangdiff[iMethod] = 0.;

///////////////////////////////
// direction reconstruction
///////////////////////////////
/* Now call perpendicular_distance for the fit, returning xs and ys */

// Hofmann et al 1999, Method 1 (HEGRA method)

    vector< float > xx( 2, 0.);
    vector< float > yy( 2, 0.);
    vector< float > ww( 2, 0.);
    vector< float > mm( 2, 0.);
    float itotweight = 0.;
    float iweight = 1.;
    float ixs = 0.;
    float iys = 0.;
    float iangdiff = 0.;
    for( unsigned int ii = 0; ii < m.size(); ii++ )
    {
        for( unsigned int jj = 1; jj < m.size(); jj++ )
        {
            if( ii == jj || ii > jj ) continue;
            xx[0] = x[ii];  yy[0] = y[ii]; ww[0] = 1.;  mm[0] = m[ii];
            xx[1] = x[jj];  yy[1] = y[jj]; ww[1] = 1.;  mm[1] = m[jj];

            rcs_perpendicular_fit(xx, yy, ww, mm, 2, &xs, &ys, &stds);

            iangdiff = sin( fabs(atan(m[jj])-atan(m[ii])) );

// discard all pairs with almost parallell lines
            if( fabs( iangdiff ) < 0.2 ) continue;

            iweight  = 1./(1./w[ii] + 1./w[jj]);
            iweight *= 1./(l[ii] + l[jj]);
            iweight *= iangdiff;

            ixs += atan(xs/( 1000. * getDetectorGeo()->getFocalLength()[0] ) ) * iweight * TMath::RadToDeg();
            iys += atan(ys/( 1000. * getDetectorGeo()->getFocalLength()[0] ) ) * iweight * TMath::RadToDeg();
            itotweight += iweight;
        }
    }
    if( itotweight > 0. )
    {
        xs = ixs / itotweight;
        ys = iys / itotweight;
    }
    else
    {
        xs = -99999.;
        ys = -99999.;
    }

    if( !fillShowerDirection( iMethod, xs, ys, stds ) ) return 0;

// core reconstruction

    prepareforCoreReconstruction( iMethod, xs, ys );

// Now call perpendicular_distance for the fit, returning ximp and yimp

    rcs_perpendicular_fit(x, y, w, m, num_images, &ximp, &yimp, &stdp);

// convert xs,ys into [deg]

    if( isnormal( ximp ) && isnormal( yimp ) )
    {
        fillShowerCore( iMethod, ximp, yimp );
        getShowerParameters()->fShower_stdP[iMethod] = stdp;
                                                  /* indicates fit ok */
        getShowerParameters()->fShower_Chi2[iMethod] = 0.0;
    }
    else
    {
        getShowerParameters()->fShower_Chi2[iMethod] = -2.;
    }

    return 0;
}


/***************** end of rcs_method_3 *********************************/

/*!
      reconstruction of shower direction and core

      Hofmann et al 1999, Method 1 (HEGRA method)

      shower direction by intersection of image axes
      shower core by intersection of lines connecting reconstruced shower
      direction and image centroids

      (difference to rcs_method_3: direction reconstruction independent of CD routines)

todo: core construction
*/
int VArrayAnalyzer::rcs_method_4( unsigned int iMethod )
{
    if( fDebug ) cout << "VArrayAnalyzer::rcs_method_4 " << iMethod << endl;
    int num_images = 0;                           /* number of images included in the fit */

    float xs = 0.;
    float ys = 0.;

    float ximp = 0.;                              /* impact point, from perpen. minimization */
    float yimp = 0.;
    float stdp = 0.;

    num_images = getShowerParameters()->fShowerNumImages[iMethod];

// are there enough images the run an array analysis
    if( num_images >= (int)fArrayAnalysisCuts->fNImages_min[iMethod] )
    {
        prepareforDirectionReconstruction( iMethod, 4 );
    }
    else
    {
        fillShowerDirection( iMethod, 0., 0., -1. );
        return 0;
    }

// don't do anything if angle between image axis is too small (for 2 images only)
    if( num_images == 2 )
    {
        float iangdiff = fabs(atan(m[0])-atan(m[1]));
        getShowerParameters()->fiangdiff[iMethod] = iangdiff*TMath::RadToDeg();
        if( iangdiff < fArrayAnalysisCuts->fAxesAngles_min[iMethod]/TMath::RadToDeg() || fabs(180./TMath::RadToDeg()-iangdiff) < fArrayAnalysisCuts->fAxesAngles_min[iMethod]/TMath::RadToDeg() )
        {
            getShowerParameters()->fShower_Chi2[iMethod] = -1.*fabs(atan(m[0])-atan(m[1]))*TMath::RadToDeg();
            return 0;
        }
    }
    else getShowerParameters()->fiangdiff[iMethod] = 0.;

///////////////////////////////
// direction reconstruction
////////////////////////////////////////////////
// Hofmann et al 1999, Method 1 (HEGRA method)
// (modified weights)

    float itotweight = 0.;
    float iweight = 1.;
    float ixs = 0.;
    float iys = 0.;
    float iangdiff = 0.;
    float b1 = 0.;
    float b2 = 0.;

    for( unsigned int ii = 0; ii < m.size(); ii++ )
    {
        for( unsigned int jj = 1; jj < m.size(); jj++ )
        {
            if( ii == jj || ii > jj ) continue;

            b1 = y[ii] - m[ii] * x[ii];
            b2 = y[jj] - m[jj] * x[jj];

// line intersection
            if( m[ii] != m[jj] ) xs = ( b2 - b1 )  / ( m[ii] - m[jj] );
            else                 xs = 0.;
            ys = m[ii] * xs + b1;

            iangdiff = fabs( sin( fabs(atan(m[jj])-atan(m[ii])) ) );

            iweight  = 1./(1./w[ii] + 1./w[jj]);      // weight 1: size of images
            iweight *= (1.-l[ii])*(1.-l[jj]);         // weight 2: elongation of images (width/length)
            iweight *= iangdiff;                      // weight 3: angular differences between the two image axis
	        iweight *= iweight;                       // use squared value

            ixs += xs * iweight;
            iys += ys * iweight;
            itotweight += iweight;
        }
    }
    if( itotweight > 0. )
    {
        xs = ixs / itotweight;
        ys = iys / itotweight;
    }
    else
    {
        xs = -99999.;
        ys = -99999.;
    }

    if( !fillShowerDirection( iMethod, xs, ys, 0. ) ) return 0;

////////////////////////////////////////////////

// core reconstruction

    prepareforCoreReconstruction( iMethod, xs, ys );

// Now call perpendicular_distance for the fit, returning ximp and yimp

    rcs_perpendicular_fit(x, y, w, m, num_images, &ximp, &yimp, &stdp);

// convert xs,ys into [deg]

    fillShowerCore( iMethod, ximp, yimp );
    getShowerParameters()->fShower_stdP[iMethod] = stdp;
                                                  /* indicates fit ok */
    getShowerParameters()->fShower_Chi2[iMethod] = 0.0;

    return 0;
}


/***************** end of rcs_method_4 *********************************/

bool VArrayAnalyzer::fillShowerDirection( unsigned int iMethod, float xs, float ys, float stds )
{

    if( isnan( xs ) || isnan( ys ) )
    {
        getShowerParameters()->fShower_Xoffset[iMethod] = -99999.;
        getShowerParameters()->fShower_Yoffset[iMethod] = -99999.;
        getShowerParameters()->fShowerZe[iMethod] = -99999.;
        getShowerParameters()->fShowerAz[iMethod] = -99999.;
        getShowerParameters()->fShower_stdS[iMethod] = -99999.;

        return false;
    }

    getShowerParameters()->fShower_Xoffset[iMethod] = xs;
    if( getDetectorGeo()->getGrIsuVersion() >= 412 ) getShowerParameters()->fShower_Yoffset[iMethod] = -1. * ys;
    else                                             getShowerParameters()->fShower_Yoffset[iMethod] = ys;

    double ze = 0.;
    double az = 0.;
    if( getTeltoAna()[0] < getPointing().size() && getPointing()[getTeltoAna()[0]] )
    {
       getPointing()[getTeltoAna()[0]]->getRotatedShowerDirection( -1.*getShowerParameters()->fShower_Yoffset[iMethod], -1.*getShowerParameters()->fShower_Xoffset[iMethod], ze, az );
    }
    if( isnan( ze ) ) ze = -99999.;
    if( isnan( az ) ) az = -99999.;
    getShowerParameters()->fShowerZe[iMethod] = ze;
    getShowerParameters()->fShowerAz[iMethod] = adjustAzimuthToRange( az );
    getShowerParameters()->fShower_stdS[iMethod] = stds;

// calculate derotated shower directions
    if( !fReader->isMC() )
    {
        double iUTC = 0.;
        double xrot = 0.;
	double yrot = 0.;
	if( getTeltoAna()[0] < getPointing().size() && getPointing()[getTeltoAna()[0]] )
	{
	   iUTC = VSkyCoordinatesUtilities::getUTC( getShowerParameters()->MJD, getShowerParameters()->time );
	   if( getTelID() < getPointing().size() && getPointing()[getTelID()] )
	   {
	      getPointing()[getTelID()]->derotateCoords( iUTC, getShowerParameters()->fShower_Xoffset[iMethod], getShowerParameters()->fShower_Yoffset[iMethod], xrot, yrot );
           }
        }	   

        getShowerParameters()->fShower_XoffsetDeRot[iMethod] = xrot;
        getShowerParameters()->fShower_YoffsetDeRot[iMethod] = -1.*yrot;
    }
    else
    {
        getShowerParameters()->fShower_XoffsetDeRot[iMethod] = getShowerParameters()->fShower_Xoffset[iMethod];
        getShowerParameters()->fShower_YoffsetDeRot[iMethod] = getShowerParameters()->fShower_Yoffset[iMethod];
    }

    return true;
}


void VArrayAnalyzer::prepareforDirectionReconstruction( unsigned int iMethodIndex, unsigned iReconstructionMethod )
{
    if( fDebug ) cout << "VArrayAnalyzer::prepareforDirectionReconstruction; preparing method " << iMethodIndex << endl;

    double iPointingErrorX = 0.;
    double iPointingErrorY = 0.;
    float i_cen_x = 0.;
    float i_cen_y = 0.;
    float i_phi = 0.;

// reset data vectors
    telID.clear();
    x.clear();
    y.clear();
    l.clear();
    m.clear();
    phi.clear();
    sinphi.clear();
    cosphi.clear();
    w.clear();
    length.clear();
    width.clear();
    asym.clear();
    loss.clear();
    dist.clear();
    pedvar.clear();
    ze.clear();
    az.clear();

/* fill the x, y, w, and m arrays for the fit */
    for ( unsigned int tel = 0; tel < getNTel(); tel++ )
    {
        setTelID( tel );
        if( getShowerParameters()->fTelIDImageSelected[iMethodIndex][tel] )
        {
            telID.push_back( tel );
// get pointing difference between expected pointing towards source and measured pointing (by command line, tracking program or pointing monitors)
            if( !fArrayAnalysisCuts->fUseEventdisplayPointing[iMethodIndex] && tel < getPointing().size() && getPointing()[tel] )
            {
                iPointingErrorX = getPointing()[tel]->getPointingErrorX();
                iPointingErrorY = getPointing()[tel]->getPointingErrorY();
            }
// do not use pointing corrections
            else
            {
                iPointingErrorX = 0.;
                iPointingErrorY = 0.;
            }
// get image centroids corrected for pointing errors
            i_cen_x = getImageParameters( getRunParameter()->fImageLL )->cen_x + iPointingErrorX;
            i_cen_y = getImageParameters( getRunParameter()->fImageLL )->cen_y + iPointingErrorY;
// (GM) centroid locations in getImageParameters( getRunParameter()->fImageLL ) are in [deg]
// (GM) (in contrary to centroids in GrIsu ([mm]))
            if( iReconstructionMethod == 4 || iReconstructionMethod == 5 )
            {
                x.push_back( i_cen_x );
                y.push_back( i_cen_y );
            }
            else
            {
                x.push_back( tan( i_cen_x/TMath::RadToDeg() )*getDetectorGeo()->getFocalLength()[tel]*1000. );
                y.push_back( tan( i_cen_y/TMath::RadToDeg() )*getDetectorGeo()->getFocalLength()[tel]*1000. );
            }
// weight is size
            w.push_back( getImageParameters( getRunParameter()->fImageLL )->size );
// calculate new 'phi' with pointing errors taken into account
            i_phi = recalculateImagePhi( iPointingErrorX, iPointingErrorY );
            if( cos( i_phi ) != 0. ) m.push_back( sin( i_phi ) / cos( i_phi ) );
            else                     m.push_back( 1.e9 );
            phi.push_back( i_phi );
            sinphi.push_back( sin( i_phi ) );
            cosphi.push_back( cos( i_phi ) );
            if(  getImageParameters( getRunParameter()->fImageLL )->length > 0. )
            {
                l.push_back( getImageParameters( getRunParameter()->fImageLL )->width / getImageParameters( getRunParameter()->fImageLL )->length );
            }
            else
            {
                l.push_back( 1. );
            }
            length.push_back( getImageParameters( getRunParameter()->fImageLL )->length );
            width.push_back( getImageParameters( getRunParameter()->fImageLL )->width );
            loss.push_back( getImageParameters( getRunParameter()->fImageLL )->loss );
            asym.push_back( getImageParameters( getRunParameter()->fImageLL )->asymmetry );
            dist.push_back( getImageParameters( getRunParameter()->fImageLL )->dist );
            pedvar.push_back( getImageParameters( getRunParameter()->fImageLL )->fmeanPedvar_Image );
            ze.push_back( 90.-getShowerParameters()->fTelElevation[tel] );
            az.push_back( getShowerParameters()->fTelAzimuth[tel] );
        }
    }
}


void VArrayAnalyzer::prepareforCoreReconstruction( unsigned int iMethodIndex, float xs, float ys )
{
    vector< float > xtel;                         /* arrays for telescope locations */
    vector< float > ytel;
    vector< float > ztel;
    xtelnew.clear();
    ytelnew.clear();
    ztelnew.clear();
    for ( unsigned int tel = 0; tel < getNTel(); tel++ )
    {
        setTelID( tel );
        xtel.push_back( getImageParameters( getRunParameter()->fImageLL )->Tel_x_SC );
        xtelnew.push_back( 0. );
        ytel.push_back( getImageParameters( getRunParameter()->fImageLL )->Tel_y_SC );
        ytelnew.push_back( 0. );
        ztel.push_back( getImageParameters( getRunParameter()->fImageLL )->Tel_z_SC );
        ztelnew.push_back( 0. );
    }
    rcs_rotate_delta( xtel, ytel, ztel, xtelnew, ytelnew, ztelnew, xs/TMath::RadToDeg(), ys/TMath::RadToDeg(), getNTel() );

/* fill the x, y, w, and m arrays  */
    x.clear(); y.clear(); m.clear(); w.clear();
    float i_cen_x = 0.;
    float i_cen_y = 0.;
    for ( unsigned int tel = 0; tel < getNTel(); tel++ )
    {
        setTelID( tel );
        if( getShowerParameters()->fTelIDImageSelected[iMethodIndex][tel] )
        {
            x.push_back( xtelnew[tel] );          /* telescope locations */
            y.push_back( ytelnew[tel] );
            w.push_back( getImageParameters( getRunParameter()->fImageLL )->size );
            i_cen_x = getImageParameters( getRunParameter()->fImageLL )->cen_x - xs;
            i_cen_y = getImageParameters( getRunParameter()->fImageLL )->cen_y - ys;
///////////////////////////////
// account for differences in coordinate systems between grisudet and real data
// y coordinate is flipped in grisudet (pointing into the ground and not into the sky for stored position)
// this is fixed in grisudet from version 4.12 on
            if( (fReader->isGrisuMC() ) && getDetectorGeo()->getGrIsuVersion() < 412 )
            {
                i_cen_y *= -1.;
            }
///////////////////////////////
            m.push_back( -1.*i_cen_y / i_cen_x );
        }
    }
}


/*!
      reconstruction of shower direction and core

      Table and MLP based stereo disp method

      todo: core construction
*/
int VArrayAnalyzer::rcs_method_5( unsigned int iMethod, unsigned int iDisp  )
{
    if( fDebug ) cout << "VArrayAnalyzer::rcs_method_5/6 " << iMethod << endl;

// number of images included in the fit
    int num_images = 0;

// impact point, from perpen. minimization
    float xs = 0.;
    float ys = 0.;

    float ximp = 0.;
    float yimp = 0.;
    float stdp = 0.;

    num_images = getShowerParameters()->fShowerNumImages[iMethod];

// are there enough images the run an array analysis
    if( num_images >= (int)fArrayAnalysisCuts->fNImages_min[iMethod] )
    {
        prepareforDirectionReconstruction( iMethod, 5 );
    }
    else
    {
        fillShowerDirection( iMethod, 0., 0., -1. );
        return 0;
    }

// check that MLPAnalyzer exists
    if( iDisp == 5 )
    {
        if( iMethod >= fMLPAnalyzer.size() || !fMLPAnalyzer[iMethod] || fMLPAnalyzer[iMethod]->isZombie() )
        {
            fillShowerDirection( iMethod, 0., 0., -2. );
            return 0;
        }
    }
// check that disp table analyzer exists
    else if( iDisp == 6 )
    {
        if( iMethod >= fDispAnalyzer.size() || !fDispAnalyzer[iMethod] || fDispAnalyzer[iMethod]->isZombie() )
        {
            fillShowerDirection( iMethod, 0., 0., -2. );
            return 0;
        }
    }

/////////////////////////////////////////////////////
// direction reconstruction with MLP or disp tables
////////////////////////////////////////////////////

    float disp = 0.;

    vector< float > v_xs;
    vector< float > v_ys;
    vector< float > v_disp;
    vector< float > v_weight;

    for( unsigned int ii = 0; ii < m.size(); ii++ )
    {
        if( iDisp == 5 )
        {
            disp = fMLPAnalyzer[iMethod]->evaluate( width[ii], length[ii], asym[ii], w[ii], dist[ii] );
	    v_xs.push_back( x[ii] - disp * cosphi[ii] );
	    v_ys.push_back( y[ii] - disp * sinphi[ii] );
            getShowerParameters()->addDISPPoint( telID[ii], iMethod, v_xs.back(), v_ys.back(), 1. );
        }
        else if( iDisp == 6 )
        {
            disp = fDispAnalyzer[iMethod]->evaluate( width[ii], length[ii], w[ii], pedvar[ii], ze[ii], az[ii], true );
        }
        v_disp.push_back( disp );
        if( fDispAnalyzer[iMethod]->getDispE() > 0. ) v_weight.push_back( 1./fDispAnalyzer[iMethod]->getDispE() );
	else                                          v_weight.push_back( 1. );
    }
// calculate simple mean for MLP analyzer (preliminary)
    if( iDisp == 5 ) calculateMeanDirection( xs, ys, v_xs, v_ys, v_weight );
// calculate mean direction for disp method
    else
    {
       fDispAnalyzer[iMethod]->calculateMeanDirection( xs, ys, x, y, cosphi, sinphi, v_disp, v_weight );

       for( unsigned int ii = 0; ii < m.size(); ii++ )
       {
          getShowerParameters()->addDISPPoint( telID[ii], iMethod, fDispAnalyzer[iMethod]->getXcoordinate_disp( ii ), fDispAnalyzer[iMethod]->getYcoordinate_disp( ii ), 1. );
       }
    }

    if( !fillShowerDirection( iMethod, xs, ys, 0. ) ) return 0;

////////////////////////////////////////////////

// core reconstruction (still to replace by DISP method)

    prepareforCoreReconstruction( iMethod, xs, ys );

// Now call perpendicular_distance for the fit, returning ximp and yimp

    rcs_perpendicular_fit(x, y, w, m, num_images, &ximp, &yimp, &stdp);

// convert xs,ys into [deg]

    fillShowerCore( iMethod, ximp, yimp );
    getShowerParameters()->fShower_stdP[iMethod] = stdp;
                                                  /* indicates fit ok */
    getShowerParameters()->fShower_Chi2[iMethod] = 0.0;

    return 0;
}


/*!
    calculate mean direction from a vector
*/
float VArrayAnalyzer::calculateMeanDirection( float &xs, float &ys, vector< float > v_xs, vector< float > v_ys, vector< float > v_weight )
{
    float itotweight = 0.;
    float ixs = 0.;
    float iys = 0.;

    for( unsigned int ii = 0; ii < v_xs.size(); ii++ )
    {
        ixs += v_xs[ii] * v_weight[ii];
        iys += v_ys[ii] * v_weight[ii];
        itotweight += v_weight[ii];
    }
    if( itotweight > 0. )
    {
        xs = ixs / itotweight;
        ys = iys / itotweight;
    }
    else
    {
        xs = -99999.;
        ys = -99999.;
    }
    return -1.;
}


// get Monte Carlo shower parameters and fill them into showerpars tree
bool VArrayAnalyzer::fillSimulationEvent()
{
#ifndef NOVBF
// ignore pedestal events
// (assume that vbf files only have pedestal events)
    if( fReader->getATEventType() == VEventType::PED_TRIGGER )
    {
       return false;
    }
#endif

    if( fReader->isMC() )
    {
       if( getMCParameters() )
       {
// fill MC parameter tree
	   getMCParameters()->runNumber = getRunNumber();
	   getMCParameters()->eventNumber = getEventNumber();
	   getMCParameters()->MCprimary = fReader->getMC_primary();
	   getMCParameters()->MCenergy  = fReader->getMC_energy();
	   getMCParameters()->MCxcore   = fReader->getMC_X();
	   getMCParameters()->MCycore   = fReader->getMC_Y();
	   getMCParameters()->MCxcos    = fReader->getMC_Xcos();
	   getMCParameters()->MCycos    = fReader->getMC_Ycos();
	   getMCParameters()->MCTel_Xoff= fReader->getMC_Xoffset();
	   getMCParameters()->MCTel_Yoff= fReader->getMC_Yoffset();
	   getMCParameters()->MCze = fReader->getMC_Ze();
	   getMCParameters()->MCaz = fReader->getMC_Az();
	   getMCParameters()->fill();
        }
    }
    return true;
}



/*!
      reconstruction of shower direction and core

      combination of method 4 and 6

      weighted by f(cos(ze)) (after M.Beilicke 2010)

*/
int VArrayAnalyzer::rcs_method_7( unsigned int iMethod )
{
    double cosze = 0.;
    if( getTeltoAna()[0] < getPointing().size() && getPointing()[getTeltoAna()[0]] )
    {
       cosze = cos( (90.-getPointing()[getTeltoAna()[0]]->getTelElevation())/TMath::RadToDeg() );
    }
// weight calculation according to Beilicke 2010
    double weight = 0.;
    if( cosze < 0.4 ) weight = 1.;
    else              weight = TMath::Exp( -12.5 * (cosze-0.4)*(cosze-0.4) );

// first call intersection of lines
    rcs_method_4( iMethod );

    float xoff_4 = getShowerParameters()->fShower_Xoffset[iMethod];
    float yoff_4 = getShowerParameters()->fShower_Yoffset[iMethod];
    float stds_4 = getShowerParameters()->fShower_stdS[iMethod];

//  disp method
    rcs_method_5( iMethod, 6 );

    float xoff_6 = getShowerParameters()->fShower_Xoffset[iMethod];
    float yoff_6 = getShowerParameters()->fShower_Yoffset[iMethod];

// calculate weighted mean between the two methods
    float xoff_7 = xoff_4 * (1.-weight) + xoff_6 * weight;
    float yoff_7 = yoff_4 * (1.-weight) + yoff_6 * weight;
    if( getDetectorGeo()->getGrIsuVersion() >= 412 ) yoff_7 *= -1.;

    if( !fillShowerDirection( iMethod, xoff_7, yoff_7, stds_4 ) ) return 0;

// (core reconstruction: same in method 4 and 7)

    return 0;
}


/*!
      reconstruction of shower direction and core

      combine method 4 and 6

      (preliminary: optimize)

      todo: core construction
*/
int VArrayAnalyzer::rcs_method_8( unsigned int iMethod )
{
    if( fDebug ) cout << "VArrayAnalyzer::rcs_method_8 " << iMethod << endl;
    int num_images = 0;                           /* number of images included in the fit */

    float xs = 0.;
    float ys = 0.;
    float xd = 0.;
    float yd = 0.;

    float ximp = 0.;                              /* impact point, from perpen. minimization */
    float yimp = 0.;
    float stdp = 0.;

    num_images = getShowerParameters()->fShowerNumImages[iMethod];

// are there enough images the run an array analysis
    if( num_images >= (int)fArrayAnalysisCuts->fNImages_min[iMethod] )
    {
        prepareforDirectionReconstruction( iMethod, 4 );
    }
    else
    {
        fillShowerDirection( iMethod, 0., 0., -1. );
        return 0;
    }

///////////////////////////////
// direction reconstruction
////////////////////////////////////////////////
// check that disp table analyzer exists
    if( iMethod >= fDispAnalyzer.size() || !fDispAnalyzer[iMethod] || fDispAnalyzer[iMethod]->isZombie() )
    {
        fillShowerDirection( iMethod, 0., 0., -2. );
        return 0;
    }

// weights
    float idispweight = 0.;
    float itotweight = 0.;
    float iweight = 1.;
    float iangdiff = 0.;
// line intersection
    float ixs = 0.;
    float iys = 0.;
    float b1 = 0.;
    float b2 = 0.;
// disp vectors
    vector< float > v_disp( 2, 0. );
    vector< float > v_weight( 2, 0. );
    vector< float > v_x( 2, 0. );
    vector< float > v_y( 2, 0. );
    vector< float > v_cosphi( 2, 0. );
    vector< float > v_sinphi( 2, 0. );
// debug variable
    int   npairs = 0;

////////////////////////////////////////////////////////
// loop over all pairs of images
////////////////////////////////////////////////////////
    for( unsigned int ii = 0; ii < m.size(); ii++ )
    {
        for( unsigned int jj = 1; jj < m.size(); jj++ )
        {
            if( ii == jj || ii > jj ) continue;

////////////////////////////////////////////////////////
// determine weight between line intersection and disp method
////////////////////////////////////////////////////////
            iangdiff = fabs( sin( fabs(atan(m[jj])-atan(m[ii])) ) );
            if( iangdiff * TMath::RadToDeg() > 10. ) idispweight = 0.;
	    else
	    {
	       idispweight = 1.-TMath::Exp( -0.04*(iangdiff*TMath::RadToDeg()-10.)*(iangdiff*TMath::RadToDeg()-10.));    // not optimized yet
            }

////////////////////////////////////////////////////////
// reconstructed shower direction from line intersection
////////////////////////////////////////////////////////
            b1 = y[ii] - m[ii] * x[ii];
            b2 = y[jj] - m[jj] * x[jj];

            if( m[ii] != m[jj] ) xs = ( b2 - b1 )  / ( m[ii] - m[jj] );
            else                 xs = 0.;
            ys = m[ii] * xs + b1;

////////////////////////////////////////////////////////
// reconstructed shower direction from disp
////////////////////////////////////////////////////////

// run disp method only for nonzero weights
            if( idispweight > 1.e-4 )
	    {
// P1
	       v_disp[0] = fDispAnalyzer[iMethod]->evaluate( width[ii], length[ii], w[ii], pedvar[ii], ze[ii], az[ii], true  );
	       v_x[0] = x[ii];
	       v_y[0] = y[ii];
	       v_cosphi[0] = cosphi[ii];
	       v_sinphi[0] = sinphi[ii];
	       if( fDispAnalyzer[iMethod]->getDispE() > 0. ) v_weight[0] = 1./fDispAnalyzer[iMethod]->getDispE();
	       else                                          v_weight[0] = 1.;
// P2
	       v_disp[1] = fDispAnalyzer[iMethod]->evaluate( width[jj], length[jj], w[jj], pedvar[jj], ze[jj], az[jj], true );
	       v_x[1] = x[jj];
	       v_y[1] = y[jj];
	       v_cosphi[1] = cosphi[jj];
	       v_sinphi[1] = sinphi[jj];
	       if( fDispAnalyzer[iMethod]->getDispE() > 0. ) v_weight[1] = 1./fDispAnalyzer[iMethod]->getDispE();
	       else                                          v_weight[1] = 1.;
	       fDispAnalyzer[iMethod]->calculateMeanDirection( xd, yd, v_x, v_y, v_cosphi, v_sinphi, v_disp, v_weight );
            }
	    else
	    {
	       xd = xs;
	       yd = ys;
            }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////           
// temporary (debug&dev)
	    if( iMethod ==0 && npairs < VDST_MAXRECMETHODS )
	    {
	       getShowerParameters()->fShower_PairXS[npairs] = xs;
	       getShowerParameters()->fShower_PairYS[npairs] = ys;
	       getShowerParameters()->fShower_PairXD[npairs] = xd;
	       getShowerParameters()->fShower_PairYD[npairs] = yd;
	       getShowerParameters()->fShower_PairAngDiff[npairs] = iangdiff;
	       getShowerParameters()->fShower_PairDispWeight[npairs] = idispweight;
            }
	    npairs++; 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////           
////////////////////////////////////////////////////////
// direction for this pair is weighted mean between disp and geo method
////////////////////////////////////////////////////////
	    xs = xs * (1.-idispweight) + xd * idispweight;
	    ys = ys * (1.-idispweight) + yd * idispweight;

// weights for individual images from shower size, width, length
            iweight  = 1./(1./w[ii] + 1./w[jj]);      // weight 1: size of images
            iweight *= (1.-l[ii])*(1.-l[jj]);         // weight 2: elongation of images (width/length)
// remove this weight compared to method 4: disp method is used for small angles between pairs of lines
//            iweight *= iangdiff;                      // weight 3: angular differences between the two image axis 
	    iweight *= iweight;                       // use squared value

////////////////////////////////////////////////////////
// add up for final direction estimate
////////////////////////////////////////////////////////
            ixs += xs * iweight;
            iys += ys * iweight;
            itotweight += iweight;

        }
    }
    getShowerParameters()->fShower_NPair = npairs;
    if( itotweight > 0. )
    {
        xs = ixs / itotweight;
        ys = iys / itotweight;
    }
    else
    {
        xs = -99999.;
        ys = -99999.;
    }
    if( num_images == 2 ) getShowerParameters()->fShower_Chi2[iMethod] = -1.*iangdiff;
    getShowerParameters()->fiangdiff[iMethod] = 0.;

    if( !fillShowerDirection( iMethod, xs, ys, 0. ) ) return 0;

////////////////////////////////////////////////////////
// core reconstruction
// 
// Observe: not yet combined disp and geo method
// 
////////////////////////////////////////////////////////

    prepareforCoreReconstruction( iMethod, xs, ys );

// Now call perpendicular_distance for the fit, returning ximp and yimp

    rcs_perpendicular_fit(x, y, w, m, num_images, &ximp, &yimp, &stdp);

// convert xs,ys into [deg]

    fillShowerCore( iMethod, ximp, yimp );
    getShowerParameters()->fShower_stdP[iMethod] = stdp;
                                                  /* indicates fit ok */
    getShowerParameters()->fShower_Chi2[iMethod] = 0.0;

    return 0;
}
