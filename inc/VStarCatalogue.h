//! VStarCatalogue star catalogue
// Revision $Id: VStarCatalogue.h,v 1.1.8.1.18.4.2.8.8.1.2.4.2.1 2010/12/23 15:47:18 gmaier Exp $

#ifndef VSTARCATALOGUE_H
#define VSTARCATALOGUE_H

#include "TFile.h"
#include "TMath.h"
#include "TObject.h"
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TSQLServer.h>
#include <TSystem.h>
#include "TTree.h"

#include "VASlalib.h"
#include "VGlobalRunParameter.h"
#include "VUtilities.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

struct sStar
{
    unsigned int fStarID;
    string fStarName;
    double fDec2000;
    double fRA2000;
    double fDecCurrentEpoch;
    double fRACurrentEpoch;
    double fRunGalLong1958;
    double fRunGalLat1958;
    double fBrightness_V;
    double fBrightness_B;
    double fMajorDiameter;                        // this is either the source diameter or the possitional error
    double fMinorDiameter;
    double fPositionAngle;
    double fMajorDiameter_68;
    double fMinorDiameter_68;
    double fPositionAngle_68;
    double fSignificance;
    double fSpectralIndex;
    double fSpectralIndexError;
    vector< double > fFluxEnergyMin;
    vector< double > fFluxEnergyMax;
    vector< double > fFlux;
    vector< double > fFluxError;
    bool   fVariability;
    vector< string > fOtherNames;
    string fType;
    vector< string > fAssociations;
    int fQualityFlag;
};

class VStarCatalogue : public TObject, public VGlobalRunParameter 
{
    private:
        bool   fDebug;

        string fCatalogue;
        unsigned fCatalogueVersion;

        vector< sStar > fStars;
        vector< sStar > fStarsinFOV;

        bool readCatalogue();
        sStar readCommaSeparatedLine_Fermi( string, int );
        sStar readCommaSeparatedLine_Fermi_Catalogue( string, int );

    public:

        VStarCatalogue();
        ~VStarCatalogue() {}
        bool          init( double MJD );
        bool          init( double MJD, string iCatalogue );

//    void          getStar( unsigned int ID, double &dec, double &ra, double &brightness );
        unsigned int  getNStar() { return fStars.size(); }
        double        getStarBrightness( unsigned int ID, string iBand = "B" );
        string        getStarCatalogueName() { return fCatalogue; }
        unsigned int  getStarID( unsigned int ID );
        string        getStarName( unsigned int ID );
        double        getStar_l( unsigned int ID );
        double        getStar_b( unsigned int ID );
        double        getStarMajorDiameter( unsigned int ID );
	double        getStarSpectralIndex( unsigned int ID );
        double        getStarDec2000( unsigned int ID );
        double        getStarRA2000( unsigned int ID );
        double        getStarDecCurrentEpoch( unsigned int ID );
        double        getStarRACurrentEpoch( unsigned int ID );
        vector< double > getStarFluxEnergyMin( unsigned int ID );
        vector< double > getStarFluxEnergyMax( unsigned int ID );
        vector< double > getStarFlux( unsigned int ID );
        vector< double > getStarFluxError( unsigned int ID );
        vector< string > getStarOtherNames( unsigned int ID );
	string        getStarType( unsigned int ID );
	vector< string > getStarAssociations( unsigned int ID );
        vector< sStar > getListOfStars() { return fStars; }
        vector< sStar > getListOfStarsinFOV() { return fStarsinFOV; }
        void          printCatalogue( unsigned int i_nRows = 0, double iMinBrightness = 999999., string iBand = "B" );
        void          printStarsInFOV();
        void          printStarsInFOV( double iMinBrightness, string iBand = "B" );
        void          purge();
        bool          readVERITASsources( string );
        unsigned int  setFOV( double ra, double dec, double FOV_x, double FOV_y, bool bJ2000 );
        bool          writeCatalogueToRootFile( string iRootFile );

        ClassDef(VStarCatalogue,2);
};
#endif
