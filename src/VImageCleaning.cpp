/*! \class VImageCleaning

     collection of different image cleaning methods

 */

#include "VImageCleaning.h"
#include "TopoTrigger.h"
#ifndef VIMAGENNCLEANING_H
#include "NNImageCleaningServiceFunc.h"
#define VIMAGENNCLEANING_H
#endif

VImageCleaning::VImageCleaning( VEvndispData* iData )
{
	fData = iData;
	
	fNSBscale = fData->getRunParameter()->fNSBscale;
	
	kInitNNImageCleaning = false;
	if( fData && fData->getImageCleaningParameter()->getImageCleaningMethod() == "TIMENEXTNEIGHBOUR" )
	{
		kInitNNImageCleaning = InitNNImageCleaning();
	}
	nRings        = 3;
	CoincWinLimit = 8;   // GMGM what unit is this?
}

void VImageCleaning::printDataError( string iFunctionName )
{
	cout << iFunctionName;
	cout << " error: no pointer to data class set" << endl;
	exit( -1 );
}

/*!
  tailcut cleaning with fixed thresholds
   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
*/
void VImageCleaning::cleanImageFixed( double hithresh, double lothresh, double brightthresh )
{
	if( !fData )
	{
		printDataError( "VImageCleaning::cleanImageFixed" );
	}
	
	fData->setImage( false );
	fData->setBorder( false );
	fData->setBrightNonImage( false );
	fData->setImageBorderNeighbour( false );
	unsigned int i_nchannel = fData->getNChannels();
	
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getSums()[i] > hithresh )
		{
			if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead( i, fData->getHiLo()[i] ) )
			{
				fData->setImage( i, true );
				for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
				{
					unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
					if( k < fData->getImage().size() && fData->getSums()[k] > lothresh && !fData->getImage()[k] )
					{
						fData->setBorder( k, true );
					}
				}
			}
		}
		if( fData->getSums()[i] > brightthresh )
		{
			if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead( i, fData->getHiLo()[i] ) )
			{
				fData->setBrightNonImage( i, true );
			}
		}
	}
	
	// (preli) set the trigger vector in MC case (preli)
	// trigger vector are image/border tubes
	if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4
			|| fData->getReader()->getDataFormatNum() == 6 )
	{
		fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
	}
	// (end of preli)
	if( fData->getRunParameter()->frecoverImagePixelNearDeadPixel )
	{
		recoverImagePixelNearDeadPixel();
	}
	if( fData->getRunParameter()->fFillImageBorderNeighbours )
	{
		fillImageBorderNeighbours();
	}
}


/*!

  signal-to-noise tailcut cleaning

   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold

*/
void VImageCleaning::cleanImagePedvars( double hithresh, double lothresh, double brightthresh )
{
	if( fData->getDebugFlag() )
	{
		cout << "VImageCleaning::cleanImagePedvars " << fData->getTelID() << endl;
	}
	
	fData->setImage( false );
	fData->setBorder( false );
	fData->setBrightNonImage( false );
	fData->setImageBorderNeighbour( false );
	unsigned int i_nchannel = fData->getNChannels();
	double i_pedvars_i = 0.;
	double i_pedvars_k = 0.;
	unsigned int k = 0;
	
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead( i, fData->getHiLo()[i] ) )
		{
			continue;
		}
		i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i] )[i];
		
		if( fData->getSums()[i] > hithresh * i_pedvars_i )
		{
			fData->setImage( i, true );
			fData->setBorder( i, false );
			for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
			{
				k = fData->getDetectorGeo()->getNeighbours()[i][j];
				if( k < i_nchannel )
				{
					i_pedvars_k = fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k] )[k];
					if( !fData->getImage()[k] && fData->getSums()[k] > lothresh * i_pedvars_k )
					{
						fData->setBorder( k, true );
					}
				}
			}
		}
		if( fData->getSums()[i] > brightthresh  * i_pedvars_i )
		{
			fData->setBrightNonImage( i, true );
		}
	}
	
	// (preli) set the trigger vector in MC case (preli)
	// trigger vector are image/border tubes
	if( fData->getReader() )
	{
		if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4
				|| fData->getReader()->getDataFormatNum() == 6 )
		{
			fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
		}
	}
	// (end of preli)
	
	recoverImagePixelNearDeadPixel();
	fillImageBorderNeighbours();
}


//*****************************************************************************************************
// NN image cleaning (Maxim)
//
//  Searches N-fold connected groups (4nn close-packed group) and 2+1 sparse groups
//  with dynamic cut in  GroupThresh-GroupCoincTime phase space
//
//  NOTE: IPR curves, used for cuts,  should be obtained with the SAME signal extraction method
//        as the image under study
// preliminary
int VImageCleaning::LocMax( int n, float* ptr, float& max )
{
	if( n <= 0 )
	{
		return -1;
	}
	max = ptr[0];
	Int_t xmax = 0;
	for( Int_t i = 1; i < n; i++ )
	{
		if( max < ptr[i] )
		{
			max = ptr[i];
			xmax = i;
		}
	}
	return xmax;
}
int VImageCleaning::LocMin( int n, float* ptr, float& min ) //ptr[i]>0
{
	if( n <= 0 )
	{
		return -1;
	}
	min = ptr[0];
	Int_t xmin = 0;
	for( Int_t i = 1; i < n; i++ )
	{
		if( min > ptr[i] )
		{
			min = ptr[i];
			xmin = i;
		}
	}
	return xmin;
}

/*
 *
 * initialize time-next-neighbour cleaning
 *
 *
 */
bool VImageCleaning::InitNNImageCleaning()
{
	TString refIPR, MSTrefIPR, SSTrefIPR;
	TString prefixProd2 = "";
#ifdef CTA_PROD2
	prefixProd2 += "Prod2";
#endif
	refIPR = fData->getRunParameter()->fIPR1File + ".root";
	MSTrefIPR = fData->getRunParameter()->fIPR2File + ".root";
	//SCSSTrefIPR=fData->getRunParameter()->fIPR3File;
	SSTrefIPR = fData->getRunParameter()->fIPR4File + ".root";
	cout << "refIPR = " << refIPR << " ifAnalyseFlash:" << fData->getRunParameter()->fPerformFlashCamAnalysis[1] << endl;
	cout << "MSTrefIPR = " << MSTrefIPR << " ifAnalyseFlash:" << fData->getRunParameter()->fPerformFlashCamAnalysis[2] << endl;
	cout << "SSTrefIPR = " << SSTrefIPR << " ifAnalyseFlash:" << fData->getRunParameter()->fPerformFlashCamAnalysis[4] << endl;
	
	if( !fData->getRunParameter()->fPerformFlashCamAnalysis[1] )
	{
		gIPR = ReadIPRGraph( refIPR,   "IPRcharge" );
		fFADCtoPhe[1] = ReadConvFactorsHist( refIPR, "hConvFactors", 1 );
	}
	if( !fData->getRunParameter()->fPerformFlashCamAnalysis[2] )
	{
		gIPR2 = ReadIPRGraph( MSTrefIPR, "IPRcharge" );
		fFADCtoPhe[2] = ReadConvFactorsHist( MSTrefIPR, "hConvFactors", 1 );
	}
	//    if(!fData->getRunParameter()->fPerformFlashCamAnalysis[3]){gIPR3=ReadIPRGraph(SSTrefIPR,"IPRcharge");fFADCtoPhe[3]=ReadConvFactorsHist(SSTrefIPR,"hConvFactors",1);}
	if( !fData->getRunParameter()->fPerformFlashCamAnalysis[4] )
	{
		gIPR4 = ReadIPRGraph( SSTrefIPR, "IPRcharge" );
		fFADCtoPhe[4] = ReadConvFactorsHist( SSTrefIPR, "hConvFactors", 1 );
	}
	
	if( fData->getRunParameter()->fPerformFlashCamAnalysis[1] )
	{
		gIPR = ReadIPRGraph( refIPR,   "IPRchargeFlash" );
		fFADCtoPhe[1] = ReadConvFactorsHist( refIPR, "hConvFactors", 2 );
	}
	if( fData->getRunParameter()->fPerformFlashCamAnalysis[2] )
	{
		gIPR2 = ReadIPRGraph( MSTrefIPR, "IPRchargeFlash" );
		fFADCtoPhe[2] = ReadConvFactorsHist( MSTrefIPR, "hConvFactors", 2 );
	}
	//    if(fData->getRunParameter()->fPerformFlashCamAnalysis[3]) {gIPR3=ReadIPRGraph(SSTrefIPR,"IPRchargeFlash");fFADCtoPhe[3]=ReadConvFactorsHist(SSTrefIPR,"hConvFactors",2);}
	if( fData->getRunParameter()->fPerformFlashCamAnalysis[4] )
	{
		gIPR4 = ReadIPRGraph( SSTrefIPR, "IPRchargeFlash" );
		fFADCtoPhe[4] = ReadConvFactorsHist( SSTrefIPR, "hConvFactors", 2 );
	}
	if( gIPR == NULL || gIPR2 == NULL || gIPR4 == NULL )
	{
		cout << "VImageCleaning::InitNNImageCleaning() one of IPR graphs is null... return" << endl;
		return false;
	}
	
	// GMGM what are these values?
	float fFakeImageProb = 0.5E-3; // 0.2%  for LST ->less for MST, SST
	float SimTime = 100.; //ns
	float fMinRate = fFakeImageProb / ( SimTime * 1E-9 * float( 3 ) ); //ns
	
	float CombFactor[5] = {60000.*20., 950000., 130000., 12000., 2.}; //for 2400 pixels
	const int types = 5; // GM hardwired number of telescope types
	fProb4nnCurves   = new TObjArray( types );
	fProb3nnrelCurves = new TObjArray( types );
	fProb2plus1Curves = new TObjArray( types );
	fProb2nnCurves   = new TObjArray( types );
	fProbBoundCurves = new TObjArray( types );
	
	for( int t = 0; t < types; t++ )
	{
		if( t < 1 || t > 4 )
		{
			continue;
		}
		float ChargeMax = gIPR->GetXaxis()->GetXmax();
		fProb4nnCurves->AddAt( RateContour( t, "ProbCurve4nn", fMinRate, 4, CombFactor[0], 0, ChargeMax ), t );
		fProb3nnrelCurves->AddAt( RateContour( t, "ProbCurve3nnrel", fMinRate, 3, CombFactor[2], 0, ChargeMax ), t );
		fProb2plus1Curves->AddAt( RateContour( t, "ProbCurve2plus1", fMinRate, 3, CombFactor[1], 0, ChargeMax ), t );
		fProb2nnCurves->AddAt( RateContour( t, "ProbCurve2nn", fMinRate, 2, CombFactor[3], 0, ChargeMax ), t );
		fProbBoundCurves->AddAt( RateContourBound( t, "ProbCurveBound", fMinRate, 4.0, CombFactor[4], 0, ChargeMax ), t );
		fFADCsampleRate[t] = fData->getRunParameter()->fFADCsampleRate[t];
		if( fFADCtoPhe[t] < 0. )
		{
			fFADCtoPhe[t] = 1;    // working in FADC counts directly
		}
		//fFADCtoPhe[t]=fData->getRunParameter()->fFADCtoPhe[t];
		std::cout << "fFADCtoPhe[" << t << "]=" << fFADCtoPhe[t] << " ChargeMax:" << gIPR->GetXaxis()->GetXmax() << " ifActiveType:" << fData->getRunParameter()->ifActiveType[t] << std::endl;
	}
	// init IPRs
	fIPRdim = 200;
	IPR = new float* [VDST_MAXTELTYPES];
	for( int t = 0; t < VDST_MAXTELTYPES; t++ )
	{
		IPR[t] = new float[fIPRdim];
	}
	for( int t = 0; t < VDST_MAXTELTYPES; t++ )
		for( unsigned int th = 0; th < fIPRdim; th++ )
		{
			IPR[t][th] = 0.;
		}
		
	return true;
}

void VImageCleaning::ScaleCombFactors( int type, float scale )
{
	float CombFactor[5] = {60000.*20., 950000., 130000., 6000., 2.}; //for 2400 pixels
	TF1* f4nn   = ( TF1* )fProb4nnCurves->At( type );
	TF1* f3nnrel = ( TF1* )fProb3nnrelCurves->At( type );
	TF1* f2plus1 = ( TF1* )fProb2plus1Curves->At( type );
	TF1* f2nn   = ( TF1* )fProb2nnCurves->At( type );
	f4nn->SetParameter( 2, CombFactor[0]*scale );
	f3nnrel->SetParameter( 2, CombFactor[2]*scale );
	f2plus1->SetParameter( 2, CombFactor[1]*scale );
	f2nn->SetParameter( 2, CombFactor[3]*scale );
}
void VImageCleaning::ResetCombFactors( int type )
{
	float CombFactor[5] = {60000.*20., 950000., 130000., 12000., 2.}; //for 2400 pixels
	ScaleCombFactors( type, fData->getNChannels() / 2400. ); // for this amount of pixels
	TF1* f4nn   = ( TF1* )fProb4nnCurves->At( type );
	TF1* f3nnrel = ( TF1* )fProb3nnrelCurves->At( type );
	TF1* f2plus1 = ( TF1* )fProb2plus1Curves->At( type );
	TF1* f2nn   = ( TF1* )fProb2nnCurves->At( type );
	f4nn->SetParameter( 2, CombFactor[0] );
	f3nnrel->SetParameter( 2, CombFactor[2] );
	f2plus1->SetParameter( 2, CombFactor[1] );
	f2nn->SetParameter( 2, CombFactor[3] );
}

