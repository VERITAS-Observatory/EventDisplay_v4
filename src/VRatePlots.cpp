/*! \class VRatePlots
 *  \brief fill some graphs with rates, significances, etc.
 *
 *  \author
 *    Gernot Maier
 *
 *   Revision $Id: VRatePlots.cpp,v 1.6.2.1.26.1.10.1 2010/03/08 07:45:09 gmaier Exp $
 */

#include "VRatePlots.h"

VRatePlots::VRatePlots()
{
    hList = new TList();
    hListRun = 0;
    hListTime = new TList();

}


VRatePlots::VRatePlots( VAnaSumRunParameter *iPar, map< int, double > itime )
{
    fTime = itime;

    vector< int > iruns;
    for( unsigned int i = 0; i < iPar->fRunList.size(); i++ ) iruns.push_back( iPar->fRunList[i].fRunOn );
    fRuns = iruns;

// define graphs
    hList = new TList();
    hListRun = new TList();
    hListTime = new TList();

    defineRunGraphs( (int)iruns.size() );
    defineTimeGraphs( (int)iruns.size() );
}


void VRatePlots::defineRunGraphs( int gsize )
{
    hSig_run = new TGraph( gsize );
    hSig_run->SetName( "gSigRun" );
    hSig_run->SetTitle( "significance vs. run number" );
    hList->Add( hSig_run );
    hListRun->Add( hSig_run );

    hSigMax_run = new TGraph( gsize );
    hSigMax_run->SetName( "gSigMaxRun" );
    hSigMax_run->SetTitle( "maximum significance (from 2D plot) vs. run number" );
    hList->Add( hSigMax_run );
    hListRun->Add( hSigMax_run );

    hOn_run = new TGraphErrors( gsize );
    hOn_run->SetName( "gOnRun" );
    hOn_run->SetTitle( "on events vs. run number" );
    hList->Add( hOn_run );
    hListRun->Add( hOn_run );

    hOff_run = new TGraphErrors( gsize );
    hOff_run->SetName( "gOffRun" );
    hOff_run->SetTitle( "off events vs. run number" );
    hList->Add( hOff_run );
    hListRun->Add( hOff_run );

    hRate_run = new TGraphErrors( gsize );
    hRate_run->SetName( "gRateRun" );
    hRate_run->SetTitle( "rate vs. run number" );
    hList->Add( hRate_run );
    hListRun->Add( hRate_run );

    TIter next( hListRun );
    while( TGraph *obj = (TGraph*)next() )
    {
        obj->SetMarkerStyle( 20 );
        obj->SetLineWidth( 2 );
        obj->SetMarkerSize( 2 );
    }
}


void VRatePlots::defineTimeGraphs( int gsize )
{
    hSig_time = new TGraph( gsize );
    hSig_time->SetName( "gSigTime" );
    hSig_time->SetTitle( "significance vs. time" );
    hList->Add( hSig_time );
    hListTime->Add( hSig_time );

    hSigMax_time = new TGraph( gsize );
    hSigMax_time->SetName( "gSigMaxTime" );
    hSigMax_time->SetTitle( "maximum significance vs. time" );
    hList->Add( hSigMax_time );
    hListTime->Add( hSigMax_time );

    hOn_time = new TGraphErrors( gsize );
    hOn_time->SetName( "gOnTime" );
    hOn_time->SetTitle( "on events vs. time" );
    hList->Add( hOn_time );
    hListTime->Add( hOn_time );

    hOff_time = new TGraphErrors( gsize );
    hOff_time->SetName( "gOffTime" );
    hOff_time->SetTitle( "off events vs. time" );
    hList->Add( hOff_time );
    hListTime->Add( hOff_time );

    hRate_time = new TGraphErrors( gsize );
    hRate_time->SetName( "gRateTime" );
    hRate_time->SetTitle( "rate vs. time" );
    hList->Add( hRate_time );
    hListTime->Add( hRate_time );

    TIter next( hListTime );
    while( TGraph *obj = (TGraph*)next() )
    {
        obj->SetMarkerStyle( 20 );
        obj->SetLineWidth( 2 );
        obj->SetMarkerSize( 2 );
    }
}


