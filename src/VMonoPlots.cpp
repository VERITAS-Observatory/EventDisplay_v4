/*! \class VMonoPlots
    \brief
    \author
      Jamie Holder 
      Gernot Maier

      Revision $Id: VMonoPlots.cpp,v 1.7.2.1.36.1.4.1 2011/02/15 16:12:13 gmaier Exp $
*/

#include "VMonoPlots.h"

VMonoPlots::VMonoPlots( bool ison, CData *tree, string i_hsuffix, VAnaSumRunParameter *ipara, unsigned int itelid )
{
    fDebug = false;
    fIsOn = ison;

    fTelID = itelid;
    fRunPar = ipara;

    f_eta = 1.35;
    f_r0  = 0.25;

//   if (tree != 0)  cout << "Telescope " << fTelID+1 << ": Found parameter tree. Histogram suffix " << i_hsuffix << endl;
    if (tree == 0)
    {
        cout << "VMonoPlots::VMonoPlots error: tree not found " << fTelID << endl;
        exit( -1 );
    }
    fData = tree;

    hisList = new TList();
    hListParameterHistograms = new TList();
    hListSkyMaps = new TList();

    char i_key[100];

    sprintf(i_key,"halpha_%s", i_hsuffix.c_str());
    halpha = new TH1D(i_key,"Alpha histogram",36,0.,90);
    halpha->SetXTitle("Alpha");
    halpha->SetYTitle("No. of Events");
    hisList->Add( halpha );
    hListParameterHistograms->Add( halpha );

    sprintf(i_key,"hlength_%s", i_hsuffix.c_str());
    hlength = new TH1D(i_key,"Length histogram",50,0.,1.0);
    hlength->SetXTitle("Length");
    hlength->SetYTitle("No. of Events");
    hisList->Add( hlength );
    hListParameterHistograms->Add( hlength );

    sprintf(i_key,"hwidth_%s", i_hsuffix.c_str());
    hwidth = new TH1D(i_key,"Width histogram",50,0.,0.5);
    hwidth->SetXTitle("Width");
    hwidth->SetYTitle("No. of Events");
    hisList->Add( hwidth );
    hListParameterHistograms->Add( hwidth );

    sprintf(i_key,"hdist_%s", i_hsuffix.c_str());
    hdist = new TH1D(i_key,"Distance histogram",50,0.,2.0);
    hdist->SetXTitle("Distance");
    hdist->SetYTitle("No. of Events");
    hisList->Add( hdist );
    hListParameterHistograms->Add( hdist );

    sprintf(i_key,"hlos_%s", i_hsuffix.c_str());
    hlos = new TH1D(i_key,"Length/Size histogram",100,0.,0.002);
    hlos->SetXTitle("Length/Size");
    hlos->SetYTitle("No. of Events");
    hisList->Add( hlos );
    hListParameterHistograms->Add( hlos );

    sprintf(i_key,"hasymm_%s", i_hsuffix.c_str());
    hasymm = new TH1D(i_key,"Asymmetry histogram",50,-2.0,2.0);
    hasymm->SetXTitle("Asymmetry");
    hasymm->SetYTitle("No. of Events");
    hisList->Add( hasymm );
    hListParameterHistograms->Add( hasymm );

    sprintf(i_key,"htgrad_x_%s", i_hsuffix.c_str());
    htgrad_x = new TH1D(i_key,"Tgrad_x histogram",50,-10.0,10.0);
    htgrad_x->SetXTitle("Tgrad_x");
    htgrad_x->SetYTitle("No. of Events");
    hisList->Add( htgrad_x );
    hListParameterHistograms->Add( htgrad_x );

    sprintf(i_key,"htdist_%s", i_hsuffix.c_str());
    htdist = new TH1D(i_key,"Tdist histogram",50,0.0,10.0);
    htdist->SetXTitle("Tdist");
    htdist->SetYTitle("No. of Events");
    hisList->Add( htdist );
    hListParameterHistograms->Add( htdist );

    sprintf(i_key,"hlogsize_%s", i_hsuffix.c_str());
    hlogsize = new TH1D(i_key,"Histogram of Log_{10}(Size)",100,0,5);
    hlogsize->SetXTitle("Log_{10}(Size)");
    hlogsize->SetYTitle("No. of Events");
    hisList->Add( hlogsize );
    hListParameterHistograms->Add( hlogsize );

    sprintf(i_key,"hntubes_%s", i_hsuffix.c_str());
    hntubes = new TH1D(i_key,"Histogram of Number of Tubes in Image",499,0,499);
    hntubes->SetXTitle("No. of Tubes");
    hntubes->SetYTitle("No. of Events");
    hisList->Add( hntubes );
    hListParameterHistograms->Add( hntubes );

// 2D plots

//! setup the sky map histogram
    sprintf(i_key,"hmap_%s", i_hsuffix.c_str());
    hmap = new TH2D(i_key,"Sky Map",50,-2.0,2.0,50,-2.0,2.0);
    hmap->SetXTitle("X-Position on Sky (degrees)");
    hmap->SetYTitle("Y-position on Sky(degrees)");
    hisList->Add( hmap );
    hListSkyMaps->Add( hmap );

//! setup the centroid map histogram
    sprintf(i_key,"hcen_cam_%s", i_hsuffix.c_str());
    hcen_cam = new TH2D(i_key,"Centroid Map on Camera",50,-2.0,2.0,50,-2.0,2.0);
    hcen_cam->SetXTitle("X-Position on Camera (degrees)");
    hcen_cam->SetYTitle("Y-position on Camera(degrees)");
    hisList->Add( hcen_cam );
    hListSkyMaps->Add( hcen_cam );

//! setup the derotated centroid map histogram
    sprintf(i_key,"hcen_sky_%s",  i_hsuffix.c_str());
    hcen_sky = new TH2D(i_key,"Centroid Map on Sky",50,-2.0,2.0,50,-2.0,2.0);
    hcen_sky->SetXTitle("X-Position on Sky (degrees)");
    hcen_sky->SetYTitle("Y-position on Sky(degrees)");
    hisList->Add( hcen_sky );
    hListSkyMaps->Add( hcen_sky );

    TIter next( hisList );
    string it;
    char hname[200];
    while( TH1 *obj = (TH1*)next() )
    {
        obj->Sumw2();
// add telescope number to title
        sprintf( hname, "%s (Telescope %d)", obj->GetTitle(), fTelID+1 );
        obj->SetTitle( hname );
    }

////// PRELIMINARY ////////////
//
//  should be a vector of VAstroSources to work with different pair offsets
//
//      use here always detail from first entry in fRunPar->fRunList

//! define the source parameters
    VTargets fTarget;
    double i_ra=0;
    double i_dec=0;

    string iTarget = fRunPar->fRunList[0].fTarget;
    double i_off = 0.;
    if( !fIsOn ) i_off = fRunPar->fRunList[0].fPairOffset/60./24.*360.*M_PI/180.0;

    {
        if( fTarget.selectTargetbyName( iTarget ) )
        {
            i_dec = fTarget.getTargetDecJ2000();
            i_ra  = fTarget.getTargetRAJ2000();
        }
        else
        {
            cout << "ERROR: invalid target " << iTarget << endl;
            exit( 0 );
        }
        slaPreces("FK5",2000.0, 2006.0, &i_ra, &i_dec);

	fAstro = new VSkyCoordinates();
	fAstro->setTelDec_deg( i_dec );
	fAstro->setTelRA_deg( i_ra + i_off );
    }

//! define the cuts
    fCuts = new VGammaHadronCuts();
    fCuts->resetCutValues();
}


