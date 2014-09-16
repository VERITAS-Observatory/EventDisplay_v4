/*! \file   plot_CTA
    \brief  CTA related plotting macros

*/

#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TText.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class VPlotCTAArrayLayout_TelescopeList
{
	public:
	
		int       fTelID;
		ULong64_t fTelType;
		string    fTelTypeName;
		int       fTelID_hyperArray;
		float     fTel_x;
		float     fTel_y;
		int       fMarkerColor;
		float     fMarkerSize;
		int       fMarkerType;
		
		VPlotCTAArrayLayout_TelescopeList();
		~VPlotCTAArrayLayout_TelescopeList() {}
};


class VPlotCTAArrayLayout
{
	private:
	
		vector< VPlotCTAArrayLayout_TelescopeList* > fTelescopeList;
		vector< VPlotCTAArrayLayout_TelescopeList* > fTelescopeList_subArray;
		
		void             drawTelescope( VPlotCTAArrayLayout_TelescopeList*, string iText );
		vector< string > getListofArrrays( string iArrayFile );
		
	public:
		VPlotCTAArrayLayout();
		~VPlotCTAArrayLayout() {}
		
		TCanvas* plot_array( string iArrayName = "", double xmax = 1450., double ymax = 1450., string iPrintCanvas = "", bool drawTelescopeNumbers = true );
		void     plot_fromList( string iArrayFile, string iDir, double xmax = 1450., double ymax = 1450.,
								bool drawTelescopeNumbers = true );
		bool     readArrayFromRootFile( string iFile );
		void     printArrayCosts();
		void     printListOfTelescopes();
		void     printTelescopeDistances( int iTelID, float iDistanceMax = 1.e99 );
		bool     printTelescopeIDs_for_differentHyperArray( string iFile );
		bool     setSubArray( string iSubArrayFile = "" );
};

VPlotCTAArrayLayout::VPlotCTAArrayLayout()
{
}

void VPlotCTAArrayLayout::printListOfTelescopes()
{
	for( unsigned int i = 0; i < fTelescopeList_subArray.size(); i++ )
	{
		cout << "Telescope " << i << " ID: " << fTelescopeList_subArray[i]->fTelID;
		cout << " (HA " << fTelescopeList_subArray[i]->fTelID_hyperArray << ")";
		cout << ", type " << fTelescopeList_subArray[i]->fTelType << " (" << fTelescopeList_subArray[i]->fTelTypeName << ")";
		cout << ", [" << fTelescopeList_subArray[i]->fTel_x << "," << fTelescopeList_subArray[i]->fTel_y << "]" << endl;
	}
}

void VPlotCTAArrayLayout::printArrayCosts()
{
	unsigned int iLST = 0;
	unsigned int iMST = 0;
	unsigned int iDCSST = 0;
	unsigned int iSCSST = 0;
	
	if( fTelescopeList_subArray.size() == 0 )
	{
		fTelescopeList_subArray = fTelescopeList;
	}
	
	for( unsigned int i = 0; i < fTelescopeList_subArray.size(); i++ )
	{
		if( fTelescopeList_subArray[i]->fTelTypeName.find( "LST" ) != string::npos )
		{
			iLST++;
		}
		else if( fTelescopeList_subArray[i]->fTelTypeName.find( "MST" ) != string::npos )
		{
			iMST++;
		}
		else if( fTelescopeList_subArray[i]->fTelTypeName.find( "DC-SST" ) != string::npos )
		{
			iDCSST++;
		}
		else if( fTelescopeList_subArray[i]->fTelTypeName.find( "SC-SST" ) != string::npos )
		{
			iSCSST++;
		}
	}
	
	// in MEuro
	float euro_LST = 6.279;
	float euro_MST = 1.713;
	float euro_7mSST = 0.954;
	float euro_4mSST = 0.497;
	
	cout << endl;
	cout << "# of LSTs: " << iLST << " (" << iLST* euro_LST << " MEuro)" <<  endl;
	cout << "# of MSTs: " << iMST << " (" << iMST* euro_MST << " MEuro)" << endl;
	cout << "# of DCSSTs: " << iDCSST << " (" << iDCSST* euro_7mSST << " MEuro)" << endl;
	cout << "# of SCSSTs: " << iSCSST << " (" << iSCSST* euro_4mSST << " MEuro)" << endl;
	cout << "===============================================" << endl;
	cout << "Tot: " << iLST* euro_LST + iMST* euro_MST + iDCSST* euro_7mSST + iSCSST* euro_4mSST << " MEuro" << endl;
	cout << endl;
	
}


