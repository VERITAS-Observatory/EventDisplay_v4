/*!  VPlotCTARequirements

    plot CTA sensitivity requirements

*/

#include "VPlotCTARequirements.h"

VPlotCTARequirements::VPlotCTARequirements()
{
   fSetOfRequirementID = 0;

   setRequirement();
}

bool VPlotCTARequirements::setRequirement( int iRequirementID )
{
   fSetOfRequirementID = iRequirementID;

////////////////////////////////////////////////////////////
// requirements for South
//    from MAN-PO/121004, version 2.5, June 14, 2013
    if( fSetOfRequirementID == 0 )
    {

       fReqEnergyRange_min = 0.02;
       fReqEnergyRange_max = 20.;

// from Figure 3
       fReqDifferentialSensitivity50h = new TGraph( 4 );
       fReqDifferentialSensitivity50h->SetPoint( 0, log10(  0.03 ), 1.8e-11 );
       fReqDifferentialSensitivity50h->SetPoint( 1, log10(  0.10 ), 2.0e-12 );
       fReqDifferentialSensitivity50h->SetPoint( 2, log10(  1.00 ), 2.1e-13 );
       fReqDifferentialSensitivity50h->SetPoint( 3, log10( 10.00 ), 1.8e-13 );
       setGraphPlottingStyle( fReqDifferentialSensitivity50h, 2 );

       fGoalDifferentialSensitivity50h = new TGraph( 4 );
       fGoalDifferentialSensitivity50h->SetPoint( 0, log10(  0.03 ), 1.0e-11 );
       fGoalDifferentialSensitivity50h->SetPoint( 1, log10(  0.10 ), 0.8e-12 );
       fGoalDifferentialSensitivity50h->SetPoint( 2, log10(  1.00 ), 0.8e-13 );
       fGoalDifferentialSensitivity50h->SetPoint( 3, log10( 10.00 ), 0.9e-13 );
       setGraphPlottingStyle( fGoalDifferentialSensitivity50h, 3 );

       fReqEffectiveArea = 0;
       fGoalEffectiveArea = new TGraph( 5 );
       fGoalEffectiveArea->SetPoint( 0, log10(  0.03 ), 1.0e4 );
       fGoalEffectiveArea->SetPoint( 1, log10(  0.10 ), 6.0e4 );
       fGoalEffectiveArea->SetPoint( 2, log10(  1.00 ), 1.0e5 );
       fGoalEffectiveArea->SetPoint( 3, log10( 10.00 ), 4.0e5 );
       fGoalEffectiveArea->SetPoint( 4, log10( 100.00 ), 7.0e5 );
       setGraphPlottingStyle( fGoalEffectiveArea, 3 );

// from Figure 4
       fReqAngularResolution = new TGraph( 4 );
       fReqAngularResolution->SetPoint( 0, log10(  0.03 ), 0.28 );
       fReqAngularResolution->SetPoint( 1, log10(  0.10 ), 0.15 );
       fReqAngularResolution->SetPoint( 2, log10(  1.00 ), 0.06 );
       fReqAngularResolution->SetPoint( 3, log10( 10.00 ), 0.035 );
       setGraphPlottingStyle( fReqAngularResolution, 2 );

       fGoalAngularResolution = 0;

// from Figure 5
       fReqEnergyResolution = new TGraph( 4 );
       fReqEnergyResolution->SetPoint( 0, log10(  0.03 ), 0.40 );
       fReqEnergyResolution->SetPoint( 1, log10(  0.10 ), 0.20 );
       fReqEnergyResolution->SetPoint( 2, log10(  1.00 ), 0.10 );
       fReqEnergyResolution->SetPoint( 3, log10( 10.00 ), 0.10 );
       setGraphPlottingStyle( fReqEnergyResolution, 2 );

       fGoalEnergyResolution = 0;
   }
   else
   {
       cout << "VPlotCTARequirements::setRequirement error: invalid requirement set ID: allowed is 0" << endl;
       return false;
   }
   return true;
}

void VPlotCTARequirements::printRequirements( int iRequirementID )
{

}

void VPlotCTARequirements::plotRequirement_EnergyRange( TCanvas *c, bool iPlotGoalRequirements )
{

}

TGraph* VPlotCTARequirements::plotRequirement_DifferentialSensitivity50h( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqDifferentialSensitivity50h ) 
    {
       cout << "VPlotCTARequirements::plotRequirement_DifferentialSensitivity50h: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqDifferentialSensitivity50h, true );
    if( iPlotGoalRequirements ) plotRequirements( fGoalDifferentialSensitivity50h, true );

    return fReqDifferentialSensitivity50h;
}

TGraph* VPlotCTARequirements::plotRequirement_EnergyResolution( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqEnergyResolution ) 
    {
       cout << "VPlotCTARequirements::plotRequirement_EnergyResolution: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqEnergyResolution );
    if( iPlotGoalRequirements ) plotRequirements( fGoalEnergyResolution );

    return fReqEnergyResolution;
}

TGraph* VPlotCTARequirements::plotRequirement_AngularResolution( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqAngularResolution ) 
    {
       cout << "VPlotCTARequirements::plotRequirement_AngularResolution: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqAngularResolution );
    if( iPlotGoalRequirements ) plotRequirements( fGoalAngularResolution );

    return fReqAngularResolution;
}

TGraph* VPlotCTARequirements::plotRequirement_EffectiveArea( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqEffectiveArea && !fGoalEffectiveArea ) 
    {
       cout << "VPlotCTARequirements::plotRequirement_EffectiveArea: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqEffectiveArea, true );
    if( iPlotGoalRequirements ) plotRequirements( fGoalEffectiveArea, true );

    return fReqAngularResolution;
}

void VPlotCTARequirements::plotRequirements( TGraph *g, bool iLog )
{
   if( !g ) return;

   double x = 0;
   double y = 0;
   double y_low = 0.;

   for( int i = 0; i < g->GetN(); i++ )
   {
       g->GetPoint( i, x, y );

       if( iLog ) y_low = 0.5 * y;
       else       y_low = 0.8 * y;
       TArrow *a = new TArrow( x, y, x, y_low, 0.01, "|-|>" );
       setArrowPlottingStyle( a, g->GetMarkerColor(), 1, 2 );
       a->Draw();
   }
}    

