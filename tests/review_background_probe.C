#include "TH1D.h"
#include <cassert>
#include <cmath>
void review_background_probe() {
 TH1D a("a","",2,0,2), b("b","",2,0,2), out("out","",2,0,2);
 a.SetBinContent(1,100);a.SetBinContent(2,10);
 b.SetBinContent(1,10);b.SetBinContent(2,100);
 out.Sumw2();out.Add(&a,.1);out.Add(&b,.2);
 assert(fabs(out.GetBinContent(1)-12)<1e-9);
 assert(fabs(out.GetBinContent(2)-21)<1e-9);
 assert(fabs(pow(out.GetBinError(1),2)-1.4)<1e-9);
 assert(fabs(pow(out.GetBinError(2),2)-4.1)<1e-9);
}
