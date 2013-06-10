/*! \class VImageCleaning

     collection of different image cleaning methods

 */

#include "VImageCleaning.h"
#include "TopoTrigger.h"
#ifndef VIMAGENNCLEANING_H
#include "NNImageCleaningServiceFunc.h"
#define VIMAGENNCLEANING_H
#endif

VImageCleaning::VImageCleaning( VEvndispData *iData )
{
    fData = iData;

// NN Image Cleaning
    kInitNNImageCleaning = false;
    if( fData && fData->getImageCleaningParameter()->getImageCleaningMethod() == "TIMENEXTNEIGHBOUR" )
    {
       kInitNNImageCleaning = InitNNImageCleaning();
    }

    nRings        = 2;
    CoincWinLimit = 16;
}

void VImageCleaning::printDataError( string iFunctionName )
{
   cout << iFunctionName;
   cout << " error: no pointer to data class set" << endl;
   exit( -1 );
}

/*!
  tailcut cleaning with fixed thresholds
   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
*/
void VImageCleaning::cleanImageFixed(double hithresh, double lothresh, double brightthresh )
{
    if( !fData ) printDataError( "VImageCleaning::cleanImageFixed" );

    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );
    fData->setImageBorderNeighbour( false );
    unsigned int i_nchannel = fData->getNChannels();

    for ( unsigned int i=0; i < i_nchannel; i++)
    {
        if( fData->getSums()[i] > hithresh )
        {
            if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] )
            {
                fData->setImage( i, true );
                for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                {
                    unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
                    if ( k < fData->getImage().size() && fData->getSums()[k] > lothresh && !fData->getImage()[k] ) fData->setBorder( k, true );
                }
            }
        }
        if( fData->getSums()[i] > brightthresh )
        {
            if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] ) fData->setBrightNonImage( i, true );
        }
    }

// (preli) set the trigger vector in MC case (preli)
// trigger vector are image/border tubes
    if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 
     || fData->getReader()->getDataFormatNum() == 6 ) 
     {
         fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
     }
// (end of preli)
    if( fData->getRunParameter()->frecoverImagePixelNearDeadPixel ) recoverImagePixelNearDeadPixel();
    if( fData->getRunParameter()->fFillImageBorderNeighbours ) fillImageBorderNeighbours();
}


/*!

  signal-to-noise tailcut cleaning

   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold

*/
void VImageCleaning::cleanImagePedvars( double hithresh, double lothresh, double brightthresh )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::cleanImagePedvars " << fData->getTelID() << endl;

    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );
    fData->setImageBorderNeighbour( false );
    unsigned int i_nchannel = fData->getNChannels();
    double i_pedvars_i = 0.;
    double i_pedvars_k = 0.;
    unsigned int k = 0;

    for ( unsigned int i = 0; i < i_nchannel; i++)
    {
        if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead(fData->getHiLo()[i])[i]) continue;
        i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i])[i];

        if( fData->getSums()[i] > hithresh * i_pedvars_i )
        {
            fData->setImage( i, true );
            fData->setBorder( i, false );
            for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
            {
                k = fData->getDetectorGeo()->getNeighbours()[i][j];
                if( k < i_nchannel )
                {
                    i_pedvars_k = fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k])[k];
                    if( !fData->getImage()[k] && fData->getSums()[k] > lothresh * i_pedvars_k ) fData->setBorder( k, true );
                }
            }
        }
        if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) fData->setBrightNonImage( i, true );
    }

// (preli) set the trigger vector in MC case (preli)
// trigger vector are image/border tubes
    if( fData->getReader() )
    {
       if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 
        || fData->getReader()->getDataFormatNum() == 6 )
	{
	   fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
        }
    }
// (end of preli)

    recoverImagePixelNearDeadPixel();
    fillImageBorderNeighbours();
}


//*****************************************************************************************************
// NN image cleaning (Maxim)
//
//  Searches N-fold connected groups (4nn close-packed group) and 2+1 sparse groups
//  with dynamic cut in  GroupThresh-GroupCoincTime phase space
//
//  NOTE: IPR curves, used for cuts,  should be obtained with the SAME signal extraction method
//        as the image under study
// preliminary
int VImageCleaning::LocMax(int n, float *ptr, float &max)
{
    if  (n <= 0) return -1;
    max = ptr[0];
    Int_t xmax = 0;
    for  (Int_t i = 1; i < n; i++) {
     if (max < ptr[i])  {
         max = ptr[i];
         xmax = i;
     }
    }
    return xmax;
}
int VImageCleaning::LocMin(int n, float *ptr, float &min) //ptr[i]>0
{
    if  (n <= 0) return -1;
    min = ptr[0];
    Int_t xmin = 0;
    for  (Int_t i = 1; i < n; i++) {
     if (min > ptr[i])  {
         min = ptr[i];
         xmin = i;
     }
    }
    return xmin;
}

bool VImageCleaning::InitNNImageCleaning()
{
    TString refIPR=Form("/afs/ifh.de/user/s/shayduk/lustre7/NSBdata/Stat/IPRcharge/IPRTelType1NSBscale%2.1fFWHM2.6E3.root",1.0);
    float fFakeImageProb=0.5E-3; // 0.2%  for LST ->less for MST, SST
    float SimTime=100.;//ns
    float fMinRate=fFakeImageProb/( SimTime*1E-9 *float(3) );//ns

    float CombFactor[5]={60000.,520000.,130000.,25000.,2}; //for 2400 pixels
    gIPR=ReadIPRGraph(refIPR,"IPRcharge");
    if(gIPR==NULL) return false;
    fProbCurve4nn=RateContour("ProbCurve4nn",fMinRate,4,CombFactor[0],0,20);
    fProbCurve3nnrel=RateContour("ProbCurve3nnrel",fMinRate,3,CombFactor[2],0,20);
    fProbCurve2plus1=RateContour("ProbCurve2plus1",fMinRate,3,CombFactor[1],0,20);
    fProbCurve2nn=RateContour("ProbCurve2nn",fMinRate,2,CombFactor[3],0,20);
    fProbCurveBound=RateContourBound("ProbCurveBound",fMinRate,3.5,CombFactor[4],0,20);

    fProbCurve2nn->SetParameter(0,2*fMinRate);
    SetCameraTypes();
    InitNeighbours();
    return true;
}

bool VImageCleaning::BoundarySearch(int type, float thresh, TF1* fProbCurve, float refdT, int refvalidity, int idx)
{
    //idx - should be next neigbour of core!!!
    //skip core pix
    if((VALIDITYBUF[idx]>1.9&&VALIDITYBUF[idx]<5.1) ) return false;

    float TimeForReSearch=0.;
    bool iffound=false;
    unsigned int     nnmax = 0;
    int neighbor[7];
    GetNeighbors(type,neighbor,idx);
    for(int j=1;j<7;j++) {if(neighbor[j]>=0) nnmax++;}
    Int_t n=0;
    float time=0.;

    // reftime from core pixels
    for(unsigned int j=1; j<=nnmax; j++){
        const Int_t idx2 = neighbor[j];
        Float_t t=TIMES[idx2];
        if(t>0.&&VALIDITYBUF[idx2]>1.9&&VALIDITYBUF[idx2]<5.1){time+=t; n++;}

    }
    // check THIS pix and ring of neighbours
    if(n>0.5){
        TimeForReSearch=time/float(n);
        float dT=fabs(TIMES[idx]-TimeForReSearch);
        float times[2]={refdT,dT}; float maxtime=1E6; LocMax(2,times,maxtime);
        float charges[2]={thresh,INTENSITY[idx]}; float mincharge=0; LocMin(2,charges,mincharge);
        if(maxtime<CoincWinLimit && maxtime<fProbCurve->Eval(mincharge))
        {
            if(VALIDITYBOUND[idx]!=refvalidity) VALIDITYBOUND[idx]=refvalidity;
            iffound=true;
        }

        for(unsigned int j=1; j<=nnmax; j++){
            const Int_t idx2 = neighbor[j];
            if((TIMES[idx2]>0.&&VALIDITYBUF[idx2]>1.9&&VALIDITYBUF[idx2]<5.1)||VALIDITYBOUND[idx2]==refvalidity){continue;}

            float dT2=fabs(TIMES[idx2]-TimeForReSearch);
            times[1]=dT2; LocMax(2,times,maxtime);
            charges[1]=INTENSITY[idx2]; LocMin(2,charges,mincharge);

            if(maxtime<CoincWinLimit && maxtime<fProbCurve->Eval(mincharge))
            {VALIDITYBOUND[idx2]=refvalidity;  iffound=true;}
        }
    }
    return iffound;
}