bool VImageCleaning::BoundarySearch( int type, float thresh, TF1* fProbCurve, float refdT, int refvalidity, int idx )
{
	//idx - should be next neigbour of core!!!
	//skip core pix
	if( ( VALIDITYBUF[idx] > 1.9 && VALIDITYBUF[idx] < 6.1 ) )
	{
		return false;
	}
	
	// check for valid pixel number
	if( idx >= ( int )fData->getDetectorGeo()->getNeighbours().size() )
	{
		return false;
	}
	
	float TimeForReSearch = 0.;
	bool iffound = false;
	Int_t n = 0;
	float time = 0.;
	
	// reftime from core pixels
	for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[idx].size(); j++ )
	{
		const Int_t idx2 = fData->getDetectorGeo()->getNeighbours()[idx][j];
		Float_t t = TIMES[idx2];
		if( t > 0. && VALIDITYBUF[idx2] > 1.9 && VALIDITYBUF[idx2] < 5.1 )
		{
			time += t;
			n++;
		}
	}
	// check THIS pix and ring of neighbours
	if( n > 0.5 )
	{
		TimeForReSearch = time / float( n );
		float dT = fabs( TIMES[idx] - TimeForReSearch );
		float times[2] = {refdT, dT};
		float maxtime = 1E6;
		LocMax( 2, times, maxtime );
		float charges[2] = {thresh, INTENSITY[idx]};
		float mincharge = 0;
		LocMin( 2, charges, mincharge );
		if( maxtime < CoincWinLimit && maxtime < fProbCurve->Eval( mincharge ) && VALIDITY[idx] > 0.5 )
		{
			if( VALIDITYBOUND[idx] != refvalidity )
			{
				VALIDITYBOUND[idx] = refvalidity;
			}
			iffound = true;
		}
		
		for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[idx].size(); j++ )
		{
			const Int_t idx2 = fData->getDetectorGeo()->getNeighbours()[idx][j];
			if( ( TIMES[idx2] > 0. && VALIDITYBUF[idx2] > 1.9 && VALIDITYBUF[idx2] < 5.1 ) || VALIDITYBOUND[idx2] == refvalidity )
			{
				continue;
			}
			
			float dT2 = fabs( TIMES[idx2] - TimeForReSearch );
			times[1] = dT2;
			LocMax( 2, times, maxtime );
			charges[1] = INTENSITY[idx2];
			LocMin( 2, charges, mincharge );
			
			if( maxtime < CoincWinLimit && maxtime < fProbCurve->Eval( mincharge ) && VALIDITY[idx2] > 0.5 )
			{
				VALIDITYBOUND[idx2] = refvalidity;
				iffound = true;
			}
		}
	}
	return iffound;
}


int VImageCleaning::NNGroupSearchProbCurve( int type, TF1* fProbCurve, float PreCut ) // if Nfold =3 it will search for 2nn+1, including sparse groups (with the empty pix in between)
{
	int NN = ( int )fProbCurve->GetParameter( 1 ); //"Nfold"
	
	Float_t nn2;
	Float_t nn3;
	Float_t nn4;
	
	Int_t NN2 = 0;
	Int_t NN3 = 0;
	Int_t NN4 = 0;
	
	int NSBpix = 5;
	int numpix = fData->getDetectorGeo()->getNumChannels();
	
	for( int PixNum = 0; PixNum < numpix; PixNum++ )
	{
		Double_t q = VALIDITY[PixNum];
		if( q < 0.5 || INTENSITY[PixNum] < PreCut )
		{
			continue;
		}
		
		if( PixNum >= ( int )fData->getDetectorGeo()->getNeighbours().size() )
		{
			continue;
		}
		
		for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[PixNum].size(); j++ )
		{
			const Int_t PixNum2 = fData->getDetectorGeo()->getNeighbours()[PixNum][j];
			if( PixNum2 < 0 )
			{
				continue;
			}
			Double_t q1 = VALIDITY[PixNum2];
			
			if( q1 < 0.5 || INTENSITY[PixNum2] < PreCut )
			{
				continue;
			}
			Double_t dT = fabs( TIMES[PixNum] - TIMES[PixNum2] );
			float charges[2] = {INTENSITY[PixNum], INTENSITY[PixNum2]};
			float mincharge = 0;
			LocMin( 2, charges, mincharge );
			//if(dT>TrigWin2NN) continue;
			if( dT > CoincWinLimit || dT > fProbCurve->Eval( mincharge ) )
			{
				continue;
			}
			
			if( VALIDITYBUF[PixNum] < 2.9 )
			{
				VALIDITYBUF[PixNum] = 2;
			}
			if( VALIDITYBUF[PixNum2] < 2.9 )
			{
				VALIDITYBUF[PixNum2] = 2;
			}
			//if(VALIDITYBUF[PixNum2]<2.9) {VALIDITYBUF[PixNum2]=2;cout<<"charges:"<<charges[0]<<":"<<dT<<":"<<charges[1]<<" mincharge:"<<mincharge<<endl;}
			if( NN == 2 )
			{
				break;
			}
			// Boundary search (sparse group 2+1)
			// *************************
			if( VALIDITYBUF[PixNum2] == 2 && NN == 3 )
			{
				bool iffound = false;
				for( unsigned int k = 0; k < fData->getDetectorGeo()->getNeighbours()[PixNum].size(); k++ )
				{
					if( BoundarySearch( type, mincharge, fProbCurve, dT, 3, fData->getDetectorGeo()->getNeighbours()[PixNum][k] ) )
					{
						iffound = true;
					}
				}
				if( PixNum2 >= ( int )fData->getDetectorGeo()->getNeighbours().size() )
				{
					continue;
				}
				for( unsigned int k = 0; k < fData->getDetectorGeo()->getNeighbours()[PixNum2].size(); k++ )
				{
					if( BoundarySearch( type, mincharge, fProbCurve, dT, 3, fData->getDetectorGeo()->getNeighbours()[PixNum2][k] ) )
					{
						iffound = true;
					}
				}
				//cout<<"iffound:"<<iffound<<endl;
				if( iffound )
				{
					if( VALIDITYBUF[PixNum] < 3.9 )
					{
						VALIDITYBUF[PixNum] = 3;
					}
					if( VALIDITYBUF[PixNum2] < 3.9 )
					{
						VALIDITYBUF[PixNum2] = 3;
					}
				}
			}
			if( NN == 3 )
			{
				break;
			}
			// *************************
			
			if( NSBpix > 2 )
			{
				//Double_t y=PixelY[type*maxpixels + PixNum2];
				//Double_t x=PixelX[type*maxpixels + PixNum2];
				Double_t x = fData->getDetectorGeo()->getX()[PixNum2];
				Double_t y = fData->getDetectorGeo()->getY()[PixNum2];
				
				Int_t idxm = -1;
				Int_t idxp = -1;
				//Int_t idxq=-1;
				Int_t nn = 0;
				for( unsigned int kk = 0; kk < fData->getDetectorGeo()->getNeighbours()[PixNum].size(); kk++ )
				{
					const Int_t k = fData->getDetectorGeo()->getNeighbours()[PixNum][kk];
					if( k < 0 )
					{
						continue;
					}
					//Double_t xx=x-PixelX[type*maxpixels + k];
					//Double_t yy=y-PixelY[type*maxpixels + k];
					Double_t xx = x - fData->getDetectorGeo()->getX()[k];
					Double_t yy = y - fData->getDetectorGeo()->getY()[k];
					
					Double_t dist = sqrt( xx * xx + yy * yy );
					//Double_t diam=2.*PixelR[type*maxpixels];
					Double_t diam = 2.*fData->getDetectorGeo()->getTubeRadius()[1];
					if( dist > 0.01 * diam && dist < 1.1 * diam )
					{
						if( nn )
						{
							idxp = k;
						}
						else
						{
							idxm = k;
						}
						nn++;
					}
				}
				if( idxp < 0 )
				{
					continue;
				}
				
				Double_t q2 = VALIDITY[idxp];
				if( q2 < 0.5 || INTENSITY[idxp] < PreCut )
				{
					continue;
				}
				
				dT = fabs( TIMES[PixNum] - TIMES[idxp] );
				double dt2 = fabs( TIMES[PixNum2] - TIMES[idxp] );
				float charges2[2] = {mincharge, INTENSITY[idxp]};
				LocMin( 2, charges2, mincharge );
				float times2[2] = {dT, dt2};
				float maxtime = 1E6;
				LocMax( 2, times2, maxtime );
				if( maxtime > CoincWinLimit || maxtime > fProbCurve->Eval( mincharge ) )
				{
					continue;
				}
				
				if( VALIDITYBUF[PixNum] < 3.9 )
				{
					VALIDITYBUF[PixNum] = 3;
				}
				if( VALIDITYBUF[PixNum2] < 3.9 )
				{
					VALIDITYBUF[PixNum2] = 3;
				}
				if( VALIDITYBUF[idxp] < 3.9 )
				{
					VALIDITYBUF[idxp] = 3;
				}
				//if(VALIDITYBUF[idxp]<3.9)   {VALIDITYBUF[idxp]=3;cout<<"PreThresh:"<<PreCut<<" 3nn charges:"<<charges[0]<<"::::"<<dT<<","<<dt2<<", maxtime="<<maxtime<<"::::"<<charges[1]<<" mincharge:"<<mincharge<<endl;}
				if( NN == 3 )
				{
					break;
				}
				if( NSBpix > 3 )
				{
					if( idxm < 0 )
					{
						continue;
					}
					Double_t q3 = VALIDITY[idxm];
					if( q3 < 0.5 || INTENSITY[idxm] < PreCut )
					{
						continue;
					}
					dT = fabs( TIMES[PixNum] - TIMES[idxm] );
					dt2 = fabs( TIMES[PixNum2] - TIMES[idxm] );
					double dt3 = fabs( TIMES[idxp] - TIMES[idxm] );
					float charges3[2] = {mincharge, INTENSITY[idxm]};
					LocMin( 2, charges3, mincharge );
					float times3[4] = {maxtime, dT, dt2, dt3};
					LocMax( 4, times3, maxtime );
					
					if( maxtime > CoincWinLimit || maxtime > fProbCurve->Eval( mincharge ) )
					{
						continue;
					}
					
					VALIDITYBUF[PixNum] = 4;
					VALIDITYBUF[PixNum2] = 4;
					VALIDITYBUF[idxp] = 4;
					VALIDITYBUF[idxm] = 4;
					//cout<<"PreThresh:"<<PreCut<<" 4nn charges:"<<charges[0]<<"::::"<<dT<<","<<dt2<<","<<dt3<<", maxtime="<<maxtime<<"::::"<<charges[1]<<" mincharge:"<<mincharge<<endl;
					
				}
			}
		}
		
	}//end of for() loop
	
	for( Int_t i = 0; i < numpix; i++ )
	{
		float q = VALIDITYBUF[i];
		if( q > 1.5 && q < 2.1 )
		{
			NN2++;
		}
		if( q > 2.5 && q < 3.1 )
		{
			NN3++;
		}
		if( q > 3.5 && q < 4.1 )
		{
			NN4++;
		}
		if( VALIDITYBUF[i] < 1.9 )
		{
			VALIDITYBUF[i] = -1;
		}
	}
	
	nn2 = float( NN2 ) / 2.;
	nn3 = float( NN3 ) / 3.;
	nn4 = float( NN4 ) / 4.;
	
	int ngroups = 0;
	if( NN == 2 )
	{
		ngroups = int( nn2 + 0.5 );
	}
	if( NN == 3 )
	{
		ngroups = int( nn3 + 0.5 );
	}
	if( NN == 4 )
	{
		ngroups = int( nn4 + 0.5 );
	}
	return ngroups; //*/
}

