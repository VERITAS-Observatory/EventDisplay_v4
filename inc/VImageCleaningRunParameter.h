//! VImageCleaningRunParameter run parameters for image cleaning

#ifndef VImageCleaningRunParameter_H
#define VImageCleaningRunParameter_H

#include <iostream>

using namespace std;

class VImageCleaningRunParameter
{

    public:

    unsigned int fTelID;

    unsigned int fImageCleaningMethod;        // 0: standard two level cleaning; 1: time cluster cleaning, 2: Maxim...

    double fimagethresh;              // parameter for image threshold
    double fborderthresh;             // parameter for border threshold
    double fbrightnonimagetresh;      // parameter for bright pixels threshold

    bool fUseFixedThresholds;                 // use fixed image/border thresholds instead of multiples of pedestal variances

// time cluster cleaning
    double ftimecutpixel;             // HP: parameter for time cut between pixels
    double ftimecutcluster;           // HP: parameter for time cut between clusters
    int    fminpixelcluster;             // HP: parameter for minimum number of pixels in cluster
    int    floops;                       // HP: parameter for number of loops for border pixel finding

    VImageCleaningRunParameter();
   ~VImageCleaningRunParameter() {};

    string       getImageCleaningMethod();
    unsigned int getImageCleaningMethodIndex() { return fImageCleaningMethod; }
    bool         initialize();
    void         print();
    bool         setImageCleaningMethod( string iMethod );
    void         setTelID( unsigned int iTelID ) { fTelID = iTelID; }

};


#endif
