/*! \class VEvndispReconstructionParameter
    \brief reading and storage class for eventdisplay reconstruction parameters

    note: due to historical reasons, the term 'method' is not used in the right way
    in this routine and in VArrayAnalyzer.cpp

    one value per telescope

    there should be one instance per reconstruction method of this class

*/

#include "VEvndispReconstructionParameter.h"

ClassImp( VEvndispReconstructionParameter )

VEvndispReconstructionParameter::VEvndispReconstructionParameter()
{
	fDebug = false;
        fNTel_type = 0;
	fRunPara = 0;
	fNMethods = 0;
}


VEvndispReconstructionParameter::VEvndispReconstructionParameter( vector< ULong64_t > i_telType, VEvndispRunParameter* iRunPara )
{
	reset();
	
	fRunPara = iRunPara;
	
	fTel_type_V = i_telType;
	
	// get set with telescope types
	for( unsigned int i = 0; i < i_telType.size(); i++ )
	{
		fTel_type.insert( fTel_type_V[i] );
	}
	fNTel_type = fTel_type.size();
	
}

void VEvndispReconstructionParameter::reset()
{
	fDebug = false;
        fNTel_type = 0;
	fRunPara = 0;
	fNMethods = 0;
}


/*

     apply array analysis cuts for this set of image parameters

*/
bool VEvndispReconstructionParameter::applyArrayAnalysisCuts( unsigned int iMeth, unsigned int iTel, unsigned int iTelType,
		VImageParameter* iImageParameter, unsigned short int iLocalTriggerType,
		VStarCatalogue* iStarCatalogue )
{
	// sanity checks
	if( iMeth >= fNMethods )
	{
		cout << "VEvndispReconstructionParameter::applyArrayAnalysisCuts error: invalid method number " << iMeth << "\t" << fNMethods << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	if( iTelType >= fNTel_type )
	{
		cout << "VEvndispReconstructionParameter::applyArrayAnalysisCuts error: invalid telescope type " << iTelType << "\t" << fNTel_type << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	if( !iImageParameter )
	{
		cout << "VEvndispReconstructionParameter::applyArrayAnalysisCuts error: no image parameters given" << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	
	if( fDebug )
	{
		cout << "APPLY ARRAY ANALYSIS CUTS FOR METHOD " << iMeth << " AND TELESCOPE " << iTel + 1 << ", TYPE " << iTelType << endl;
	}
	
	// return value
	bool iArrayCut = true;
	
	// eventstatus
	if( iImageParameter->eventStatus > 0 )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: event status > 0: " << iImageParameter->eventStatus << endl;
		}
	}
	
	////////////////////////////////////////////
	// L2 trigger type (mainly for CTA prod2)
	
	// 9999: any trigger
	if( fL2TriggerType[iMeth][iTelType] != 9999 )
	{
		bitset< 8 > i_L2TrigType( iLocalTriggerType );
		if( !i_L2TrigType.test( fL2TriggerType[iMeth][iTelType] ) )
		{
			iArrayCut = false;
		}
		if( !iArrayCut && fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut Tel " << iTel + 1 << ", type " << iTelType;
			cout << " (meth " << iMeth << "): L2 trigger type ";
			cout << iLocalTriggerType << " (" << iImageParameter->fTrig_type;
			cout << ")  test: " << i_L2TrigType.test( fL2TriggerType[iMeth][iTelType] );
			cout << " (require " << fL2TriggerType[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// image size
	if( iImageParameter->size < fSize_min[iMeth][iTelType] || iImageParameter->size <= 0. )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: size cut: " << iImageParameter->size;
			cout << " (" << fSize_min[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// number of ntubes (<=!!!)
	if( iImageParameter->ntubes <= fLocalNtubes_min[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: ntubes cut: " << iImageParameter->ntubes;
			cout << " (" << fLocalNtubes_min[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// number of saturated channels
	if( iImageParameter->nlowgain > fLocalNLowGain_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: nlowgain cut: " << iImageParameter->nlowgain;
			cout << " (" << fLocalNLowGain_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// image length
	if( iImageParameter->length < fLength_min[iMeth][iTelType] || iImageParameter->length > fLength_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: length cut: " << iImageParameter->length;
			cout << " (" << fLength_min[iMeth][iTelType] << ", " << fLength_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// image width
	if( iImageParameter->width < fWidth_min[iMeth][iTelType] || iImageParameter->width > fWidth_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: width cut: " << iImageParameter->width;
			cout << " (" << fWidth_min[iMeth][iTelType] << ", " << fWidth_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// image distance to camera centre
	if( iImageParameter->dist < fLocalDistance_min[iMeth][iTelType] || iImageParameter->dist > fLocalDistance_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: dist cut: " << iImageParameter->dist;
			cout << " (" << fLocalDistance_min[iMeth][iTelType] << ", " << fLocalDistance_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// image alpha
	if( iImageParameter->alpha < fLocalAlpha_min[iMeth][iTelType] || iImageParameter->alpha > fLocalAlpha_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: alpha cut: " << iImageParameter->alpha;
			cout << " (" << fLocalAlpha_min[iMeth][iTelType] << ", " << fLocalAlpha_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// loss cut
	if( iImageParameter->loss > fLoss_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: loss cut: " << iImageParameter->loss;
			cout << " (" << fLoss_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// cut on successfull LL reconstruction on the edge of the FOV
	if( fRunPara )
	{
		if( iTel < fRunPara->fLogLikelihoodLoss_min.size() )
		{
			// (GM) changed d20120405 if( iImageParameter->loss > fRunPara->fLogLikelihoodLoss_min[iTel] && iImageParameter->Fitstat < 2 && iImageParameter->Fitstat >=0 )
			// (GM): fitstat cut: do we have to require a successfull covariance matrix? Not clear...
			if( iImageParameter->loss > fRunPara->fLogLikelihoodLoss_min[iTel] && iImageParameter->Fitstat < 1 )
			{
				iArrayCut = false;
				if( fDebug )
				{
					cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: fit stat cut: ";
					cout << iImageParameter->Fitstat << endl;
				}
			}
			// check number of events at the edge of the FOV
			if( iTel < fRunPara->fLogLikelihoodLoss_min.size() && iTel < fRunPara->fLogLikelihood_Ntubes_min.size() )
			{
				if( iImageParameter->loss   >  fRunPara->fLogLikelihoodLoss_min[iTel]
						&& iImageParameter->ntubes <= fRunPara->fLogLikelihood_Ntubes_min[iTel] )
				{
					iArrayCut = false;
					if( fDebug )
					{
						cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: LL ntubes cut: ";
						cout << iImageParameter->ntubes;
						cout << " (" <<  fRunPara->fLogLikelihood_Ntubes_min[iTel] << ")" << endl;
					}
				}
			}
		}
	}
	
	////////////////////////////////////////////
	// width/length cut
	if( iImageParameter->length > 0. && iImageParameter->width / iImageParameter->length > fWidthLength_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: width/length cut: " << iImageParameter->length << ", " << iImageParameter->width;
			cout << " (" << fWidthLength_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// image asymmetry
	if( iImageParameter->asymmetry < fAsym_min[iMeth][iTelType] || iImageParameter->asymmetry > fAsym_max[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: asymmetry cut: " << iImageParameter->asymmetry;
			cout << " (" << fAsym_min[iMeth][iTelType] << ", " << fAsym_max[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// fui: fraction of image under 2D Gauss
	if( iImageParameter->fui < fFui_min[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: fui cut: " << iImageParameter->fui;
			cout << " (" << fFui_min[iMeth][iTelType] << ")" << endl;
		}
	}
	
	////////////////////////////////////////////
	// user set: remove image
	if( !fLocalUseImage[iMeth][iTelType] )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: image removed by user selection" << endl;
		}
	}
	
	/////////////////////////////////////////////
	// MC only: cut on MC energy (use with care!)
	if( fRunPara->isMC()
			&& ( ( iImageParameter->MCenergy < fMCEnergy_linTeV_min[iMeth][iTelType] || iImageParameter->MCenergy > fMCEnergy_linTeV_max[iMeth][iTelType] ) ) )
	{
		iArrayCut = false;
		if( fDebug )
		{
			cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: MC energy cut:" << endl;
			cout << iImageParameter->MCenergy << " [" << fMCEnergy_linTeV_min[iMeth][iTelType] << "," << fMCEnergy_linTeV_max[iMeth][iTelType] << "]" << endl;
		}
		
	}
	
	////////////////////////////////////////////
	// remove image which is too close to a bright star
	// (use list of image and border pixels)
	if( iStarCatalogue && fRunPara && iImageParameter->ntubes < fRunPara->fMinStarNTubes )
	{
		for( unsigned int i = 0; i < iImageParameter->fImageBorderPixelPosition_x.size(); i++ )
		{
			if( i < iImageParameter->fImageBorderPixelPosition_y.size() )
			{
				if( iStarCatalogue->getDistanceToClosestStar( iImageParameter->fImageBorderPixelPosition_x[i],
						iImageParameter->fImageBorderPixelPosition_y[i] ) < fRunPara->fMinStarPixelDistance_deg )
				{
					iArrayCut = false;
					if( fDebug )
					{
						cout << "Telescope " << iTel + 1 << endl;
						cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: bright star cut: ";
						cout << iStarCatalogue->getDistanceToClosestStar( iImageParameter->cen_x, iImageParameter->cen_y );
						cout << " (" << fRunPara->fMinStarPixelDistance_deg << " deg )" << endl;
					}
					if( !iArrayCut )
					{
						break;
					}
				}
			}
		}
	}
	
	if( fDebug )
	{
		cout << "VEvndispReconstructionParameter::applyArrayAnalysisCut: cut: " << iArrayCut << endl;
	}
	
	return iArrayCut;
}

/*
    add a new cut method
*/
void VEvndispReconstructionParameter::addNewMethod( unsigned int iRecordID )
{
	// first check if record ID already exists
	for( unsigned int i = 0; i < fRecordCounter.size(); i++ )
	{
		if( fRecordCounter[i] == iRecordID )
		{
			return;
		}
	}
	fNMethods++;
	
	// standard values
	
	vector< int > i_t;
	vector< double > i_d;
	vector< bool > i_b;
	vector< unsigned int > i_u;
	
	fRecordCounter.push_back( iRecordID );
	// standard array reconstruction method is '0'
	fMethodID.push_back( 0 );
	fNImages_min.push_back( 2 );
	fAxesAngles_min.push_back( 0. );
	fMLPFileName.push_back( "" );
        vector< string > i_temp_string;
	fTMVAFileNameVector.push_back( i_temp_string );
        vector< double > i_temp_double;
        fMTVAZenithBin.push_back( i_temp_double );
	fDispFileName.push_back( "" );
	fMODDISP_MinAngleForDisp.push_back( 25. );
	fMODDISP_MinAngleExpFactor.push_back( 0.02 );
	
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_t.push_back( 2 );
	}
	fLocalNtubes_min.push_back( i_t );
	i_t.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_t.push_back( 10000 );
	}
	fLocalNLowGain_max.push_back( i_t );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1. );
	}
	fLocalDistance_min.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e3 );
	}
	fLocalDistance_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1. );
	}
	fSize_min.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fSize_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1.e-2 );    // to allow for width==0
	}
	fWidth_min.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1.e10 );
	}
	fLength_min.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fLength_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fWidth_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1.e10 );
	}
	fAsym_min.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fAsym_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1.e3 );
	}
	fLocalAlpha_min.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e3 );
	}
	fLocalAlpha_max.push_back( i_d );
	// C. Duke 20oct06
	i_b.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_b.push_back( true );
	}
	fLocalUseImage.push_back( i_b );
	i_b.clear();
	fUseEventdisplayPointing.push_back( false );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fLoss_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fWidthLength_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1.e10 );
	}
	fFui_min.push_back( i_d );
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_u.push_back( 9999 );
	}
	fL2TriggerType.push_back( i_u );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( 1.e10 );
	}
	fMCEnergy_linTeV_max.push_back( i_d );
	i_d.clear();
	for( unsigned int i = 0; i < fNTel_type; i++ )
	{
		i_d.push_back( -1.e10 );
	}
	fMCEnergy_linTeV_min.push_back( i_d );
}