int VImageCleaning::NNGroupSearchProbCurve(int type, TF1* fProbCurve, float PreCut) // if Nfold =3 it will search for 2nn+1, including sparse groups (with the empty pix in between)
{
    int NN=(int)fProbCurve->GetParameter(1);//"Nfold"

    Float_t nn2;
    Float_t nn3;
    Float_t nn4;

    Int_t NN2=0;
    Int_t NN3=0;
    Int_t NN4=0;

    int NSBpix=5;
    int numpix=fNumPixels[type];

    for(int PixNum=0;PixNum<numpix;PixNum++)
    {
        unsigned int     nnmax = 0;
        int neighbor[7];
        GetNeighbors(type,neighbor,PixNum);
        for(int j=1;j<7;j++) {if(neighbor[j]>=0) nnmax++;}

        Double_t q=VALIDITY[PixNum];
        if(q<0.5 || INTENSITY[PixNum]<PreCut) continue;

        for (unsigned int j=1; j<=nnmax; j++)
        {
            const Int_t PixNum2 = neighbor[j];
            if(PixNum2<0) continue;
            Double_t q1=VALIDITY[PixNum2];

            if(q1<0.5 || INTENSITY[PixNum2]<PreCut) continue;
            Double_t dT=fabs(TIMES[PixNum]-TIMES[PixNum2]);
            float charges[2]={INTENSITY[PixNum],INTENSITY[PixNum2]}; float mincharge=0; LocMin(2,charges,mincharge);
            //if(dT>TrigWin2NN) continue;
            if(dT>CoincWinLimit || dT>fProbCurve->Eval(mincharge)) continue;

            if(VALIDITYBUF[PixNum]<2.9)  {VALIDITYBUF[PixNum]=2;}
            if(VALIDITYBUF[PixNum2]<2.9) {VALIDITYBUF[PixNum2]=2;}
            //if(VALIDITYBUF[PixNum2]<2.9) {VALIDITYBUF[PixNum2]=2;cout<<"charges:"<<charges[0]<<":"<<dT<<":"<<charges[1]<<" mincharge:"<<mincharge<<endl;}
            if(NN==2) break;
            // Boundary search (sparse group 2+1)
            //*************************
            if(VALIDITYBUF[PixNum2]==2&&NN==3)
            {
                bool iffound=false;
                int neighbor2[7];
                unsigned int     nnmax2 = 0;
                GetNeighbors(type,neighbor2,PixNum2);
                for(int m=1;m<7;m++) {if(neighbor2[m]>=0) nnmax2++;}
                for (unsigned int k=1; k<=nnmax; k++){
                    if(BoundarySearch(type, mincharge, fProbCurve,dT,3,neighbor[k])) iffound=true;
                }
                for (unsigned int k=1; k<=nnmax2; k++){
                    if(BoundarySearch(type, mincharge, fProbCurve,dT,3,neighbor2[k])) iffound=true;
                }
                //cout<<"iffound:"<<iffound<<endl;
                if(iffound)
                {
                    if(VALIDITYBUF[PixNum]<3.9) {VALIDITYBUF[PixNum]=3;}
                    if(VALIDITYBUF[PixNum2]<3.9){VALIDITYBUF[PixNum2]=3;}
                }
            }
            if(NN==3) break;
            //*************************

            if(NSBpix>2)
            {
                //Double_t y=PixelY[type*maxpixels + PixNum2];
                //Double_t x=PixelX[type*maxpixels + PixNum2];
                Double_t x=fData->getDetectorGeo()->getX()[PixNum2];
                Double_t y=fData->getDetectorGeo()->getY()[PixNum2];

                Int_t idxm=-1;
                Int_t idxp=-1;
                //Int_t idxq=-1;
                Int_t nn=0;
                for(unsigned int kk=1; kk<=nnmax; kk++)
                {
                    const Int_t k = neighbor[kk];
                    if(k<0) continue;
                    //Double_t xx=x-PixelX[type*maxpixels + k];
                    //Double_t yy=y-PixelY[type*maxpixels + k];
                    Double_t xx=x-fData->getDetectorGeo()->getX()[k];
                    Double_t yy=y-fData->getDetectorGeo()->getY()[k];

                    Double_t dist=sqrt(xx*xx+yy*yy);
                    //Double_t diam=2.*PixelR[type*maxpixels];
                    Double_t diam=2.*fData->getDetectorGeo()->getTubeRadius()[1];
                    if( dist>0.01*diam && dist<1.1*diam )
                    {
                        if(nn) idxp=k; else idxm=k;
                        nn++;
                    }
                }
                if(idxp<0)continue;

                Double_t q2=VALIDITY[idxp];
                if(q2<0.5 || INTENSITY[idxp]<PreCut) continue;

                dT=fabs(TIMES[PixNum]-TIMES[idxp]);
                double dt2=fabs(TIMES[PixNum2]-TIMES[idxp]);
                float charges2[2]={mincharge,INTENSITY[idxp]}; LocMin(2,charges2,mincharge);
                float times2[2]={dT,dt2}; float maxtime=1E6; LocMax(2,times2,maxtime);
                if(maxtime>CoincWinLimit || maxtime>fProbCurve->Eval(mincharge)) continue;

                if(VALIDITYBUF[PixNum]<3.9) {VALIDITYBUF[PixNum]=3;}
                if(VALIDITYBUF[PixNum2]<3.9){VALIDITYBUF[PixNum2]=3;}
                if(VALIDITYBUF[idxp]<3.9)   {VALIDITYBUF[idxp]=3;}
                //if(VALIDITYBUF[idxp]<3.9)   {VALIDITYBUF[idxp]=3;cout<<"PreThresh:"<<PreCut<<" 3nn charges:"<<charges[0]<<"::::"<<dT<<","<<dt2<<", maxtime="<<maxtime<<"::::"<<charges[1]<<" mincharge:"<<mincharge<<endl;}
                if(NN==3) break;
                if(NSBpix>3)
                {
                    if(idxm<0)continue;
                    Double_t q3=VALIDITY[idxm];
                    if(q3<0.5 || INTENSITY[idxm]<PreCut) continue;
                    dT=fabs(TIMES[PixNum]-TIMES[idxm]);
                    dt2=fabs(TIMES[PixNum2]-TIMES[idxm]);
                    double dt3=fabs(TIMES[idxp]-TIMES[idxm]);
                    float charges3[2]={mincharge,INTENSITY[idxm]}; LocMin(2,charges3,mincharge);
                    float times3[4]={maxtime,dT,dt2,dt3}; LocMax(4,times3,maxtime);

                    if(maxtime>CoincWinLimit || maxtime>fProbCurve->Eval(mincharge)) continue;

                    VALIDITYBUF[PixNum]=4;
                    VALIDITYBUF[PixNum2]=4;
                    VALIDITYBUF[idxp]=4;
                    VALIDITYBUF[idxm]=4;
                    //cout<<"PreThresh:"<<PreCut<<" 4nn charges:"<<charges[0]<<"::::"<<dT<<","<<dt2<<","<<dt3<<", maxtime="<<maxtime<<"::::"<<charges[1]<<" mincharge:"<<mincharge<<endl;
     
                }
            }
        }

    }//end of for() loop

    for (Int_t i=0;i<numpix;i++)
    {
        float q=VALIDITYBUF[i];
        if(q>1.5 && q<2.1) {NN2++;}
        if(q>2.5 && q<3.1) {NN3++;}
        if(q>3.5 && q<4.1) {NN4++;}
        if(VALIDITYBUF[i]<1.9) VALIDITYBUF[i]=-1;
    }

    nn2=float(NN2)/2.;
    nn3=float(NN3)/3.;
    nn4=float(NN4)/4.;

    int ngroups=0;
    if(NN==2) ngroups=int(nn2+0.5);
    if(NN==3) ngroups=int(nn3+0.5);
    if(NN==4) ngroups=int(nn4+0.5);
    return ngroups;
}

