/*!  VCTARequirements

    CTA requirements (hardwired values)

*/

#include "VCTARequirements.h"

VCTARequirements::VCTARequirements()
{
   fSetOfRequirementID = 0;

   setRequirement();
}

bool VCTARequirements::setRequirement( int iRequirementID )
{
   fSetOfRequirementID = iRequirementID;

////////////////////////////////////////////////////////////
// requirements 
// from MAN-PO/121004, version 2.5, June 14, 2013

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

////////////////////////////////////////////////////////////
// sensitivity 

// SOUTH 50 h
    if( fSetOfRequirementID == 0 )
    {
// from JH (mail 2013/08/30)
       fReqDifferentialSensitivity = new TGraph( 1 );
       setGraphPlottingStyle( fReqDifferentialSensitivity, 2, 1., 20, 1., 0, 2 );
       fGoalDifferentialSensitivity = new TGraph( 1 );
       setGraphPlottingStyle( fGoalDifferentialSensitivity, 3, 1., 20, 1., 0, 2 );
       for( int i = 0; i < 80; i++ )
       {
          fReqDifferentialSensitivity->SetPoint( i, -1.9 + 0.05 * i, VCTASensitivityRequirements::Flux_req50_E2erg_south( 
	                                                            TMath::Power( 10., -1.9 + 0.05 * i ) ) );
          fGoalDifferentialSensitivity->SetPoint( i, -1.9 + 0.05 * i, VCTASensitivityRequirements::Flux_goal50_E2erg_south( 
	                                                            TMath::Power( 10., -1.9 + 0.05 * i ) ) );
       }
   }
// SOUTH 5 h
   else if( fSetOfRequirementID == 1 )
   {
       fReqDifferentialSensitivity = new TGraph( 1 );
       setGraphPlottingStyle( fReqDifferentialSensitivity, 2, 1., 20, 1., 0, 2 );
       fGoalDifferentialSensitivity = 0;
       for( int i = 0; i < 80; i++ )
       {
          fReqDifferentialSensitivity->SetPoint( i, -1.9 + 0.05 * i, VCTASensitivityRequirements::Flux_req5_E2erg_south( 
	                                                            TMath::Power( 10., -1.9 + 0.05 * i ) ) );
       }
   }
// NORTH 50 h
   else if( fSetOfRequirementID == 2 )
   {
// from JH (mail 2013/08/30)
       fReqDifferentialSensitivity = new TGraph( 1 );
       setGraphPlottingStyle( fReqDifferentialSensitivity, 2, 1., 20, 1., 0, 2 );
       fGoalDifferentialSensitivity = new TGraph( 1 );
       setGraphPlottingStyle( fGoalDifferentialSensitivity, 3, 1., 20, 1., 0, 2 );
       for( int i = 0; i < 80; i++ )
       {
          fReqDifferentialSensitivity->SetPoint( i, -1.9 + 0.05 * i, VCTASensitivityRequirements::Flux_req50_E2erg_north( 
	                                                            TMath::Power( 10., -1.9 + 0.05 * i ) ) );
          fGoalDifferentialSensitivity->SetPoint( i, -1.9 + 0.05 * i, VCTASensitivityRequirements::Flux_goal50_E2erg_north( 
	                                                            TMath::Power( 10., -1.9 + 0.05 * i ) ) );
       }
   }
// NORTH 5 h
   else if( fSetOfRequirementID == 3 )
   {
       fReqDifferentialSensitivity = new TGraph( 1 );
       setGraphPlottingStyle( fReqDifferentialSensitivity, 2, 1., 20, 1., 0, 2 );
       fGoalDifferentialSensitivity = 0;
       for( int i = 0; i < 80; i++ )
       {
          fReqDifferentialSensitivity->SetPoint( i, -1.9 + 0.05 * i, VCTASensitivityRequirements::Flux_req5_E2erg_north( 
	                                                            TMath::Power( 10., -1.9 + 0.05 * i ) ) );
       }
   }
   else
   {
       cout << "VCTARequirements::setRequirement error: invalid requirement set ID: allowed is 0" << endl;
       return false;
   }

////////////////////////////////////////////////////////
// effective areas

// SOUTH
   if( fSetOfRequirementID == 0 || fSetOfRequirementID == 1 )
   {
       fReqEnergyRange_min = 0.02;
       fReqEnergyRange_max = 300.;

       fReqEffectiveArea = 0;
       fGoalEffectiveArea = new TGraph( 5 );
       fGoalEffectiveArea->SetPoint( 0, log10(  0.03 ), 1.0e4 );
       fGoalEffectiveArea->SetPoint( 1, log10(  0.10 ), 6.0e4 );
       fGoalEffectiveArea->SetPoint( 2, log10(  1.00 ), 1.0e6 );
       fGoalEffectiveArea->SetPoint( 3, log10( 10.00 ), 4.0e6 );
       fGoalEffectiveArea->SetPoint( 4, log10( 100.00 ), 7.0e6 );
       setGraphPlottingStyle( fGoalEffectiveArea, 3 );
   }
// NORTH
   else if( fSetOfRequirementID == 2 || fSetOfRequirementID == 3 )
   {
       fReqEnergyRange_min = 0.02;
       fReqEnergyRange_max = 20.;

       fReqEffectiveArea = 0;
       fGoalEffectiveArea = new TGraph( 4 );
       fGoalEffectiveArea->SetPoint( 0, log10(  0.03 ), 1.0e4 );
       fGoalEffectiveArea->SetPoint( 1, log10(  0.10 ), 6.0e4 );
       fGoalEffectiveArea->SetPoint( 2, log10(  1.00 ), 0.5e6 );
       fGoalEffectiveArea->SetPoint( 3, log10( 10.00 ), 1.0e6 );
       setGraphPlottingStyle( fGoalEffectiveArea, 3 );
   }
   return true;
}

