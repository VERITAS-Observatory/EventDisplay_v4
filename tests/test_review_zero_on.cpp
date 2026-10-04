#include "VOnOff.h"
#include <cassert>
#include <cmath>
int main() {
 TH2D on("map_on","",2,-1,1,2,-1,1), off("map_off","",2,-1,1,2,-1,1), alpha("alpha","",2,-1,1,2,-1,1);
 for(int i=1;i<=2;i++) for(int j=1;j<=2;j++) alpha.SetBinContent(i,j,.1);
 off.SetBinContent(1,1,10);alpha.SetBinContent(1,2,0);
 VOnOff analysis;
 TH2D* sig=analysis.do2DSignificance(&on,&off,&alpha);
 assert(fabs(sig->GetBinContent(1,1)-VStatistics::calcSignificance(0,10,.1))<1.e-8);
 assert(sig->GetBinContent(1,2)==-9999.);
 assert(sig->GetBinContent(2,2)==0.);
 analysis.fill1DSignificanceHistogram();
 TH1D* distribution=dynamic_cast<TH1D*>(gDirectory->Get("hsig1D"));
 assert(distribution && distribution->GetEntries()==3);
 assert(distribution->GetBinContent(0)==0);
 return 0;
}