int VImageCleaning::NNGroupSearchProbCurveRelaxed(int type, TF1* fProbCurve, float PreCut)
{
    Int_t NN2=0;
    Int_t NN3=0;
    Int_t NN4=0;
    Double_t dT=0, dt2=0, dt3=0;

    int numpix=fNumPixels[type];

    for(int PixNum=0;PixNum<numpix;PixNum++)
    {
        unsigned int     nnmax = 0;
        int neighbor[7];
        GetNeighbors(type,neighbor,PixNum);
        for(int j=1;j<7;j++) {if(neighbor[j]>=0) nnmax++;}

        Double_t q=VALIDITY[PixNum];
        if(q<0.5 || INTENSITY[PixNum]<PreCut) continue;
        int NNcnt=1;
        int pix1=0, pix2=0, pix3=0;
        for (unsigned int j=1; j<=nnmax; j++)
        {
            const Int_t PixNum2 = neighbor[j];
            if(PixNum2<0) continue;
            Double_t q1=VALIDITY[PixNum2];

            if(q1<0.5 || INTENSITY[PixNum2]<PreCut) continue;
            dT=fabs(TIMES[PixNum]-TIMES[PixNum2]);
            //if(dT>TrigWin3NNrelax) continue;
            float charges[2]={INTENSITY[PixNum],INTENSITY[PixNum2]}; float mincharge=0; LocMin(2,charges,mincharge);
            //if(dT>TrigWin2NN) continue;
            if(dT>CoincWinLimit || dT>fProbCurve->Eval(mincharge)) continue;
            float maxtime=1E6;
            if(dT<fProbCurve->Eval(mincharge) && q1> 0.5 && INTENSITY[PixNum2]>PreCut) {
                pix1=PixNum;
                if(PixNum2!=pix2){pix3=PixNum2;}
                if(pix2==0)      {pix2=PixNum2;}
                dt2=fabs(TIMES[pix2]-TIMES[pix3]);
                dt3=fabs(TIMES[pix1]-TIMES[pix3]);
                float charges2[3]={mincharge,INTENSITY[pix3],INTENSITY[pix2]}; LocMin(3,charges2,mincharge);
                float times2[3]={dT,dt2,dt3}; LocMax(3,times2,maxtime);

                if(maxtime<CoincWinLimit && maxtime<fProbCurve->Eval(mincharge)){NNcnt++;}
            }

            if(NNcnt>2){
            if(VALIDITYBUF[pix1]<4.9) {VALIDITYBUF[pix1]=5;}
            if(VALIDITYBUF[pix2]<4.9) {VALIDITYBUF[pix2]=5;}
            if(VALIDITYBUF[pix3]<4.9) {VALIDITYBUF[pix3]=5;}//cout<<"PreThresh:"<<PreCut<<" 3nn charges:"<<INTENSITY[pix3]<<","<<INTENSITY[pix2]<<" mincharge:"<<mincharge<<"maxtime:"<<maxtime<<endl; }
            }
        }

    }//end of for() loop

    for (Int_t i=0;i<numpix;i++)
    {
        float q=VALIDITYBUF[i];
        if(q>1.5 && q<2.1) {NN2++;}
        if(q>4.5 && q<5.1) {NN3++;}
        if(q>3.5 && q<4.1) {NN4++;}
        if(VALIDITYBUF[i]<1.9) VALIDITYBUF[i]=-1;
    }
    if(NN3>2) {return NN3/3;}
    if(NN3<3) {return 0;}
    return NN3/3;
}

