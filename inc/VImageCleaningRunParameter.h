//! VImageCleaningRunParameter run parameters for image cleaning

#ifndef VImageCleaningRunParameter_H
#define VImageCleaningRunParameter_H

#include <iostream>

using namespace std;

class VImageCleaningRunParameter
{

    public:

    unsigned int fTelID;

    unsigned int fImageCleaningMethod;   // 0: standard two level cleaning; 1: time cluster cleaning, 2: Maxim..., 
                                         // 3: trace correlation method

// standard two-level image/border cleaning
    double fimagethresh;              // parameter for image threshold
    double fborderthresh;             // parameter for border threshold
    double fbrightnonimagetresh;      // parameter for bright pixels threshold

    bool fUseFixedThresholds;         // use fixed image/border thresholds instead of multiples of pedestal variances

// time cluster cleaning
    double ftimecutpixel;             // HP: parameter for time cut between pixels
    double ftimecutcluster;           // HP: parameter for time cut between clusters
    int    fminpixelcluster;          // HP: parameter for minimum number of pixels in cluster
    int    floops;                    // HP: parameter for number of loops for border pixel finding

// Trace Correlation Cleaning
    double fCorrelationCleanBoardThresh;  // AMc parameter for lower border threshold
    double fCorrelationCleanCorrelThresh; // AMc parameter for trace correlation level (0.6-1.0)
    int    fCorrelationCleanNpixThresh;   // AMc Maximum Number of pixels to apply correlation cleaning to (eg 10-15)


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