void VCTARequirements::printRequirements( int iRequirementID )
{

}

void VCTARequirements::plotRequirement_EnergyRange( TCanvas *c, bool iPlotGoalRequirements )
{

}

TGraph* VCTARequirements::plotRequirement_DifferentialSensitivity( TCanvas *c, bool iPlotGoalRequirements, string iUnit )
{
    if( !c ) return 0;

    if( !fReqDifferentialSensitivity ) 
    {
       cout << "VCTARequirements::plotRequirement_DifferentialSensitivity: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqDifferentialSensitivity, true, true );
    if( iPlotGoalRequirements ) plotRequirements( fGoalDifferentialSensitivity, true, true );

    return fReqDifferentialSensitivity;
}

TGraph* VCTARequirements::plotRequirement_EnergyResolution( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqEnergyResolution ) 
    {
       cout << "VCTARequirements::plotRequirement_EnergyResolution: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqEnergyResolution );
    if( iPlotGoalRequirements ) plotRequirements( fGoalEnergyResolution );

    return fReqEnergyResolution;
}

TGraph* VCTARequirements::plotRequirement_AngularResolution( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqAngularResolution ) 
    {
       cout << "VCTARequirements::plotRequirement_AngularResolution: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqAngularResolution );
    if( iPlotGoalRequirements ) plotRequirements( fGoalAngularResolution );

    return fReqAngularResolution;
}

TGraph* VCTARequirements::plotRequirement_EffectiveArea( TCanvas *c, bool iPlotGoalRequirements )
{
    if( !c ) return 0;

    if( !fReqEffectiveArea && !fGoalEffectiveArea ) 
    {
       cout << "VCTARequirements::plotRequirement_EffectiveArea: error, no requirement given" << endl;
       return 0;
    }

    c->cd();
    plotRequirements( fReqEffectiveArea, true );
    if( iPlotGoalRequirements ) plotRequirements( fGoalEffectiveArea, true );

    return fReqAngularResolution;
}

void VCTARequirements::plotRequirements( TGraph *g, bool iLog, bool iLine )
{
   if( !g ) return;

   if( iLine )
   {
      g->Draw( "l" );
      return;
   }

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

