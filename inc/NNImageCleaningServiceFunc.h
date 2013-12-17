#ifndef VIMAGENNCLEANING_H
#define VIMAGENNCLEANING_H

#include "GeometryCam.h"
#include "TGraph.h"
#include <TString.h>
TGraph* gIPR;
TGraph* gIPR2;
TGraph* gIPR3;
TGraph* gIPR4;
TGraph* ReadIPRGraph(TString fname, TString graphname)
{
    TFile* fptr=new TFile(fname,"READ");
    if(!fptr){cout<<"[ReadIPRGraph]:: Can't find file"<<endl;  if(getchar()=='q') return NULL;}
    TGraph* g=(TGraph*)fptr->Get(graphname.Data());
    fptr->Close();
    fptr->Delete();
    return g;
}
float ReadConvFactorsHist(TString fname, TString hname, int bin)
{
    TFile* fptr=new TFile(fname,"READ");
    if(!fptr){cout<<"[ReadConvFactorsHist]:: Can't find file"<<endl;  if(getchar()=='q') return 0.;}
    TH1F* h=(TH1F*)fptr->Get(hname.Data());
    if(h==NULL){return -1.;}
    float conv=h->GetBinContent(bin);
    fptr->Close();
    fptr->Delete();
    return conv;
}
Double_t EquiRate(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;//Hz
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(par[1]-1) * TMath::Log( par[0]/(par[2]*pow(valIPR,par[1])) ) );
    return f;
}
Double_t EquiRateMST(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR2->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR2->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;//Hz
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(par[1]-1) * TMath::Log( par[0]/(par[2]*pow(valIPR,par[1])) ) );
    return f;
}
Double_t EquiRateSCSST(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR3->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR3->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;//Hz
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(par[1]-1) * TMath::Log( par[0]/(par[2]*pow(valIPR,par[1])) ) );
    return f;
}

Double_t EquiRateSST(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR4->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR4->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;//Hz
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(par[1]-1) * TMath::Log( par[0]/(par[2]*pow(valIPR,par[1])) ) );
    return f;
}

Double_t EquiRateBound(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;
    Double_t valIPRref=gIPR->Eval(par[1],spl,""); if(valIPRref<100 || par[1]>=ChargeMax) valIPRref=100;
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(2.-1.) * TMath::Log( par[0]/(par[2]*valIPR*valIPRref) ) );
    return f;
}
Double_t EquiRateBoundMST(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR2->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR2->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;
    Double_t valIPRref=gIPR2->Eval(par[1],spl,""); if(valIPRref<100 || par[1]>=ChargeMax) valIPRref=100;

    Double_t f = 1.0E9*TMath::Exp( 1./(float)(2.-1.) * TMath::Log( par[0]/(par[2]*valIPR*valIPRref) ) );
    return f;
}
Double_t EquiRateBoundSCSST(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR3->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR3->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;
    Double_t valIPRref=gIPR3->Eval(par[1],spl,""); if(valIPRref<100 || par[1]>=ChargeMax) valIPRref=100;
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(2.-1.) * TMath::Log( par[0]/(par[2]*valIPR*valIPRref) ) );
    return f;
}
Double_t EquiRateBoundSST(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    float ChargeMax=gIPR4->GetXaxis()->GetXmax();
    Double_t valIPR=gIPR4->Eval(xx,spl,""); if(valIPR<100 || xx>=ChargeMax) valIPR=100;
    Double_t valIPRref=gIPR4->Eval(par[1],spl,""); if(valIPRref<100 || par[1]>=ChargeMax) valIPRref=100;
    Double_t f = 1.0E9*TMath::Exp( 1./(float)(2.-1.) * TMath::Log( par[0]/(par[2]*valIPR*valIPRref) ) );
    return f;
}
TF1* RateContour(int teltype, TString funcname, float fRate, float fNfold, float fCombFactor, float xlow, float xup)
{
    TF1* f1=NULL;
    if(teltype<1 || teltype>4) {f1=new TF1(funcname,EquiRate,xlow,xup,3);}//LST setting for all types
    if(teltype==1)f1= new TF1(funcname,EquiRate,xlow,xup,3);
    if(teltype==2)f1= new TF1(funcname,EquiRateMST,xlow,xup,3);
    if(teltype==3)f1= new TF1(funcname,EquiRateSCSST,xlow,xup,3);
    if(teltype==4)f1= new TF1(funcname,EquiRateSST,xlow,xup,3);
    f1->SetParameters(fRate,fNfold, fCombFactor);
    f1->SetParNames("Rate","Nfold","CombFactor");
    return f1;
}

TF1* RateContourBound(int teltype, TString funcname, float fRate, float refThresh, float fCombFactor, float xlow, float xup)
{
    TF1* f1=NULL;
    if(teltype<1 || teltype>4) {f1=new TF1(funcname,EquiRateBound,xlow,xup,3);}//LST setting for all types
    if(teltype==1)f1= new TF1(funcname,EquiRateBound,xlow,xup,3);
    if(teltype==2)f1= new TF1(funcname,EquiRateBoundMST,xlow,xup,3);
    if(teltype==3)f1= new TF1(funcname,EquiRateBoundSCSST,xlow,xup,3);
    if(teltype==4)f1= new TF1(funcname,EquiRateBoundSST,xlow,xup,3);
    f1->SetParameters(fRate,refThresh, fCombFactor);
    f1->SetParNames("Rate","RefThresh","CombFactor");
    return f1;
}

/*
TF1* RateContourBound(TString funcname, float fRate, float fCombFactor, float xlow, float xup)
{
    TF1* f1= new TF1(funcname,EquiRate,xlow,xup,3);
    f1->SetParameters(fRate,3, fCombFactor);
    f1->SetParNames("Rate","Nfold","CombFactor");
    return f1;
}
*/
// Sort function form ROOT::TMath
template <typename Element, typename Index> void SortArray(Index n, const Element* a, Index* index, Bool_t down)
//void Sort(Index n, const Element* a, Index* index, Bool_t down)
{
   // Sort the n elements of the  array a of generic templated type Element.
   // In output the array index of type Index contains the indices of the sorted array.
   // If down is false sort in increasing order (default is decreasing order).

   // NOTE that the array index must be created with a length >= n
   // before calling this function.
   // NOTE also that the size type for n must be the same type used for the index array
   // (templated type Index)

   for(Index i = 0; i < n; i++) { index[i] = i; }
   if ( down )
      std::sort(index, index + n, CompareDesc<const Element*>(a) );
   else
      std::sort(index, index + n, CompareAsc<const Element*>(a) );
}

#endif