/*!
 * fill all histograms
 *
 * return number of events passing cuts
 */
double VMonoPlots::fillHistograms( unsigned int itelid, int irun )
{
    if( fDebug ) cout << "VMonoPlots::fillHistograms telescope " << itelid << endl;
    if( fData->fChain == 0 ) return -1.;

    fTelID = itelid;

    TIter next( hisList );
    while( TH1 *obj = (TH1*)next() )
    {
        obj->Reset();
    }

    Int_t nentries = Int_t(fData->fChain->GetEntries());
    cout << endl;
    cout << "\t Telescope " << fTelID+1 << endl;
    if( fIsOn ) cout << "\t on run,  total number of entries in data chain " << nentries << "\t" << irun << endl;
    else        cout << "\t off run, total number of entries in data chain " << nentries << "\t" << irun << endl;

    int iInitRun = 0;

    double i_count = 0;
    for( int n = 0; n < nentries; n++ )
    {
        fData->fChain->GetEntry( n );

        if( irun < 0 || fData->runNumber == irun )
        {
            if( fData->runNumber != iInitRun )
            {
                initializeRun( fData->runNumber );
                iInitRun = fData->runNumber;
            }

            if(cutSize() && cutLength() && cutWidth() && cutDistance() && cutLoS() && cutAsymm() )
                halpha->Fill(fData->alpha[fTelID]);
            if(cutSize() && cutAlpha() && cutWidth() && cutDistance() && cutLoS() && cutAsymm() )
                hlength->Fill(fData->length[fTelID]);
            if(cutSize() && cutLength() && cutAlpha() && cutDistance() && cutLoS() && cutAsymm() )
                hwidth->Fill(fData->width[fTelID]);
            if(cutSize() && cutLength() && cutWidth() && cutAlpha() && cutLoS() && cutAsymm() )
                hdist->Fill(fData->dist[fTelID]);
            if(cutSize() && cutLength() && cutWidth() && cutDistance() && cutAlpha() && cutAsymm() )
                hlos->Fill(fData->los[fTelID]);
            if(cutSize() && cutLength() && cutWidth() && cutDistance() && cutLoS() && cutAlpha() )
                hasymm->Fill(fData->asym[fTelID]);
            if(cutLength() && cutWidth() && cutDistance() && cutLoS() && cutAsymm() && cutAlpha() && fData->size[fTelID]>0. )
                hlogsize->Fill(log10(fData->size[fTelID]));

            if(cutSize() && cutLength() && cutWidth() &&  cutLoS() && cutAsymm() )
            {
                makeCentroidsMap();
                makeTwoD_BoxSmooth();
            }

            if(cutSize() && cutLength() && cutWidth() && cutDistance() && cutLoS() && cutAsymm() && cutAlpha() )
            {
                hntubes->Fill(fData->ntubes[fTelID]);

                htgrad_x->Fill(fData->tgrad_x[fTelID]);

                double tdist = fabs(fData->tgrad_x[fTelID]-(3.99*fData->dist[fTelID]-3.39));
                htdist->Fill(tdist);

                i_count++;
            }
        }
    }
    if( fDebug ) cout << "VMonoPlots::fillHistograms number of events passing cuts: " << i_count << endl;
    return i_count;
}


