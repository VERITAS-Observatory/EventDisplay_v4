//! VArrayPointing relevant pointing information for array (e.g. reference pointing)

#ifndef VArrayPointing_H
#define VArrayPointing_H

#include "TMath.h"
#include "TTree.h"

#include <iomanip>
#include <iostream>

#include "VSkyCoordinates.h"

using namespace std;

class VArrayPointing : public VSkyCoordinates
{
  private:

  TTree *fPointingTree;

  void initializePointingTree();

  public:

   VArrayPointing( bool bInitTree = true );
  ~VArrayPointing() {}
   void fillPointingTree();
   void terminate();

};

#endif