int VImageCleaning::NNGroupSearchProbCurveRelaxed( int type, TF1* fProbCurve, float PreCut )
{
	int NN = ( int )fProbCurve->GetParameter( 1 ); //"Nfold"
	Int_t NN2 = 0;
	Int_t NN3 = 0;
	Int_t NN4 = 0;
	Double_t dT = 0, dt2 = 0, dt3 = 0;
	
	int numpix = fData->getDetectorGeo()->getNumChannels();
	
	for( int PixNum = 0; PixNum < numpix; PixNum++ )
	{
		int nng3[3];
		
		Double_t q = VALIDITY[PixNum];
		if( q < 0.5 || INTENSITY[PixNum] < PreCut )
		{
			continue;
		}
		int NNcnt = 1;
		int pix1 = 0, pix2 = 0, pix3 = 0, pix4 = 0;
		if( PixNum >= ( int )fData->getDetectorGeo()->getNeighbours().size() )
		{
			continue;
		}
		for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[PixNum].size(); j++ )
		{
			const Int_t PixNum2 = fData->getDetectorGeo()->getNeighbours()[PixNum][j];
			if( PixNum2 < 0 )
			{
				continue;
			}
			Double_t q1 = VALIDITY[PixNum2];
			
			if( q1 < 0.5 || INTENSITY[PixNum2] < PreCut )
			{
				continue;
			}
			dT = fabs( TIMES[PixNum] - TIMES[PixNum2] );
			//if(dT>TrigWin3NNrelax) continue;
			float charges[2] = {INTENSITY[PixNum], INTENSITY[PixNum2]};
			float mincharge = 0;
			LocMin( 2, charges, mincharge );
			//if(dT>TrigWin2NN) continue;
			if( dT > CoincWinLimit || dT > fProbCurve->Eval( mincharge ) )
			{
				continue;
			}
			float maxtime = 1E6;
			if( dT < fProbCurve->Eval( mincharge ) && q1 > 0.5 && INTENSITY[PixNum2] > PreCut )
			{
				pix1 = PixNum;
				if( PixNum2 != pix2 )
				{
					pix3 = PixNum2;
				}
				if( pix2 == 0 )
				{
					pix2 = PixNum2;
				}
				dt2 = fabs( TIMES[pix2] - TIMES[pix3] );
				dt3 = fabs( TIMES[pix1] - TIMES[pix3] );
				float charges2[3] = {mincharge, INTENSITY[pix3], INTENSITY[pix2]};
				LocMin( 3, charges2, mincharge );
				float times2[3] = {dT, dt2, dt3};
				LocMax( 3, times2, maxtime );
				
				if( maxtime < CoincWinLimit && maxtime < fProbCurve->Eval( mincharge ) )
				{
					NNcnt++;
				}
			}
			
			if( NNcnt > 2 )
			{
				if( VALIDITYBUF[pix1] < 4.9 )
				{
					VALIDITYBUF[pix1] = 5;
				}
				nng3[0] = pix1;
				if( VALIDITYBUF[pix2] < 4.9 )
				{
					VALIDITYBUF[pix2] = 5;
				}
				nng3[1] = pix2;
				if( VALIDITYBUF[pix3] < 4.9 )
				{
					VALIDITYBUF[pix3] = 5;
				}
				nng3[2] = pix3; //cout<<"PreThresh:"<<PreCut<<" 3nn charges:"<<INTENSITY[pix3]<<","<<INTENSITY[pix2]<<" mincharge:"<<mincharge<<"maxtime:"<<maxtime<<endl; }
				short VALIDITYLOCAL[numpix];
				memset( VALIDITYLOCAL, 0, sizeof( VALIDITYLOCAL ) );
				for( int n = 0; n < 3; n++ )
				{
					VALIDITYLOCAL[nng3[n]] = 10;
				}
				if( NN == 3 )
				{
					break;
				}
				//4 connected pixels
				for( int n = 0; n < 3; n++ )
				{
					if( nng3[n] >= ( int )fData->getDetectorGeo()->getNeighbours().size() )
					{
						continue;
					}
					for( unsigned int jj = 0; jj < fData->getDetectorGeo()->getNeighbours()[nng3[n]].size(); jj++ )
					{
						const Int_t testpixnum = fData->getDetectorGeo()->getNeighbours()[nng3[n]][jj];
						//if(testpixnum<0 || VALIDITY[testpixnum]<0.5 || VALIDITYBUF[testpixnum]==5) continue;
						if( testpixnum < 0 || VALIDITYLOCAL[testpixnum] == 10 )
						{
							continue;
						}
						if( INTENSITY[testpixnum] < PreCut )
						{
							continue;
						}
						
						float minchargeloc = 0, maxtimeloc = 0;
						float charges3[2] = {mincharge, INTENSITY[testpixnum]};
						LocMin( 2, charges3, minchargeloc );
						float times3[4] = {maxtime, fabs( TIMES[testpixnum] - TIMES[nng3[0]] ), fabs( TIMES[testpixnum] - TIMES[nng3[1]] ), fabs( TIMES[testpixnum] - TIMES[nng3[2]] )};
						LocMax( 4, times3, maxtimeloc );
						//cout<<"refpix:"<<nng3[n]<<" pixnum:"<<neighbor1[jj]<<" mincharge:"<<mincharge<<" "<<minchargeloc<<" maxtime:"<<maxtime<<" "<<maxtimeloc<<endl;
						
						if( maxtimeloc < CoincWinLimit && maxtimeloc < fProbCurve->Eval( minchargeloc ) )
						{
							NNcnt++;// cout<<"4conn group found. added pix:"<<pix4<<" | "<<pix1<<" "<<pix2<<" "<<pix3<<endl;
							pix4 = testpixnum;
							float groupsize = INTENSITY[pix1] + INTENSITY[pix2] + INTENSITY[pix3] + INTENSITY[pix4];
							if( groupsize > PreCut * 4.*1.5 )
							{
								if( VALIDITYBUF[pix1] < 5.9 )
								{
									VALIDITYBUF[pix1] = 6;
								}
								if( VALIDITYBUF[pix2] < 5.9 )
								{
									VALIDITYBUF[pix2] = 6;
								}
								if( VALIDITYBUF[pix3] < 5.9 )
								{
									VALIDITYBUF[pix3] = 6;
								}
								if( VALIDITYBUF[pix4] < 5.9 )
								{
									VALIDITYBUF[pix4] = 6;
								}
							}
							//cout<<"group size:"<<groupsize<<endl;
						}
					}
				}
			}// end if NNcnt>2
		}
		
	}//end of for() loop
	
	for( Int_t i = 0; i < numpix; i++ )
	{
		float q = VALIDITYBUF[i];
		if( q > 1.5 && q < 2.1 )
		{
			NN2++;
		}
		if( q > 4.5 && q < 5.1 )
		{
			NN3++;
		}
		if( q > 5.5 && q < 6.1 )
		{
			NN4++;
		}
		if( VALIDITYBUF[i] < 1.9 )
		{
			VALIDITYBUF[i] = -1;
		}
	}
	if( NN3 > 2 && NN == 3 )
	{
		return NN3 / 3;
	}
	if( NN4 > 3 && NN == 4 )
	{
		return NN4 / 4;
	}
	if( NN3 < 3 )
	{
		return 0;
	}
	return 0;
}
void VImageCleaning::DiscardIsolatedPixels( int type )
{
	unsigned int numpix = fData->getDetectorGeo()->getNumChannels();
	
	for( unsigned int PixNum = 0; PixNum < numpix; PixNum++ )
	{
		if( VALIDITY[PixNum] < 1.9 )
		{
			continue;
		}
		unsigned int NumOfNeighbor = 0;
		if( PixNum >= fData->getDetectorGeo()->getNeighbours().size() )
		{
			continue;
		}
		for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[PixNum].size(); j++ )
		{
			const Int_t PixNum2 = fData->getDetectorGeo()->getNeighbours()[PixNum][j];
			if( PixNum2 < 0 )
			{
				continue;
			}
			if( VALIDITY[PixNum2] > 1.9 )
			{
				NumOfNeighbor++;
			}
		}
		if( NumOfNeighbor < 0.5 )
		{
			VALIDITY[PixNum] = 0;
		}
	}
}
void VImageCleaning::DiscardTimeOutlayers( int type )
{
	unsigned int numpix = fData->getDetectorGeo()->getNumChannels();
	unsigned int Tcnt = 0;
	float meanTw = 0;
	float sigmaT = 0;
	float meanT = 0;
	float sigmaTw = 0;
	float sumw = 0;
	for( unsigned int pixnum = 0; pixnum < numpix; pixnum++ )
	{
		if( VALIDITY[pixnum] < 1.9 )
		{
			continue;
		}
		meanT += TIMES[pixnum];
		sigmaT += TIMES[pixnum] * TIMES[pixnum];
		meanTw += INTENSITY[pixnum] * TIMES[pixnum];
		sigmaTw += INTENSITY[pixnum] * TIMES[pixnum] * TIMES[pixnum];
		sumw += INTENSITY[pixnum];
		Tcnt++;
	}
	if( Tcnt > 1 )
	{
		meanT /= Tcnt;
		meanTw /= sumw;
		float radicand = ( sigmaT - Tcnt * meanT * meanT ) / ( float( Tcnt ) - 1. );
		float radicand2 = ( sigmaTw - sumw * meanTw * meanTw ) / ( float( sumw ) - 1. );
		if( radicand > 0 )
		{
			sigmaT = sqrt( radicand );
		}
		else
		{
			sigmaT = 1E6;
		}
		if( radicand2 > 0 )
		{
			sigmaTw = sqrt( radicand2 );
		}
		else
		{
			sigmaTw = 1E6;
		}
		for( unsigned int pp = 0; pp < numpix; pp++ )
		{
			if( VALIDITY[pp] < 1.9 )
			{
				continue;
			}
			if( fabs( TIMES[pp] - meanTw ) > 2.4 * sigmaTw )
			{
				//cout<<"TelType:"<<type<<"pix:"<<pp<<" "<<TIMES[pp]-meanTw<<" "<<sigmaTw<<endl;
				VALIDITY[pp] = 0;
			}
		}
	}
}

void VImageCleaning::DiscardLocalTimeOutlayers( int type, float NNthresh[6] )
{
	//   unsigned int numpix=fNumPixels[type];
	unsigned int numpix = fData->getDetectorGeo()->getNumChannels();
	unsigned int nimagepix = 0;
	DiscardIsolatedPixels( type );
	for( unsigned int pixnum = 0; pixnum < numpix; pixnum++ )
	{
		if( VALIDITY[pixnum] < 1.9 )
		{
			continue;
		}
		nimagepix++;
	}
	//******************************************************************
	// discard groups with no neighbouring group in the vicinity of 6pixels
	for( unsigned int pixnum = 0; pixnum < numpix; pixnum++ )
	{
		if( VALIDITY[pixnum] < 1.9 )
		{
			continue;
		}
		
		Double_t diam = 2.*fData->getDetectorGeo()->getTubeRadius()[1];
		Double_t x = fData->getDetectorGeo()->getX()[pixnum] / diam; // coord in pixels units
		Double_t y = fData->getDetectorGeo()->getY()[pixnum] / diam; // coord in pixels units
		
		unsigned int pixcnt = 0;
		unsigned int pixzerocnt = 0;
		// loop over vicinity of 2 rings around pixnum
		for( unsigned int pp = 0; pp < numpix; pp++ )
		{
			if( VALIDITY[pp] < 1.9 || pp == pixnum )
			{
				continue;
			}
			Double_t xx = x - fData->getDetectorGeo()->getX()[pp] / diam; // coord in pixels units
			Double_t yy = y - fData->getDetectorGeo()->getY()[pp] / diam; // coord in pixels units
			Double_t dist = sqrt( xx * xx + yy * yy );
			if( dist > 6.1 )
			{
				continue;
			}
			if( dist < 2.1 )
			{
				pixcnt++;
			}
			if( dist > 2.1 )
			{
				pixzerocnt++;
			}
			//cout<<"Tel:"<<fData->getTelID()<<" RefPix:"<<pixnum<<"  pix:"<<pp<<endl;
		}
		//cout<<"Tel:"<<fData->getTelID()<<"  Nimagepix:"<<nimagepix<<" RefPix:"<<pixnum<<" Vicinity(<2.1) cnt:"<<pixcnt<<" Vicinity(>2.1) cnt:"<<pixzerocnt<<endl;
		//if(nimagepix>4&&pixcnt<4&&pixzerocnt==0) VALIDITY[pixnum]=0;
		if( nimagepix > 7 && pixcnt < 7 && pixzerocnt == 0 )
		{
			VALIDITY[pixnum] = 0;
		}
	}
	//******************************************************************
	// loop over accepted pixels
	for( unsigned int pixnum = 0; pixnum < numpix; pixnum++ )
	{
		if( VALIDITY[pixnum] < 1.9 )
		{
			continue;
		}
		
		unsigned int Tcnt = 0;
		float meanTw = 0;
		float sigmaT = 0;
		float meanT = 0;
		float sigmaTw = 0;
		float sumw = 0;
		Double_t diam = 2.*fData->getDetectorGeo()->getTubeRadius()[1];
		Double_t x = fData->getDetectorGeo()->getX()[pixnum] / diam; // coord in pixels units
		Double_t y = fData->getDetectorGeo()->getY()[pixnum] / diam; // coord in pixels units
		
		// loop over vicinity of 2 rings around pixnum
		for( unsigned int pp = 0; pp < numpix; pp++ )
		{
			if( VALIDITY[pp] < 1.9 || pp == pixnum )
			{
				continue;
			}
			Double_t xx = x - fData->getDetectorGeo()->getX()[pp] / diam; // coord in pixels units
			Double_t yy = y - fData->getDetectorGeo()->getY()[pp] / diam; // coord in pixels units
			Double_t dist = sqrt( xx * xx + yy * yy );
			if( dist > 2.1 )
			{
				continue;
			}
			//cout<<"Tel:"<<fData->getTelID()<<" RefPix:"<<pixnum<<"  pix:"<<pp<<endl;
			meanT += TIMES[pp];
			sigmaT += TIMES[pp] * TIMES[pp];
			meanTw += INTENSITY[pp] * TIMES[pp];
			sigmaTw += INTENSITY[pp] * TIMES[pp] * TIMES[pp];
			sumw += INTENSITY[pp];
			Tcnt++;
		}
		if( Tcnt > 1 && nimagepix > 4 )
		{
			meanT /= ( float )Tcnt;
			meanTw /= sumw;
			float radicand = ( sigmaT - Tcnt * meanT * meanT ) / ( float( Tcnt ) - 1. );
			float radicand2 = ( sigmaTw - sumw * meanTw * meanTw ) / ( float( sumw ) - 1. );
			if( radicand > 0 )
			{
				sigmaT = sqrt( radicand );
			}
			else
			{
				sigmaT = 1E6;
			}
			if( radicand2 > 0 )
			{
				sigmaTw = sqrt( radicand2 );
			}
			else
			{
				sigmaTw = 1E6;
			}
			
			//float SNRlimit=5.0; if(Tcnt==2) SNRlimit=8.;
			float SNRlimit = 5.0;
			if( Tcnt == 2 )
			{
				SNRlimit = 9.;
			}
			float sigmalimit = 0.11; // sigmaT>sigmalimit:  time clustering due to finite sampling rate
			if( fabs( TIMES[pixnum] - meanT ) / sigmaT > SNRlimit && sigmaT > sigmalimit && INTENSITY[pixnum] < 1.5 * NNthresh[3] )
			{
				// cout<<"Tel:"<<fData->getTelID()<<" RefPix:"<<pixnum<<" Entrie cnt:"<<Tcnt<<" dTw:"<<TIMES[pixnum]-meanTw<<" SigdTw:"<<sigmaTw<<
				//     " SNRw:"<<fabs(TIMES[pixnum]-meanTw)/sigmaTw<<" dT:"<<TIMES[pixnum]-meanT<<" SigdT:"<<sigmaT<<" SNRw:"<<fabs(TIMES[pixnum]-meanT)/sigmaT<<endl;
				VALIDITY[pixnum] = 0;
			}
		}
	}
	//******************************************************************
}


