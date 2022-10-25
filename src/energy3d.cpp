#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <functional>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TProfile2D.h"

#include "energy3d.h"

//For the Inputfile, leaves
Float_t		InOutSel3D;		// elevation of shower direction (deg)
Float_t 	InOutsigmaL3D;		// longitudinal (3D-length)
Float_t 	InOutsigmaT3D;		// transverse (3D-width)
Float_t 	InOutNc3D;     		// total number of Cherenkov photons emitted by the shower 
Float_t 	InOutRWidth3D;
Float_t 	InOutSaz3D;		// azimuth of shower direction (deg)
Float_t 	InOutSmax3D;		// height of shower maximum (along the shower axis)
Float_t 	InOutDepth3D;
Float_t 	InOutXcore3D;		// shower core in ground coordinates
Float_t 	InOutYcore3D;		// shower core in ground coordinates
Float_t 	InOutGoodness3D;	// Goodness i.e how much a shower is gamma like
Float_t 	InOutXoff3D;		// Xcore offset 3D
Float_t 	InOutYoff3D;		// Ycore offset 3D

Float_t 	InOutSel3DErr;		//error in the analysed data value
Float_t 	InOutsigmaL3DErr;		
Float_t 	InOutsigmaT3DErr;
Float_t 	InOutNc3DErr;	
Float_t 	InOutRWidth3DErr;		
Float_t 	InOutSaz3DErr;	
Float_t 	InOutSmax3DErr;		
Float_t 	InOutXcore3DErr;		
Float_t 	InOutYcore3DErr;		
			
Double_t 	InOutpedvars1;
Double_t 	InOutpedvars2;
Double_t 	InOutpedvars3;
Double_t 	InOutpedvars4;
		
//For the Tempfiles, leaves
Float_t 	TempSel3D;		//template Sel3d value
Float_t 	TempsigmaL3D;	
Float_t 	TempsigmaT3D; 	
Float_t 	TempNc3D;     	
Float_t 	TempRWidth3D;
Float_t 	TempSaz3D;	
Float_t 	TempSmax3D;	
Float_t 	TempDepth3D;
Float_t 	TempXcore3D;	
Float_t 	TempYcore3D;	
Float_t 	TempGoodness3D;
Float_t 	TempXoff3D;	
Float_t 	TempYoff3D;	
Float_t 	TempEnergy3D;	

Float_t 	TempMCe0;		// The energy (Montecarlo energy)

Float_t 	TempEnergySel3D;	// pos in the template file of the coresponding energy to Sel resorted
Float_t 	TempEnergysigmaL3D;	
Float_t 	TempEnergysigmaT3D; 	
Float_t 	TempEnergyNc3D;     	 
Float_t 	TempEnergyRWidth3D;
Float_t 	TempEnergySaz3D;	
Float_t 	TempEnergySmax3D;	
Float_t 	TempEnergyDepth3D;

Float_t 	ReturnSel3D;		//Only relevant returnvalue, to go from any other parameter to their energy, the rest are still preserved in case someone needs them
Float_t 	ReturnsigmaL3D;
Float_t 	ReturnsigmaT3D;
Float_t 	ReturnNc3D;
Float_t 	ReturnRWidth3D;
Float_t 	ReturnSaz3D;
Float_t 	ReturnSmax3D;
Float_t 	ReturnDepth3D;
		
Float_t 	PlasementSel3D;		//the original placement of Sel3d before resorting
Float_t 	PlasementsigmaL3D;
Float_t 	PlasementsigmaT3D;
Float_t 	PlasementNc3D;
Float_t 	PlasementRWidth3D;
Float_t 	PlasementSaz3D;
Float_t 	PlasementSmax3D;
Float_t 	PlasementDepth3D;

Float_t 	TempErrorSel3D;		//error for the template value
Float_t 	TempErrorsigmaL3D;	
Float_t 	TempErrorsigmaT3D; 	
Float_t 	TempErrorNc3D;     	
Float_t 	TempErrorRWidth3D;
Float_t 	TempErrorSaz3D;	
Float_t 	TempErrorSmax3D;
Float_t 	TempErrorDepth3D;
Float_t 	TempErrorXcore3D;
Float_t 	TempErrorYcore3D;
		

