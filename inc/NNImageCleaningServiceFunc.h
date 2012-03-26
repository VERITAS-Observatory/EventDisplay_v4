#ifndef VIMAGENNCLEANING_H
#define VIMAGENNCLEANING_H

#include "GeometryCam.h"
#include "TGraph.h"
#include <TString.h>
TGraph* gIPR;
TGraph* ReadIPRGraph(TString fname, TString graphname)
{
    TFile* fptr=new TFile(fname,"READ");
    if(!fptr){cout<<"[ReadIPRGraph]:: Can't find file"<<endl;  if(getchar()=='q') return NULL;}
    TGraph* g=(TGraph*)fptr->Get(graphname.Data());
    fptr->Close();
    fptr->Delete();
    return g;
}
Double_t EquiRate(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    Double_t f = 1E9*TMath::Exp( 1./(float)(par[1]-1) * TMath::Log( par[0]/(par[2]*pow(gIPR->Eval(xx,spl,""),par[1])) ) );
    return f;
}

Double_t EquiRateBound(Double_t *x, Double_t *par) // returns coinctime in ns !!!
{
    TSpline *spl=NULL;
    Float_t xx =x[0];
    Double_t f = 1E9*TMath::Exp( 1./(float)(2.-1.) * TMath::Log( par[0]/(par[2]*gIPR->Eval(xx,spl,"")*gIPR->Eval(par[1],spl,"")) ) );
    return f;
}
TF1* RateContour(TString funcname, float fRate, float fNfold, float fCombFactor, float xlow, float xup)
{
    TF1* f1= new TF1(funcname,EquiRate,xlow,xup,3);
    f1->SetParameters(fRate,fNfold, fCombFactor);
    f1->SetParNames("Rate","Nfold","CombFactor");
    return f1;
}
TF1* RateContourBound(TString funcname, float fRate, float refThresh, float fCombFactor, float xlow, float xup)
{
    TF1* f1= new TF1(funcname,EquiRateBound,xlow,xup,3);
    f1->SetParameters(fRate,refThresh, fCombFactor);
    f1->SetParNames("Rate","RefThresh","CombFactor");
    return f1;
}


#endif