void  VImageCleaning::SetNeighborRings( int type, unsigned short* VALIDITYBOUNDBUF, float* TIMESReSearch, float* REFTHRESH )
{
	unsigned int nfirstringpix = 0;
	//    unsigned int numpix=fNumPixels[type];
	unsigned int numpix = fData->getDetectorGeo()->getNumChannels();
	//Define search region, driven by found core pixels
	for( unsigned int p = 0; p < numpix; p++ )
	{
		TIMESReSearch[p] = 0.;
		REFTHRESH[p] = 0.;
		VALIDITYBOUNDBUF[p] = 0;
		if( VALIDITY[p] > 1.9 )
		{
			VALIDITYBOUNDBUF[p] = 2;
		}
	}
	for( unsigned int iRing = 0; iRing < nRings; iRing++ )
	{
		for( unsigned int idx = 0; idx < numpix; idx++ )
		{
			if( VALIDITYBOUNDBUF[idx] == 2 )
			{
				continue;
			}
			if( ( iRing > 0 ) && ( VALIDITYBOUNDBUF[idx] < iRing + 7 ) && ( VALIDITYBOUNDBUF[idx] > 1.9 ) )
			{
				continue;
			}
			float time = 0.;
			float refthresh = 0.;
			int n = 0;
			if( idx >= fData->getDetectorGeo()->getNeighbours().size() )
			{
				continue;
			}
			for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[idx].size(); j++ )
			{
				int idx2 = fData->getDetectorGeo()->getNeighbours()[idx][j];
				if( idx2 < 0 || VALIDITYBOUNDBUF[idx2] < 1.9 )
				{
					continue;
				}
				
				if( iRing == 0 )
				{
					if( VALIDITYBOUNDBUF[idx2] < 1.9 || VALIDITYBOUNDBUF[idx2] == iRing + 7 )
					{
						continue;
					}
					//n++;
					if( TIMESReSearch[idx2] > 0.01 )
					{
						time += TIMESReSearch[idx2];
						refthresh += INTENSITY[idx2];
						n++;
					}
					else
					{
						Float_t t = TIMES[idx2];
						if( t > 0. )
						{
							time += t;
							refthresh += INTENSITY[idx2];
							n++;
						}
					}
				}
				if( iRing > 0 )
				{
					if( VALIDITYBOUNDBUF[idx2] == iRing + 6 )
					{
						time += TIMESReSearch[idx2];
						refthresh += REFTHRESH[idx2];
						n++;
					}
				}
			}
			if( iRing == 0 && n > 0.5 )
			{
				TIMESReSearch[idx] = time / float( n );
				REFTHRESH[idx] = refthresh / float( n );
			}
			if( iRing > 0 && n > 0.5 )
			{
				TIMESReSearch[idx] = time / float( n );
				REFTHRESH[idx] = refthresh / float( n );
			}
			if( n > 0.5 )
			{
				VALIDITYBOUNDBUF[idx] = iRing + 7;
				if( iRing == 0 )
				{
					nfirstringpix++;
				}
				// cout<<"ring:"<<iRing<<" VALIDITY:"<<VALIDITYBOUNDBUF[idx]<<" pix:"<<idx<<" TimeReSearch:"<<TIMESReSearch[idx]<<" RefTh:"<<REFTHRESH[idx]<<endl;
			}
		}
	} // loop over rings
}

/*
 * time-next-neighbour cleaning (service function)
 *
 *
 */
float VImageCleaning::ImageCleaningCharge( int type, int& ngroups )
{
	unsigned int numpix = fData->getDetectorGeo()->getNumChannels();
	float corr = 1.;
	//                 [p.e.]4nn   2+1      3nn       2nn      Bound.   Bound Ref Charge
	float PreThresh[6] = {2.0 / corr, 3.0 / corr, 2.8 / corr, 5.2 / corr, 1.8 / corr, 4.0 / corr};
	// GMGM hardcoded number of telescope type, why is this not an array?
	if( type == 1 )
	{
		FillPreThresholds( gIPR, PreThresh );
	}
	if( type == 2 )
	{
		FillPreThresholds( gIPR2, PreThresh );
	}
	//if(type==3)FillPreThresholds(gIPR3, PreThresh);
	if( type == 4 )
	{
		FillPreThresholds( gIPR4, PreThresh );
	}
	ngroups = 0;
	memset( VALIDITYBOUND, 0, sizeof( VALIDITYBOUND ) );
	memset( VALIDITY, 0, sizeof( VALIDITY ) );
	memset( VALIDITYBUF, 0, sizeof( VALIDITYBUF ) );
	ResetCombFactors( type );
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( INTENSITY[p] > PreThresh[4] )
		{
			VALIDITY[p] = 1;
			VALIDITYBUF[p] = 1;
		}
		else
		{
			VALIDITY[p] = -1;
			VALIDITYBUF[p] = -1;
		}
	}
	
	//*************** Image Cleaning ******************************************************************************************
	ngroups = NNGroupSearchProbCurve( type, ( TF1* )fProb2nnCurves->At( type ), PreThresh[3] );
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITYBUF[p] == 2 )
		{
			VALIDITY[p] = 2;
		}
	}
	ngroups += NNGroupSearchProbCurve( type, ( TF1* )fProb2plus1Curves->At( type ), PreThresh[1] );
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITYBUF[p] == 3 || VALIDITYBOUND[p] == 3 )
		{
			VALIDITY[p] = 3;
		}
	}
	ngroups += NNGroupSearchProbCurveRelaxed( type, ( TF1* )fProb3nnrelCurves->At( type ), PreThresh[2] );
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITYBUF[p] == 5 )
		{
			VALIDITY[p] = 5;
		}
	}
	ngroups += NNGroupSearchProbCurveRelaxed( type, ( TF1* )fProb4nnCurves->At( type ), PreThresh[0] );
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITYBUF[p] == 6 )
		{
			VALIDITY[p] = 6;
		}
	}
	//ngroups+=NNGroupSearchProbCurve(type,(TF1*)fProb4nnCurves->At(type), PreThresh[0]);           for(unsigned int p=0;p<numpix;p++){if(VALIDITYBUF[p]==4) VALIDITY[p]=4; }
	// GMGM don't understund PreThresh[..] index
	// *********************************************************************
	//*********************************************************************
	// Boundary
	unsigned int ncorepix = 0;
	unsigned int ncore4nnpix = 0;
	unsigned int nboundsearchpix = 0;
	unsigned int nfirstringpix = 0;
	unsigned int nboundary = 0;
	float TIMESReSearch[numpix];
	unsigned short VALIDITYBOUNDBUF[numpix];
	float REFTHRESH[numpix];
	
	//*****************************************************************************************************************
	//boundary search (same as core search above but reduced search area (vicinity of core pixels) )
	//Define search region for boundary
	SetNeighborRings( type, &VALIDITYBOUNDBUF[0], &TIMESReSearch[0], &REFTHRESH[0] );
	//Reset  validity buffers (Important)
	for( unsigned int p = 0; p < numpix; p++ )
	{
		//if(VALIDITYBOUNDBUF[p]>1.9) {if(VALIDITYBOUNDBUF[p]>6){nboundsearchpix++;} VALIDITYBUF[p]=1;VALIDITYBOUND[p]=0;}//VALIDITY[p];}
		if( VALIDITYBOUNDBUF[p] > 1.9 )
		{
			{
				nboundsearchpix++;
			}
			VALIDITYBUF[p] = 1;
			VALIDITYBOUND[p] = 0;
		}//VALIDITY[p];}
		if( VALIDITYBOUNDBUF[p] < 1.9 )
		{
			VALIDITYBUF[p] = 0;
			VALIDITY[p] = 0;
		}
	}
	
	// all found pixels are set also to CORE pixels!!!
	if( ngroups > 0 )
	{
		ScaleCombFactors( type, float( nboundsearchpix ) / ( numpix * 1.5 ) );
		NNGroupSearchProbCurve( type, ( TF1* )fProb2nnCurves->At( type ), 0.8 * PreThresh[3] ); // GMGM why are thresholds scaled by 0.8 or 0.9?
		for( unsigned int p = 0; p < numpix; p++ )
		{
			if( VALIDITY[p] > 1.9 )
			{
				continue;
			}
			if( VALIDITYBUF[p] == 2 )
			{
				VALIDITY[p] = 2;
			}
		}
		
		NNGroupSearchProbCurve( type, ( TF1* )fProb2plus1Curves->At( type ), 0.8 * PreThresh[1] );
		for( unsigned int p = 0; p < numpix; p++ )
		{
			if( VALIDITY[p] > 1.9 )
			{
				continue;
			}
			if( VALIDITYBUF[p] == 3 || VALIDITYBOUND[p] == 3 )
			{
				VALIDITY[p] = 3;
			}
		}
		
		NNGroupSearchProbCurveRelaxed( type, ( TF1* )fProb3nnrelCurves->At( type ), 0.8 * PreThresh[2] );
		for( unsigned int p = 0; p < numpix; p++ )
		{
			if( VALIDITY[p] > 1.9 )
			{
				continue;
			}
			if( VALIDITYBUF[p] == 5 )
			{
				VALIDITY[p] = 5;
			}
		}
		
		NNGroupSearchProbCurveRelaxed( type, ( TF1* )fProb4nnCurves->At( type ), 0.9 * PreThresh[0] );
		for( unsigned int p = 0; p < numpix; p++ )
		{
			if( VALIDITY[p] > 1.9 )
			{
				continue;
			}
			if( VALIDITYBUF[p] == 6 )
			{
				VALIDITY[p] = 6;
			}
		}
		ResetCombFactors( type );
	}
	//calc corepix:
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITY[p] > 1.9 && VALIDITY[p] < 6.1 )
		{
			ncorepix++;
		}
	}
	float imagecharges[ncorepix];
	int cnt = 0;
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITY[p] > 1.9 && VALIDITY[p] < 6.1 )
		{
			imagecharges[cnt] = INTENSITY[p];
			cnt++;
		}
	}
	
	//set rings of boundaries for newly found core pixels
	SetNeighborRings( type, &VALIDITYBOUNDBUF[0], &TIMESReSearch[0], &REFTHRESH[0] );
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITYBOUNDBUF[p] == 7 )
		{
			nfirstringpix++;
			// cout<<"Pix:"<<p<<" secondPulse: "<<fData->getSumsSecond()[p]<<" t:"<<fData->getTimesSecond()[p]<<endl;
		}
	}
	
	
	// BOUNDARY pixel search (usually very few pixels are found)
	//only first ring
	TF1* fProbCurveBound = ( TF1* )fProbBoundCurves->At( type );
	for( Int_t iRing = 0; iRing < 1; iRing++ )
	{
		for( UInt_t idx = 0; idx < numpix; idx++ )
		{
			if( ( VALIDITY[idx] < 5.1 ) && ( VALIDITY[idx] > 1.9 ) )
			{
				continue;
			}
			if( ( iRing > 0 ) && ( VALIDITY[idx] < iRing + 7 ) && ( VALIDITY[idx] > 1.9 ) )
			{
				continue;
			}
			int n = 0;
			float time = 0.;
			float charge = 0.;
			
			if( idx >= fData->getDetectorGeo()->getNeighbours().size() )
			{
				continue;
			}
			for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[idx].size(); j++ )
			{
				const Int_t idx2 = fData->getDetectorGeo()->getNeighbours()[idx][j];
				if( idx2 < 0 || VALIDITYBOUNDBUF[idx2] < 1.9 )
				{
					continue;
				}
				
				if( iRing == 0 )
				{
					if( VALIDITY[idx2] < 1.9 || VALIDITY[idx2] == iRing + 7 )
					{
						continue;
					}
					if( TIMESReSearch[idx2] > 0. )
					{
						time += TIMESReSearch[idx2];
						charge += REFTHRESH[idx2];
						n++;
					}
					else
					{
						Float_t t = TIMES[idx2];
						if( t > 0. )
						{
							time += t;
							charge += INTENSITY[idx2];
							n++;
						}
					}
					//n++;
				}
				if( iRing > 0 )
				{
					if( VALIDITYBOUNDBUF[idx2] == iRing + 6 )
					{
						if( TIMESReSearch[idx2] > 0. )
						{
							time += TIMESReSearch[idx2];
							charge += REFTHRESH[idx2];
							n++;
						}
						else
						{
							Float_t t = TIMES[idx2];
							if( t > 0. )
							{
								time += t;
								charge += INTENSITY[idx2];
								n++;
							}
						}
					}
				}
			}
			if( n > 0.5 )
			{
				if( INTENSITY[idx] < PreThresh[4] )
				{
					continue;
				}
				TIMESReSearch[idx] = time / float( n );
				charge /= float( n );
				if( charge > 2.*PreThresh[3] )
				{
					charge = 2 * PreThresh[3];
				}
				float dT = fabs( TIMES[idx] - TIMESReSearch[idx] );
				//fProbCurveBound->SetParameter( 1, PreThresh[5] );
				fProbCurveBound->SetParameter( 1, charge );
				float charges[2] = {INTENSITY[idx], fProbCurveBound->GetParameter( 1 )};
				float refth = 0.;
				LocMin( 2, charges, refth );
				//fProbCurveBound->SetParameter( 2, 2.*( 1. + ncorepix * ( iRing )*pow( double( iRing + 1 ), 2. ) ) );
				fProbCurveBound->SetParameter( 2, 2.*nfirstringpix );
				if( dT < 0.6 * CoincWinLimit && dT < fProbCurveBound->Eval( refth ) )
				{
					VALIDITY[idx] = iRing + 7;
					//cout<<"pix:"<<idx<<" dt:"<<dT<<" Intensity:"<<INTENSITY[idx]<<" refth:"<<refth<<" Eval:"<<fProbCurveBound->Eval(refth)<<endl;
				}
			}
		}
	} // end of loop over rings
	
	if( ncorepix > 4 )
	{
		// order is important
		DiscardLocalTimeOutlayers( type, PreThresh );
		DiscardIsolatedPixels( type );
	}
	
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITY[p] > 6.1 )
		{
			nboundary++;
		}
		if( VALIDITY[p] == 6 )
		{
			ncore4nnpix++;
		}
	}
	//Discard small images and 4nn with no boundaries: Important protection from fully fake images which can destroy  shower dir reconstuction
	// prob of any of fake group is <0.8%
	// after this discarding prob of fake image <0.05% !!!
	
	if( ( ( ncorepix + nboundary ) < 4 ) || ( ncore4nnpix == 4 && nboundary == 0 && ncorepix == 4 ) )
	{
		for( unsigned int p = 0; p < numpix; p++ )
		{
			if( VALIDITY[p] > 1.9 )
			{
				VALIDITY[p] = 1;
			}
		}
		ngroups = 0;
	}
	int imageflag = 0;
	float SIZE = 0.;
	
	for( unsigned int p = 0; p < numpix; p++ )
	{
		if( VALIDITY[p] > 1.9 )
		{
			SIZE += INTENSITY[p];
			imageflag++;
		}
	}
	// GMGM    if(imageflag==0) SIZE=-1;
	if( imageflag == 0 )
	{
		SIZE = 0.;
	}
	return SIZE; //*/
}