float VImageCleaning::ImageCleaningCharge(int type, float NSBscale, int& ngroups)
{
    int numpix=fNumPixels[type];
    float PreThresh[5]={2.0,3.2,3.1, 4.7,1.3};
    ngroups=0;
    memset(VALIDITYBOUND,0,sizeof(VALIDITYBOUND));
    memset(VALIDITY,0,sizeof(VALIDITY));
    memset(VALIDITYBUF,0,sizeof(VALIDITYBUF));

    for (int p=0;p<numpix; p++)
    {
        if(INTENSITY[p]>PreThresh[4]*sqrt(NSBscale)/sqrt(0.6)) { VALIDITY[p]=1; VALIDITYBUF[p]=1;  }
        else               { VALIDITY[p]=-1; VALIDITYBUF[p]=-1;}
    }
    //*************** Image Cleaning ******************************************************************************************
    //ngroups=NNGroupSearchProbCurve(type,fProbCurve2nn, PreThresh[3]*sqrt(NSBscale)/sqrt(0.6)); for(int p=0;p<numpix;p++){if(VALIDITYBUF[p]==2) VALIDITY[p]=2; }
    //ngroups+=NNGroupSearchProbCurve(type,fProbCurve2plus1, PreThresh[1]*sqrt(NSBscale)/sqrt(0.6)); for(int p=0;p<numpix;p++){if(VALIDITYBUF[p]==3 ||VALIDITYBOUND[p]==3) VALIDITY[p]=3; }
    ngroups=NNGroupSearchProbCurveRelaxed(type,fProbCurve3nnrel, PreThresh[2]*sqrt(NSBscale)/sqrt(0.6)); for(int p=0;p<numpix;p++){if(VALIDITYBUF[p]==5) VALIDITY[p]=5; }
    //ngroups+=NNGroupSearchProbCurve(type,fProbCurve4nn, PreThresh[0]*sqrt(NSBscale)/sqrt(0.6)); for(int p=0;p<numpix;p++){if(VALIDITYBUF[p]==4) VALIDITY[p]=4; }
    //*********************************************************************
    // Boundary
    float TIMESReSearch[numpix];
    for(int p=0;p<numpix;p++){TIMESReSearch[p]=0.;}

    for(unsigned int iRing=0;iRing<nRings;iRing++){
        for(Int_t idx=0; idx<numpix;idx++){
            if((VALIDITY[idx]<5.1)&&(VALIDITY[idx]>1.9)) continue;
            if((iRing>0)&&(VALIDITY[idx]<(int)iRing+6)&&((float)VALIDITY[idx]>1.9)) continue;

            unsigned int     nnmax = 0;
            int neighbor[7];
            GetNeighbors(type,neighbor,idx);
            for(int j=1;j<7;j++) {if(neighbor[j]>=0) nnmax++;}
            Int_t n=0;
            float time=0.;
            float timerms=0;

            for(unsigned int j=1; j<=nnmax; j++){
                const Int_t idx2 = neighbor[j];
                if(idx2<0||VALIDITY[idx2]<1.9) continue;

                if(iRing==0){
                    if((float)VALIDITY[idx2]<1.9 || VALIDITY[idx2]==(int)iRing+6) continue;
                    if(TIMESReSearch[idx2]>0.)   { time+=TIMESReSearch[idx2]; timerms+=TIMESReSearch[idx2]*TIMESReSearch[idx2]; n++;}
                    else                           { Float_t t=TIMES[idx2]; if(t>0.){time+=t; timerms+=t*t; n++;}}
                }
                if(iRing>0){
                    if(VALIDITY[idx2]==(int)iRing+5){
                        if(TIMESReSearch[idx2]>0.)   { time+=TIMESReSearch[idx2]; timerms+=TIMESReSearch[idx2]*TIMESReSearch[idx2]; n++;}
                        else                           { Float_t t=TIMES[idx2]; if(t>0.){time+=t; timerms+=t*t; n++;}}
                    }
                }
            }

            if(n>0.5){
                TIMESReSearch[idx]=time/float(n);
                float radicand=0;
                if(n>1) radicand=(timerms - float(n)*TIMESReSearch[idx]*TIMESReSearch[idx])/(float(n)-1.);
                else {radicand=1.;}
                if(radicand<0){timerms=1000;} else {timerms=sqrt(radicand);}
                float dT=fabs(TIMES[idx]-TIMESReSearch[idx]);

                if(dT<10. && dT<fProbCurveBound->Eval(INTENSITY[idx]))
                {
                    VALIDITY[idx]=iRing+6;

                }
            }
        }
    } // loop over rings
    unsigned int ncorepix=0;
    unsigned int ncore4nnpix=0;
    unsigned int nboundary=0;

    for(int p=0;p<numpix;p++)
    {
        if(VALIDITY[p]>1.9 && VALIDITY[p]<5.1 ) ncorepix++;
        if(VALIDITY[p]>5.1 ) nboundary++;
        if(VALIDITY[p]==4) ncore4nnpix++;
    }
    //Discard small images and 4nn with no boundaries: Important protection from fully fake images which can destroy  shower dir reconstuction
    // prob of any of fake group is <0.8%
    // after this discarding prob of fake image <0.05% !!!
    if(((ncorepix+nboundary)<4)||(ncore4nnpix==4&&nboundary==0)){
        for(int p=0;p<numpix;p++)
        {
            if(VALIDITY[p]>1.9) VALIDITY[p]=1;
        }
        ngroups=0;
    }
    int imageflag=0;
    float SIZE=0;

    for(int p=0;p<numpix;p++){ if(VALIDITY[p]>1.9) {SIZE+=INTENSITY[p]; imageflag++;}}
    if(imageflag==0) SIZE=-1;
    return SIZE;
}
// main function
void VImageCleaning::cleanNNImageFixed()
{
    if( !fData ) printDataError( "VImageCleaning::cleanNNImageFixed" );
    if( !kInitNNImageCleaning) printDataError( "VImageCleaning::cleanNNImageFixed image cleaning not initialized" );
    // calculates the valarray of tubes to be included in the parameterization
    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );
    fData->setImageBorderNeighbour( false );
    unsigned int i_nchannel = fData->getNChannels();
    //FIXME it is ugly :)
    int type=0;
    if(i_nchannel==1039) type=0;
    if(i_nchannel==2841) type=1;
    if(i_nchannel==1765) type=2;
    if(i_nchannel==1417) type=3;   // problem
    if(i_nchannel==1417) type=3;   //
    if(i_nchannel==931)  type=5;
    float sizecheck=0;
    int ngroups=0;
    float FADCtoPhe=0.0722449;// cnt/phe for 3ns int
    float FADCsampleRate=2.;// GHz
    //prepare for image cleaning
    for ( unsigned int i=0; i < i_nchannel; i++)
    {
        INTENSITY[i]=0;
        TIMES[i]=-1;
        VALIDITY[i]=-1;
        if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] )
        {
            INTENSITY[i]=FADCtoPhe*fData->getSums()[i];
            TIMES[i]=fData->getTraceAverageTime()[i]/FADCsampleRate;
        }
    }
    sizecheck = ImageCleaningCharge(type, 1.0, ngroups);

    //set pixel's flags
    for ( unsigned int i=0; i < i_nchannel; i++)
    {
        if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] )
        {
            //if(VALIDITY[i]>1.9&&VALIDITY[i]<5.1) fData->setImage( i, true );
            //if(VALIDITY[i]>5.1) fData->setBorder( i, true );
            if(INTENSITY[i]>6) fData->setBorder( i, true );
            if(INTENSITY[i]>12) fData->setImage( i, true );
            if(INTENSITY[i]>12)cout<<"Type"<<type<<" size:"<<sizecheck<<"phes.  pix:"<<i<<" charge:"<<INTENSITY[i]<<" arrtime:"<<TIMES[i]<<" valid:"<<VALIDITY[i]<<"ngroups:"<<ngroups<<endl;

        }
    }
    recoverImagePixelNearDeadPixel();
    //fillImageBorderNeighbours();
}

void VImageCleaning::cleanNNImagePedvars()
{

}
// end of NN image cleaning
//*****************************************************************************************************




/*!
  Image cleaning routine using pixel timing information
  based on Nepomuks PhD thesis time-cluster cleaning algorithm
   - uses fixed time differences for discrimination of pixels/clusters
   - adjusts time difference according to the time gradient
   - handles single core pixel
   - BrightNonImages not completely implemented yet (needs checks - but who is really using them?)

   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
   \par timeCutPixel time diffeence between pixels
   \par timeCutCluster time difference between clusters
   \par minNumPixel minimum number of pixels in a cluster
   \par loop_max number of loops
*/

void VImageCleaning::cleanImageFixedWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max )
{
    cleanImageWithTiming( hithresh, lothresh, brightthresh, timeCutPixel, timeCutCluster, minNumPixel, loop_max, true );
}

void VImageCleaning::cleanImagePedvarsWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max )
{
    cleanImageWithTiming( hithresh, lothresh, brightthresh, timeCutPixel, timeCutCluster, minNumPixel, loop_max, false );
}


void VImageCleaning::cleanImageWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max, bool isFixed )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::cleanImageWithTiming " << fData->getTelID() << "\t" << brightthresh << endl;

    // check if time gradient was already calculated
    if( fData->getImageParameters()->tgrad_x == 0 && fData->getImageParameters()->tint_x == 0 )
      {
	timeCutPixel = 5.0;
	timeCutCluster = fData->getNSamples();         // = number of readout samples
      }
    else 
      {
	float tpix = fData->getImageParameters()->tgrad_x * 0.15; // VERITAS pixel size (HARDCODED)
	if( tpix > timeCutPixel )
	  { 
	    timeCutPixel = tpix + 0.1;                     // added some uncertenties adhoc
	  }
      }
//     cout << "TimeImageCleaning: TC = " << timeCutPixel << " / " << timeCutCluster << " L = " << loop_max << endl;

    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );

    unsigned int i_nchannel = fData->getNChannels();
    
    //////////////////////////////////////////////////////
    // STEP 1: Select all pixels with a signal > hithresh 
    //

    double i_pedvars_i = 0.;
    for ( unsigned int i = 0; i < i_nchannel; i++)
    {
        if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead(fData->getHiLo()[i])[i]) continue;
	
	if( isFixed )
	{
	    if( fData->getSums()[i] > hithresh )     fData->setImage( i, true );
//	    if( fData->getSums()[i] > brightthresh ) fData->setBrightNonImage( i, true );
	}
	else
	{
	    i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i])[i];
	    if( fData->getSums()[i] > hithresh * i_pedvars_i )      fData->setImage( i, true );