bool VPlotCTAArrayLayout::setSubArray( string iSubArrayFile )
{
	fTelescopeList_subArray.clear();
	if( iSubArrayFile.size() == 0 )
	{
		return true;
	}
	// read list of telescope IDs
	ifstream is;
	is.open( iSubArrayFile.c_str(), ifstream::in );
	if( !is )
	{
		cout << "error, subarray file not found: " << endl;
		cout << iSubArrayFile << endl;
		return false;
	}
	
	int iTelID = 0;
	string is_line;
	while( getline( is, is_line ) )
	{
		if( is_line.size() > 0 )
		{
			istringstream is_stream( is_line );
			is_stream >> iTelID;
			if( iTelID == 0 )
			{
				continue;
			}
			
			for( unsigned int i = 0; i <  fTelescopeList.size(); i++ )
			{
				if( iTelID == fTelescopeList[i]->fTelID )
				{
					fTelescopeList_subArray.push_back( fTelescopeList[i] );
				}
			}
		}
	}
	is.close();
	
	cout << "Selected sub array with " << fTelescopeList_subArray.size() << " telescopes (from " << fTelescopeList.size() << ")" << endl;
	
	return true;
}


/*
 * read list of telescopes from root file
 *
*/

bool VPlotCTAArrayLayout::readArrayFromRootFile( string iFile )
{
	fTelescopeList.clear();
	fTelescopeList_subArray.clear();
	
	TFile* f1 = new TFile( iFile.c_str() );
	if( f1->IsZombie() )
	{
		return false;
	}
	
	TTree* t = ( TTree* )f1->Get( "telconfig" );
	if( !t )
	{
		return false;
	}
	
	cout << "Telconfig tree found with " << t->GetEntries() << " telescopes" << endl;
	
	float iTelX = 0.;
	float iTelY = 0.;
	ULong64_t iTelType = 0;
	int iTelID = 0;
	unsigned int iTelIDHA = 0;
	
	t->SetBranchAddress( "TelID", &iTelID );
	t->SetBranchAddress( "TelType", &iTelType );
	if( t->GetBranchStatus( "TelID_hyperArray" ) )
	{
		t->SetBranchAddress( "TelID_hyperArray", &iTelIDHA );
	}
	t->SetBranchAddress( "TelX", &iTelX );
	t->SetBranchAddress( "TelY", &iTelY );
	
	for( int i = 0; i < t->GetEntries(); i++ )
	{
		t->GetEntry( i );
		
		fTelescopeList.push_back( new VPlotCTAArrayLayout_TelescopeList() );
		
		fTelescopeList.back()->fTelID = iTelID;
		fTelescopeList.back()->fTelType = iTelType;
		fTelescopeList.back()->fTelID_hyperArray = ( int )iTelIDHA;
		fTelescopeList.back()->fTel_x = iTelX;
		fTelescopeList.back()->fTel_y = iTelY;
		// LSTs (1)
		if( iTelType == 138704810 || iTelType == 141305009 || iTelType == 141305109 )
		{
			fTelescopeList.back()->fTelTypeName = "23m-LST";
			fTelescopeList.back()->fMarkerColor = 2;
			fTelescopeList.back()->fMarkerSize = 2;
			fTelescopeList.back()->fMarkerType = 24;
		}
		// standard MSTs (2)
		else if( iTelType == 10007818 || iTelType == 10408418 || iTelType == 10008118 )
		{
			fTelescopeList.back()->fTelTypeName = "12m-MST";
			fTelescopeList.back()->fMarkerColor = 1;
			fTelescopeList.back()->fMarkerSize = 1.5;
		}
		// large pixel MSTs (4)
		else if( iTelType == 10009725 )
		{
			fTelescopeList.back()->fTelTypeName = "12m-MST-LPix";
			fTelescopeList.back()->fMarkerColor = 6;
			fTelescopeList.back()->fMarkerSize = 1.5;
		}
		// standard SSTs (3)
		else if( iTelType == 3709725 || iTelType == 3709425 || iTelType == 3710125 )
		{
			fTelescopeList.back()->fTelTypeName = "7m-DC-SST";
			fTelescopeList.back()->fMarkerColor = 3;
			fTelescopeList.back()->fMarkerSize = 1;
		}
		// 7m telescopes (5, prod1) or SCT (prod2)
		else if( iTelType == 7309930  || iTelType == 201509515 )
		{
			fTelescopeList.back()->fTelTypeName = "4m-SC-SST";
			fTelescopeList.back()->fMarkerColor = 4;
			fTelescopeList.back()->fMarkerSize = 1;
		}
		else
		{
			cout << "unknown telescope type: " << iTelType << endl;
		}
	}
	
	return true;
}