const char *Noise[10] 	= {"075", "100", "150", "200", "250", "325", "425", "550", "750", "1000"};

int MSPN = 4;	//Main Search Parameter Number
int CoL = 1;	//lower search value number
int CoH = 7;	//higher search value number
		
const int NumberOfParameters = 8;
int TheEnergyIsAt = 13;	  
int TempEnergyStart = TheEnergyIsAt+1; 
int ReturnStart = TempEnergyStart + NumberOfParameters;	  
int PlasementStart = ReturnStart + NumberOfParameters;  
		
double Totalpedvar = 0;

int noiseloop;
int SearchWidth;	
int HighSearch;
int LowSearch;

Float_t XandY;			//This will be used to store the current smalest minimisation value of the parameters
Float_t BigtoSmalNumb;		//for the parameter minimasation
Float_t Energy3D;		//in this the output energy will be stored
Float_t OutEnergy = 0;		//the best fitting energy value going to be saved in the outputfiles when best minimisation found 

int ChangeValues = 0;		//position for the best fitting value on the "Main Search Parameter" 

char TempNoiseName[100];

using namespace std ;

bool sortingfloat(const pair<Float_t,Float_t>&i, const pair<Float_t,Float_t>&j)
{
    return i.first < j.first;
}


int main(int argc, char *argv[]){

	string Inputfilename;
	//string Outputfilename;
	string Templatefilename;


	if (argc==7) {
		for ( int i = 0 ; i<argc ; i++ ) {
			if ( string(argv[i]) == "-inputfile" ) {
				Inputfilename = argv[i+1] ; 
			}
	//	else if ( strcmp( argv[i], "-outputfile" ) == 1) {
	//		Outputfilename = argv[i+1] ;
	//	}
			else if ( string(argv[i]) == "-template" ) {
				Templatefilename = argv[i+1] ; 
			}
		}
         	cout << "Inputfilename: " << Inputfilename << endl  ;
          	cout << "Templatefilename: "<< Templatefilename << endl ;
	}
	else {
    		cout <<"Usage: ./energy3d -inputfilename <filename> -outputfilename <filename> -templatefilename <filename>"<<endl;
		cout <<"Example: ./energy3d -inputfilename <filedirectory/12345.root> -outputfilename <filedirectory/12345_energy3d.root> -templatefilename <templatedir/Merged/Template3D_V6_ATM21_123.root>"<<endl;    		
		return 0;
  	}

	char *InName=new char[ Inputfilename.length()+1 ]; 
	//char *OutName=new char[ Outputfilename.size()+1 ]; 
	char *TempName=new char[ Templatefilename.length()+1 ]; 
	strcpy(InName, Inputfilename.c_str());
	strcpy(TempName, Templatefilename.c_str());
	cout << "InName: " << InName << endl;
	cout << "TempName: " << TempName << endl;
	

	// The Inputfile //
	TFile *InTfile = new TFile( InName );
	TTree *InTTrees = (TTree*)InTfile->Get( "showerpars" );
	TTree *InTTreem = (TTree*)InTfile->Get( "model3Dpars" );
	TTree *t1 = (TTree*)InTfile->Get( "Tel_1/calib_1" );
	TTree *t2 = (TTree*)InTfile->Get( "Tel_2/calib_2" );
	TTree *t3 = (TTree*)InTfile->Get( "Tel_3/calib_3" );
	TTree *t4 = (TTree*)InTfile->Get( "Tel_4/calib_4" );
		
	InTTreem->SetBranchAddress( "Sel3D", 		&InOutSel3D );
	InTTreem->SetBranchAddress( "sigmaL3D", 	&InOutsigmaL3D );
	InTTreem->SetBranchAddress( "sigmaT3D", 	&InOutsigmaT3D );
	InTTreem->SetBranchAddress( "Nc3D", 		&InOutNc3D );
	InTTreem->SetBranchAddress( "RWidth3D", 	&InOutRWidth3D );
	InTTreem->SetBranchAddress( "Saz3D",		&InOutSaz3D ); 
	InTTreem->SetBranchAddress( "Smax3D", 		&InOutSmax3D );
	InTTreem->SetBranchAddress( "Depth3D", 		&InOutDepth3D );
	InTTreem->SetBranchAddress( "Xcore3D", 		&InOutXcore3D );
	InTTreem->SetBranchAddress( "Ycore3D", 		&InOutYcore3D );
	InTTreem->SetBranchAddress( "Xoff3D", 		&InOutXoff3D );
	InTTreem->SetBranchAddress( "Yoff3D", 		&InOutYoff3D );
	InTTreem->SetBranchAddress( "Goodness3D", 	&InOutGoodness3D );

	t1->SetBranchAddress( "pedvar", 		&InOutpedvars1 );
	t2->SetBranchAddress( "pedvar", 		&InOutpedvars2 );
	t3->SetBranchAddress( "pedvar", 		&InOutpedvars3 );
	t4->SetBranchAddress( "pedvar", 		&InOutpedvars4 );

	InTTreem->SetBranchAddress( "ErrorSel3D", 	&InOutSel3DErr );
	InTTreem->SetBranchAddress( "ErrorsigmaL3D", 	&InOutsigmaL3DErr );
	InTTreem->SetBranchAddress( "ErrorsigmaT3D", 	&InOutsigmaT3DErr );
	InTTreem->SetBranchAddress( "ErrorNc3D", 	&InOutNc3DErr );
	InTTreem->SetBranchAddress( "ErrRWidth3D", 	&InOutRWidth3DErr );
	InTTreem->SetBranchAddress( "ErrorSaz3D",	&InOutSaz3DErr ); 
	InTTreem->SetBranchAddress( "ErrorSmax3D", 	&InOutSmax3DErr );
	InTTreem->SetBranchAddress( "ErrorXcore3D", 	&InOutXcore3DErr );
	InTTreem->SetBranchAddress( "ErrorYcore3D", 	&InOutYcore3DErr );

	int InEntries = InTTrees->GetEntries();
	int t1Entries = t1->GetEntries();
	int t2Entries = t2->GetEntries();
	int t3Entries = t3->GetEntries();
	int t4Entries = t4->GetEntries();
					
	vector < pair< Float_t, Float_t > > ThreeDForSorting;
	vector < vector < Float_t > > Parameters3DForSorting;
	vector < Float_t > ForSorting3DParameters;

	for ( int InSort = 0; InSort < InEntries; InSort++){
		InTTreem->GetEntry( InSort );						
		ThreeDForSorting.push_back( pair < Float_t, int > ( InOutRWidth3D, InSort ));
	}

	sort( ThreeDForSorting.begin(), ThreeDForSorting.end(), sortingfloat );	
	for ( int InS = 0; InS < InEntries; InS++){
		InTTreem->GetEntry( InS );
		InTTrees->GetEntry( InS );
	/*	InOutSaz3D = 180-InOutSaz3D;
		if (InOutSaz3D < 270){
			InOutSaz3D+=90;
		}
		else if (InOutSaz3D >= 270){
		   	InOutSaz3D-=270;
		}*/
		ForSorting3DParameters.push_back( 	InOutSel3D);		//0
		ForSorting3DParameters.push_back( 	InOutsigmaL3D);		//1
		ForSorting3DParameters.push_back( 	InOutsigmaT3D); 	//2
		ForSorting3DParameters.push_back( 	InOutNc3D);		//3
		ForSorting3DParameters.push_back( 	InOutRWidth3D);		//4
		ForSorting3DParameters.push_back(	InOutSaz3D);		//5
		ForSorting3DParameters.push_back( 	InOutSmax3D);		//6		
		ForSorting3DParameters.push_back(	InOutDepth3D);		//7
		ForSorting3DParameters.push_back( 	InOutXcore3D);		//8
		ForSorting3DParameters.push_back( 	InOutYcore3D);		//9
		ForSorting3DParameters.push_back( 	InOutGoodness3D);	//10
		ForSorting3DParameters.push_back( 	InS);			//11
		ForSorting3DParameters.push_back( 	InOutRWidth3DErr);	//12
		Parameters3DForSorting.push_back( 	ForSorting3DParameters);				
		ForSorting3DParameters.clear();
	}


	for (int ped1 = 0; ped1 < t1Entries; ped1++ ){
		t1->GetEntry( ped1 );
		Totalpedvar += InOutpedvars1;
	}
	for (int ped2 = 0; ped2 < t2Entries; ped2++ ){
		t2->GetEntry( ped2 );
		Totalpedvar += InOutpedvars2;
	}
	for (int ped3 = 0; ped3 < t3Entries; ped3++ ){
		t3->GetEntry( ped3 );
		Totalpedvar += InOutpedvars3;
	}
	for (int ped4 = 0; ped4 < t4Entries; ped4++ ){
		t4->GetEntry( ped4 );
		Totalpedvar += InOutpedvars4;
	}

	double meanpedvar = Totalpedvar / ( t1Entries + t2Entries + t3Entries + t4Entries );

	if ( meanpedvar <= 3.57269 ){ 
		noiseloop = 0;	//075
	}
	else if ( meanpedvar > 3.57269 && meanpedvar <= 4.26753){ 
		noiseloop = 1;	//100
	}
	else if ( meanpedvar > 4.26753 && meanpedvar <= 5.05496){ 
		noiseloop = 2;	//150
	}
	else if ( meanpedvar > 5.05496 && meanpedvar <= 5.71548){ 
		noiseloop = 3;	//200
	}
	else if ( meanpedvar > 5.71548 && meanpedvar <= 6.42216){  
		noiseloop = 4;	//250
	}
	else if ( meanpedvar > 6.42216 && meanpedvar <= 7.28621){ 
		noiseloop = 5;	//325
	}
	else if ( meanpedvar > 7.28621 && meanpedvar <= 8.25194){ 
		noiseloop = 6;	//425
	}
	else if ( meanpedvar > 8.25194 && meanpedvar <= 9.47758){ 
		noiseloop = 7;	//550
	}
	else if ( meanpedvar > 9.47758 && meanpedvar <= 10.9854){
		noiseloop = 8;	//750
	}
	else { //meanpedvar > 10.9854
		noiseloop = 9;	//1000
	}

	// Template data //
	vector< Float_t > TempRow;
	vector < vector < Float_t > > TwoDTempVector;
	TFile *TempTfile = new TFile(TempName);
	sprintf( TempNoiseName, "ModelPars3D_NOISE%s", Noise[ noiseloop ] );
	TTree *PointerToTempTtreeM = (TTree*)TempTfile->Get( TempNoiseName );

	int NTemp = PointerToTempTtreeM->GetEntries();	
	PointerToTempTtreeM->SetBranchAddress( "Sel3D", 	&TempSel3D );			//These are the values from ForSorting.first, thus should be the final "Sel"values
	PointerToTempTtreeM->SetBranchAddress( "sigmaL3D",	&TempsigmaL3D );
	PointerToTempTtreeM->SetBranchAddress( "sigmaT3D",	&TempsigmaT3D );			
	PointerToTempTtreeM->SetBranchAddress( "Nc3D", 		&TempNc3D );			
	PointerToTempTtreeM->SetBranchAddress( "RWidth3D", 	&TempRWidth3D );				
	PointerToTempTtreeM->SetBranchAddress( "Saz3D",	 	&TempSaz3D ); 
	PointerToTempTtreeM->SetBranchAddress( "Smax3D", 	&TempSmax3D );			
	PointerToTempTtreeM->SetBranchAddress( "Depth3D", 	&TempDepth3D );
	PointerToTempTtreeM->SetBranchAddress( "Xcore3D", 	&TempXcore3D );
	PointerToTempTtreeM->SetBranchAddress( "Ycore3D", 	&TempYcore3D );
	PointerToTempTtreeM->SetBranchAddress( "Goodness3D", 	&TempGoodness3D );
	PointerToTempTtreeM->SetBranchAddress( "Xoff3D", 	&TempXoff3D );
	PointerToTempTtreeM->SetBranchAddress( "Yoff3D", 	&TempYoff3D );
	PointerToTempTtreeM->SetBranchAddress( "MCe0", 		&TempMCe0 );
		
	PointerToTempTtreeM->SetBranchAddress( "EnergySel3D", 	 &TempEnergySel3D );		//These are the values from ForSorting.second, thus should go into the return
	PointerToTempTtreeM->SetBranchAddress( "EnergysigmaL3D", &TempEnergysigmaL3D );
	PointerToTempTtreeM->SetBranchAddress( "EnergysigmaT3D", &TempEnergysigmaT3D );
	PointerToTempTtreeM->SetBranchAddress( "EnergyNc3D", 	 &TempEnergyNc3D );
	PointerToTempTtreeM->SetBranchAddress( "EnergyRWidth3D", &TempEnergyRWidth3D );
	PointerToTempTtreeM->SetBranchAddress( "EnergySaz3D",	 &TempEnergySaz3D ); 			
	PointerToTempTtreeM->SetBranchAddress( "EnergySmax3D", 	 &TempEnergySmax3D );
	PointerToTempTtreeM->SetBranchAddress( "EnergyDepth3D",  &TempEnergyDepth3D );
			
	PointerToTempTtreeM->SetBranchAddress( "ReturnSel3D", 	 &ReturnSel3D );		//These are the values from return.first, thus should go into the Energy
	PointerToTempTtreeM->SetBranchAddress( "ReturnsigmaL3D", &ReturnsigmaL3D );
	PointerToTempTtreeM->SetBranchAddress( "ReturnsigmaT3D", &ReturnsigmaT3D );
	PointerToTempTtreeM->SetBranchAddress( "ReturnNc3D", 	 &ReturnNc3D );
	PointerToTempTtreeM->SetBranchAddress( "ReturnRWidth3D", &ReturnRWidth3D );
	PointerToTempTtreeM->SetBranchAddress( "ReturnSaz3D",	 &ReturnSaz3D ); 
	PointerToTempTtreeM->SetBranchAddress( "ReturnSmax3D", 	 &ReturnSmax3D );
	PointerToTempTtreeM->SetBranchAddress( "ReturnDepth3D",  &ReturnDepth3D );	

	PointerToTempTtreeM->SetBranchAddress( "PlasementSel3D",	&PlasementSel3D );		//These are the values from return.second, thus should go into the get the Sel
	PointerToTempTtreeM->SetBranchAddress( "PlasementsigmaL3D",	&PlasementsigmaL3D );
	PointerToTempTtreeM->SetBranchAddress( "PlasementsigmaT3D", 	&PlasementsigmaT3D );
	PointerToTempTtreeM->SetBranchAddress( "PlasementNc3D", 	&PlasementNc3D );
	PointerToTempTtreeM->SetBranchAddress( "PlasementRWidth3D", 	&PlasementRWidth3D );
	PointerToTempTtreeM->SetBranchAddress( "PlasementSaz3D",	&PlasementSaz3D ); 
	PointerToTempTtreeM->SetBranchAddress( "PlasementSmax3D", 	&PlasementSmax3D );
	PointerToTempTtreeM->SetBranchAddress( "PlasementDepth3D", 	&PlasementDepth3D );

	PointerToTempTtreeM->SetBranchAddress( "ErrorRWidth3D", 	&TempErrorRWidth3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorSmax3D",	 	&TempErrorSmax3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorsigmaT3D", 	&TempErrorsigmaT3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorsigmaL3D", 	&TempErrorsigmaL3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorXcore3D",	 	&TempErrorXcore3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorYcore3D",	 	&TempErrorYcore3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorSel3D",	 	&TempErrorSel3D );
	PointerToTempTtreeM->SetBranchAddress( "ErrorSaz3D",	 	&TempErrorSaz3D );
	
	for ( int i_temp = 0; i_temp < NTemp; i_temp++ ) {
		PointerToTempTtreeM->GetEntry( i_temp );    

		TempRow.push_back( TempSel3D );		//0	//All of are sorted according to themselvs, eccept for the Energy
		TempRow.push_back( TempsigmaL3D );	//1	
		TempRow.push_back( TempsigmaT3D ); 	//2
		TempRow.push_back( TempNc3D ); 		//3	
		TempRow.push_back( TempRWidth3D );	//4
		TempRow.push_back( TempSaz3D );		//5
		TempRow.push_back( TempSmax3D );	//6
		TempRow.push_back( TempDepth3D );	//7
		TempRow.push_back( TempXcore3D );	//8	
		TempRow.push_back( TempYcore3D );	//9	
		TempRow.push_back( TempXoff3D );	//10
		TempRow.push_back( TempYoff3D );	//11
		TempRow.push_back( TempGoodness3D ); 	//12
		TempRow.push_back( TempMCe0 );		//13	//This one is not sorted, because it is the energy to which 16-25 is pointing, the so called original positions.
	
		TempRow.push_back( TempEnergySel3D );		//14	//THESE points to the original position, "parallel" to the Energy 
		TempRow.push_back( TempEnergysigmaL3D );	//15	
		TempRow.push_back( TempEnergysigmaT3D ); 	//16	
		TempRow.push_back( TempEnergyNc3D ); 		//17
		TempRow.push_back( TempEnergyRWidth3D ); 	//18
		TempRow.push_back( TempEnergySaz3D );		//19
		TempRow.push_back( TempEnergySmax3D );		//20	
		TempRow.push_back( TempEnergyDepth3D ); 	//21
		
		TempRow.push_back( ReturnSel3D );		//22	//THESE points to the original position of the parameters 
		TempRow.push_back( ReturnsigmaL3D );		//23	
		TempRow.push_back( ReturnsigmaT3D ); 		//24	
		TempRow.push_back( ReturnNc3D ); 		//25
		TempRow.push_back( ReturnRWidth3D ); 		//26
		TempRow.push_back( ReturnSaz3D );		//27
		TempRow.push_back( ReturnSmax3D );		//28
		TempRow.push_back( ReturnDepth3D ); 		//29
				
		TempRow.push_back( PlasementSel3D );		//30	//THESE points to the original position of the Energy belonging to the resorted parameters
		TempRow.push_back( PlasementsigmaL3D );		//31	
		TempRow.push_back( PlasementsigmaT3D ); 	//32	
		TempRow.push_back( PlasementNc3D ); 		//33
		TempRow.push_back( PlasementRWidth3D );		//34
		TempRow.push_back( PlasementSaz3D );		//35
		TempRow.push_back( PlasementSmax3D );		//36
		TempRow.push_back( PlasementDepth3D );		//37

		TempRow.push_back( TempErrorRWidth3D );		//38
		TempRow.push_back( TempErrorSmax3D );		//39
		TempRow.push_back( TempErrorsigmaT3D );		//40
		TempRow.push_back( TempErrorsigmaL3D );		//41
		TempRow.push_back( TempErrorSel3D );		//42
		TempRow.push_back( TempErrorSaz3D );		//43
		TempRow.push_back( TempErrorXcore3D );		//44
		TempRow.push_back( TempErrorYcore3D );		//45
			
		TwoDTempVector.push_back( TempRow );
		TempRow.clear();
	}

	// Energy search //	
	Float_t ENER[ InEntries ];

	for ( int EnergySearchIn = 0 ; EnergySearchIn < InEntries; EnergySearchIn++ ) {
		BigtoSmalNumb = 1000000000;
		XandY = 0;
		ChangeValues = 0;
		while ( TwoDTempVector[ ChangeValues ][ MSPN ] <= Parameters3DForSorting[ ThreeDForSorting.at( EnergySearchIn ).second ][ MSPN ]  && ChangeValues < NTemp ){
			ChangeValues++;
		}
		if ( TwoDTempVector[ ChangeValues ][ MSPN ] == 0 && Parameters3DForSorting[ EnergySearchIn][ MSPN ] == 0 ){
			SearchWidth = ceil( NTemp * (abs(TwoDTempVector[TwoDTempVector[TwoDTempVector[ ChangeValues ][TempEnergyStart+MSPN]][PlasementStart+MSPN]][38]) /1 +abs(Parameters3DForSorting[ EnergySearchIn ][ 12 ])/ 1) );
		}
		else if ( TwoDTempVector[ ChangeValues ][ MSPN ] == 0 ){
			SearchWidth = ceil( NTemp * (abs(TwoDTempVector[TwoDTempVector[TwoDTempVector[ ChangeValues ][TempEnergyStart+MSPN]][PlasementStart+MSPN]][38]) /1 +abs(Parameters3DForSorting[ EnergySearchIn ][ 12 ])/ abs(Parameters3DForSorting[ EnergySearchIn][ MSPN ]) ));
		}
		else if ( Parameters3DForSorting[ EnergySearchIn][ MSPN ] == 0 ){
			SearchWidth = ceil( NTemp * (abs(TwoDTempVector[TwoDTempVector[TwoDTempVector[ ChangeValues ][TempEnergyStart+MSPN]][PlasementStart+MSPN]][38]) /abs(TwoDTempVector[ ChangeValues ][ MSPN ]) +abs(Parameters3DForSorting[ EnergySearchIn ][ 12 ])/ 1) );
		}
		else {
			SearchWidth = ceil( NTemp * (abs(TwoDTempVector[TwoDTempVector[TwoDTempVector[ ChangeValues ][TempEnergyStart+MSPN]][PlasementStart+MSPN]][38]) /abs(TwoDTempVector[ ChangeValues ][ MSPN ])+abs(Parameters3DForSorting[ EnergySearchIn ][ 12 ])/ abs(Parameters3DForSorting[ EnergySearchIn][ MSPN ]) )); //NAH, now both template and data errors are taken into account in the search length
		}

		if (SearchWidth< 100){
			SearchWidth = 100000;
		}
		if ( ChangeValues-SearchWidth < 2 ) { 
			LowSearch = 2;
		}	
		else {
			LowSearch = ChangeValues-SearchWidth;
		}
		if ( ChangeValues+SearchWidth > NTemp-1 ) {
			HighSearch = NTemp-1;
		}
		else {
			HighSearch = ChangeValues+SearchWidth;
		}

		for ( int ES = LowSearch ; ES <= HighSearch; ES++ ) {
			XandY = 0;
			for ( int Columns = CoL; Columns <= CoH; Columns++ ) {
				if ( Columns != 5 ){
					XandY = XandY + abs(TwoDTempVector[TwoDTempVector[TwoDTempVector[ES][TempEnergyStart+MSPN]][PlasementStart+Columns]][Columns]-Parameters3DForSorting[ ThreeDForSorting.at( EnergySearchIn ).second ][ Columns ]);
				}
			}
			if ( XandY < BigtoSmalNumb ){
				BigtoSmalNumb = XandY;
				OutEnergy = TwoDTempVector[ TwoDTempVector[ TwoDTempVector[ ES ][ TempEnergyStart+MSPN]][ ReturnStart]][ TheEnergyIsAt ]; //Allways only ReturnStart, NOT ReturStart+MSPN!!!!!!! because it was sorted acording to SEL......
			}
		}	
		ENER[ int(Parameters3DForSorting[ ThreeDForSorting.at( EnergySearchIn ).second ][ 11 ]) ] = OutEnergy;
	}


	//creating the energy3d branch and filling it in the outputfile
	TBranch *NewBranch = InTTreem->Branch( "Energy3D", &Energy3D, "Energy3D/F");;
	for (int FillEnergy3DBranch = 0; FillEnergy3DBranch < InEntries; FillEnergy3DBranch++){
		Energy3D = ENER[ FillEnergy3DBranch ];
		NewBranch->Fill();
	}
	// save only the new version of the tree
	InTfile->Write("", TObject::kOverwrite);
	delete[] TempName;
	delete[] InName;
	delete TempTfile;
	delete InTfile;
	TwoDTempVector.clear();
}