//	    if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) fData->setBrightNonImage( i, true );
	}
    }

    //////////////////////////////////////////////////////////
    // STEP 2: Make clusters 
    //
    //       - group touching core pixels to clusters 
    //         (only if the pixels are coincident in time!)


    // REALLY NEEDED: in case there is still something in the memory, reset all channels to cluster id 0
    //                needs to be checked for memory leaks!!!
    for( unsigned int i = 0; i < i_nchannel; i++ ) fData->setClusterID( i, 0 );

    int i_cluster=0;
    int c_id=0;

    for( unsigned int i = 0; i < i_nchannel; i++ )
      {
	if( fData->getImage()[i] )
	  {
	    if( fData->getClusterID()[i] != 0 )
	      {
		c_id = fData->getClusterID()[i];
	      }
	    else if( i_cluster==0 || fData->getClusterID()[i] == 0 )
	      {
		i_cluster++;
		c_id = i_cluster;
	      }
	    else
	      cout << "WARNING: Something looks wrong - this should not happen\n" 
		   << "["<<i_cluster << "/" << c_id << "] " << i << endl;

	    fData->setClusterID( i, c_id );

	    for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
	      {
		unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
		if( fData->getImage()[k] && fData->getClusterID()[k] == 0 )
		  {
  		    if ( fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel ) 
 		      {
			fData->setClusterID( k, c_id );
 		      }
		  }
	      }
	  }
      }

    //////////////////////////////////////////////////////////
    // STEP 3: Calculate for each cluster weighted mean time 
    //
    //   - each cluster gets its own "clustertime" and "clustersize"
    //   - the cluster with the largest size is the "main" cluster
    //
    // new: for time difference adjustment calculate "cenx" and "ceny"

    int i_clusterNpix;      // number of pixels in cluster
    double i_clustersize;   // size of cluster 
    double i_clustertime;   // weighted mean time of all pixels in a cluster 

    double i_cenx, i_ceny;
    double i_clustercenx=0.;   // X center of gravity of the cluster
    double i_clusterceny=0.;   // Y center of gravity of the cluster

    double i_mainclustersize=0;   // size of the "main cluster"

    int cluster = 0;
    while( cluster <= i_cluster ){

      i_clusterNpix=0;
      i_clustersize=0.;
      i_clustertime=0.;
      i_clustercenx=0.; i_cenx=0.;
      i_clusterceny=0.; i_ceny=0.;

      for( unsigned int i = 0; i < i_nchannel; i++ )
	{
   	  if( fData->getClusterID()[i] == cluster && fData->getImage()[i] )
	    {
	      i_clusterNpix++; 

	      i_clustersize += fData->getSums()[i]; 
	      i_clustertime += ( fData->getSums()[i] * fData->getTZeros()[i] ); 
	      
	      double xi = fData->getDetectorGeo()->getX()[i];
	      double yi = fData->getDetectorGeo()->getY()[i];
	      
	      i_cenx += ( fData->getSums()[i] * xi );
	      i_ceny += ( fData->getSums()[i] * yi );
	    }
	}
      if( i_clustersize != 0 ) 
	{
	  i_clustertime = i_clustertime / i_clustersize;
	  i_clustercenx = i_cenx / i_clustersize;
	  i_clusterceny = i_ceny / i_clustersize;
	} 
      else 
	{ 
	  i_clustertime = -99; 
	  i_clustersize = -99; 
	  i_clustercenx = -99; 
	  i_clusterceny = -99;	
	}

      fData->setClusterNpix( cluster, i_clusterNpix );
      fData->setClusterSize( cluster, i_clustersize );
      fData->setClusterTime( cluster, i_clustertime );
      fData->setClusterCenx( cluster, i_clustercenx );
      fData->setClusterCeny( cluster, i_clusterceny );

//       cout << "### " << cluster << " Npix=" << i_clusterNpix << " size=" << i_clustersize << " time=" << i_clustertime 
// 	   << " cenx=" << i_clustercenx << " ceny=" << i_clusterceny << endl;

      if( i_clustersize == -99 ) fData->setMainClusterID( 0 );
      else if( i_clustersize >= i_mainclustersize )
        {
          i_mainclustersize = i_clustersize;
          fData->setMainClusterID( cluster );
        }
      cluster++;
    }
//     cout << "MAIN CLUSTER " << getMainClusterID()
// 	 << ": Npix=" << getClusterNpix()[ getMainClusterID() ] 
// 	 << " Size=" << getClusterSize()[ getMainClusterID() ] 
// 	 << " Time=" << getClusterTime()[ getMainClusterID() ] << endl;

    //////////////////////////////////////////////////////////////////////////////////////////
    // STEP 4: eliminate all clusters with time differences > Tcluster to the main cluster 
    //
    //         NEW: use tgrad_x to calculate Tcluster


    int i_ID = 0;

    float i_mainX=0.;      // c.o.g. (main cluster)
    float i_mainY=0.;      // c.o.g. (main cluster)
    float i_mainXpos=0.;   // position on major axis
    float i_mainXtime=0.;  // time on major axis

    float i_clusterX, i_clusterY;  // c.o.g. (cluster)
    float i_clusterXpos;           // position on major axis
    float i_clusterXtime;          // time on major axis

    if( fData->getImageParameters()->tgrad_x != 0 && fData->getImageParameters()->tint_x != 0 ) 
      {   
	i_mainX = fData->getClusterCenx()[ fData->getMainClusterID() ];
	i_mainY = fData->getClusterCeny()[ fData->getMainClusterID() ];
	i_mainXpos = i_mainX * fData->getImageParameters()->cosphi + i_mainY * fData->getImageParameters()->sinphi;
	i_mainXtime = fData->getImageParameters()->tgrad_x * i_mainXpos;
	
	i_clusterX = 0.;
	i_clusterY = 0.;
	i_clusterXpos = 0.;
	i_clusterXtime = 0.;

	cluster = 1;
	while( cluster <= i_cluster )
	  {
	    i_clusterX = fData->getClusterCenx()[ cluster ];
	    i_clusterY = fData->getClusterCeny()[ cluster ];
	    i_clusterXpos = i_clusterX*fData->getImageParameters()->cosphi + i_clusterY*fData->getImageParameters()->sinphi;
	    i_clusterXtime = fData->getImageParameters()->tgrad_x*i_clusterXpos;
	    
	    for( unsigned int i = 0; i < i_nchannel; i++ )
	      {
		i_ID = fData->getClusterID()[i];
		if( i_ID == 0 || i_ID == fData->getMainClusterID() ) continue;

		if( i_ID == cluster && fData->getImage()[i] )
		  {
// 		    cout<< "  " <<i_ID << "  delta_T  = " << fabs(xtime - xtimemain) << endl;
// 		    cout<< "  " <<i_ID << "  time diff= " << fabs( getClusterTime()[i_ID] - getClusterTime()[getMainClusterID()]) << endl;
// 		    cout<< i_ID << " " << i << ": " << getClusterTime()[i_ID] << "-" << getClusterTime()[getMainClusterID()] << "=" 
// 			<< getClusterTime()[i_ID] - getClusterTime()[getMainClusterID()]  
// 			<< " fabs(" << i_clusterXtime << "-" << i_mainXtime << "=" << i_clusterXtime - i_mainXtime << ") = " 
// 			<< fabs( (getClusterTime()[i_ID]-getClusterTime()[getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) << endl;
		    
		    if( fabs( (fData->getClusterTime()[i_ID]-fData->getClusterTime()[fData->getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) > timeCutCluster )
		      {
// 			cout << " --> "<< i << " cluster time cleaning : " 
// 			     << fabs( (getClusterTime()[i_ID]-getClusterTime()[getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) <<  endl;
			fData->setImage( i, false );
			fData->setClusterID( i, -99 );
		      }
		  }
	      }
	    cluster++;
	  }
      }
    else 
      {
	for( unsigned int i = 0; i < i_nchannel; i++ )
	  {
	    if( fData->getClusterID()[i] == 0 ) continue;
	    
	    if( fData->getImage()[i] )
	      {
		i_ID = fData->getClusterID()[i];
		i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i])[i];
		
		if( i_ID != fData->getMainClusterID() && fabs( fData->getClusterTime()[i_ID] - fData->getClusterTime()[fData->getMainClusterID()] ) > timeCutCluster )
		  {
// 		    cout << "----> "<< i << " was removed with cluster time cleaning" << endl;
		    fData->setImage( i, false );
		    fData->setClusterID( i, -99 );
	      
// 	      if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) setBrightNonImage( i, true );
//  	      if( fData->getSums()[i] > hithresh  * i_pedvars_i ) setBrightNonImage( i, true );
		  }
	      }
	  }
      }

    /////////////////////////////////////////////////////////////////    
    // STEP 5: find boundary pixels (and add them to the cluster) 
    // 
    //  selection criteria:
    //  - signal above lothresh
    //  - loop through already cleaned direct neighbors with time difference < Tpixel
    //
    //  this step can be reiterated more than once (number of loops)
    //
    
    for( int loop=0; loop < loop_max; loop++ )
      {
	int counter=0;
	int tmp_border[i_nchannel];
	int tmp_cluster[i_nchannel];

	double i_pedvars_k = 0.;
	for ( unsigned int i = 0; i < i_nchannel; i++)
	  {
// 	    if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 || 
//  		fData->getClusterID()[i] == 0 || fData->getClusterID()[i] == -99 ) continue;
	    
	    i_ID = fData->getClusterID()[i];
	    if( fData->getImage()[i] || fData->getBorder()[i] )
	      {
		for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
		  {
		    unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];

		    if( isFixed )
		    {
			if( !fData->getImage()[k] && !fData->getBorder()[k] && fData->getSums()[k] > lothresh 
			    && fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
			{
			    tmp_border[counter]=k;
			    tmp_cluster[counter]=i_ID;
			    counter++;
			}
		    }
		    else 
		    {
			i_pedvars_k = fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k])[k];
			if( !fData->getImage()[k] && !fData->getBorder()[k] 
			    && fData->getSums()[k] > lothresh * i_pedvars_k 
			    && fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
			{
			    tmp_border[counter]=k;
			    tmp_cluster[counter]=i_ID;
			    counter++;
			}
		    }
		  }
	      }
	  }
	if( fData->getMainClusterID() != 0 )
	  {
	    //  	    cout << "setBorder: ";
	    for( int pixel = 0; pixel < counter; pixel++ )
	      {
  		//cout << tmp_border[pixel] << "(" << tmp_cluster[pixel] << ") , ";
		fData->setBorder( tmp_border[pixel], true );
 		fData->setClusterID( tmp_border[pixel], tmp_cluster[pixel] );
	      }
  	    //cout << endl;
	  }
      }

    /////////////////////////////////////////////////////////////////////
    // STEP 6: merge touching clusters and reset cluster parameters 
    //

    mergeClusters();

