/* \class VTMVADispAnalyzer
   \brief TMVA based disp analysis

*/

#include "VTMVADispAnalyzer.h"

VTMVADispAnalyzer::VTMVADispAnalyzer( string iFile, vector<ULong64_t> iTelTypeList )
{
	fDebug = false;
	
	bZombie = true;
	
	fWidth = 0.;
	fLength = 0.;
	fSize = 0.;
	fPedvar = 0.;
	fAsymm = 0.;
	fTGrad = 0.;
	fZe = 0.;
	fAz = 0.;
	fLoss = 0.;
	fXcore = 0.;
	fYcore = 0.;
	fcross = 0.;
	fDist = 0.;
	
	fTelescopeTypeList = iTelTypeList;
	
	if( fTelescopeTypeList.size() == 0 )
	{
		cout << "VTMVADispAnalyzer initializion error: telescope type list length is zero" << endl;
		bZombie = true;
		return;
	}
	
	for( unsigned int i = 0; i < fTelescopeTypeList.size(); i++ )
	{
	
      fTMVAReader[fTelescopeTypeList[i]] = new TMVA::Reader( "!Color:!Silent" );
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "width", &fWidth );
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "length", &fLength );
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "size", &fSize );
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "tgrad_x*tgrad_x", &fTGrad );
//      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "Xcore", &fXcore);
//      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "Ycore", &fYcore );
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "cross", &fcross );
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "asym", &fAsymm);
      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "loss", &fLoss );
//      fTMVAReader[fTelescopeTypeList[i]]->AddVariable( "dist", &fDist );
		
		ostringstream iFileName;
		iFileName << iFile << fTelescopeTypeList[i] << ".weights.xml";
		cout << "initializing TMVA disp analyzer: " <<  iFileName.str() << endl;
		if( !fTMVAReader[fTelescopeTypeList[i]]->BookMVA( "BDTDisp", iFileName.str().c_str() ) )
		{
			cout << "VTMVADispAnalyzer initializion error: xml weight file not found:" << endl;
			cout << "\t" << iFileName.str() << endl;
			bZombie = true;
			return;
		}
	}
	bZombie = false;
}

float VTMVADispAnalyzer::evaluate( float iWidth, float iLength, float iSize, float iAsymm, float iLoss, float iTGrad, float icen_x, float icen_y, float xoff_4, float yoff_4, ULong64_t iTelType, float iZe, float iAz )
{
	fWidth = iWidth;
	fLength = iLength;
	fSize = iSize;
	fTGrad = iTGrad * iTGrad;
	fZe = iZe;
	fAz = iAz;
	fcross = sqrt( (icen_y+yoff_4)*(icen_y+yoff_4) + (icen_x-xoff_4)*(icen_x-xoff_4) );
	fLoss = iLoss;
	fAsymm = iAsymm;

	
	
	if( fTMVAReader.find( iTelType ) != fTMVAReader.end() && fTMVAReader[iTelType] )
	{
	
	
		return ( fTMVAReader[iTelType]->EvaluateRegression( "BDTDisp" ) )[0];
	}
	
	return -1.;
}

void VTMVADispAnalyzer::terminate()
{
	return;
}