/*
 *  time-next-neighbour cleaning
 *
 */
void VImageCleaning::cleanNNImageFixed()
{
	if( !fData )
	{
		printDataError( "VImageCleaning::cleanNNImageFixed" );
	}
	if( !kInitNNImageCleaning )
	{
		printDataError( "VImageCleaning::cleanNNImageFixed image cleaning not initialized" );
	}
	// calculates the valarray of tubes to be included in the parameterization
	fData->setImage( false );
	fData->setBorder( false );
	fData->setBrightNonImage( false );
	fData->setImageBorderNeighbour( false );
	unsigned int i_nchannel = fData->getNChannels();
	
	int type = getTrigSimTelType( fData->getTelType( fData->getTelID() ) );
	
	float sizecheck = 0;
	int ngroups = 0;
	
	//measure IPR
	float FADCslice = fData->getDetectorGeo()->getLengthOfSampleTimeSlice( fData->getTelID() );
	unsigned int nsamples = fData->getNSamples( fData->getTelID() );
	float ScanWindow = float( 0.5 * nsamples ) * FADCslice; //ns    half of long FADC trace is used for signal extraction in VTraceHandler (long trace files should ve used for IPR determination)
	if( fData->getRunParameter()->fTraceIntegrationMethod[fData->getTelID()] == 4 )
	{
		int EventCntLimit = 400000; // statistics needed to get IPRs with good quality up to ~40 phe threshold
		FillIPR( type );
		if( IPR[type][0] >= EventCntLimit )
		{
			GetIPRGraph( type, ScanWindow );
		}
	}
	//prepare for image cleaning (setting the charge and time for every pixel)
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		INTENSITY[i] = 0;
		TIMES[i] = -1;
		VALIDITY[i] = 0;
		if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead( fData->getHiLo()[i] )[i] )
		{
			INTENSITY[i] = fFADCtoPhe[type] * fData->getSums()[i];
			TIMES[i] = fData->getTraceAverageTime()[i] * FADCslice;
			if( TIMES[i] < 1.5 )
			{
				INTENSITY[i] = 0;
			}
		}
	}
	sizecheck = ImageCleaningCharge( type, ngroups );
	//set pixel's flags
	unsigned int ncorepix = 0;
	//    unsigned int nimagepix=0;
	if( ngroups > 0 )
	{
	
		float maxcharge = 0;
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			if( VALIDITY[i] > 1.9 )
			{
				if( INTENSITY[i] > maxcharge )
				{
					maxcharge = INTENSITY[i];
				}
			}
		}
		
		//        float BorderEdge=2.*sqrt(fNSBscale)/sqrt(0.6)*pow(sizecheck/40.,0.76);
		//float BorderEdge=2.*sqrt(fNSBscale)/sqrt(0.6)*pow(maxcharge/7.,0.76);
		//cout<<"BorderEdge:"<<BorderEdge<<" maxcharge:"<<maxcharge<<" ratio:"<<maxcharge/sizecheck<<endl;
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			fData->setImage( i, false );
			fData->setBorder( i, false );
			if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead( fData->getHiLo()[i] )[i] )
			{
				if( VALIDITY[i] > 1.9 && VALIDITY[i] < 6.1 )
				{
					fData->setImage( i, true );
					ncorepix++;
				}
				if( VALIDITY[i] > 6.1 )
				{
					fData->setBorder( i, true );
				}
				//if(VALIDITY[i]>1.9&&INTENSITY[i]>BorderEdge) {fData->setImage( i, true ); ncorepix++;}
				//if(VALIDITY[i]>6.1&&INTENSITY[i]>BorderEdge) fData->setBorder( i, true );
				//if(VALIDITY[i]>1.9) {fData->setImage( i, true ); ncorepix++;}
				//if(VALIDITY[i]>1.9&&INTENSITY[i]<BorderEdge) fData->setBorder( i, true );
				//if(VALIDITY[i]>1.9&&INTENSITY[i]>BorderEdge){nimagepix++;}
			}
		}
		/*
		if(nimagepix<5)
		{
		    for ( unsigned int i=0; i < i_nchannel; i++)
		    {
		        fData->setImage( i, false); fData->setBorder( i, false);
		        if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] )
		        {
		            if(VALIDITY[i]>1.9) {fData->setImage( i, true );}
		            if(VALIDITY[i]>6.1) fData->setBorder( i, true );
		        }
		    }
		}*/
	}
	recoverImagePixelNearDeadPixel();
	fillImageBorderNeighbours();
}

int  VImageCleaning::getTrigSimTelType( unsigned int fTelType )
{
	int type = 0;
	if( fTelType == 138704810 )
	{
		type = 1;    //LST
	}
	if( fTelType == 10408418 )
	{
		type = 2;    //MST
	}
	if( fTelType == 201509515 )
	{
		type = 3;    //SC-SST
	}
	if( fTelType ==  3709425 )
	{
		type = 4;    //DC-SST
	}
	//Prod1
	if( fTelType == 141305009 )
	{
		type = 1;    //LST
	}
	if( fTelType == 10007818 )
	{
		type = 2;    //MST
	}
	if( fTelType ==  3709725 )
	{
		type = 3;    //DC-SST
	}
	return type;
}

void VImageCleaning::cleanNNImagePedvars()
{

}

void VImageCleaning::FillPreThresholds( TGraph* gipr, float NNthresh[5] )
{
	// assuming monotonous ipr curve (as it should be)
	//                    4nn   2+1   3nn   2nn   bound  ref for bound
	//float ThreshFreq[6]={8.5E6,2.4E6,4.2E6,1.01E5,1.1E7,8.555E5};
	float ThreshFreq[6] = {8.5E6, 2.4E6, 4.2E6, 1.5E5, 1.1E7, 4.0E5};
	float gXres = ( gipr->GetXaxis()->GetXmax() - gipr->GetXaxis()->GetXmin() ) / gipr->GetN();
	for( int i = 0; i < 6; i++ )
	{
		for( int t = 0; t < gipr->GetN(); t++ )
		{
			float x = gipr->GetXaxis()->GetXmin() + gXres * ( float( t ) );
			float val = gipr->Eval( x );
			if( val <= ThreshFreq[i] )
			{
				NNthresh[i] = x;
				break;
			}
		}
	}
	if( NNthresh[4] < 0.001 )
	{
		NNthresh[4] = NNthresh[3] / 3.4;   //pre-threshold for boundary, if not set by IPR curve scanning due to IPR curve behaviour for small thresholds
	}
	if( NNthresh[0] < 0.001 )
	{
		NNthresh[0] = NNthresh[3] / 2.75;   //pre-threshold for boundary, if not set by IPR curve scanning
	}
	for( int i = 0; i < 6; i++ )
	{
		// std::cout<<"thresh:"<<NNthresh[i]<<" phe( approx. valid for Prod2 LST):"<<NNthresh[i]/33.<<std::endl;
	}
	
}