void VEvndispReconstructionParameter::print_arrayAnalysisCuts()
{
	cout << endl;
	cout << "------------------------------" << endl;
	cout << "----- Array Analysis Cuts ----" << endl;
	if( fNMethods > 1 )
	{
		cout << "------(" << fNMethods << " methods)------" << endl;
	}
	else
	{
		cout << "------(" << fNMethods << " method)------" << endl;
	}
	cout << endl;
	set< ULong64_t >::iterator fTel_type_iter;
	for( unsigned m = 0; m < fNMethods; m++ )
	{
		cout << "\t set number: " << m << ", array reconstruction method: " << fMethodID[m];
		if( fUseEventdisplayPointing[m] )
		{
			cout << ", use eventdisplay pointing (no pointing correction from DB or pointing monitor)";
		}
		cout << endl;
		cout << "\t\t minimum number of images: " << fNImages_min[m] << endl;
		cout << "\t\t minimum angle between image axes [deg]: " << fAxesAngles_min[m] << endl;
		if( fMLPFileName[m].size() > 0 )
		{
			cout << "\t\t MLP file: " << fMLPFileName[m] << endl;
		}
		if( m < fTMVAFileNameVector.size() && fTMVAFileNameVector[m].size() > 0 )
		{
                        for( unsigned int ze = 0; ze < fTMVAFileNameVector[m].size(); ze++ )
                        {
                            if( fTMVAFileNameVector[m][ze].size() > 0 && fTMVAFileNameVector[m][ze].find( "USE_BDT_METHOD" ) != string::npos )
                            {
                                cout << "\t\t TMVA (BDT) file used from method " << fTMVAFileNameVector[m][ze] << endl;
                            }
			    else
			    {
                                cout << "\t\t TMVA (BDT) file for zenith angle ";
                                if( m < fMTVAZenithBin.size() && ze < fMTVAZenithBin[m].size() ) cout << fMTVAZenithBin[m][ze] << " deg: ";
                                cout << fTMVAFileNameVector[m][ze] << endl;
                            }
                        }
		}
		if( fDispFileName[m].size() > 0 )
		{
			cout << "\t\t DISP table: " << fDispFileName[m] << endl;
		}
		if( fMethodID[m] == 9 )
		{
			cout << "\t\t Minimum angle to change from geometric to MVA method: [deg]: " << fMODDISP_MinAngleForDisp[m];
			cout << " (exp " << fMODDISP_MinAngleExpFactor[m] << ")" << endl;
		}
		cout << "\t\t Telescope\t\t\t\t\t\t\t ";
		for( unsigned int i = 0; i < fLocalNtubes_min[m].size(); i++ )
		{
			cout << i + 1 << "\t";
		}
		cout << endl;
		cout << "\t\t Telescope type\t\t\t\t\t\t\t ";
		for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
		{
			cout << *fTel_type_iter << "\t";
		}
		cout << endl;
		cout << "\t\t use image in reconstruction:\t\t\t\t\t ";
		for( unsigned int i = 0; i < fLocalUseImage[m].size(); i++ )
		{
			cout << fLocalUseImage[m][i] << "\t";
		}
		cout << endl;
		if( m < fL2TriggerType.size() )
		{
			cout << "\t\t L2 trigger type: \t\t\t\t\t\t ";
			for( unsigned int i = 0; i < fL2TriggerType[m].size(); i++ )
			{
				cout << fL2TriggerType[m][i] << "\t";
			}
			cout << endl;
		}
		cout << "\t\t minimum number of pixels per image:\t\t\t\t ";
		for( unsigned int i = 0; i < fLocalNtubes_min[m].size(); i++ )
		{
			cout << fLocalNtubes_min[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t maximum number of low gain pixel per image:\t\t\t ";
		for( unsigned int i = 0; i < fLocalNLowGain_max[m].size(); i++ )
		{
			cout << fLocalNLowGain_max[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t maximum local distance [deg]:\t\t\t\t\t ";
		for( unsigned int i = 0; i < fLocalDistance_max[m].size(); i++ )
		{
			cout << fLocalDistance_max[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t minimum image size [dc]:\t\t\t\t\t ";
		for( unsigned int i = 0; i < fSize_min[m].size(); i++ )
		{
			cout << fSize_min[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t minimum image width [deg]:\t\t\t\t\t ";
		for( unsigned int i = 0; i < fWidth_min[m].size(); i++ )
		{
			cout << fWidth_min[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t minimum asym :\t\t\t\t\t\t\t ";
		for( unsigned int i = 0; i < fAsym_min[m].size(); i++ )
		{
			cout << fAsym_min[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t maximum asym :\t\t\t\t\t\t\t ";
		for( unsigned int i = 0; i < fAsym_max[m].size(); i++ )
		{
			cout << fAsym_max[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t maximum loss on edge of the camera [%]:\t\t\t ";
		for( unsigned int i = 0; i < fLoss_max[m].size(); i++ )
		{
			cout << fLoss_max[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t maximum ratio width/length:\t\t\t\t\t ";
		for( unsigned int i = 0; i < fWidthLength_max[m].size(); i++ )
		{
			cout << fWidthLength_max[m][i] << "\t";
		}
		cout << endl;
		cout << "\t\t minimum image/border pixel fraction under image: \t\t";
		for( unsigned int i = 0; i < fFui_min[m].size(); i++ )
		{
			cout << fFui_min[m][i] << "\t";
		}
		cout << endl;
		if( fRunPara->isMC() )
		{
			cout << "\t\t cut on MC energy: \t\t\t\t\t\t ";
			for( unsigned int i = 0; i < fMCEnergy_linTeV_min[m].size(); i++ )
			{
				if( i < fMCEnergy_linTeV_max[m].size() )
				{
					cout << "[" << fMCEnergy_linTeV_min[m][i] << "," << fMCEnergy_linTeV_max[m][i] <<  "]\t";
				}
			}
			cout << endl;
		}
		
		for( unsigned int i = 0; i < fNTel_type; i++ )
		{
			if( fLocalNtubes_min[m][i] < 2 || fLoss_max[m][i] == 1.e10 )
			{
				cout << "Warning: not clear if array analysis cuts have been given for telescope type ";
				cout << i + 1 << " [set number: " << m << "]" << endl;
			}
		}
	}
	
	if( fRunPara && fRunPara->fStarCatalogueName.size() > 0 )
	{
		cout << endl;
		cout << "reading star catalogue from: " << fRunPara->fStarCatalogueName << endl;
		cout << "\t minimum brightness (B): " << fRunPara->fMinStarBrightness_B;
		if( fRunPara->fMinStarPixelDistance_deg < 1.e20 )
		{
			cout << " (max pixel distance " << fRunPara->fMinStarPixelDistance_deg << " deg)";
		}
		if( fRunPara->fMinStarNTubes < 100000 )
		{
			cout << ", (max number of pixels: " << fRunPara->fMinStarNTubes << ")";
		}
		cout << endl;
	}
	cout << "------------------------------" << endl;
	cout << endl;
}


/*!

   A RECMETHOD LINE STARTS ALWAYS A NEW RECORD

   line without '*' in the beginning are ignored

*/
unsigned int VEvndispReconstructionParameter::read_arrayAnalysisCuts( string ifile )
{
	if( ifile.size() == 0 )
	{
		return 0;
	}
	
	ifstream is;
	is.open( ifile.c_str(), ifstream::in );
	if( !is )
	{
		cout << "VEvndispReconstructionParameter::read_arrayAnalysisCuts error while opening array analysis cut file: " << ifile << endl;
		return 0;
	}
	cout << endl;
	cout << "reading reconstruction parameters (e.g. image quality cuts) from: " << endl;
	cout << ifile << endl;
	string iLine;
	string iTemp;
	string iTemp2;
	string iTemp3;
	string iTemp4;
	string iTemp5;
	string iTemp6;
	ULong64_t t_type = 0;
	int t_temp = 0;
	vector< int > v_temp;
	int m_temp = -1;
	
	while( getline( is, iLine ) )
	{
		// line without '*' in the beginning are ignored
		if( iLine.size() > 0 && iLine.substr( 0, 1 ) == "*" )
		{
			istringstream is_stream( iLine );
			is_stream >> iTemp;
			// proceed
			is_stream >> iTemp;
			// telescope type
			v_temp.clear();
			if( atoi( iTemp.c_str() ) >= 0 )
			{
				t_type = ULong64_t( atoi( iTemp.c_str() ) );
				t_temp = getTelescopeType_counter( t_type );
				v_temp = getTelescopeType_counterVector( t_type );
			}
			else if( atoi( iTemp.c_str() ) < -10 &&  atoi( iTemp.c_str() ) > -1000 )
			{
				t_type = ULong64_t( -1 * atoi( iTemp.c_str() ) );
				t_temp = getTelescopeType_counter_from_MirrorArea( t_type );
				v_temp = getTelescopeType_counter_from_MirrorAreaVector( t_type );
			}
			else if( atoi( iTemp.c_str() ) < -1000 )
			{
				t_type = ULong64_t( -1 * atoi( iTemp.c_str() ) );
				t_temp = getTelescopeType_counter_from_MirrorArea_and_PixelSize( t_type );
				v_temp = getTelescopeType_counter_from_MirrorArea_and_PixelSizeVector( t_type );
			}
			else
			{
				t_temp = -1;
			}
			// unknown telescope types
			if( t_temp == -2 )
			{
				continue;
			}
			
			// read variable identifier
			is_stream >> iTemp;
			iTemp = VUtilities::upperCase( iTemp );
			is_stream >> iTemp2;
			if( !is_stream.eof() )
			{
				is_stream >> iTemp3;
			}
			else
			{
				iTemp3 = "";
			}
			if( !is_stream.eof() )
			{
				is_stream >> iTemp4;
			}
			else
			{
				iTemp4 = "";
			}
			if( !is_stream.eof() )
			{
				is_stream >> iTemp5;
			}
			else
			{
				iTemp5 = "";
			}
			if( !is_stream.eof() )
			{
				is_stream >> iTemp6;
			}
			else
			{
				iTemp6 = "";
			}
			
			//////////////////////////////////////////////////////////////////////////////////////////////
			// fadc trace analysis
			if( iTemp == "FADCANALYSIS" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fTraceIntegrationMethod.size() )
						{
							fRunPara->fTraceIntegrationMethod[i] = ( unsigned int )atoi( iTemp2.c_str() );
						}
					}
				}
				continue;
			}
			// double pass options
			else if( iTemp == "FADCDOUBLEPASS" && fRunPara )
			{
				if( iTemp2.size() > 0 )
				{
					fRunPara->fDoublePass = atoi( iTemp2.c_str() );
				}
				if( iTemp3.size() > 0 )
				{
					for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
					{
						if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
						{
							if( i < fRunPara->fsumwindow_pass1.size() )
							{
								fRunPara->fsumwindow_pass1[i] = atoi( iTemp3.c_str() );
							}
						}
					}
				}
				if( iTemp4.size() > 0 )
				{
					for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
					{
						if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
						{
							if( i < fRunPara->fTraceIntegrationMethod_pass1.size() )
							{
								fRunPara->fTraceIntegrationMethod_pass1[i] = ( unsigned int )atoi( iTemp4.c_str() );
							}
						}
					}
				}
				// set double pass error option
				if( iTemp5.size() > 0 )
				{
					fRunPara->fDoublePassErrorWeighting2005 = !( bool )atoi( iTemp5.c_str() );
				}
                                // set maximal allowed time difference (in samples) between lg and hg T0
                                if( iTemp6.size() > 0 )
                                {
					for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
					{
						if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
						{
							if( i < fRunPara->fSumWindowMaxTimeDifferenceLGtoHG.size() )
							{
								fRunPara->fSumWindowMaxTimeDifferenceLGtoHG[i] = atof( iTemp6.c_str() );
							}
						}
					}
                                }
                                       
				continue;
			}
			else if( iTemp == "FADCSUMMATIONWINDOW" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fsumwindow_1.size() )
						{
							fRunPara->fsumwindow_1[i] = atoi( iTemp2.c_str() );
						}
						if( iTemp3.size() > 0 )
						{
							if( i < fRunPara->fsumwindow_2.size() )
							{
								fRunPara->fsumwindow_2[i] = atoi( iTemp3.c_str() );
							}
						}
						else
						{
							// window 1 and 2 are the same unless stated differently
							if( i < fRunPara->fsumwindow_2.size() )
							{
								fRunPara->fsumwindow_2[i] = atoi( iTemp2.c_str() );
							}
						}
					}
				}
				continue;
			}
			else if( iTemp == "FADCSUMMATIONSTART" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fsumfirst.size() )
						{
							fRunPara->fsumfirst[i] = atoi( iTemp2.c_str() );
						}
						if( iTemp3.size() > 0 )
						{
							if( i < fRunPara->fTraceWindowShift.size() )
							{
								fRunPara->fTraceWindowShift[i] = atoi( iTemp3.c_str() );
							}
						}
						if( iTemp4.size() > 0 )
						{
							if( i < fRunPara->fsumfirst_start_at_T0.size() )
							{
								if( iTemp4 == "T0" )
								{
									fRunPara->fsumfirst_start_at_T0[i] = true;
								}
								else
								{
									fRunPara->fsumfirst_start_at_T0[i] = false;
								}
							}
						}
					}
				}
				continue;
			}
			// image cleaning parameters
			else if( iTemp == "IMAGECLEANINGMETHOD" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fImageCleaningParameters.size() )
						{
							if( !fRunPara->fImageCleaningParameters[i]->setImageCleaningMethod( iTemp2 ) )
							{
								cout << "VEvndispReconstructionParameter: unknown image cleaning method: " << iTemp2 << endl;
							}
							if( iTemp3.size() > 0 )
							{
								if( iTemp3 == "FIXED" )
								{
									fRunPara->fImageCleaningParameters[i]->fUseFixedThresholds = true;
								}
								else
								{
									fRunPara->fImageCleaningParameters[i]->fUseFixedThresholds = false;
								}
							}
						}
					}
				}
				continue;
			}
			else if( iTemp == "IMAGECLEANINGTHRESHOLDS" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fImageCleaningParameters.size() )
						{
							fRunPara->fImageCleaningParameters[i]->fimagethresh = atof( iTemp2.c_str() );
						}
						if( iTemp3.size() > 0 )
						{
							if( i < fRunPara->fImageCleaningParameters.size() )
							{
								fRunPara->fImageCleaningParameters[i]->fborderthresh = atof( iTemp3.c_str() );
							}
						}
					}
				}
				continue;
			}
			else if( iTemp == "TIMECLEANINGPARAMETERS" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fImageCleaningParameters.size() )
						{
							fRunPara->fImageCleaningParameters[i]->ftimecutpixel = atof( iTemp2.c_str() );
						}
						if( iTemp3.size() > 0 )
						{
							if( i < fRunPara->fImageCleaningParameters.size() )
							{
								fRunPara->fImageCleaningParameters[i]->ftimecutcluster = atof( iTemp3.c_str() );
							}
						}
						if( iTemp4.size() > 0 )
						{
							if( i < fRunPara->fImageCleaningParameters.size() )
							{
								fRunPara->fImageCleaningParameters[i]->fminpixelcluster = atoi( iTemp4.c_str() );
							}
						}
						if( iTemp5.size() > 0 )
						{
							if( i < fRunPara->fImageCleaningParameters.size() )
							{
								fRunPara->fImageCleaningParameters[i]->floops = atoi( iTemp5.c_str() );
							}
						}
					}
				}
				continue;
			}
			else if( iTemp == "CORRELATIONCLEANINGPARAMETER" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fImageCleaningParameters.size() )
						{
							fRunPara->fImageCleaningParameters[i]->fCorrelationCleanBoardThresh = atof( iTemp2.c_str() );
						}
						if( iTemp3.size() > 0 && i < fRunPara->fImageCleaningParameters.size() )
						{
							fRunPara->fImageCleaningParameters[i]->fCorrelationCleanCorrelThresh = atof( iTemp3.c_str() );
						}
						if( iTemp4.size() > 0 && i < fRunPara->fImageCleaningParameters.size() )
						{
							fRunPara->fImageCleaningParameters[i]->fCorrelationCleanNpixThresh = atoi( iTemp4.c_str() );
						}
					}
				}
				continue;
			}
			else if( iTemp == "TIMETWOLEVELPARAMETERS" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fImageCleaningParameters.size() )
						{
							fRunPara->fImageCleaningParameters[i]->ftimediff = atof( iTemp2.c_str() );
						}
					}
				}
				continue;
			}

			else if( iTemp == "BRIGHTSTARS" && fRunPara )
			{
				fRunPara->fStarCatalogueName = iTemp2;
				if( iTemp3.size() > 0 )
				{
					fRunPara->fMinStarBrightness_B = atof( iTemp3.c_str() );
				}
				if( iTemp4.size() > 0 )
				{
					fRunPara->fMinStarPixelDistance_deg = atof( iTemp4.c_str() );
				}
				if( iTemp5.size() > 0 )
				{
					fRunPara->fMinStarNTubes = atoi( iTemp5.c_str() );
				}
				continue;
			}
			else if( iTemp == "LLEDGEFIT" && fRunPara )
			{
				for( unsigned int i = 0; i < fTel_type_V.size(); i++ )
				{
					if( t_temp < 0 || getTelescopeType_counter( fTel_type_V[i] ) == t_temp )
					{
						if( i < fRunPara->fLogLikelihoodLoss_min.size() )
						{
							fRunPara->fLogLikelihoodLoss_min[i] = atof( iTemp2.c_str() );
						}
						if( iTemp3.size() > 0 )
						{
							if( i < fRunPara->fLogLikelihood_Ntubes_min.size() )
							{
								fRunPara->fLogLikelihood_Ntubes_min[i] = atoi( iTemp3.c_str() );
							}
						}
					}
				}
				continue;
			}
			
			// FORCELL
			// Set in EVNDISP.reconstruction.runparameter , for example:
			// * -1 FORCELL 1
			// if set to 1, the imagefitting will use log-likelihood image fitting for all
			// images, regardless of whether the image is on the edge or not.
			// This will ignore the options specified by 'LLEDGEFIT'
			// If option is set to 0 or is not present, will behave normally
			// (i.e. LL image fit on edges, hillas ellipse in middle of camera)
			else if( iTemp == "FORCELL" && fRunPara )
			{
				if( atoi( iTemp2.c_str() ) == 1 )
				{
					fRunPara->fForceLLImageFit = true ;
				}
				else
				{
					fRunPara->fForceLLImageFit = false ;
				}
				cout << endl;
				cout << "FORCELL set to " << fRunPara->fForceLLImageFit << endl;
				cout << endl;
				continue;
			}

			// Model3D: reconstruction ID for starting values
			else if( iTemp == "MODEL3DSTARTID" && fRunPara )
			{
			        fRunPara->fIDstartDirectionModel3D = atoi( iTemp2.c_str() );
				continue;
			}

			/////////////////////////////////////////////////
                        // check for exit statement
                        if( iTemp == "EXIT" )
                        {
                            return fNMethods;
                        }
			// check for non-MC exist statement
			if( iTemp == "MCONLY" && !fRunPara->isMC() )
                        {
                            return fNMethods;
                        }

			
			/////////////////////////////////////////////////
			/////////////////////////////////////////////////
			/////////////////////////////////////////////////
			// A NEW RECORD STARTS ALWAYS WITH 'RECMETHOD'
			/////////////////////////////////////////////////
			/////////////////////////////////////////////////
			if( iTemp == "RECMETHOD" )
			{
				m_temp = fNMethods;
				// reset all parameters for this method number
				addNewMethod( m_temp );
				fMethodID[m_temp] = atoi( iTemp2.c_str() );
				// hardwired: allowed array reconstruction numbers
				if( fMethodID[m_temp] != 0 && fMethodID[m_temp] != 3 && fMethodID[m_temp] != 4
						&& fMethodID[m_temp] != 5 && fMethodID[m_temp] != 6 && fMethodID[m_temp] != 7 && fMethodID[m_temp] != 8 && fMethodID[m_temp] != 9 )
				{
					cout << "VEvndispReconstructionParameter: invalid array reconstruction method: " << fMethodID[m_temp] << endl;
					cout << "(allowed is 0,3,4,5,6,7,8,9)" << endl;
					cout << "...exiting" << endl;
					exit( 0 );
				}
				continue;
			}
			// check if a first record was created
			if( m_temp < 0 || fMethodID.size() == 0 )
			{
				cout << "VEvndispReconstructionParameter::read_arrayAnalysisCuts error: no valid set of cuts found " << endl;
				cout << "invalid line is: " << endl;
				cout << "\t " << iLine << endl;
				cout << "start set of cuts with the line: " << endl;
				cout << "* -1 RECMETHOD 0" << endl;
				exit( 0 );
			}
			if( iTemp == "USEEVNPOINTING" )
			{
				fUseEventdisplayPointing[m_temp] = true;
			}
			else if( iTemp == "MNIMAGE" )
			{
				fNImages_min[m_temp] = atoi( iTemp2.c_str() );
			}
			else if( iTemp == "MINANGLE" )
			{
				fAxesAngles_min[m_temp] = atof( iTemp2.c_str() );
			}
			else if( iTemp == "MLPFILE" )
			{
				fMLPFileName[m_temp] = iTemp2;
			}
			else if( iTemp == "TMVABDTFILE" )
			{
                                fMTVAZenithBin[m_temp].push_back( atof( iTemp2.c_str() ) );
                                if( iTemp3.size() > 0 )
                                {
                                    fTMVAFileNameVector[m_temp].push_back( iTemp3 );
                                }
			}
			else if( iTemp == "DISPFILE" )
			{
				fDispFileName[m_temp] = iTemp2;
			}
			else if( iTemp == "GEOMETRIC_MINANGLE" )
			{
				fMODDISP_MinAngleForDisp[m_temp] = atof( iTemp2.c_str() );
				if( iTemp3.size() > 0 )
				{
					fMODDISP_MinAngleExpFactor[m_temp] = atof( iTemp3.c_str() );
				}
			}
			else if( iTemp == "MINTUBES" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLocalNtubes_min[m_temp].size(); i++ )
					{
						fLocalNtubes_min[m_temp][i] = atoi( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLocalNtubes_min[m_temp][v_temp[i]] = atoi( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXLOWGAIN" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLocalNLowGain_max[m_temp].size(); i++ )
					{
						fLocalNLowGain_max[m_temp][i] = atoi( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLocalNLowGain_max[m_temp][v_temp[i]] = atoi( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINDIST" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLocalDistance_min[m_temp].size(); i++ )
					{
						fLocalDistance_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLocalDistance_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXDIST" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLocalDistance_max[m_temp].size(); i++ )
					{
						fLocalDistance_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLocalDistance_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINSIZE" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fSize_min[m_temp].size(); i++ )
					{
						fSize_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fSize_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINWIDTH" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fWidth_min[m_temp].size(); i++ )
					{
						fWidth_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fWidth_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			// trigger type (used e.g. in CTA prod2)
			else if( iTemp == "L2TRIGGERTYPE" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fL2TriggerType[m_temp].size(); i++ )
					{
						fL2TriggerType[m_temp][i] = ( unsigned int )( atoi( iTemp2.c_str() ) );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fL2TriggerType[m_temp][v_temp[i]] = ( unsigned int )( atoi( iTemp2.c_str() ) );
					}
			}
			else if( iTemp == "MAXSIZE" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fSize_max[m_temp].size(); i++ )
					{
						fSize_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fSize_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXWIDTH" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fWidth_max[m_temp].size(); i++ )
					{
						fWidth_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fWidth_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINLENGTH" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLength_min[m_temp].size(); i++ )
					{
						fLength_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLength_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXLENGTH" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLength_max[m_temp].size(); i++ )
					{
						fLength_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLength_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINASYM" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fAsym_min[m_temp].size(); i++ )
					{
						fAsym_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fAsym_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXASYM" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fAsym_max[m_temp].size(); i++ )
					{
						fAsym_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fAsym_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINALPHA" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLocalAlpha_min[m_temp].size(); i++ )
					{
						fLocalAlpha_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLocalAlpha_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXALPHA" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLocalAlpha_max[m_temp].size(); i++ )
					{
						fLocalAlpha_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLocalAlpha_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			// C. Duke 20oct06 new record in cut file
			else if( iTemp == "USEIMAGE" )
			{
				if( t_temp >= 0 )                 // use defaults for telescope number < 0
				{
					int tmpi = atoi( iTemp2.c_str() );
					if( !tmpi ) for( unsigned int i = 0; i < v_temp.size(); i++ )
						{
							fLocalUseImage[m_temp][v_temp[i]] = false;
						}
				}
			}
			else if( iTemp == "MAXLOSS" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fLoss_max[m_temp].size(); i++ )
					{
						fLoss_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fLoss_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MAXWIDTHLENGTH" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fWidthLength_max[m_temp].size(); i++ )
					{
						fWidthLength_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fWidthLength_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MINFUI" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fFui_min[m_temp].size(); i++ )
					{
						fFui_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fFui_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MCENERGYRANGE_MIN" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fMCEnergy_linTeV_min[m_temp].size(); i++ )
					{
						fMCEnergy_linTeV_min[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fMCEnergy_linTeV_min[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else if( iTemp == "MCENERGYRANGE_MAX" )
			{
				if( t_temp < 0 ) for( unsigned int i = 0; i < fMCEnergy_linTeV_max[m_temp].size(); i++ )
					{
						fMCEnergy_linTeV_max[m_temp][i] = atof( iTemp2.c_str() );
					}
				else for( unsigned int i = 0; i < v_temp.size(); i++ )
					{
						fMCEnergy_linTeV_max[m_temp][v_temp[i]] = atof( iTemp2.c_str() );
					}
			}
			else
			{
				cout << "\t VEvndispReconstructionParameter::read_arrayAnalysisCuts warning: unknown identifier: " << iTemp << endl;
			}
		}                                         // if( iLine.size() > 0 && iLine.substr( 0, 1 ) == "*" )
	}                                             // while( getline( is, iLine ) )
	return fNMethods;
}


int VEvndispReconstructionParameter::getTelescopeType_counter( ULong64_t t )
{
	unsigned int z = 0;
	set< ULong64_t >::iterator fTel_type_iter;
	for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
	{
		if( *fTel_type_iter == t )
		{
			return z;
		}
		z++;
	}
	return -2;
}

/*
   use mirror area flag to identify array analysis cut
*/
int VEvndispReconstructionParameter::getTelescopeType_counter_from_MirrorArea( ULong64_t t )
{
	unsigned int z = 0;
	ULong64_t v = 0;
	set< ULong64_t >::iterator fTel_type_iter;
	for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
	{
		v = *fTel_type_iter;
		v /= 100000;
		
		if( v > 2000 )
		{
			v -= 2000;
		}
		if( v > 1000 )
		{
			v -= 1000;
		}
		if( v == t )
		{
			return z;
		}
		z++;
	}
	return -2;
}

/*
   use mirror area and pixel size to identify array analysis cut
*/
int VEvndispReconstructionParameter::getTelescopeType_counter_from_MirrorArea_and_PixelSize( ULong64_t t )
{
	unsigned int z = 0;
	ULong64_t v = 0;
	ULong64_t v2 = 0;
	set< ULong64_t >::iterator fTel_type_iter;
	for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
	{
		v = *fTel_type_iter;
		
		ULong64_t v1 = v / 100000;
		if( v1 > 1000 )
		{
			v1 -= 1000;
		}
		v2 = v1 * 100 + ( v % 100 );
		
		if( v2 == t )
		{
			return z;
		}
		z++;
	}
	return -2;
}


vector< int > VEvndispReconstructionParameter::getTelescopeType_counterVector( ULong64_t t )
{
	unsigned int z = 0;
	vector< int > v;
	set< ULong64_t >::iterator fTel_type_iter;
	for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
	{
		if( *fTel_type_iter == t )
		{
			v.push_back( ( int )z );
		}
		z++;
	}
	return v;
}

/*
   use mirror area flag to identify array analysis cut
*/
vector< int > VEvndispReconstructionParameter::getTelescopeType_counter_from_MirrorAreaVector( ULong64_t t )
{
	unsigned int z = 0;
	ULong64_t v = 0;
	vector< int > x;
	set< ULong64_t >::iterator fTel_type_iter;
	for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
	{
		v = *fTel_type_iter;
		v /= 100000;
		
		if( v > 2000 )
		{
			v -= 2000;
		}
		if( v > 1000 )
		{
			v -= 1000;
		}
		if( v == t )
		{
			x.push_back( z );
		}
		z++;
	}
	return x;
}

/*
   use mirror area and pixel size to identify array analysis cut
*/
vector< int > VEvndispReconstructionParameter::getTelescopeType_counter_from_MirrorArea_and_PixelSizeVector( ULong64_t t )
{
	unsigned int z = 0;
	vector< int > x;
	ULong64_t v = 0;
	ULong64_t v2 = 0;
	set< ULong64_t >::iterator fTel_type_iter;
	for( fTel_type_iter = fTel_type.begin(); fTel_type_iter != fTel_type.end(); fTel_type_iter++ )
	{
		v = *fTel_type_iter;
		
		ULong64_t v1 = v / 100000;
		if( v1 > 1000 )
		{
			v1 -= 1000;
		}
		v2 = v1 * 100 + ( v % 100 );
		
		if( v2 == t )
		{
			x.push_back( z );
		}
		z++;
	}
	return x;
}