void VMonoPlots::writeHistograms()
{
    hisList->Write();
}


void VMonoPlots::calcSourceLocation(double i_eta, double & i_xorig, double & i_yorig)
{
//! Calculate the source location using the method of Lessard et al. (Astropart. Phys, 15, 1, 2001).
//! Asymmetry is taken into account (I think -need to check this)
    double i_disp=0;
    if ( fData->length[fTelID] > 0. ) i_disp = i_eta*(1.-fData->width[fTelID]/fData->length[fTelID]);
//! first source origin
    double i_delta_x=i_disp*fData->cosphi[fTelID];
    double i_delta_y=i_disp*fData->sinphi[fTelID];
    i_xorig=fData->cen_x[fTelID]-i_delta_x;
    i_yorig=fData->cen_y[fTelID]-i_delta_y;
}


void VMonoPlots::makeCentroidsMap()
{
    hcen_cam->Fill( fData->cen_x[fTelID], fData->cen_y[fTelID] );

    double i_UTC=VSkyCoordinatesUtilities::getUTC(fData->MJD,fData->Time);
    double i_xderot=0.;
    double i_yderot=0.;
    fAstro->derotateCoords(i_UTC,fData->cen_x[fTelID],fData->cen_y[fTelID],i_xderot,i_yderot);
    hcen_sky->Fill(i_xderot,i_yderot);
}


void VMonoPlots::makeTwoD_BoxSmooth()
{
//! Constructs a 2D skymap of reconstructed source location on the camera plane
//! smoothed by a box (i.e accept all events within a given radius)
//! f_r0 is the radius of the smoothing box

    double i_xo,i_yo;
    calcSourceLocation(f_eta, i_xo, i_yo);

//! Derotate the source Location
    double i_UTC=VSkyCoordinatesUtilities::getUTC(fData->MJD,fData->Time);
    double i_xderot=0.;
    double i_yderot=0.;
    fAstro->derotateCoords(i_UTC,i_xo,i_yo,i_xderot,i_yderot);

//! loop over all bins and fill those within box radius
    for(Int_t i=0; i<hmap->GetNbinsX();i++)
    {
        for(Int_t j=0; j<hmap->GetNbinsY();j++)
        {
            double i_xbin=hmap->GetXaxis()->GetBinCenter(i+1);
            double i_ybin=hmap->GetYaxis()->GetBinCenter(j+1);
            double i_r=sqrt((i_xderot-i_xbin)*(i_xderot-i_xbin)
                +(i_yderot-i_ybin)*(i_yderot-i_ybin));
            if (i_r<=f_r0)
            {
                hmap->Fill(i_xbin,i_ybin,1.0);
            }
        }
    }
    return;
}


void VMonoPlots::initializeRun( int irun )
{
    for( unsigned int i = 0; i < fRunPar->fRunList.size(); i++ )
    {
        if( irun == fRunPar->fRunList[i].fRunOn || irun == fRunPar->fRunList[i].fRunOff )
        {
            if( fRunPar->fRunList[i].fCutFile != "" )
            {
                fCuts->readCuts( fRunPar->fRunList[i].fCutFile );
            }
            else
            {
                fCuts->resetCutValues();
            }
            break;
        }
    }
}