void VImageCleaning::CalcSliceRMS()  //TEMP
{
	int type = getTrigSimTelType( fData->getTelType( fData->getTelID() ) );
	float pedrms = 0.;
	float pedmean = 0.;
	unsigned int i_nchannel = fData->getNChannels();
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead( fData->getHiLo()[i] )[i] )
		{
			float val = fData->getSums()[i];
			pedmean += val;
			pedrms += val * val;
		}
	}
	pedmean /= float( i_nchannel );
	pedrms = sqrt( ( pedrms - float( i_nchannel ) * pedmean * pedmean ) / float( i_nchannel - 1 ) );
	cout << "Type:" << type << " ped:" << pedmean << " rms:" << pedrms << endl;
}
void VImageCleaning::FillIPR( unsigned int type ) //tel type
{
	float  gIPRUp = 1500.; //charge in FADC counts
	float  gIPRLo = 0.;
	float  gIPRStep = ( gIPRUp - gIPRLo ) / float( fIPRdim );
	float THRESH[fIPRdim];
	float RATE[fIPRdim];
	float RATEFlashCam[fIPRdim];
	float RATEall[fIPRdim];
	for( unsigned int thbin = 0; thbin < fIPRdim; thbin++ )
	{
		RATE[thbin] = 0;
		RATEall[thbin] = 0;
		RATEFlashCam[thbin] = 0;
		THRESH[thbin] = gIPRLo + float( thbin ) * gIPRStep;
	}
	
	//loop over pixels
	unsigned int i_nchannel = fData->getNChannels();
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead( fData->getHiLo()[i] )[i] )
		{
			// event counter
			IPR[type][0] += 1;
			for( unsigned int thbin = 1; thbin < fIPRdim; thbin++ )
			{
				float val = gIPRLo + float( thbin ) * gIPRStep;
				//if(INTENSITY[i]>val) RATE[thbin]+=1.;
				IPR[0][thbin] = val;
				if( fData->getSums()[i] > val )
				{
					RATE[thbin] += 1.;
				}
				if( fData->getSums()[i] > val )
				{
					IPR[type][thbin] += 1.;
				}
				//if(INTENSITYFLASH[p]>val) RATEFlashCam[thbin]+=1.;
				//if((INTENSITY[p]>val || INTENSITIES[1][p])){cout<<val<<" Cnts (max/all):"<<RATE[thbin]<<"/"<<RATEall[thbin]<<endl;}
				//for(int s=0;s<INTENSITIES[0][p];s++){if(INTENSITIES[s+1][p]>val) RATEall[thbin]+=1.;}
			}
			
		}
	}
	/*
	std::cout<<"IPR scan  (TelTypes):  Type1   Type2   Type3   Type4"<<std::endl;
	for(unsigned int th=0;th<fIPRdim;th++)
	{
	    //std::cout<<"Thresh[fadc]"<<float(th)*gIPRStep<<"  Cnts: "<<IPR[1][th]<<" "<<IPR[2][th]<<" "<<IPR[3][th]<<" "<<IPR[4][th]<<std::endl;
	    if(th==0) printf("NumOfmeasurements:      %5.0f  %5.0f  %5.0f  %5.0f \n",IPR[1][th],IPR[2][th],IPR[3][th],IPR[4][th]);
	    if(th>0 ) printf("DT[fadc]:%3.2f counts:  %5.0f  %5.0f  %5.0f  %5.0f \n",IPR[0][th],IPR[1][th],IPR[2][th],IPR[3][th],IPR[4][th]);
	}
	*/
}

TGraphErrors* VImageCleaning::GetIPRGraph( unsigned int type, float ScanWindow )
{
	TString title = Form( "IPRtype%dExtractor%d", type, fData->getRunParameter()->fTraceIntegrationMethod[fData->getTelID()] ); //FIXME always wrong extractor ID
	float RATES[fIPRdim],  RATESERR[fIPRdim];
	float RATESX[fIPRdim], RATESXERR[fIPRdim];
	
	for( unsigned int i = 0; i < fIPRdim; i++ )
	{
		RATES[i] = IPR[type][i];
		float ConvToHz = ( ScanWindow * IPR[type][0] ) / 1E9;
		if( ConvToHz < 0.9E-9 )
		{
			break;
		}
		RATES[i] /= ConvToHz;
		RATESX[i] = IPR[0][i];
		RATESERR[i] = sqrt( RATES[i] * ConvToHz ) / ConvToHz;
		//RATESXERR[i]=0.1*float(i)/float(res);
		RATESXERR[i] = 0.;
	}
	TGraphErrors* gRate = new TGraphErrors( fIPRdim, RATESX, RATES, RATESXERR, RATESERR );
	gRate->SetTitle( "IPRcharge" );
	gRate->SetName( "IPRcharge" );
	gRate->GetXaxis()->SetTitle( "Threshold, FADC" );
	gRate->GetYaxis()->SetTitle( "Rate, Hz" );
	gRate->SetMinimum( 1 );
	
	TFile* fgraph = new TFile( "$CTA_USER_DATA_DIR/" + title + ".root", "RECREATE" );
	gRate->Write();
	fgraph->Close();
	std::cout << "[TriggerAnalogueSummation::GetIPRGraph()]: graph root file written:" << "$CTA_USER_DATA_DIR/" + title + ".root" << std::endl;
	return gRate;
}


// end of NN image cleaning
//*****************************************************************************************************




/*!
  Image cleaning routine using pixel timing information
  based on Nepomuks PhD thesis time-cluster cleaning algorithm
   - uses fixed time differences for discrimination of pixels/clusters
   - adjusts time difference according to the time gradient
   - handles single core pixel
   - BrightNonImages not completely implemented yet (needs checks - but who is really using them?)

   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
   \par timeCutPixel time diffeence between pixels
   \par timeCutCluster time difference between clusters
   \par minNumPixel minimum number of pixels in a cluster
   \par loop_max number of loops
*/

void VImageCleaning::cleanImageFixedWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max )
{
	cleanImageWithTiming( hithresh, lothresh, brightthresh, timeCutPixel, timeCutCluster, minNumPixel, loop_max, true );
}

void VImageCleaning::cleanImagePedvarsWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max )
{
	cleanImageWithTiming( hithresh, lothresh, brightthresh, timeCutPixel, timeCutCluster, minNumPixel, loop_max, false );
}