//     for( unsigned int i = 0; i < i_nchannel; i++ ) setClusterID( i, 0 );
    for( int x=0; x <= i_cluster; x++ )
      {
	fData->setClusterNpix( x, 0 ); 
	fData->setClusterSize( x, 0 );
	fData->setClusterTime( x, 0 );
      } 
   
    i_mainclustersize=0;
    cluster = 1;

    while( cluster <= i_cluster ){

      i_clusterNpix=0;
      i_clustersize=0.;
      i_clustertime=0.;

      for( unsigned int i = 0; i < i_nchannel; i++ )
	{
 	  if( fData->getClusterID()[i] == cluster )
	    {
	      if( fData->getImage()[i] || fData->getBorder()[i] ) 
		{
		  i_clusterNpix++;
		  i_clustersize += fData->getSums()[i];
		  i_clustertime += fData->getSums()[i] * fData->getTZeros()[i];
		} 
	      else fData->setClusterID( i, -99 );
	    }
	}
      if( i_clustersize != 0 ) i_clustertime = i_clustertime/i_clustersize;
      else { i_clustertime = -99.; i_clustersize = -99; }

      fData->setClusterNpix( cluster, i_clusterNpix );
      fData->setClusterSize( cluster, i_clustersize );
      fData->setClusterTime( cluster, i_clustertime );

//       if( i_clusterNpix > 0 )
// 	cout << "     CLUSTER " <<cluster<< ": Npix=" <<i_clusterNpix<< " Size=" <<i_clustersize<< " Time=" << i_clustertime << endl;

      if( i_clustersize == -99 ) fData->setMainClusterID( 0 );
      else if( i_clustersize >= i_mainclustersize )
        {
          i_mainclustersize = i_clustersize;
          fData->setMainClusterID( cluster );
        }
      cluster++;
    }

    //  count number of clusters before removing (useful for hadron rejection?)
    set< int > tmp_counter_uncleaned;
    for( unsigned int i = 0; i < fData->getNChannels(); i++)
      {  
	if ( fData->getImage()[i] || fData->getBorder()[i] )
	  {
	    i_ID = fData->getClusterID()[i]; 
	    if ( i_ID != 0 || i_ID != -99 ) 
	      {
		tmp_counter_uncleaned.insert( i_ID );
	      }
	  }
      }
    fData->setNcluster_uncleaned( tmp_counter_uncleaned.size() );

    /////////////////////////////////////////////////////////////
    // STEP 7: eliminate clusters with less then XX pixels 
    //         & clusters where one core pixel has less then 2 direct border pixels
    
    removeSmallClusters( minNumPixel );


    /////////////////////////////////////////////////////////////
    // FINAL STEP: 
    //
    //  - count number of clusters (useful for hadron rejection?)

    set< int > tmp_counter_cleaned;
    for( unsigned int i = 0; i < fData->getNChannels(); i++)
      {  
	if ( fData->getImage()[i] || fData->getBorder()[i] )
	  {
	    i_ID = fData->getClusterID()[i]; 
	    if ( i_ID != 0 || i_ID != -99 ) 
	      {
		tmp_counter_cleaned.insert( i_ID );
	      }
	  }
      }
    fData->setNcluster_cleaned( tmp_counter_cleaned.size() );



//       cout << "BRIGHT NON IMAGES : ";
//       for( unsigned int i = 0; i < i_nchannel; i++ )
//       {
//       // check for bright non images
//       if( fData->getBrightNonImage()[i] )
//       {
//       if( fData->getImage()[i] ) setBrightNonImage( i, false );
//       else if( fData->getBorder()[i] ) setBrightNonImage( i, false );
//       }
      
//       if( fData->getBrightNonImage()[i] )
//       cout << i << ", ";
//       }
//       cout << endl;


    if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 || fData->getReader()->getDataFormatNum() == 6 ) 
      fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
    

    fillImageBorderNeighbours();
}


void VImageCleaning::mergeClusters()
{
  int i_clusterID;
  int k_clusterID;

  for( unsigned int i = 0; i < fData->getNChannels(); i++)
    {  
      if ( fData->getImage()[i] || fData->getBorder()[i] ) 
	{
	  i_clusterID = fData->getClusterID()[i];
	  unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
	  
	  for( unsigned int j = 0; j < i_neighbour_size; j++ )
	    {
	      unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
	      k_clusterID = fData->getClusterID()[k];
	      
	      if ( (fData->getImage()[k]||fData->getBorder()[k]) && k_clusterID !=0 && k_clusterID != fData->getClusterID()[i] ) 
		{
		  for( unsigned int n = 0; n < fData->getNChannels(); n++)
		    {
		      if ( (fData->getImage()[n]||fData->getBorder()[n]) && fData->getClusterID()[n] == fData->getClusterID()[k] ) 
			{
			  fData->setClusterID( n, i_clusterID );
			}
		    }
		}
	    }
	}
    }
}


