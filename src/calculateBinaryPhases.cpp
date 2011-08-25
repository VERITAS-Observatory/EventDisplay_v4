/*! \file  calculateBinaryPhases
 *  \brief calculate orbital phase of a binary and save it to a tree called phase
 *
 *   use off events which pass all gamma/hadron separation cuts
 *
 *   requires runlist version 4 or later
 *
 * \author
 *   Ester Aliu 
 *
 *   Revision $Id$
 */

#include "VOrbitalPhase.h"
#include "VGlobalRunParameter.h"

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


int main( int argc, char *argv[] )
{

    cout << endl;
    cout << "calculateBinaryPhases " << VGlobalRunParameter::getEVNDISP_VERSION() << endl << endl;
    cout << "calculate orbital phase of a binary and save it in a tree called phase" << endl;
    cout << endl;

    VOrbitalPhase *fOrb = new VOrbitalPhase();
    fOrb->initialize(argc, argv);
    // this can be done from the command line. These functions can overwrite those, though. 
    //fOrb->setOrbit(0.1980962019);
    //fOrb->setRefMJD(54801.97065348);
    fOrb->printBinaryPars();
    fOrb->fill();
    fOrb->terminate();
   
 
    return 1;
}