void VImageCleaning::cleanImageWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max, bool isFixed )
{
	if( fData->getDebugFlag() )
	{
		cout << "VImageCleaning::cleanImageWithTiming " << fData->getTelID() << "\t" << brightthresh << endl;
	}
	
	// check if time gradient was already calculated
	if( fData->getImageParameters()->tgrad_x == 0 && fData->getImageParameters()->tint_x == 0 )
	{
		timeCutPixel = 5.0;
		timeCutCluster = fData->getNSamples();         // = number of readout samples
	}
	else
	{
		float tpix = fData->getImageParameters()->tgrad_x * 0.15; // VERITAS pixel size (HARDCODED)
		if( tpix > timeCutPixel )
		{
			timeCutPixel = tpix + 0.1;                     // added some uncertenties adhoc
		}
	}
	//     cout << "TimeImageCleaning: TC = " << timeCutPixel << " / " << timeCutCluster << " L = " << loop_max << endl;
	
	fData->setImage( false );
	fData->setBorder( false );
	fData->setBrightNonImage( false );
	
	unsigned int i_nchannel = fData->getNChannels();
	
	//////////////////////////////////////////////////////
	// STEP 1: Select all pixels with a signal > hithresh
	//
	
	double i_pedvars_i = 0.;
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead( fData->getHiLo()[i] )[i] )
		{
			continue;
		}
		
		if( isFixed )
		{
			if( fData->getSums()[i] > hithresh )
			{
				fData->setImage( i, true );
			}
			//	    if( fData->getSums()[i] > brightthresh ) fData->setBrightNonImage( i, true );
		}
		else
		{
			i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i] )[i];
			if( fData->getSums()[i] > hithresh * i_pedvars_i )
			{
				fData->setImage( i, true );
			}
			//	    if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) fData->setBrightNonImage( i, true );
		}
	}
	
	//////////////////////////////////////////////////////////
	// STEP 2: Make clusters
	//
	//       - group touching core pixels to clusters
	//         (only if the pixels are coincident in time!)
	
	
	// REALLY NEEDED: in case there is still something in the memory, reset all channels to cluster id 0
	//                needs to be checked for memory leaks!!!
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		fData->setClusterID( i, 0 );
	}
	
	int i_cluster = 0;
	int c_id = 0;
	
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getImage()[i] )
		{
			if( fData->getClusterID()[i] != 0 )
			{
				c_id = fData->getClusterID()[i];
			}
			else if( i_cluster == 0 || fData->getClusterID()[i] == 0 )
			{
				i_cluster++;
				c_id = i_cluster;
			}
			else
				cout << "WARNING: Something looks wrong - this should not happen\n"
					 << "[" << i_cluster << "/" << c_id << "] " << i << endl;
					 
			fData->setClusterID( i, c_id );
			
			if( i >= fData->getDetectorGeo()->getNeighbours().size() )
			{
				continue;
			}
			for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
			{
				unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
				if( fData->getImage()[k] && fData->getClusterID()[k] == 0 )
				{
					if( fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
					{
						fData->setClusterID( k, c_id );
					}
				}
			}
		}
	}
	
	//////////////////////////////////////////////////////////
	// STEP 3: Calculate for each cluster weighted mean time
	//
	//   - each cluster gets its own "clustertime" and "clustersize"
	//   - the cluster with the largest size is the "main" cluster
	//
	// new: for time difference adjustment calculate "cenx" and "ceny"
	
	int i_clusterNpix;      // number of pixels in cluster
	double i_clustersize;   // size of cluster
	double i_clustertime;   // weighted mean time of all pixels in a cluster
	
	double i_cenx, i_ceny;
	double i_clustercenx = 0.; // X center of gravity of the cluster
	double i_clusterceny = 0.; // Y center of gravity of the cluster
	
	double i_mainclustersize = 0; // size of the "main cluster"
	
	int cluster = 0;
	while( cluster <= i_cluster )
	{
	
		i_clusterNpix = 0;
		i_clustersize = 0.;
		i_clustertime = 0.;
		i_clustercenx = 0.;
		i_cenx = 0.;
		i_clusterceny = 0.;
		i_ceny = 0.;
		
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			if( fData->getClusterID()[i] == cluster && fData->getImage()[i] )
			{
				i_clusterNpix++;
				
				i_clustersize += fData->getSums()[i];
				i_clustertime += ( fData->getSums()[i] * fData->getTZeros()[i] );
				
				double xi = fData->getDetectorGeo()->getX()[i];
				double yi = fData->getDetectorGeo()->getY()[i];
				
				i_cenx += ( fData->getSums()[i] * xi );
				i_ceny += ( fData->getSums()[i] * yi );
			}
		}
		if( i_clustersize != 0 )
		{
			i_clustertime = i_clustertime / i_clustersize;
			i_clustercenx = i_cenx / i_clustersize;
			i_clusterceny = i_ceny / i_clustersize;
		}
		else
		{
			i_clustertime = -99;
			i_clustersize = -99;
			i_clustercenx = -99;
			i_clusterceny = -99;
		}
		
		fData->setClusterNpix( cluster, i_clusterNpix );
		fData->setClusterSize( cluster, i_clustersize );
		fData->setClusterTime( cluster, i_clustertime );
		fData->setClusterCenx( cluster, i_clustercenx );
		fData->setClusterCeny( cluster, i_clusterceny );
		
		//       cout << "### " << cluster << " Npix=" << i_clusterNpix << " size=" << i_clustersize << " time=" << i_clustertime
		// 	   << " cenx=" << i_clustercenx << " ceny=" << i_clusterceny << endl;
		
		if( i_clustersize == -99 )
		{
			fData->setMainClusterID( 0 );
		}
		else if( i_clustersize >= i_mainclustersize )
		{
			i_mainclustersize = i_clustersize;
			fData->setMainClusterID( cluster );
		}
		cluster++;
	}
	//     cout << "MAIN CLUSTER " << getMainClusterID()
	// 	 << ": Npix=" << getClusterNpix()[ getMainClusterID() ]
	// 	 << " Size=" << getClusterSize()[ getMainClusterID() ]
	// 	 << " Time=" << getClusterTime()[ getMainClusterID() ] << endl;
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// STEP 4: eliminate all clusters with time differences > Tcluster to the main cluster
	//
	//         NEW: use tgrad_x to calculate Tcluster
	
	
	int i_ID = 0;
	
	float i_mainX = 0.;    // c.o.g. (main cluster)
	float i_mainY = 0.;    // c.o.g. (main cluster)
	float i_mainXpos = 0.; // position on major axis
	float i_mainXtime = 0.; // time on major axis
	
	float i_clusterX, i_clusterY;  // c.o.g. (cluster)
	float i_clusterXpos;           // position on major axis
	float i_clusterXtime;          // time on major axis
	
	if( fData->getImageParameters()->tgrad_x != 0 && fData->getImageParameters()->tint_x != 0 )
	{
		i_mainX = fData->getClusterCenx()[ fData->getMainClusterID() ];
		i_mainY = fData->getClusterCeny()[ fData->getMainClusterID() ];
		i_mainXpos = i_mainX * fData->getImageParameters()->cosphi + i_mainY * fData->getImageParameters()->sinphi;
		i_mainXtime = fData->getImageParameters()->tgrad_x * i_mainXpos;
		
		i_clusterX = 0.;
		i_clusterY = 0.;
		i_clusterXpos = 0.;
		i_clusterXtime = 0.;
		
		cluster = 1;
		while( cluster <= i_cluster )
		{
			i_clusterX = fData->getClusterCenx()[ cluster ];
			i_clusterY = fData->getClusterCeny()[ cluster ];
			i_clusterXpos = i_clusterX * fData->getImageParameters()->cosphi + i_clusterY * fData->getImageParameters()->sinphi;
			i_clusterXtime = fData->getImageParameters()->tgrad_x * i_clusterXpos;
			
			for( unsigned int i = 0; i < i_nchannel; i++ )
			{
				i_ID = fData->getClusterID()[i];
				if( i_ID == 0 || i_ID == fData->getMainClusterID() )
				{
					continue;
				}
				
				if( i_ID == cluster && fData->getImage()[i] )
				{
					// 		    cout<< "  " <<i_ID << "  delta_T  = " << fabs(xtime - xtimemain) << endl;
					// 		    cout<< "  " <<i_ID << "  time diff= " << fabs( getClusterTime()[i_ID] - getClusterTime()[getMainClusterID()]) << endl;
					// 		    cout<< i_ID << " " << i << ": " << getClusterTime()[i_ID] << "-" << getClusterTime()[getMainClusterID()] << "="
					// 			<< getClusterTime()[i_ID] - getClusterTime()[getMainClusterID()]
					// 			<< " fabs(" << i_clusterXtime << "-" << i_mainXtime << "=" << i_clusterXtime - i_mainXtime << ") = "
					// 			<< fabs( (getClusterTime()[i_ID]-getClusterTime()[getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) << endl;
					
					if( fabs( ( fData->getClusterTime()[i_ID] - fData->getClusterTime()[fData->getMainClusterID()] ) - ( i_clusterXtime - i_mainXtime ) ) > timeCutCluster )
					{
						// 			cout << " --> "<< i << " cluster time cleaning : "
						// 			     << fabs( (getClusterTime()[i_ID]-getClusterTime()[getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) <<  endl;
						fData->setImage( i, false );
						fData->setClusterID( i, -99 );
					}
				}
			}
			cluster++;
		}
	}
	else
	{
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			if( fData->getClusterID()[i] == 0 )
			{
				continue;
			}
			
			if( fData->getImage()[i] )
			{
				i_ID = fData->getClusterID()[i];
				i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i] )[i];
				
				if( i_ID != fData->getMainClusterID() && fabs( fData->getClusterTime()[i_ID] - fData->getClusterTime()[fData->getMainClusterID()] ) > timeCutCluster )
				{
					// 		    cout << "----> "<< i << " was removed with cluster time cleaning" << endl;
					fData->setImage( i, false );
					fData->setClusterID( i, -99 );
					
					// 	      if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) setBrightNonImage( i, true );
					//  	      if( fData->getSums()[i] > hithresh  * i_pedvars_i ) setBrightNonImage( i, true );
				}
			}
		}
	}
	
	/////////////////////////////////////////////////////////////////
	// STEP 5: find boundary pixels (and add them to the cluster)
	//
	//  selection criteria:
	//  - signal above lothresh
	//  - loop through already cleaned direct neighbors with time difference < Tpixel
	//
	//  this step can be reiterated more than once (number of loops)
	//
	
	for( int loop = 0; loop < loop_max; loop++ )
	{
		int counter = 0;
		int tmp_border[i_nchannel];
		int tmp_cluster[i_nchannel];
		
		double i_pedvars_k = 0.;
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			// 	    if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 ||
			//  		fData->getClusterID()[i] == 0 || fData->getClusterID()[i] == -99 ) continue;
			
			i_ID = fData->getClusterID()[i];
			if( fData->getImage()[i] || fData->getBorder()[i] )
			{
				for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
				{
					unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
					
					if( isFixed )
					{
						if( !fData->getImage()[k] && !fData->getBorder()[k] && fData->getSums()[k] > lothresh
								&& fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
						{
							tmp_border[counter] = k;
							tmp_cluster[counter] = i_ID;
							counter++;
						}
					}
					else
					{
						i_pedvars_k = fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k] )[k];
						if( !fData->getImage()[k] && !fData->getBorder()[k]
								&& fData->getSums()[k] > lothresh * i_pedvars_k
								&& fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
						{
							tmp_border[counter] = k;
							tmp_cluster[counter] = i_ID;
							counter++;
						}
					}
				}
			}
		}
		if( fData->getMainClusterID() != 0 )
		{
			//  	    cout << "setBorder: ";
			for( int pixel = 0; pixel < counter; pixel++ )
			{
				//cout << tmp_border[pixel] << "(" << tmp_cluster[pixel] << ") , ";
				fData->setBorder( tmp_border[pixel], true );
				fData->setClusterID( tmp_border[pixel], tmp_cluster[pixel] );
			}
			//cout << endl;
		}
	}
	
	/////////////////////////////////////////////////////////////////////
	// STEP 6: merge touching clusters and reset cluster parameters
	//
	
	mergeClusters();
	
	//     for( unsigned int i = 0; i < i_nchannel; i++ ) setClusterID( i, 0 );
	for( int x = 0; x <= i_cluster; x++ )
	{
		fData->setClusterNpix( x, 0 );
		fData->setClusterSize( x, 0 );
		fData->setClusterTime( x, 0 );
	}
	
	i_mainclustersize = 0;
	cluster = 1;
	
	while( cluster <= i_cluster )
	{
	
		i_clusterNpix = 0;
		i_clustersize = 0.;
		i_clustertime = 0.;
		
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			if( fData->getClusterID()[i] == cluster )
			{
				if( fData->getImage()[i] || fData->getBorder()[i] )
				{
					i_clusterNpix++;
					i_clustersize += fData->getSums()[i];
					i_clustertime += fData->getSums()[i] * fData->getTZeros()[i];
				}
				else
				{
					fData->setClusterID( i, -99 );
				}
			}
		}
		if( i_clustersize != 0 )
		{
			i_clustertime = i_clustertime / i_clustersize;
		}
		else
		{
			i_clustertime = -99.;
			i_clustersize = -99;
		}
		
		fData->setClusterNpix( cluster, i_clusterNpix );
		fData->setClusterSize( cluster, i_clustersize );
		fData->setClusterTime( cluster, i_clustertime );
		
		//       if( i_clusterNpix > 0 )
		// 	cout << "     CLUSTER " <<cluster<< ": Npix=" <<i_clusterNpix<< " Size=" <<i_clustersize<< " Time=" << i_clustertime << endl;
		
		if( i_clustersize == -99 )
		{
			fData->setMainClusterID( 0 );
		}
		else if( i_clustersize >= i_mainclustersize )
		{
			i_mainclustersize = i_clustersize;
			fData->setMainClusterID( cluster );
		}
		cluster++;
	}
	
	//  count number of clusters before removing (useful for hadron rejection?)
	set< int > tmp_counter_uncleaned;
	for( unsigned int i = 0; i < fData->getNChannels(); i++ )
	{
		if( fData->getImage()[i] || fData->getBorder()[i] )
		{
			i_ID = fData->getClusterID()[i];
			if( i_ID != 0 || i_ID != -99 )
			{
				tmp_counter_uncleaned.insert( i_ID );
			}
		}
	}
	fData->setNcluster_uncleaned( tmp_counter_uncleaned.size() );
	
	/////////////////////////////////////////////////////////////
	// STEP 7: eliminate clusters with less then XX pixels
	//         & clusters where one core pixel has less then 2 direct border pixels
	
	removeSmallClusters( minNumPixel );
	
	
	/////////////////////////////////////////////////////////////
	// FINAL STEP:
	//
	//  - count number of clusters (useful for hadron rejection?)
	
	set< int > tmp_counter_cleaned;
	for( unsigned int i = 0; i < fData->getNChannels(); i++ )
	{
		if( fData->getImage()[i] || fData->getBorder()[i] )
		{
			i_ID = fData->getClusterID()[i];
			if( i_ID != 0 || i_ID != -99 )
			{
				tmp_counter_cleaned.insert( i_ID );
			}
		}
	}
	fData->setNcluster_cleaned( tmp_counter_cleaned.size() );
	
	
	
	//       cout << "BRIGHT NON IMAGES : ";
	//       for( unsigned int i = 0; i < i_nchannel; i++ )
	//       {
	//       // check for bright non images
	//       if( fData->getBrightNonImage()[i] )
	//       {
	//       if( fData->getImage()[i] ) setBrightNonImage( i, false );
	//       else if( fData->getBorder()[i] ) setBrightNonImage( i, false );
	//       }
	
	//       if( fData->getBrightNonImage()[i] )
	//       cout << i << ", ";
	//       }
	//       cout << endl;
	
	
	if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 || fData->getReader()->getDataFormatNum() == 6 )
	{
		fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
	}
	
	
	fillImageBorderNeighbours();
}


void VImageCleaning::mergeClusters()
{
	int i_clusterID;
	int k_clusterID;
	
	for( unsigned int i = 0; i < fData->getNChannels(); i++ )
	{
		if( fData->getImage()[i] || fData->getBorder()[i] )
		{
			i_clusterID = fData->getClusterID()[i];
			unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
			
			for( unsigned int j = 0; j < i_neighbour_size; j++ )
			{
				unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
				k_clusterID = fData->getClusterID()[k];
				
				if( ( fData->getImage()[k] || fData->getBorder()[k] ) && k_clusterID != 0 && k_clusterID != fData->getClusterID()[i] )
				{
					for( unsigned int n = 0; n < fData->getNChannels(); n++ )
					{
						if( ( fData->getImage()[n] || fData->getBorder()[n] ) && fData->getClusterID()[n] == fData->getClusterID()[k] )
						{
							fData->setClusterID( n, i_clusterID );
						}
					}
				}
			}
		}
	}
}


void VImageCleaning::removeSmallClusters( int minPix )
{
	int i_cluster = 0;
	
	for( unsigned int i = 0; i < fData->getNChannels(); i++ )
	{
		i_cluster = fData->getClusterID()[i];
		if( i_cluster == 0 || i_cluster == -99 )
		{
			continue;
		}
		
		// remove clusters with less then minPix
		if( fData->getClusterNpix()[i_cluster] < minPix )
		{
			// 	  if( fData->getImage()[i] || fData->getBorder()[i] ) cout << "ELIMINATOR ACTIVE for cluster " << i_cluster << " : ";
			
			if( fData->getImage()[i] )
			{
				// 	      cout << i << ", ";
				fData->setImage( i, false );
				fData->setClusterID( i, -99 );
			}
			else if( fData->getBorder()[i] )
			{
				// 	      cout << i << ", ";
				fData->setBorder( i, false );
				fData->setClusterID( i, -99 );
			}
			// 	  cout << endl;
		}
		
		// remove single core pixels with less than two direct border pixels
		int c1 = 0;
		int c2 = 0;
		
		bool dont_remove = false;
		
		if( fData->getImage()[i] )
		{
			unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
			for( unsigned int j = 0; j < i_neighbour_size; j++ )
			{
				unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
				if( fData->getImage()[k] )
				{
					dont_remove = true;
				}
				else if( fData->getBorder()[k] )
				{
					c1++;
				}
				else if( k < fData->getDead().size() && fData->getDead( fData->getHiLo()[k] )[k] )
				{
					for( unsigned l = 0; l < i_neighbour_size; l++ )
					{
						unsigned int m = fData->getDetectorGeo()->getNeighbours()[i][l];
						if( m != i && m < fData->getBorder().size() && ( fData->getBorder()[m] || fData->getImage()[m] ) )
						{
							c2++;
						}
					}
				}
			}
			if( dont_remove )
			{
				continue;
			}
		}
		
		if( c1 + c2 < 2 && fData->getImage()[i] )
		{
			// 	  cout << "----> "<< i << " was removed as single core pixel (incl ";
			fData->setImage( i, false );
			fData->setBorder( i, false );
			fData->setBrightNonImage( i, true );
			fData->setClusterID( i, -99 );
			
			// remove the rest of the single core cluster (if it exists)
			unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
			for( unsigned int j = 0; j < i_neighbour_size; j++ )
			{
				unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
				if( fData->getBorder()[k] )
				{
					fData->setBorder( k, false );
					// 		  cout << k << " ";
					
					for( unsigned l = 0; l < fData->getDetectorGeo()->getNNeighbours()[k]; l++ )
					{
						unsigned int m = fData->getDetectorGeo()->getNeighbours()[k][l];
						if( fData->getBorder()[m] )
						{
							fData->setBorder( m, false );
							// 			  cout << m << " ";
						}
					}
				}
			}
			// 	  cout << ")" << endl;
		}
	}
	
}

/*
 * fill a list with pixels which are neighbouring image or border pixels
 *
 */
void VImageCleaning::fillImageBorderNeighbours()
{
	fData->setImageBorderNeighbour( false );
	for( unsigned int i = 0; i < fData->getNChannels(); i++ )
	{
		if( fData->getImage()[i] || fData->getBorder()[i] )
		{
			// a pixel is its own neighbour :-)
			fData->getImageBorderNeighbour()[i] = true;
			// loop over all neighbours
			unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
			for( unsigned int j = 0; j < i_neighbour_size; j++ )
			{
				unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
				if( k < fData->getImageBorderNeighbour().size() && !fData->getDead()[k] )
				{
					fData->getImageBorderNeighbour()[k] = true;
				}
			}
		}
	}
}