void VImageCleaning::removeSmallClusters( int minPix )
{
  int i_cluster = 0;

  for( unsigned int i = 0; i < fData->getNChannels(); i++ )
    {
      i_cluster = fData->getClusterID()[i];
      if( i_cluster == 0 || i_cluster == -99 ) continue;
      
// remove clusters with less then minPix
      if( fData->getClusterNpix()[i_cluster] < minPix )
	{
// 	  if( fData->getImage()[i] || fData->getBorder()[i] ) cout << "ELIMINATOR ACTIVE for cluster " << i_cluster << " : ";
	  
	  if( fData->getImage()[i] )
	    {
// 	      cout << i << ", ";
	      fData->setImage( i, false );
	      fData->setClusterID( i, -99 );
	    }
	  else if( fData->getBorder()[i] )
	    {
// 	      cout << i << ", ";
	      fData->setBorder( i, false );
	      fData->setClusterID( i, -99 );
	    }
// 	  cout << endl;
	}
	  
// remove single core pixels with less than two direct border pixels
      int c1=0;
      int c2=0;

      bool dont_remove = false;
      
      if( fData->getImage()[i] )
	{
	  unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
	  for( unsigned int j = 0; j < i_neighbour_size; j++ )
	    {
	      unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
	      if( fData->getImage()[k] ) 
		{
		  dont_remove=true;
		}
	      else if ( fData->getBorder()[k] )
		{
		  c1++;
		}
	      else if( k < fData->getDead().size() && fData->getDead(fData->getHiLo()[k])[k] )
		{
		  for( unsigned l = 0; l < i_neighbour_size; l++ )
		    {
		      unsigned int m = fData->getDetectorGeo()->getNeighbours()[i][l];
		      if ( m != i && m < fData->getBorder().size() && (fData->getBorder()[m] || fData->getImage()[m]) )
			{
			  c2++;
			}
		    }
		}
	    }
	  if( dont_remove ) continue;
	}
      
      if( c1+c2 < 2 && fData->getImage()[i] )
	{
// 	  cout << "----> "<< i << " was removed as single core pixel (incl ";
	  fData->setImage( i, false );
	  fData->setBorder( i, false );
	  fData->setBrightNonImage( i, true );
	  fData->setClusterID( i, -99 );
	  
// remove the rest of the single core cluster (if it exists)
	  unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
	  for( unsigned int j = 0; j < i_neighbour_size; j++ )
	    {
	      unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
	      if ( fData->getBorder()[k] )
		{
		  fData->setBorder( k, false );
// 		  cout << k << " ";
		  
		  for( unsigned l = 0; l < fData->getDetectorGeo()->getNNeighbours()[k]; l++ )
		    {
		      unsigned int m = fData->getDetectorGeo()->getNeighbours()[k][l];
		      if ( fData->getBorder()[m] )
			{
			  fData->setBorder( m, false );
// 			  cout << m << " ";
			}
		    }
		}
	    }
// 	  cout << ")" << endl;
	}
    }
  
}


/*
  HP END
*/


void VImageCleaning::fillImageBorderNeighbours()
{
    fData->setImageBorderNeighbour( false );
// assume fData->getImageBorderNeighbour() are all false
    for( unsigned int i = 0; i < fData->getNChannels(); i++)
    {
        if ( fData->getImage()[i] || fData->getBorder()[i] )
        {
// a pixel is its own neighbour :-)
            fData->getImageBorderNeighbour()[i] = true;

            unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
            for( unsigned int j = 0; j < i_neighbour_size; j++ )
            {
                unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
                if( k < fData->getImageBorderNeighbour().size() && !fData->getDead()[k] ) fData->getImageBorderNeighbour()[k] = true;
            }
        }
    }
}


void VImageCleaning::recoverImagePixelNearDeadPixel()
{
// loop again to remove isolated image pixels
// if neighbour is dead, check neighbours of this dead channel (see e.g. run 329 event 709)
    bool i_neigh = false;
    unsigned int i_neighbour_size = 0;
    unsigned int k = 0;

    for( unsigned int i = 0; i < fData->getNChannels(); i++)
    {
        if ( fData->getImage()[i])
        {
            i_neigh = false;
            i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
            for( unsigned int j = 0; j < i_neighbour_size; j++ )
            {
                k = fData->getDetectorGeo()->getNeighbours()[i][j];
                if ( k < fData->getBorder().size() && ( fData->getBorder()[k] ||fData->getImage()[k] ) )
                {
                    fData->setImage( i, true );
                    i_neigh = true;
                    break;
                }
                else if( k < fData->getDead().size() && fData->getDead(fData->getHiLo()[k])[k] )
                {
                    for( unsigned l = 0; l < i_neighbour_size; l++ )
                    {
                        unsigned int m = fData->getDetectorGeo()->getNeighbours()[i][l];
                        if ( m != i && m < fData->getBorder().size() && (fData->getBorder()[m] || fData->getImage()[m]) )
                        {
                            fData->setImage( i, true );
                            i_neigh = true;
                            break;
                        }
                    }
                }
                if( !i_neigh ) fData->setImage( i, false );
            }
        }
        if( fData->getBrightNonImage()[i] )
        {
            if( fData->getImage()[i] ) fData->setBrightNonImage( i, false );
            else if( fData->getBorder()[i] ) fData->setBrightNonImage( i, false );
        }
    }

}

//
// MS: produce a trigger map for calculation of  binary-image Hillas parameters and apply some sort of cleaning algorithm to reject noisy pixels
//
void VImageCleaning::cleanTriggerFixed( double hithresh, double lothresh )
{
   if( fData->getDebugFlag() ) 
   {
       cout << "VImageCleaning::cleanTriggerFixed() " << hithresh << "\t" << lothresh << endl;
   }
// MS: produce a trigger-level cleaned image. The logic here is that one might want to require
// only patches of 3 nearest neighbors to constitute a valid center pixel.
    fData->setTrigger( false );
    unsigned int i_nchannel = fData->getNChannels();

    for( unsigned int i=0; i< i_nchannel; i++ )
    {
        if( fData->getDetectorGeo()->getAnaPixel()[i] > 0  && !fData->getDead(fData->getHiLo()[i])[i] )
        {
                                                  // use the CFD hits
            if( fData->getRunParameter()->fPWmethod == 0 || fData->getRunParameter()->fPWmethod == 1 )
            {
                if( fData->getReader()->getFullTrigVec()[i]  )
                {
// ensure that at least XX neighbors are above this threshold
                    int local_neighbors = 0;
                    for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                    {
                        unsigned int k=fData->getDetectorGeo()->getNeighbours()[i][j];
                        if( k < fData->getTrigger().size()  && fData->getReader()->getFullTrigVec()[k]  )
                            local_neighbors++;
                    }
                    if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors )
                        fData->setTrigger( i, true );
                }
            }

                                                  // use a software cleaning algorithm for determining the trigger hits from the FADCs
            if( fData->getRunParameter()->fPWmethod == 2 || fData->getRunParameter()->fPWmethod == 3 )
            {
                if( fData->getSums()[i] > fData->getRunParameter()->fPWcleanThreshold )
                {
// ensure that at least XX neighbors are above this threshold
                    int local_neighbors = 0;
                    for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                    {
                        unsigned int k=fData->getDetectorGeo()->getNeighbours()[i][j];
                        if( k < fData->getTrigger().size()  && fData->getSums()[k] > fData->getRunParameter()->fPWcleanThreshold  )
                            local_neighbors++;
                    }
                    if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors )
                        fData->setTrigger( i, true );
                }
            }
        }
    }

                                                  // then include the next row of pixels that "made" the center pixel alive
    if( fData->getRunParameter()->fPWmethod == 1 || fData->getRunParameter()->fPWmethod == 3 )
    {
        vector< bool > tmp_trigger( i_nchannel, 0 );

        for( unsigned int i=0; i< i_nchannel; i++ )
        {
            if( fData->getDetectorGeo()->getAnaPixel()[i] > 0  && !fData->getDead(fData->getHiLo()[i])[i] )
            {

                if( fData->getReader()->getFullTrigVec()[i]  )
                {
                    int local_neighbors = 0;
                    for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                    {
                        unsigned int k=fData->getDetectorGeo()->getNeighbours()[i][j];
                        if( k < fData->getTrigger().size()  &&  fData->getTrigger()[k]  )
                            local_neighbors++;
                    }
                    if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors - 1 )
                        tmp_trigger[i] = 1;
                }
            }
        }

        for( unsigned int i=0; i< i_nchannel; i++ )
        {
            fData->setTrigger( i, (tmp_trigger[i] | fData->getTrigger()[i]) );
        }

    }

