/*! \class VFrogParameters
    \brief  VFrogParameters storage class for shower Frogs reconstruction 

    \author Gareth Hughes
*/

#include "VFrogParameters.h"

VFrogParameters::VFrogParameters()
{
    fDebug = false;
    fTreeFrog = 0;
    reset();

}

void VFrogParameters::initTree( string iName, string iTitle )
{

    printf("FROGPUT In VFrogParameters %s %s\n",iName.c_str(),iTitle.c_str());
    
    fTreeFrog = new TTree( iName.c_str(), iTitle.c_str() );
    fTreeFrog->SetMaxTreeSize(1000*Long64_t(2000000000));
    fTreeFrog->SetAutoSave(1000);              

//  Branches:
    fTreeFrog->Branch("frogsEventID", &frogsEventID, "frogsEventID/I" );
    fTreeFrog->Branch("frogsGSLConStat", &frogsGSLConStat, "frogsGSLConStat/I" );
    fTreeFrog->Branch("frogsNB_iter", &frogsNB_iter, "frogsNB_iter/I" );
    fTreeFrog->Branch("frogsNImages", &frogsNImages, "frogsNImages/I" );
    fTreeFrog->Branch("frogsXS", &frogsXS, "frogsXS/F" );
    fTreeFrog->Branch("frogsXSerr", &frogsXSerr, "frogsXSerr/F" );
    fTreeFrog->Branch("frogsYS", &frogsYS, "frogsYS/F" );
    fTreeFrog->Branch("frogsYSerr", &frogsYSerr, "frogsYSerr/F" );
    fTreeFrog->Branch("frogsXP", &frogsXP, "frogsXP/F" );
    fTreeFrog->Branch("frogsXPerr", &frogsXPerr, "frogsXPerr/F" );
    fTreeFrog->Branch("frogsYP", &frogsYP, "frogsYP/F" );
    fTreeFrog->Branch("frogsYPerr", &frogsYPerr, "frogsYPerr/F" );
    fTreeFrog->Branch("frogsXPGC", &frogsXPGC, "frogsXPGC/F" );
    fTreeFrog->Branch("frogsYPGC", &frogsYPGC, "frogsYPGC/F" );
    fTreeFrog->Branch("frogsEnergy", &frogsEnergy, "frogsEnergy/F" );
    fTreeFrog->Branch("frogsEnergyerr", &frogsEnergyerr, "frogsEnergyerr/F" );
    fTreeFrog->Branch("frogsLambda", &frogsLambda, "frogsLambda/F" );
    fTreeFrog->Branch("frogsLambdaerr", &frogsLambdaerr, "frogsLambdaerr/F" );
    fTreeFrog->Branch("frogsGoodnessImg", &frogsGoodnessImg, "frogsGoodnessImg/F" );
    fTreeFrog->Branch("frogsNpixImg", &frogsNpixImg, "frogsNpixImg/I" );
    fTreeFrog->Branch("frogsGoodnessBkg", &frogsGoodnessBkg, "frogsGoodnessBkg/F" );
    fTreeFrog->Branch("frogsNpixBkg", &frogsNpixBkg, "frogsNpixBkg/I" );
    fTreeFrog->Branch("frogsTelGoodnessImg0", &frogsTelGoodnessImg0, "frogsTelGoodnessImg0/F" );
    fTreeFrog->Branch("frogsTelGoodnessImg1", &frogsTelGoodnessImg1, "frogsTelGoodnessImg1/F" );
    fTreeFrog->Branch("frogsTelGoodnessImg2", &frogsTelGoodnessImg2, "frogsTelGoodnessImg2/F" );
    fTreeFrog->Branch("frogsTelGoodnessImg3", &frogsTelGoodnessImg3, "frogsTelGoodnessImg3/F" );
    fTreeFrog->Branch("frogsTelGoodnessBkg0", &frogsTelGoodnessBkg0, "frogsTelGoodnessBkg0/F" );
    fTreeFrog->Branch("frogsTelGoodnessBkg1", &frogsTelGoodnessBkg1, "frogsTelGoodnessBkg1/F" );
    fTreeFrog->Branch("frogsTelGoodnessBkg2", &frogsTelGoodnessBkg2, "frogsTelGoodnessBkg2/F" );
    fTreeFrog->Branch("frogsTelGoodnessBkg3", &frogsTelGoodnessBkg3, "frogsTelGoodnessBkg3/F" );

    fTreeFrog->Branch("frogsXPStart", &frogsXPStart, "frogsXPStart/F" );
    fTreeFrog->Branch("frogsYPStart", &frogsYPStart, "frogsYPStart/F" );
    fTreeFrog->Branch("frogsXPED", &frogsXPED, "frogsXPED/F" );
    fTreeFrog->Branch("frogsYPED", &frogsYPED, "frogsYPED/F" );
    fTreeFrog->Branch("frogsXSStart", &frogsXSStart, "frogsXSStart/F" );
    fTreeFrog->Branch("frogsYSStart", &frogsYSStart, "frogsYSStart/F" );

}



void VFrogParameters::reset()
{

//  0 all the values
     frogsEventID     = 0;
     frogsGSLConStat  = 0;
     frogsNB_iter     = 0;
     frogsNImages     = 0;
     frogsXS          = 0.0;
     frogsXSerr       = 0.0;
     frogsYS          = 0.0;
     frogsYSerr       = 0.0;
     frogsXP          = 0.0;
     frogsXPerr       = 0.0;
     frogsYP          = 0.0;
     frogsYPerr       = 0.0;
     frogsEnergy      = 0.0;
     frogsEnergyerr   = 0.0;
     frogsLambda      = 0.0;
     frogsLambdaerr   = 0.0;
     frogsGoodnessImg = 0.0;
     frogsNpixImg     = 0;
     frogsGoodnessBkg = 0.0;
     frogsNpixBkg     = 0;

 }

/*
void VFrogParameters::printParameters()
{

    cout << "Shower parameters: " << endl;
    cout << runNumber << "\t" << eventNumber << " (status " << eventStatus << ")\t" <<  MJD;
    cout << time << "\t" << fNTelescopes << endl;
    cout << "Trigger: " << endl;
    cout << fNTrig << "\t" << fLTrig << endl;

    cout << "Target elev,azim,dec,ra,wobblenorth,wobbleease" << endl;
    cout << fTargetElevation <<  "\t" << fTargetAzimuth << endl;
    cout <<  fTargetDec <<  "\t" << fTargetRA << endl;
    cout << fWobbleNorth <<  "\t" << fWobbleEast << endl;

}

*/