// Updated mean run time
void VRatePlots::fill( int irun, double iMJD, double isig, double isigmax, double ion, double ioff, double irate )
{
    fTime[irun] = iMJD;
    fill( irun, isig, isigmax, ion, ioff, irate );
}


void VRatePlots::fill( int irun, double isig, double isigmax, double ion, double ioff, double irate )
{
    int i_point = 0;
    for( i_point = 0; i_point < (int)fRuns.size(); i_point++ ) if( irun == fRuns[i_point] ) break;

    if( fTime.find( irun ) != fTime.end() )
    {
        hSig_run->SetPoint( i_point, fRuns[i_point], isig );
        hSig_time->SetPoint( i_point, fTime[irun], isig );

        hSigMax_run->SetPoint( i_point, fRuns[i_point], isigmax );
        hSigMax_time->SetPoint( i_point, fTime[irun], isigmax );

        hOn_run->SetPoint( i_point, fRuns[i_point], ion );
        hOn_run->SetPointError( i_point, 0., sqrt( ion ) );
        hOn_time->SetPoint( i_point, fTime[irun], ion );
        hOn_time->SetPointError( i_point, 0., sqrt( ion ) );

        hOff_run->SetPoint( i_point, fRuns[i_point], ioff );
        hOff_run->SetPointError( i_point, 0., sqrt( ioff ) );
        hOff_time->SetPoint( i_point, fTime[irun], ioff );
        hOff_time->SetPointError( i_point, 0., sqrt( ioff ) );

        hRate_run->SetPoint( i_point, fRuns[i_point], irate );
        if( isig > 0 ) hRate_run->SetPointError( i_point, 0., irate/isig ) ;
        else           hRate_run->SetPointError( i_point, 0., 0. );
        hRate_time->SetPoint( i_point, fTime[irun], irate );
        if( isig > 0 ) hRate_time->SetPointError( i_point, 0., irate/isig );
        else           hRate_time->SetPointError( i_point, 0., 0. );
    }

}


void VRatePlots::fill( vector< double > itime, vector< double > ion, vector< double > ioff, vector< double > isig, vector< double > irate )
{
// all vector must have the same length
    if( itime.size() != ion.size() || itime.size() != ioff.size() || itime.size() != isig.size() || itime.size() != irate.size() )
    {
        cout << "VRatePlots::fill error: data vectors with different size" << endl;
        cout << "\t " << itime.size() << "\t" << ion.size() << "\t" << ioff.size() << "\t" << isig.size() << "\t" << irate.size() << endl;
        return;
    }
// count number of valid points
    int inp = 0;
    for( unsigned int i = 0; i < itime.size(); i++ )
    {
        if( ion[i] > 0 && ioff[i] > 0 && isig[i] > 0 && irate[i] > 0 ) inp++;
    }

    defineTimeGraphs( inp );

    int z = 0;
    for( unsigned int i = 0; i < itime.size(); i++ )
    {
        if( ion[i] > 0 && ioff[i] > 0 && isig[i] > 0 && irate[i] > 0 )
        {
            hSig_time->SetPoint( z, itime[i], isig[i] );

            hOn_time->SetPoint( z, itime[i], ion[i] );
            hOn_time->SetPointError(  z, 0., sqrt( ion[i] ) );

            hOff_time->SetPoint( z, itime[i], ioff[i] );
            hOff_time->SetPointError(  z, 0., sqrt( ioff[i] ) );

            hRate_time->SetPoint( z, itime[i], irate[i] );
            if( isig[i] > 0. ) hRate_time->SetPointError( z, 0., irate[i]/isig[i] );
            else               hRate_time->SetPointError( z, 0., 0. );
            z++;
        }
    }
}


void VRatePlots::write()
{
    write( "ratePlots" );
}


void VRatePlots::write( string iname )
{
    TDirectory *iDir = gDirectory;
    gDirectory->mkdir( iname.c_str() )->cd();
    if( hList ) hList->Write();
    iDir->cd();
}