bool VPlotCTAArrayLayout::printTelescopeIDs_for_differentHyperArray( string iFile )
{
	TFile f1( iFile.c_str() );
	if( f1.IsZombie() )
	{
		return false;
	}
	
	TTree* t = ( TTree* )f1.Get( "telconfig" );
	if( !t )
	{
		return false;
	}
	
	cout << "Telconfig tree found with " << t->GetEntries() << " telescopes" << endl;
	
	float iTelX = 0.;
	float iTelY = 0.;
	ULong64_t iTelType = 0;
	int iTelID = 0;
	unsigned int iTelIDHA = 0;
	
	t->SetBranchAddress( "TelID", &iTelID );
	t->SetBranchAddress( "TelType", &iTelType );
	if( t->GetBranchStatus( "TelID_hyperArray" ) )
	{
		t->SetBranchAddress( "TelID_hyperArray", &iTelIDHA );
	}
	t->SetBranchAddress( "TelX", &iTelX );
	t->SetBranchAddress( "TelY", &iTelY );
	
	unsigned int z_tel = 0;
	for( unsigned int i = 0; i < fTelescopeList_subArray.size(); i++ )
	{
		bool bFound = false;
		if( !fTelescopeList_subArray[i] )
		{
			continue;
		}
		
		for( unsigned int j = 0; j < t->GetEntries(); j++ )
		{
			t->GetEntry( j );
			
			if( TMath::Abs( iTelX - fTelescopeList_subArray[i]->fTel_x ) < 1.e-2
					&& TMath::Abs( iTelY - fTelescopeList_subArray[i]->fTel_y ) < 1.e-2
					&& iTelType == fTelescopeList_subArray[i]->fTelType )
			{
				cout << iTelID << "\t" << iTelType << endl;
				bFound = true;
				z_tel++;
			}
		}
		if( !bFound )
		{
			cout << "TELESCOPE NOT FOUND: ID " << iTelID << ", type " << iTelType << " at [";
			cout << fTelescopeList_subArray[i]->fTel_x << "," << fTelescopeList_subArray[i]->fTel_y << "]" <<  endl;
		}
	}
	f1.Close();
	
	cout << "found " << z_tel << " telescopes (should be " << fTelescopeList_subArray.size() << ")" << endl;
	
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////


/*

   plot a single telescope

*/
void VPlotCTAArrayLayout::drawTelescope( VPlotCTAArrayLayout_TelescopeList* iD, string iText )
{
	if( !iD )
	{
		return;
	}
	
	TGraph* iG = new TGraph( 1 );
	iG->SetMarkerStyle( iD->fMarkerType );
	iG->SetMarkerSize( iD->fMarkerSize );
	iG->SetMarkerColor( iD->fMarkerColor );
	
	iG->SetPoint( 0, iD->fTel_x, iD->fTel_y );
	
	iG->Draw( "p" );
	
	if( iText.size() > 0 )
	{
		TText* iT = new TText( iD->fTel_x, iD->fTel_y, iText.c_str() );
		iT->SetTextSize( 0.014 );
		iT->SetTextColor( 2 );
		iT->Draw();
	}
}


/*
     plot array layout

*/
TCanvas* VPlotCTAArrayLayout::plot_array( string iname, double xmax, double ymax, string iPrintCanvas, bool drawTelescopeNumbers )
{
	char hname[200];
	char htitle[200];
	if( iname.size() > 0 )
	{
		sprintf( hname, "cArrayLayout_%s", iname.c_str() );
		sprintf( htitle, "array layout (%s)", iname.c_str() );
	}
	else
	{
		sprintf( hname, "cArrayLayout" );
		sprintf( htitle, "array layout" );
	}
	
	TCanvas* c = new TCanvas( hname, htitle, 10, 10, 800, 800 );
	c->SetGridx( 0 );
	c->SetGridy( 0 );
	c->SetRightMargin( 0.05 );
	c->SetLeftMargin( 0.13 );
	c->Draw();
	
	sprintf( hname, "hnull_%s", iname.c_str() );
	TH2D* hnull = new TH2D( hname, "", 100, -1.*xmax, xmax, 100, -1.*ymax, ymax );
	hnull->SetStats( 0 );
	hnull->SetXTitle( "x [m]" );
	hnull->SetYTitle( "y [m]" );
	hnull->GetYaxis()->SetTitleOffset( 1.6 );
	
	hnull->Draw();
	
	// check that sub array is set
	if( fTelescopeList_subArray.size() == 0 )
	{
		fTelescopeList_subArray = fTelescopeList;
	}
	
	for( unsigned int i = 0; i < fTelescopeList_subArray.size(); i++ )
	{
		if( !fTelescopeList_subArray[i] )
		{
			continue;
		}
		
		if( drawTelescopeNumbers )
		{
			sprintf( hname, "%d, %d", fTelescopeList_subArray[i]->fTelID, fTelescopeList_subArray[i]->fTelID_hyperArray );
			drawTelescope( fTelescopeList_subArray[i], hname );
		}
		else
		{
			drawTelescope( fTelescopeList_subArray[i], "" );
		}
		
		
	}
	// draw array name
	if( iname.size() > 0 )
	{
		TText* it = new TText( -1.*0.9 * xmax, 0.75 * ymax, iname.c_str() );
		it->SetTextSize( it->GetTextSize() * 1.5 );
		it->Draw();
	}
	if( iPrintCanvas.size() > 0 )
	{
	    sprintf( hname, "%s.%s", iname.c_str(), iPrintCanvas.c_str() );
	    c->Print( hname );
        }
	
	return c;
}
void VPlotCTAArrayLayout::printTelescopeDistances( int iTelID, float iDistanceMax )
{
	// check that sub array is set
	if( fTelescopeList_subArray.size() == 0 )
	{
		fTelescopeList_subArray = fTelescopeList;
	}
	
	// get telescope
	unsigned int iTelID_sub = 99999;
	for( unsigned int i = 0; i < fTelescopeList_subArray.size(); i++ )
	{
		if( fTelescopeList_subArray[i]->fTelID == iTelID )
		{
			iTelID_sub = i;
		}
	}
	if( iTelID_sub >= fTelescopeList_subArray.size() )
	{
		cout << "telescope ID not found" << endl;
		return;
	}
	
	// fill list
	multimap< float, unsigned int > iSortedListOfTelescopes;
	for( unsigned int i = 0; i < fTelescopeList_subArray.size(); i++ )
	{
		if( i != iTelID_sub )
		{
			float d = sqrt( ( fTelescopeList_subArray[i]->fTel_x - fTelescopeList_subArray[iTelID_sub]->fTel_x )
							* ( fTelescopeList_subArray[i]->fTel_x - fTelescopeList_subArray[iTelID_sub]->fTel_x )
							+ ( fTelescopeList_subArray[i]->fTel_y - fTelescopeList_subArray[iTelID_sub]->fTel_y )
							* ( fTelescopeList_subArray[i]->fTel_y - fTelescopeList_subArray[iTelID_sub]->fTel_y ) );
			iSortedListOfTelescopes.insert( pair< float, unsigned int >( d, i ) );
		}
	}
	
	// print list
	for( multimap<float, unsigned int>::iterator it = iSortedListOfTelescopes.begin(); it != iSortedListOfTelescopes.end(); ++it )
	{
		if( it->second < fTelescopeList_subArray.size() && it->first < iDistanceMax )
		{
			cout << "..to telescope " << fTelescopeList_subArray[it->second]->fTelID;
			cout << " (" << fTelescopeList_subArray[it->second]->fTelTypeName << "): " << it->first << "m" << endl;
		}
	}
	
}

vector< string > VPlotCTAArrayLayout::getListofArrrays( string iArrayFile )
{
	vector< string > SubArray;
	ifstream is;
	is.open( iArrayFile.c_str(), ifstream::in );
	if( !is )
	{
		return SubArray;
	}
	
	string is_line;
	while( getline( is, is_line ) )
	{
		SubArray.push_back( is_line );
	}
	
	return SubArray;
}

void VPlotCTAArrayLayout::plot_fromList( string iArrayFile, string iDir, double xmax, double ymax, bool drawTelescopeNumbers )
{
	vector< string > iSubArray = getListofArrrays( iArrayFile );
	if( iSubArray.size() == 0 )
	{
		return;
	}
	
	for( unsigned int i = 0; i < iSubArray.size(); i++ )
	{
		string iName = iDir + "/CTA.prod2" + iSubArray[i] + ".lis";
		cout << iName << endl;
		if( setSubArray( iName ) )
		{
			iName = iSubArray[i];
			TCanvas* c = plot_array( iName, xmax, ymax, "", drawTelescopeNumbers );
			iName += ".pdf";
			c->Print( iName.c_str() );
		}
	}
}

////////////////////////////////////////////////////////////

VPlotCTAArrayLayout_TelescopeList::VPlotCTAArrayLayout_TelescopeList()
{
	fTelID = 0;
	fTelType = 0;
	fTelID_hyperArray = 0;
	fTel_x = 0.;
	fTel_y = 0.;
	fTelTypeName = "";
	
	fMarkerColor = 1;
	fMarkerSize = 1;
	fMarkerType = 8;
}
