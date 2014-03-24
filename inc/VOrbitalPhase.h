//!VOrbitalPhase calculation of the orbital phase

#ifndef VORBITALPHASE
#define VORBITALPHASE

#include "CData.h"
#include "VEvndispRunParameter.h"
#include "VTableLookupRunParameter.h"

#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"

#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

class VOrbitalPhase : public TObject
{
	private:
		TFile* fInFile;
		TFile* fOutFile;
		string finputfile;
		string foutputfile;
		CData* fDataRun;
		TTree* fDataRunTree; // input tree
		TTree* fOTree;       // output tree
		int fNentries;       // entries input and output trees
		
		// data written in output tree
		double fOrbit;    // in days
		double fRefMJD;
		double fOrbitalPhase;
		
		
	public:
		VOrbitalPhase();
		~VOrbitalPhase() {};
		
		/// set and get functions
		bool setInputFile();
		bool setOutputFile( string iOption );
		bool initialize( int argc, char* argv[] );
		void setOrbit( double iOrbit )
		{
			fOrbit = iOrbit;
		};
		void setRefMJD( double iRefMJD )
		{
			fRefMJD = iRefMJD;
		}; // time of ascending node
		double getRefMJD()
		{
			return fRefMJD;
		};
		/// tasks
		void calculatePhase( double djm );
		void copyInputFile();
		void copyDirectory( TDirectory* source );
		void fill();
		bool terminate();
		void printBinaryPars();
		void printHelp();
		
		ClassDef( VOrbitalPhase, 1 );
};

#endif