/*
 * loop again to remove isolated image pixels
 * if neighbour is dead, check neighbours of this dead channel (see e.g. run 329 event 709)
 */
void VImageCleaning::recoverImagePixelNearDeadPixel()
{
	bool i_neigh = false;
	unsigned int i_neighbour_size = 0;
	unsigned int k = 0;
	
	for( unsigned int i = 0; i < fData->getNChannels(); i++ )
	{
		if( fData->getImage()[i] )
		{
			i_neigh = false;
			i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
			for( unsigned int j = 0; j < i_neighbour_size; j++ )
			{
				k = fData->getDetectorGeo()->getNeighbours()[i][j];
				if( k < fData->getBorder().size() && ( fData->getBorder()[k] || fData->getImage()[k] ) )
				{
					fData->setImage( i, true );
					i_neigh = true;
					break;
				}
				else if( k < fData->getDead().size() && fData->getDead( k, fData->getHiLo()[k] ) )
				{
					for( unsigned l = 0; l < i_neighbour_size; l++ )
					{
						unsigned int m = fData->getDetectorGeo()->getNeighbours()[i][l];
						if( m != i && m < fData->getBorder().size() && ( fData->getBorder()[m] || fData->getImage()[m] ) )
						{
							fData->setImage( i, true );
							i_neigh = true;
							break;
						}
					}
				}
				if( !i_neigh )
				{
					fData->setImage( i, false );
				}
			}
		}
		if( fData->getBrightNonImage()[i] )
		{
			if( fData->getImage()[i] )
			{
				fData->setBrightNonImage( i, false );
			}
			else if( fData->getBorder()[i] )
			{
				fData->setBrightNonImage( i, false );
			}
		}
	}
}

//
// MS: produce a trigger map for calculation of  binary-image Hillas parameters and apply some sort of cleaning algorithm to reject noisy pixels
//
void VImageCleaning::cleanTriggerFixed( double hithresh, double lothresh )
{
	if( fData->getDebugFlag() )
	{
		cout << "VImageCleaning::cleanTriggerFixed() " << hithresh << "\t" << lothresh << endl;
	}
	// MS: produce a trigger-level cleaned image. The logic here is that one might want to require
	// only patches of 3 nearest neighbors to constitute a valid center pixel.
	fData->setTrigger( false );
	unsigned int i_nchannel = fData->getNChannels();
	
	for( unsigned int i = 0; i < i_nchannel; i++ )
	{
		if( fData->getDetectorGeo()->getAnaPixel()[i] > 0  && !fData->getDead( fData->getHiLo()[i] )[i] )
		{
			// use the CFD hits
			if( fData->getRunParameter()->fPWmethod == 0 || fData->getRunParameter()->fPWmethod == 1 )
			{
				if( fData->getReader()->getFullTrigVec()[i] )
				{
					// ensure that at least XX neighbors are above this threshold
					int local_neighbors = 0;
					for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
					{
						unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
						if( k < fData->getTrigger().size()  && fData->getReader()->getFullTrigVec()[k] )
						{
							local_neighbors++;
						}
					}
					if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors )
					{
						fData->setTrigger( i, true );
					}
				}
			}
			
			// use a software cleaning algorithm for determining the trigger hits from the FADCs
			if( fData->getRunParameter()->fPWmethod == 2 || fData->getRunParameter()->fPWmethod == 3 )
			{
				if( fData->getSums()[i] > fData->getRunParameter()->fPWcleanThreshold )
				{
					// ensure that at least XX neighbors are above this threshold
					int local_neighbors = 0;
					for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
					{
						unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
						if( k < fData->getTrigger().size()  && fData->getSums()[k] > fData->getRunParameter()->fPWcleanThreshold )
						{
							local_neighbors++;
						}
					}
					if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors )
					{
						fData->setTrigger( i, true );
					}
				}
			}
		}
	}
	
	// then include the next row of pixels that "made" the center pixel alive
	if( fData->getRunParameter()->fPWmethod == 1 || fData->getRunParameter()->fPWmethod == 3 )
	{
		vector< bool > tmp_trigger( i_nchannel, 0 );
		
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			if( fData->getDetectorGeo()->getAnaPixel()[i] > 0  && !fData->getDead( fData->getHiLo()[i] )[i] )
			{
			
				if( fData->getReader()->getFullTrigVec()[i] )
				{
					int local_neighbors = 0;
					for( unsigned int j = 0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
					{
						unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
						if( k < fData->getTrigger().size()  &&  fData->getTrigger()[k] )
						{
							local_neighbors++;
						}
					}
					if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors - 1 )
					{
						tmp_trigger[i] = 1;
					}
				}
			}
		}
		
		for( unsigned int i = 0; i < i_nchannel; i++ )
		{
			fData->setTrigger( i, ( tmp_trigger[i] | fData->getTrigger()[i] ) );
		}
		
	}
	
	// assume that the image has been cleaned according to spec, then a new parameter will decide
	// wether  to apply a reduction in the number of pixels transmitted:
	// This will mask all pixels past the cut-off to be ZERO
	
	if( fData->getRunParameter()->fPWlimit > 0 )                   // then we'll transmit up to a fixed number of pixels per sector
	{
		const int n_sectors = 3;
		const int sector_max = 182;               // there are 192 channels per trigger sector, but not all are used
		int sector_count[n_sectors] = {0};
		
		for( int i = 0; i < n_sectors; i++ )
		{
			for( int j = 0; j < sector_max; j++ )
			{
				int current_pixel = sector_channel_to_pixel[j][i + 1]  - 1 ;
				
				if( fData->getTrigger()[ current_pixel  ] )
				{
					sector_count[ i  ]++;
				}
				
				if( sector_count[i] > fData->getRunParameter()->fPWlimit )
				{
					// then turn the pixel OFF, regardless wether or not it was ON
					fData->setTrigger( current_pixel, 0 );
				}
			}
			
		}
		
	}
}


void VImageCleaning::addImageChannel( unsigned int i_channel )
{
	if( fData->getDebugFlag() )
	{
		cout << "VImageCleaning::addImageChannel" << endl;
	}
	if( i_channel < fData->getImage().size() )
	{
		fData->setImageUser( i_channel, 1 );
	}
}


void VImageCleaning::removeImageChannel( unsigned int i_channel )
{
	if( fData->getDebugFlag() )
	{
		cout << "VImageCleaning::removeImageChannel" << endl;
	}
	if( i_channel < fData->getImage().size() )
	{
		fData->setImageUser( i_channel, -1 );
	}
}


void VImageCleaning::resetImageChannel( unsigned int i_channel )
{
	if( fData->getDebugFlag() )
	{
		cout << "VImageCleaning::resetImageChannel" << endl;
	}
	if( i_channel < fData->getImage().size() )
	{
		fData->setImageUser( i_channel, 0 );
	}
}


/*
   trace correlation cleaning

*/
double getTraceCorrelationValue( double Amean, double Bmean,
								 double Avar, double Bvar,
								 vector < double > vA, vector < double > vB )
{
	if( Avar == 0. || Bvar == 0. )
	{
		return 0.;
	}
	double N = 0;
	
	for( unsigned int i = 0; i < vA.size() && i < vB.size() ; i++ )
	{
		N = N + ( vA[i] - Amean ) * ( vB[i] - Bmean );
	}
	
	return N / TMath::Sqrt( Avar * Bvar );
}

double getTraceMean( vector < double > vA )
{
	if( vA.size() == 0 )
	{
		return 0.;
	}
	
	double N = 0;
	for( unsigned int i = 0; i < vA.size(); i++ )
	{
		N = N + vA[i];
	}
	
	return N / double( vA.size() );
}

double getTraceVar( vector < double > vA, double Am )
{
	double N = 0;
	for( unsigned int i = 0; i < vA.size(); i++ )
	{
		N = N + ( vA[i] - Am ) * ( vA[i] - Am );
	}
	
	return N;
}

/*
   trace correlation cleaning

*/
void VImageCleaning::cleanImageTraceCorrelate( double sigNoiseThresh, double corrThresh, double pixThresh )
{
	fData->setBorderCorrelationCoefficient( 0. );
	
	vector < vector < double > > vImageTraces( fData->getDetectorGeo()->getNChannels( fData->getTelID() ), vector<double>( ( int )fData->getNSamples(), 0 ) );
	
	unsigned int nhits = fData->getReader()->getNumChannelsHit();
	for( unsigned int i = 0; i < nhits; i++ )
	{
		unsigned int i_channelHitID = 0;
		try
		{
			i_channelHitID = fData->getReader()->getHitID( i );
			if( i_channelHitID < fData->getHiLo().size() && !fData->getDead( fData->getHiLo()[i_channelHitID] )[i_channelHitID] )
			{
			
				fData->getTraceHandler()->setTrace( fData->getReader(), fData->getNSamples(), fData->getPeds( fData->getHiLo()[i_channelHitID] )[i_channelHitID],
													fData->getPedrms( fData->getHiLo()[i_channelHitID] )[i_channelHitID], i_channelHitID, i,
													fData->getLowGainMultiplier_Trace()*fData->getHiLo()[i_channelHitID] );
													
				vImageTraces[i_channelHitID] = fData->getTraceHandler()->getTrace();
			}
		}
		catch( ... )
		{
			if( fData->getDebugLevel() == 0 )
			{
				cout << "VImageCleaning::cleanImageTraceCorrelate, index out of range (fReader->getHitID) ";
				cout << i << "(Telescope " << fData->getTelID() + 1 << ", event " << fData->getReader()->getEventNumber() << ")" << endl;
				fData->setDebugLevel( 0 );
			}
			continue;
		}
		
	}
	
	if( vImageTraces.size() > 0 )
	{
		vector < double > avepulse( fData->getNSamples(), 0 );
		vector < unsigned int > ImagePixelList;
		vector < unsigned int > NearbyPixelList;
		double AvePulseMean = 0;
		double AvePulseVar = 0;
		int nimage = 0;
		unsigned int k = 0;
		
		for( unsigned int i = 0; i < vImageTraces.size(); i++ )
		{
			if( fData->getImage()[i] || fData->getBorder()[i] )
			{
				nimage++;
				ImagePixelList.push_back( i );
				for( unsigned int j = 0; j < fData->getNSamples(); j++ )
				{
					avepulse[j] = avepulse[j] + vImageTraces[i][j];
				}
			}
		}
		
		if( nimage > 1 )
		{
			for( unsigned int j = 0; j < fData->getNSamples(); j++ )
			{
				avepulse[j] = avepulse[j] / double( nimage );
				AvePulseMean = AvePulseMean + avepulse[j];
			}
			AvePulseMean = AvePulseMean / double( fData->getNSamples() );
			
			for( unsigned int j = 0; j < fData->getNSamples(); j++ )
			{
				AvePulseVar = AvePulseVar + ( avepulse[j] - AvePulseMean ) * ( avepulse[j] - AvePulseMean );
			}
		}
		
		if( nimage > 3 && nimage < pixThresh )
		{
			//Build a list of pixels near the image pixels
			//Loop over image/border pixels
			for( unsigned int o = 0; o < ImagePixelList.size(); o++ )
			{
				unsigned int i = ImagePixelList[o];
				//Get the neighbours of this image pixel
				for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
				{
					//Check if it is already included in the neighbour list
					bool have = false;
					k = fData->getDetectorGeo()->getNeighbours()[i][j];
					for( unsigned int p = 0; p < NearbyPixelList.size(); p++ )
					{
						if( NearbyPixelList[p] == k )
						{
							have = true;
							break;
						}
					}
					//Check if it is already included in the image pixel list
					for( unsigned int p = 0; p < ImagePixelList.size(); p++ )
					{
						if( ImagePixelList[p] == k )
						{
							have = true;
							break;
						}
					}
					//Add to neighbour list if necessary
					if( !have )
					{
						NearbyPixelList.push_back( k );
					}
				}
			}
			
			
			for( unsigned int i = 0; i < NearbyPixelList.size(); i++ )
			{
				k =  NearbyPixelList[i];
				
				if( fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k] )[k] > 0. )
				{
					double tMean = getTraceMean( vImageTraces[k] );
					double tVar = getTraceVar( vImageTraces[k], tMean );
					
					
					double corv = getTraceCorrelationValue( AvePulseMean, tMean, AvePulseVar, tVar, avepulse, vImageTraces[k] );
					double sn = fData->getSums()[k] / fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k] )[k];
					
					// require that correlation coefficient and signal/noise is above certain thresholds
					if( corv > corrThresh && sn > sigNoiseThresh )
					{
						fData->setBorder( k, true );
						fData->setBorderCorrelationCoefficient( k, corv );
					}
					
				}
			}
		}
		recoverImagePixelNearDeadPixel();
		fillImageBorderNeighbours();
	}
}