// assume that the image has been cleaned according to spec, then a new parameter will decide
// wether  to apply a reduction in the number of pixels transmitted:
// This will mask all pixels past the cut-off to be ZERO

    if( fData->getRunParameter()->fPWlimit > 0 )                   // then we'll transmit up to a fixed number of pixels per sector
    {
        const int n_sectors = 3;
        const int sector_max = 182;               // there are 192 channels per trigger sector, but not all are used
        int sector_count[n_sectors] = {0};

        for( int i=0; i<n_sectors; i++ )
        {
            for( int j=0; j<sector_max; j++ )
            {
                int current_pixel = sector_channel_to_pixel[j][i+1]  -1 ;

                if(    fData->getTrigger()[ current_pixel  ]    )
                    sector_count[ i  ]++;

                if( sector_count[i] > fData->getRunParameter()->fPWlimit )
                {
                                                  // then turn the pixel OFF, regardless wether or not it was ON
                    fData->setTrigger( current_pixel, 0 );
                }
            }

        }

    }

}


void VImageCleaning::addImageChannel( unsigned int i_channel )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::addImageChannel" << endl;
    if( i_channel < fData->getImage().size() ) fData->setImageUser( i_channel, 1 );
}


void VImageCleaning::removeImageChannel( unsigned int i_channel )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::removeImageChannel" << endl;
    if( i_channel < fData->getImage().size() ) fData->setImageUser( i_channel, -1 );
}


void VImageCleaning::resetImageChannel( unsigned int i_channel )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::resetImageChannel" << endl;
    if( i_channel < fData->getImage().size() ) fData->setImageUser( i_channel, 0 );
}


/* 
   trace correlation cleaning

   AMc
*/
double getTraceCorrelationValue(double Amean, double Bmean,
				double Avar, double Bvar,
				vector < double > vA, vector < double > vB)
{
  if( Avar == 0. || Bvar == 0. ) return 0.;
  double N = 0;

  for( unsigned int i = 0; i < vA.size() && i < vB.size() ; i++ )
  {
     N = N + (vA[i] - Amean)*(vB[i] - Bmean);
  }
  
  return N/TMath::Sqrt(Avar*Bvar);
}

double getTraceMean(vector < double > vA)
{
  if( vA.size() == 0 ) return 0.;

  double N = 0;
  for( unsigned int i = 0; i < vA.size(); i++ ) N = N + vA[i];

  return N/double(vA.size());
}

double getTraceVar(vector < double > vA, double Am)
{
  double N = 0;
  for(unsigned int i = 0; i < vA.size(); i++) N = N + (vA[i] - Am)*(vA[i] - Am);
  
  return N;
}
  
/*
   trace correlation cleaning

   AMc
*/
void VImageCleaning::cleanImageTraceCorrelate( double sigNoiseThresh, double corrThresh, double pixThresh )
{
  fData->setBorderCorrelationCoefficient( 0. );

  vector < vector < double > > vImageTraces( fData->getDetectorGeo()->getNChannels(fData->getTelID()),vector<double>((int)fData->getNSamples(),0) );
 
  unsigned int nhits = fData->getReader()->getNumChannelsHit();
  for(unsigned int i = 0; i < nhits; i++ )
  {
      unsigned int i_channelHitID = 0;
      try
      {
	  i_channelHitID = fData->getReader()->getHitID(i);
	  if( i_channelHitID < fData->getHiLo().size() && !fData->getDead(fData->getHiLo()[i_channelHitID])[i_channelHitID] )
          {

	      fData->getTraceHandler()->setTrace( fData->getReader(), fData->getNSamples(),fData->getPeds(fData->getHiLo()[i_channelHitID])[i_channelHitID], 
				                  fData->getPedrms(fData->getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID, i,
				                  fData->getLowGainMultiplier( true )[i_channelHitID]*fData->getHiLo()[i_channelHitID] );

	      vImageTraces[i_channelHitID] = fData->getTraceHandler()->getTrace();
	  }
      }
      catch(...)
      {
	  if( fData->getDebugLevel() == 0 )
          {
                cout << "VImageCleaning::cleanImageTraceCorrelate, index out of range (fReader->getHitID) ";
		cout << i << "(Telescope " << fData->getTelID()+1 << ", event " << fData->getReader()->getEventNumber() << ")" << endl;
                fData->setDebugLevel( 0 );
          }
          continue;
      }

  }

  if( vImageTraces.size() > 0 )
  {
      vector < double > avepulse(fData->getNSamples(),0);
      vector < unsigned int > ImagePixelList;
      vector < unsigned int > NearbyPixelList;
      double AvePulseMean = 0;
      double AvePulseVar = 0;
      int nimage = 0;
      unsigned int k = 0;
      
      for(unsigned int i = 0; i < vImageTraces.size(); i++)
      {
	  if( fData->getImage()[i] || fData->getBorder()[i])
	  {
	      nimage++;
	      ImagePixelList.push_back(i);
	      for(unsigned int j = 0; j < fData->getNSamples(); j++)
	      {
		avepulse[j] = avepulse[j] + vImageTraces[i][j];
              }
	  }
      }
	 
      if( nimage > 1 )
      {
	  for(unsigned int j = 0; j < fData->getNSamples(); j++)
	  {
	      avepulse[j] = avepulse[j]/double(nimage);
	      AvePulseMean = AvePulseMean + avepulse[j];
	  }
	  AvePulseMean = AvePulseMean/double(fData->getNSamples());
	  
	  for(unsigned int j = 0; j < fData->getNSamples(); j++)
	  {
	    AvePulseVar = AvePulseVar + (avepulse[j] - AvePulseMean)*(avepulse[j] - AvePulseMean);
          }
      }

      if(nimage > 3 && nimage < pixThresh)
      {
//Build a list of pixels near the image pixels
//Loop over image/border pixels
	  for( unsigned int o = 0; o < ImagePixelList.size(); o++ )
	  {
	      unsigned int i = ImagePixelList[o];
//Get the neighbours of this image pixel
	      for(unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
	      {
//Check if it is already included in the neighbour list
		  bool have = false;
		  k = fData->getDetectorGeo()->getNeighbours()[i][j];
		  for(unsigned int p = 0; p < NearbyPixelList.size(); p++)
		  {
		      if(NearbyPixelList[p] == k)
		      {
			  have = true;
			  break;
		      }
		  }
//Check if it is already included in the image pixel list
		  for( unsigned int p = 0; p < ImagePixelList.size(); p++ )
		  {
		     if(ImagePixelList[p] == k)
		     {
			  have = true;
			  break;
		     }
		  }
//Add to neighbour list if necessary
		  if( !have ) NearbyPixelList.push_back( k );
		}
	    }
	
	  
	  for(unsigned int i = 0; i < NearbyPixelList.size(); i++)
	  {
	      k =  NearbyPixelList[i];

	      if( fData->getPedvars(fData->getCurrentSumWindow()[k], fData->getHiLo()[k])[k] > 0.) 
	      {
		 double tMean = getTraceMean(vImageTraces[k]);
		 double tVar = getTraceVar(vImageTraces[k],tMean);


		 double corv = getTraceCorrelationValue(AvePulseMean,tMean,AvePulseVar,tVar,avepulse,vImageTraces[k]);
		 double sn = fData->getSums()[k]/fData->getPedvars(fData->getCurrentSumWindow()[k], fData->getHiLo()[k])[k];

// require that correlation coefficient and signal/noise is above certain thresholds
		 if( corv > corrThresh && sn > sigNoiseThresh )
		 {
		     fData->setBorder(k, true );
		     fData->setBorderCorrelationCoefficient( k, corv );
		 }
	    
	    }
         }
      }
      recoverImagePixelNearDeadPixel();
      fillImageBorderNeighbours();
    }
}
