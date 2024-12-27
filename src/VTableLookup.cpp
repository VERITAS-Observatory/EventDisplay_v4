/*! \class VTableLookup
    \brief calculation of mean scaled variables and energies using MC filled tables

*/

#include "VTableLookup.h"

/*!
    \param ireadwrite W for table writing, R for reading
*/
VTableLookup::VTableLookup( char ireadwrite, unsigned int iDebug )
{
    fDebug = iDebug;

    // total number of telescopes
    fNTel = 0;
    fLookupTableFile = 0;
    fDirMSCW = 0;
    fDirMSCL = 0;
    fDirEnergySR = 0;

    // run parameters
    fTLRunParameter = 0;

    fNumberOfIgnoredEvents = 0;
    fNNoiseLevelWarnings = 0;

    // use median size for energy determination
    fUseMedianSizeforEnergyDetermination = true;

    // write only events which triggered the array (for MC only)
    fWriteNoTriggerEvent = true;

    // read or write the tables
    freadwrite = ireadwrite;
    if( freadwrite == 'W' || freadwrite == 'w' )
    {
        fwrite = true;
    }
    else
    {
        fwrite = false;
    }

    cout << endl;
    cout << "-------------------------------------------------------" << endl;
    if( fwrite )
    {
        cout << "filling lookup tables" << endl;
    }
    else
    {
        cout << "reading lookup tables" << endl;
    }
    cout << "-------------------------------------------------------" << endl;
    cout << endl;

    // maximum core distance for events to be taken into account
    fMeanNoiseLevel = 0.;

    fnmscw = 0;

    // set vectors for different tables

    // bins in azimuth (lookup tables are created for each bin)
    fTableAzLowEdge.push_back( 135. );
    fTableAzUpEdge.push_back(-135. );
    fTableAzLowEdge.push_back(-135. );
    fTableAzUpEdge.push_back(-45. );
    fTableAzLowEdge.push_back(-45. );
    fTableAzUpEdge.push_back( 45. );
    fTableAzLowEdge.push_back( 45. );
    fTableAzUpEdge.push_back( 135 );
    fTableAzBins = fTableAzLowEdge.size();
}


/*!
      \param ifile output file name
      \param ioption output file option ('recreate' or 'update')
*/
void VTableLookup::setOutputFile( string ifile, string ioption, string ioutputfile )
{
    if( fwrite )
    {
        cout << "VTableLookup::setOutputFile warning: setting outputfiles makes no sense" << endl;
    }
    fData->setOutputFile( ifile, ioption, ioutputfile );
}


/*!
     construct calculators

     this function is called in table WRITING mode

     \param itablefile  file with all the tables (root file)
     \param ize         zenith angle of simulations to be filled
     \param woff        direction offset from camera center to be filled
     \param noise       noise level (mean pedvar) to be filled
     \param isuff       histogram suffix
*/
void VTableLookup::setMCTableFiles( string itablefile, double ize, int woff, vector<double> noise, string isuff, string iFileTitle, bool iWrite1DHistograms )
{
    if( fDebug )
    {
        cout << "void VTableLookup::setMCTableFiles" << endl;
    }
    if(!fwrite )
    {
        cout << "VTableLookup::setMCTableFiles warning: setting mc table files makes no sense" << endl;
    }
    if(!fTLRunParameter )
    {
        cout << "VTableLookup::setMCTableFiles error: no lookup table run parameters" << endl;
        return;
    }
    // write 1D histograms to disk (default=off)
    fWrite1DHistograms = iWrite1DHistograms;
    // create the table file
    fLookupTableFile = new TFile( itablefile.c_str(), "UPDATE", iFileTitle.c_str() );
    if( fLookupTableFile->IsZombie() )
    {
        cout << "VTableLookup::setMCTableFiles error while opening table file: " << itablefile << endl;
        cout << "exiting..." << endl;
        exit( EXIT_FAILURE );
    }
    if( fLookupTableFile->TestBit( TFile::kRecovered ) )
    {
        cout << "VTableLookup::setMCTableFiles problems with file (TFile::kRecovered, " << fLookupTableFile->TestBit( TFile::kRecovered ) << "): " << endl;
        cout << itablefile << endl;
        cout << "exiting..." << endl;
        exit( EXIT_FAILURE );
    }
    // telescope type
    vector< VTableCalculator* > i_mscw;
    vector< VTableCalculator* > i_mscl;
    vector< VTableCalculator* > i_energySR;
    // azimuth
    vector< vector< VTableCalculator* > > ii_mscw;
    vector< vector< VTableCalculator* > > ii_mscl;
    vector< vector< VTableCalculator* > > ii_energySR;

    vector< vector< vector< VTableCalculator* > > > iii_mscw;
    vector< vector< vector< VTableCalculator* > > > iii_mscl;
    vector< vector< vector< VTableCalculator* > > > iii_energySR;
    vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscw;
    vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscl;
    vector< vector< vector< vector< VTableCalculator* > > > > iiii_energySR;

    map<ULong64_t, unsigned int > i_list_of_Tel_type = fData->getList_of_Tel_type();
    map<ULong64_t, unsigned int >::iterator iter_i_list_of_Tel_type;

    /////////////////////////////////////////////////////////////////////////
    // prepare output file and create all directories
    if( fDebug )
    {
        cout << "VTableLookup::setMCTableFiles() prepare output file and create all directories" << endl;
    }
    char hname[800];
    char htitle[800];
    // ZENITH ANGLE
    sprintf( hname, "ze_%03d", ( int )( ize * 10. + 0.5 ) );
    if( gDirectory->Get( hname ) )
    {
        gDirectory->cd( hname );
    }
    else
    {
        gDirectory->mkdir( hname )->cd();
    }
    //////////////////
    // DIRECTION OFFSET
    vector< int > i_woff_vector;
    i_woff_vector.push_back( woff );
    TDirectory* i_curDir_w = gDirectory;
    for( unsigned int w = 0; w < i_woff_vector.size(); w++ )
    {
        i_curDir_w->cd();

        sprintf( hname, "woff_%04d", i_woff_vector[w] );
        if( gDirectory->Get( hname ) )
        {
            gDirectory->cd( hname );
        }
        else
        {
            gDirectory->mkdir( hname )->cd();
        }

        ii_mscw.clear();
        ii_mscl.clear();
        ii_energySR.clear();

        //////////////////
        // AZIMUTH ANGLE
        TDirectory* i_curDir = gDirectory;
        for( unsigned int i = 0; i < fTableAzLowEdge.size(); i++ )
        {
            i_curDir->cd();

            sprintf( hname, "az_%d", i );
            sprintf( htitle, "%.1f < az < %.1f", fTableAzLowEdge[i], fTableAzUpEdge[i] );
            if( gDirectory->Get( hname ) )
            {
                gDirectory->cd( hname );
            }
            else
            {
                gDirectory->mkdir( hname, htitle )->cd();
            }

            i_mscw.clear();
            i_mscl.clear();
            i_energySR.clear();

            //////////////////
            // TELESCOPE TYPES
            TDirectory* i_curDire2 = gDirectory;
            unsigned int tel_counter = 0;
            for( iter_i_list_of_Tel_type = i_list_of_Tel_type.begin(); iter_i_list_of_Tel_type != i_list_of_Tel_type.end(); iter_i_list_of_Tel_type++ )
            {
                ULong64_t t = iter_i_list_of_Tel_type->first;

                i_curDire2->cd();
                // add directory per azimuth and telescope bin
                sprintf( hname, "tel_%lld", t );
                if( gDirectory->Get( hname ) )
                {
                    cout << "ERROR: directory " << hname << " already exist" << endl;
                    cout << "exiting..." << endl;
                    exit( 0 );
                }
                sprintf( htitle, "telescope type %lld", t );
                gDirectory->mkdir( hname, htitle )->cd();

                // NOISE LEVEL (telescope type dependent)
                sprintf( hname, "NOISE_%05d", ( int )( noise[tel_counter] * 100. ) );
                gDirectory->mkdir( hname )->cd();

                cout << "create tables in path " << gDirectory->GetPath() << endl;

                // DIRECTORIES FOR DIFFERENT TABLES
                fDirMSCW = gDirectory->mkdir( "mscw" );
                fDirMSCL = gDirectory->mkdir( "mscl" );
                fDirEnergySR = gDirectory->mkdir( "energySR" );

                // mean scaled width and length
                i_mscw.push_back( new VTableCalculator( "width", isuff.c_str(), freadwrite, fDirMSCW, false, fTLRunParameter->fPE ) );
                i_mscw.back()->setWrite1DHistograms( fWrite1DHistograms );
                i_mscw.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
                i_mscl.push_back( new VTableCalculator( "length", isuff.c_str(), freadwrite, fDirMSCL, false, fTLRunParameter->fPE ) );
                i_mscl.back()->setWrite1DHistograms( fWrite1DHistograms );
                i_mscl.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
                // energy reconstruction
                i_energySR.push_back( new VTableCalculator( "energySR", isuff.c_str(), freadwrite, fDirEnergySR, true, fTLRunParameter->fPE,
                                      fTLRunParameter->fUseMedianEnergy ) );
                i_energySR.back()->setWrite1DHistograms( fWrite1DHistograms );
                i_energySR.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
                tel_counter++;
            }   // telescope types
            ii_mscw.push_back( i_mscw );
            ii_mscl.push_back( i_mscl );
            ii_energySR.push_back( i_energySR );
        } // azimuth
        iii_mscw.push_back( ii_mscw );
        iii_mscl.push_back( ii_mscl );
        iii_energySR.push_back( ii_energySR );
    } // wobble offsets
    iiii_mscw.push_back( iii_mscw );
    iiii_mscl.push_back( iii_mscl );
    iiii_energySR.push_back( iii_energySR );
    fmscw.push_back( iiii_mscw );
    fmscl.push_back( iiii_mscl );
    fenergySizevsRadius.push_back( iiii_energySR );
}


/*!
    read in zenith angles and file names and construct calculators (one for each angle)

    this function is called in table READING mode

    expected directory structure: Ze/wobble/azimuth/telescope type/noise level

    \param itablefile   root file with all the tables
    \param isuff        histogram suffix
*/
void VTableLookup::setMCTableFiles( string itablefile, string isuff, string iInterpolString )
{
    if( fDebug )
    {
        cout << "void VTableLookup::setMCTableFiles( string itablefile, string isuff )" << endl;
    }

    // open table file
    gErrorIgnoreLevel = 20001;
    fLookupTableFile = new TFile( itablefile.c_str() );
    if( fLookupTableFile->IsZombie() )
    {
        fLookupTableFile->Close();
        const char* data_dir = gSystem->Getenv( "VERITAS_EVNDISP_AUX_DIR" );
        if( data_dir )
        {
            // try to see of file exists in directory ./tables
            ostringstream itablefile_full_path;
            itablefile_full_path << itablefile << "/Tables/" << itablefile;
            itablefile = itablefile_full_path.str();
            fLookupTableFile = new TFile( itablefile.c_str() );
            if( fLookupTableFile->IsZombie() )
            {
                cout << "VTableLookup::setMCTableFiles error (reading): unable to open table file: " << itablefile << endl;
                exit( EXIT_FAILURE );
            }
        }
        else
        {
            cout << "VTableLookup::setMCTableFiles error (reading): unable to open table file: " << itablefile << endl;
            cout << " (no $VERITAS_EVNDISP_AUX_DIR defined)" << endl;
            exit( EXIT_FAILURE );
        }
    }
    gErrorIgnoreLevel = 0;
    cout << "reading table file ( may take a while ): " << itablefile << endl;

    vector< VTableCalculator* > i_mscw;
    vector< VTableCalculator* > i_mscl;
    vector< VTableCalculator* > i_energySR;
    vector< ULong64_t > i_telType;
    vector< vector< VTableCalculator* > > ii_mscw;
    vector< vector< VTableCalculator* > > ii_mscl;
    vector< vector< VTableCalculator* > > ii_energySR;
    vector< vector< ULong64_t > > ii_telType;
    vector< vector< vector< VTableCalculator* > > > iii_mscw;
    vector< vector< vector< VTableCalculator* > > > iii_mscl;
    vector< vector< vector< VTableCalculator* > > > iii_energySR;
    vector< vector< vector< ULong64_t > > > iii_telType;
    vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscw;
    vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscl;
    vector< vector< vector< vector< VTableCalculator* > > > > iiii_energySR;

    // vector with available zenith angles [ze]
    fTableZe.clear();
    // vector with available wobble offsets [ze][woff]
    fTableZeOffset.clear();
    vector< double > i_DirectionOffset;
    // vector with available noise levels [ze][woff][az][tel]
    fTableZeOffsetAzTelNoise.clear();
    vector< vector< vector< vector< double > > > > iiii_TableZeOffsetAzTelNoise;
    vector< vector< vector< double > > > iii_TableZeOffsetAzTelNoise;
    vector< vector< double > > ii_TableZeOffsetAzTelNoise;
    vector< double > i_TableZeOffsetAzTelNoise;


    // ZENITH ANGLE
    TDirectory* iDirZe = gDirectory;
    vector< string > iDNameZE = getSortedListOfDirectories( iDirZe );
    for( unsigned z = 0; z < iDNameZE.size(); z++ )
    {
        fTableZe.push_back( atof( iDNameZE[z].substr( 3, 3 ).c_str() ) / 10. );
        iiii_mscw.clear();
        iiii_mscl.clear();
        iiii_energySR.clear();
        iiii_TableZeOffsetAzTelNoise.clear();
        iii_telType.clear();

        iDirZe->cd( iDNameZE[z].c_str() );

        // DIRECTION OFFSET
        TDirectory* iDirWoff = gDirectory;
        vector< string > iDNameWoff  = getSortedListOfDirectories( iDirWoff );
        i_DirectionOffset.clear();
        for( unsigned int w = 0; w < iDNameWoff.size(); w++ )
        {
            i_DirectionOffset.push_back( atof( iDNameWoff[w].substr( 5, 4 ).c_str() ) / 1000. );
            iii_mscw.clear();
            iii_mscl.clear();
            iii_energySR.clear();
            iii_TableZeOffsetAzTelNoise.clear();
            ii_telType.clear();

            iDirWoff->cd( iDNameWoff[w].c_str() );

            // AZIMUTH ANGLE
            TDirectory* iDirAz = gDirectory;
            vector< string > iDNameAz  = getSortedListOfDirectories( iDirAz );
            for( unsigned int a = 0; a < iDNameAz.size(); a++ )
            {
                ii_mscw.clear();
                ii_mscl.clear();
                ii_energySR.clear();
                ii_TableZeOffsetAzTelNoise.clear();
                i_telType.clear();

                iDirAz->cd( iDNameAz[a].c_str() );

                // TELESCOPE
                TDirectory* iDirTel = gDirectory;
                vector< string > iDNameTel = getSortedListOfDirectories( iDirTel );
                for( unsigned int t = 0; t < iDNameTel.size(); t++ )
                {
                    i_mscw.clear();
                    i_mscl.clear();
                    i_energySR.clear();
                    i_TableZeOffsetAzTelNoise.clear();

                    // telescope type
                    i_telType.push_back(( ULong64_t )( atoi )( iDNameTel[t].substr( 4, iDNameTel[t].size() ).c_str() ) );

                    iDirTel->cd( iDNameTel[t].c_str() );

                    // NOISE LEVEL
                    TDirectory *iDirNoise = gDirectory;
                    vector< string > iDNameNoise = getSortedListOfDirectories( iDirNoise );
                    for( unsigned int n = 0; n < iDNameNoise.size(); n++ )
                    {
                        iDirNoise->cd( iDNameNoise[n].c_str() );
                        if( fDebug == 2 )
                        {
                            cout << "DEBUG  DIR " << " " << gDirectory->GetPath() << endl;
                        }
                        i_TableZeOffsetAzTelNoise.push_back( stof( iDNameNoise[n].substr( iDNameNoise[n].find( "_" ) + 1 ) ) / 100. );

                        // READ IN ALL TABLES
                        TDirectory* iDir = ( TDirectory* )gDirectory->Get( "mscw" );
                        // get mscw directory
                        i_mscw.push_back( new VTableCalculator( "width", isuff.c_str(), freadwrite, iDir, false ) );
                        // get mscl directory
                        iDirNoise->cd( iDNameNoise[n].c_str() );
                        iDir = ( TDirectory* )gDirectory->Get( "mscl" );
                        i_mscl.push_back( new VTableCalculator( "length", isuff.c_str(), freadwrite, iDir, false ) );
                        // get energy directory (size vs radius method)
                        iDirNoise->cd( iDNameNoise[n].c_str() );
                        iDir = ( TDirectory* )gDirectory->Get( "energySR" );
                        i_energySR.push_back( new VTableCalculator( "energySR", isuff.c_str(), freadwrite, iDir, true, fTLRunParameter->fPE, fTLRunParameter->fUseMedianEnergy ) );
                    }                             // noise levels
                    ii_TableZeOffsetAzTelNoise.push_back( i_TableZeOffsetAzTelNoise );
                    ii_mscw.push_back( i_mscw );
                    ii_mscl.push_back( i_mscl );
                    ii_energySR.push_back( i_energySR );
                }                                 // telescopes
                iii_TableZeOffsetAzTelNoise.push_back( ii_TableZeOffsetAzTelNoise );
                iii_mscw.push_back( ii_mscw );
                iii_mscl.push_back( ii_mscl );
                iii_energySR.push_back( ii_energySR );
                ii_telType.push_back( i_telType );
            }                                     // az
            iiii_TableZeOffsetAzTelNoise.push_back( iii_TableZeOffsetAzTelNoise );
            iiii_mscw.push_back( iii_mscw );
            iiii_mscl.push_back( iii_mscl );
            iiii_energySR.push_back( iii_energySR );
            iii_telType.push_back( ii_telType );
        }                                         // woff
        fTableZeOffsetAzTelNoise.push_back( iiii_TableZeOffsetAzTelNoise );
        fmscw.push_back( iiii_mscw );
        fmscl.push_back( iiii_mscl );
        fenergySizevsRadius.push_back( iiii_energySR );
        fTelType_tables.push_back( iii_telType );

        fTableZeOffset.push_back( i_DirectionOffset );
    }   // ze

    // apply sanity check to the lookup table file
    if( sanityCheckLookupTableFile() )
    {
        cout << "    ...survived test of table file! " << endl;
    }
    else
    {
        cout << "     ...Error: did not survive test of table file !";
        cout << " There are missing tables in your table file: " << itablefile << endl;
        sanityCheckLookupTableFile( true ); // this will print which tables are missing
        exit( EXIT_FAILURE );
    }

    if( fDebug )
    {
        cout << "END void VTableLookup::setMCTableFiles( string itablefile, string isuff )" << endl;
    }
}

/*

     sanity checks for lookup tables (not fully implemented)

     - make sure that the same number of zenith, az, NSB tables are given for each telescope type

*/
bool VTableLookup::sanityCheckLookupTableFile( bool iPrint )
{
    // print a summary of the number of tables found
    if( iPrint == false )
    {
        cout << "Found " << fTableZe.size() << " zenith angles, " << fTableZeOffset[0].size() << " wobble offsets, " << fTableAzBins << " azimuth bins and " << fTelType_tables[0][0][0].size() << " telescope types" << endl;
    }
    return true;
}



/*!
     loop over all events, calculate mean scaled width, interpolate between zenith angles
*/
void VTableLookup::loop()
{

    if( fwrite )
    {
        fillLookupTable();
    }
    else
    {
        readLookupTable();
    }
}

/*
    fill lookup tables

    (fmscw, fmscl, fenergySR are vectors of size 1)
*/
void VTableLookup::fillLookupTable()
{
    double idummy1[fData->getMaxNbrTel()];
    double iEventWeight = 0.;
    double idummy3 = 0.;
    int fevent = 0;
    // table types
    map<ULong64_t, unsigned int> i_list_of_Tel_type = fData->getList_of_Tel_type();
    map<ULong64_t, unsigned int>::iterator iter_i_list_of_Tel_type;
    //////////////////////////////////////////////////////////////////////////////////////
    cout << "start event loop " << endl;
    // read next event
    while( fData->getNextEvent( true ) )
    {
        fevent = fData->getEventCounter();

        // print progress
        if(( fevent % 1000000 ) == 0 && fevent != 0 )
        {
            cout << "\t now at event " << fevent << endl;
        }
        if( fDebug )
        {
            cout << "now at event " << fevent << "\t" << fData->getEventStatus() << endl;
        }

        // get event weight (e.g. for spectral weighting)
        iEventWeight = fData->getEventWeight();

        // apply cuts
        if( fData->getEventStatus() && iEventWeight > 0. )
        {
            unsigned int w = 0;
            if( w >= fmscw[0][0].size() )
            {
                continue;
            }

            // loop over all telescopes types, fill according to its type
            unsigned int i_Tel_type_counter = 0;
            for( iter_i_list_of_Tel_type = i_list_of_Tel_type.begin();
                    iter_i_list_of_Tel_type != i_list_of_Tel_type.end();
                    iter_i_list_of_Tel_type++ )
            {
                // get telescope type
                ULong64_t t = iter_i_list_of_Tel_type->first;

                // This should be already the corrected/scaled size value for MC.
                float* i_s = fData->getSize( 1., t, fTLRunParameter->fUseEvndispSelectedImagesOnly );
                float* i_r = fData->getDistanceToCore( t );
                unsigned int i_type = fData->getNTel_type( t );
                ////////////////////////////////////////////////
                // for zenith-angle == 0 deg fill all az bins
                if( fabs( fData->getMCZe() ) < 3. )
                {
                    // OBS: this provides fTableAzBins times more events in the tables for this zenith angle bin
                    for( unsigned int a = 0; a < fTableAzBins; a++ )
                    {
                        // fill tables (get arrays filled for corresponding telescope type; one table per type)
                        fmscw[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s,
                            fData->getWidth( t ), idummy1, iEventWeight, idummy3, idummy1 );
                        fmscl[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s,
                                fData->getLength( t ), idummy1, iEventWeight, idummy3, idummy1 );
                        fenergySizevsRadius[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s,
                                fData->getMCEnergyArray(), idummy1, iEventWeight, idummy3, idummy1 );
                    }
                }
                ////////////////////////////////////////////////
                // for zenith-angle != 0 deg get az bin
                else
                {
                    unsigned int a = getAzBin( fData->getMCAz() );
                    // fill tables (get arrays filled for corresponding telescope type; one table per type)
                    fmscw[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s,
                            fData->getWidth( t ), idummy1, iEventWeight, idummy3, idummy1 );
                    fmscl[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s,
                            fData->getLength( t ), idummy1, iEventWeight, idummy3, idummy1 );
                    fenergySizevsRadius[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s,
                            fData->getMCEnergyArray(), idummy1, iEventWeight, idummy3, idummy1 );
                }
                i_Tel_type_counter++;
            }
        }
    }
}

/*

   read the tables

*/
void VTableLookup::readLookupTable()
{
    unsigned int i_az_bin = 0;

    double esys = 0.;
    double ze = 0.;
    double woff = 0.;
    int fevent = 0;

    // lookup table index for interpolation
    unsigned int ize_up = 0;
    unsigned int ize_low = 0;
    unsigned int iwoff_up = 0;
    unsigned int iwoff_low = 0;

    VTablesToRead *s_ZupWup    = new VTablesToRead( fNTel );
    VTablesToRead *s_ZupWlow   = new VTablesToRead( fNTel );
    VTablesToRead *s_Zup       = new VTablesToRead( fNTel );
    VTablesToRead *s_ZlowWup   = new VTablesToRead( fNTel );
    VTablesToRead *s_ZlowWlow  = new VTablesToRead( fNTel );
    VTablesToRead *s_Zlow      = new VTablesToRead( fNTel );
    VTablesToRead *s_N         = new VTablesToRead( fNTel );

    // first event
    bool bFirst = true;
    if(!fTLRunParameter->isMC )
    {
        bFirst = false;
    }

    ////////////////////////////////////////////////
    // start event loop
    while( fData->getNextEvent( false ) )
    {
        // print progress
        fevent = fData->getEventCounter();
        if(( fevent % 1000000 ) == 0 )
        {
            cout << "\t now at event " << fevent << endl;
        }

        // eventdisplay is saying that his event should be ignored
        if( fData->getEventNumber() == 99999999 )
        {
            fNumberOfIgnoredEvents++;
            continue;
        }

        // get zenith angle for first valid MC event from MC files
        if( bFirst && fData->getMCEnergy() > 0.001
                && fTLRunParameter->ze < 0. )
        {
            cout << "\t\t setting IRF ze from first event" << endl;
            if( fNTel > 0 )
            {
                fTLRunParameter->ze = TMath::Floor(( 90. - fData->getTelElevation() ) + 0.5 );
            }
            else
            {
                fTLRunParameter->ze = TMath::Floor( fData->getMCZe() + 0.5 );
            }
            if( fTLRunParameter->ze < 1.5 )
            {
                fTLRunParameter->ze = 0.;
            }
            fTLRunParameter->fWobbleOffset = ( int )( fData->getMCWobbleOffset() * 100. );
            bFirst = false;
        }
        // reset image counter
        fnmscw = 0;
        unsigned int i_noise_bin = 0;
        // fill MC energy spectra
        fData->fillMChistograms();
        // if data fails basic cuts, write default values directly to tree
        if(!fData->cut() )
        {
            if( fWriteNoTriggerEvent )
            {
                fData->reset();
                fData->fill();
            }
            // goto next event
        }
        else
        {
            //////////////////////////////////////
            // here we should have good data only
            // (ze, az, and wobble offset have been
            //  tested)
            //////////////////////////////////////

            // get direction angles for this event
            ze = fData->getZe();
            woff = fData->getWobbleOffset();
            i_az_bin = getAzBin( fData->getAz() );
            // get noise level for this event
            readNoiseLevel( false );

            if( fDebug == 2 )
            {
                cout << endl << endl << "DEBUG  NEW EVENT " << fData->getEventCounter() << endl;
            }
            /////////////////////////////
            // ZENITH (low)
            if( fDebug == 2 )
            {
                cout << "DEBUG ZENITH LOW" << endl;
            }
            for( int t = 0; t < fNTel; t++ )
            {
                if( fDebug == 2 )
                {
                    cout << "DEBUG  TELESCOPE " << t << " (T" << t + 1 << ")" << endl;
                    cout << "DEBUG      zenith " << ze << ", noise " << fNoiseLevel[t] << ", woff " << woff << ", az " << fData->getAz() << ", az bin " << i_az_bin << endl;
                }
                // get zenith angle (low)
                getIndexBoundary(&ize_up, &ize_low, fTableZe, ze );
                // get direction offset index
                getIndexBoundary(&iwoff_up, &iwoff_low, fTableZeOffset[ize_low], woff );
                // get noise bin (not interpolated; closest value)
                i_noise_bin = getNoiseBin( ize_low, iwoff_up, i_az_bin, t, fNoiseLevel[t] );
                getTables( i_noise_bin, ize_low, iwoff_up, i_az_bin, t, s_ZlowWup );
                i_noise_bin = getNoiseBin( ize_low, iwoff_low, i_az_bin, t, fNoiseLevel[t] );
                getTables( i_noise_bin, ize_low, iwoff_low, i_az_bin, t, s_ZlowWlow );
            }
            calculateMSFromTables( s_ZlowWup, esys );
            calculateMSFromTables( s_ZlowWlow, esys );
            // results in estimation of s_Zlow
            interpolate( s_ZlowWlow, fTableZeOffset[ize_low][iwoff_low], s_ZlowWup, fTableZeOffset[ize_low][iwoff_up], s_Zlow, woff );
            if( fDebug == 2 )
            {
                cout << "DEBUG WOFF INTER 1 ";
                cout << woff << " " << fTableZeOffset[ize_low][iwoff_low] << " " << fTableZeOffset[ize_low][iwoff_up];
                cout << " " << ize_low << " " << s_ZlowWlow->mscl << " " << s_ZlowWup->mscl << " " << s_Zlow->mscl << endl;
            }

            ///////////////////////////
            // ZENITH (up)
            for( int t = 0; t < fNTel; t++ )
            {
                // get zenith angle (up)
                getIndexBoundary(&ize_up, &ize_low, fTableZe, ze );
                // get direction offset index
                getIndexBoundary(&iwoff_up, &iwoff_low, fTableZeOffset[ize_up], woff );
                // noise (not interpolated; closest value)
                i_noise_bin = getNoiseBin( ize_up, iwoff_up, i_az_bin, t, fNoiseLevel[t] );
                getTables( i_noise_bin, ize_up, iwoff_up, i_az_bin, t, s_ZupWup );
                i_noise_bin = getNoiseBin( ize_up, iwoff_low, i_az_bin, t, fNoiseLevel[t] );
                getTables( i_noise_bin, ize_up, iwoff_low, i_az_bin, t, s_ZupWlow );
            }
            calculateMSFromTables( s_ZupWup, esys );
            calculateMSFromTables( s_ZupWlow, esys );
            // results in estimation of s_Zup
            interpolate( s_ZupWlow, fTableZeOffset[ize_up][iwoff_low], s_ZupWup, fTableZeOffset[ize_up][iwoff_up], s_Zup, woff );
            if( fDebug == 2 )
            {
                cout << "DEBUG  WOFF INTER 2 ";
                cout << woff << " " << fTableZeOffset[ize_up][iwoff_low] << " ";
                cout << fTableZeOffset[ize_up][iwoff_up] << " " << ize_up;
                cout << " " << s_ZupWlow->mscl << " " << s_ZupWup->mscl << " " << s_Zup->mscl << endl;
            }
            // interpolate zenith angles
            interpolate( s_Zlow, fTableZe[ize_low], s_Zup, fTableZe[ize_up], s_N, ze, true );
            if( fDebug == 2 )
            {
                cout << "DEBUG  ZE INTER 1 " << ze << " " << fTableZe[ize_low] << " ";
                cout << fTableZe[ize_up] << " ";
                cout << " " << s_Zlow->mscl << " " << s_Zup->mscl << endl;
            }

            // determine number of telescopes with MSCW values
            for( unsigned int j = 0; j < s_N->fNTel; j++ )
            {
                if( s_N->mscw_T[j] > -90. )
                {
                    fnmscw++;
                }
            }
            fData->setNMSCW( fnmscw );
            // set msc value (mean reduced scaled variables)
            // TODO - change of interpolation approach
            // fData->setMSCW( s_N->mscw );
            fData->setMSCW( VMeanScaledVariables::mean_reduced_scaled_variable(s_N->fNTel, fData->getWidth(), s_N->mscw_T, s_N->mscw_Tsigma) );
            // fData->setMSCL( s_N->mscl );
            fData->setMSCL( VMeanScaledVariables::mean_reduced_scaled_variable(s_N->fNTel, fData->getLength(), s_N->mscl_T, s_N->mscl_Tsigma) );

            fData->setMWR( VMeanScaledVariables::mean_scaled_variable(
                        s_N->fNTel, fData->getSize( 1., fTLRunParameter->fUseEvndispSelectedImagesOnly ),
                        fData->getWidth(), s_N->mscw_T ) );
            fData->setMLR( VMeanScaledVariables::mean_scaled_variable(
                        s_N->fNTel, fData->getSize( 1., fTLRunParameter->fUseEvndispSelectedImagesOnly ),
                        fData->getLength(), s_N->mscl_T ) );

            // set energy values
            fData->setEnergy( s_N->energySR, true );
            fData->setChi2( s_N->energySR_Chi2, true );
            fData->setdE( s_N->energySR_dE, true );
            // set mean reduced scaled widths and energies per telescope
            for( unsigned int j = 0; j < s_N->fNTel; j++ )
            {
                fData->setMSCWT( j, s_N->mscw_T[j], s_N->mscw_Tsigma[j] );
                fData->setMSCLT( j, s_N->mscl_T[j], s_N->mscl_Tsigma[j] );
                fData->setEnergyT( j, s_N->energySR_T[j], true );
            }

            fData->fill();
        }
        fevent++;
    }
}


void VTableLookup::interpolate( VTablesToRead* s1, double w1, VTablesToRead* s2, double w2, VTablesToRead* s, double w, bool iCos )
{
    s->mscw =          VStatistics::interpolate( s1->mscw, w1, s2->mscw, w2, w, iCos, 0.1 );
    s->mscl =          VStatistics::interpolate( s1->mscl, w1, s2->mscl, w2, w, iCos, 0.1 );
    s->energySR =      VStatistics::interpolate( s1->energySR, w1, s2->energySR, w2, w, iCos, 0.1 );
    s->energySR_Chi2 = VStatistics::interpolate( s1->energySR_Chi2, w1, s2->energySR_Chi2, w2, w, iCos, 0.1 );
    s->energySR_dE =   VStatistics::interpolate( s1->energySR_dE, w1, s2->energySR_dE, w2, w, iCos, 0.1 );

    for( unsigned int i = 0; i < s1->fNTel; i++ )
    {
        s->mscw_T[i] =      VStatistics::interpolate( s1->mscw_T[i], w1, s2->mscw_T[i], w2, w, iCos, 1.e-2 );
        s->mscl_T[i] =      VStatistics::interpolate( s1->mscl_T[i], w1, s2->mscl_T[i], w2, w, iCos, 1.e-2 );
        s->energySR_T[i] =  VStatistics::interpolate( s1->energySR_T[i], w1, s2->energySR_T[i], w2, w, iCos, 1.e-2 );
        s->mscw_Tsigma[i] = VStatistics::interpolate( s1->mscw_Tsigma[i], w1, s2->mscw_Tsigma[i], w2, w, iCos, 1.e-2 );
        s->mscl_Tsigma[i] = VStatistics::interpolate( s1->mscl_Tsigma[i], w1, s2->mscl_Tsigma[i], w2, w, iCos, 1.e-2 );
    }
}


/*!
     write everything to disk
*/
void VTableLookup::terminate()
{
    fData->terminate( fTLRunParameter );

    if( freadwrite == 'w' || freadwrite == 'W' )
    {
        cout << "writing tables to disk (outputfile is " << fLookupTableFile->GetName() << ")" << endl;
        char hname[800];
        for( unsigned int i = 0; i < fmscw.size(); i++ )
        {
            for( unsigned int t = 0; t < fmscw[i].size(); t++ )
            {
                for( unsigned int u = 0; u < fmscw[i][t].size(); u++ )
                {
                    for( unsigned int v = 0; v < fmscw[i][t][u].size(); v++ )
                    {
                        for( unsigned w = 0; w < fmscw[i][t][u][v].size(); w++ )
                        {
                            cout << "writing tables for " << fmscw[i][t][u][v][w]->getOutputDirectory()->GetMotherDir()->GetPath() << endl;
                            sprintf( hname, "ZE %d WOFF %d AZ %d TEL %d NOISE %d", i, t, u, v, w );
                            fmscw[i][t][u][v][w]->terminate( fmscw[i][t][u][v][w]->getOutputDirectory(), hname );
                            fmscl[i][t][u][v][w]->terminate( fmscl[i][t][u][v][w]->getOutputDirectory(), hname );
                            fenergySizevsRadius[i][t][u][v][w]->terminate( fenergySizevsRadius[i][t][u][v][w]->getOutputDirectory(), hname );
                            fLookupTableFile->Flush();
                        }
                    }
                }
            }
        }
        cout << "end of run (" << fLookupTableFile->GetName() << ")" << endl;
    }
    else
    {
        if( fNumberOfIgnoredEvents > 0 )
        {
            cout << endl << "\t total number of ignored events: " << fNumberOfIgnoredEvents << endl;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // large amount of objects read from subdirectory of the tablefile might result in
    // excessive time needed to close the tablefile
    // tablefile is therefore only close in table writing mode
    if( freadwrite == 'w' || freadwrite == 'W' )
    {
        cout << "closing file..." << endl;
        fLookupTableFile->Close();
    }
    else
    {
        gROOT->GetListOfFiles()->Remove( fLookupTableFile );
    }

    cout << "exiting..." << endl;
}

/*
 * get closes bin number for noise level
 * (no interpolation)
 */
unsigned int VTableLookup::getNoiseBin( unsigned int ize, unsigned int iwoff, unsigned int iaz, unsigned int tel, double noise )
{
    vector< double > noise_vector = fTableZeOffsetAzTelNoise[ize][iwoff][iaz][tel];

    unsigned int closest_index = 0;
    double iDiff = 1.e10;
    for( unsigned int i = 0; i < noise_vector.size(); i++ )
    {
        if( TMath::Abs( noise - noise_vector[i] ) < iDiff )
        {
            iDiff = TMath::Abs( noise - noise_vector[i] );
            closest_index = i;
        }
    }
    return closest_index;
}

/*
     get the corresponding bin number for an azimuth angle
     (no interpolation)

*/
unsigned int VTableLookup::getAzBin( double az )
{
    // be sure that az is bin interval [-180., 180.]
    if( az > 180. )
    {
        az -= 360.;
    }
    // expect first bin to be of type [135,-135]
    // otherwise like [-45,45]
    for( unsigned int i = 0; i < fTableAzBins; i++ )
    {
        if( i == 0 && ( az > fTableAzLowEdge[0] || az < fTableAzUpEdge[0] ) )
        {
            return 0;
        }
        else if( az > fTableAzLowEdge[i] && az < fTableAzUpEdge[i] )
        {
            return i;
        }
    }
    return 0;
}


void VTableLookup::setSpectralIndex( double iS )
{
    // expect positive spectral index
    if( iS < 0. )
    {
        iS *= -1.;
    }

    fData->setSpectralIndex( iS );

    if( TMath::Abs( iS - fData->getMCSpectralIndex() ) > 1.e-2 )
    {
        cout << "----------------------------------------------------------------" << endl;
        cout << "expect MC input spectrum with spectral index of " << fData->getMCSpectralIndex();
        cout << ", energy range [" << fData->getMCMinEnergy() << "," << fData->getMCMaxEnergy() << "] TeV" << endl;
        cout << "weight MC events to new spectral index of " <<  fData->getSpectralIndex() << endl;
    }
}



bool VTableLookup::setInputFiles( vector< string > iInputFiles )
{
    if( fDebug )
    {
        cout << "VTableLookup::setInputFiles " << iInputFiles.size() << endl;
        for( unsigned int i = 0; i < iInputFiles.size(); i++ )
        {
            cout << "\t" << iInputFiles[i] << endl;
        }
    }
    bool bMC = fData->setInputFile( iInputFiles );
    fNTel = fData->getNTel();

    f_calc_msc = new VTableCalculator( fNTel );
    f_calc_msc->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
    f_calc_energySR = new VTableCalculator( fNTel, true );
    f_calc_energySR->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );

    return bMC;
}

/*
      read list of directories from table file

      sort them to make sure that they are always in the same sequence

*/
vector< string > VTableLookup::getSortedListOfDirectories( TDirectory* iDir )
{
    vector< string > iDName;

    if(!iDir )
    {
        return iDName;
    }

    bool bWoffAltered = false;

    TList* iKeyList = iDir->GetListOfKeys();
    if( iKeyList )
    {
        TIter next( iKeyList );
        while( TNamed* iK = ( TNamed* )next() )
        {
            string i_dir_name = iK->GetName();
            if( i_dir_name.find( "log" ) != string::npos
                    || i_dir_name.find( "Log" ) != string::npos
                    || i_dir_name.find( "List" ) != string::npos )
            {
                continue;
            }
            iDName.push_back( i_dir_name );
            if( iDName.back().substr( 0, 4 ) == "woff" && iDName.back().size() == 8 )
            {
                iDName.back() = "woff_0" + iDName.back().substr( 5, iDName.back().size() );
                bWoffAltered = true;
            }
        }
    }

    sort( iDName.begin(), iDName.end() );

    if( bWoffAltered )
    {
        for( unsigned int i = 0; i < iDName.size(); i++ )
        {
            if( iDName[i].substr( 0, 6 ) == "woff_0" )
            {
                iDName[i] = "woff_" + iDName[i].substr( 6, iDName[i].size() );
            }
        }
    }

    return iDName;
}


/*

     read pedvar values for this event

*/
void VTableLookup::readNoiseLevel( bool bWriteToRunPara )
{
    if(!fData )
    {
        return;
    }

    fNoiseLevel     = fData->getNoiseLevel(!bWriteToRunPara );
    fMeanNoiseLevel = fData->getMeanNoiseLevel(!bWriteToRunPara );

    fTelToAnalyze.assign( fNoiseLevel.size(), false );

    if( fTLRunParameter && bWriteToRunPara )
    {
        fTLRunParameter->meanpedvars = fMeanNoiseLevel;
        fTLRunParameter->pedvars = fNoiseLevel;
    }
    if( bWriteToRunPara )
    {
        cout << "Mean pedvar per telescope: ";
        for( unsigned int i = 0; i < fNoiseLevel.size(); i++ )
        {
            cout << " " << fNoiseLevel[i];
        }
        cout << endl;
    }
    // check noise levels
    for( unsigned int i = 0; i < fNoiseLevel.size(); i++ )
    {
        // PE simulations with no noise values (preliminary, should somehow be included into the MC)
        if( fTLRunParameter->fPE )
        {
            if( fData->getNtubes()[i] > 0 )
            {
                fTelToAnalyze[i] = true;
            }
            else
            {
                fTelToAnalyze[i] = false;
            }
        }
        // normal data or MC run
        else
        {
            if( TMath::Abs( fNoiseLevel[i] ) < 1.e-2 && fData->getNtubes()[i] > 0 )
            {
                fTelToAnalyze[i] = false;
                if( fNNoiseLevelWarnings < 30 )
                {
                    cout << "WARNING: noise level for telescope " << i + 1 << " very low: " << fNoiseLevel[i] << " (" << !bWriteToRunPara << ")" << endl;
                }
                else if( fNNoiseLevelWarnings == 30 )
                {
                    cout << "----------- more than 30 noise level warnings, stop printing...--------------" << endl;
                }
                fNNoiseLevelWarnings++;
            }
            else if( TMath::Abs( fNoiseLevel[i] ) < 1.e-2 && fData->getNtubes()[i] < 1 )
            {
                fTelToAnalyze[i] = false;
            }
            else
            {
                fTelToAnalyze[i] = true;
            }
        }
    }

    if(( int )fNoiseLevel.size() != fNTel )
    {
        cout << " VTableLookup::readNoiseLevel ERROR: could not find mean pedestal variation for each telescope" << endl;
        cout << "exiting...." << endl;
        exit( EXIT_FAILURE );
    }
}

/*

    get the index of the two nearest elements in the vector iV to the value x

*/
void VTableLookup::getIndexBoundary( unsigned int* iup, unsigned int* ilow, vector< double >& iV, double x )
{
    if( iV.size() == 0 )
    {
        *iup = 0;
        *ilow = 0;
        return;
    }

    if( x <= iV[0] )
    {
        *iup = *ilow = 0;
    }
    else if( x >= iV[iV.size() - 1] )
    {
        *iup = *ilow = iV.size() - 1;
    }
    else
    {
        for( unsigned int i = 0; i < iV.size(); i++ )
        {
            if( x > iV[i] )
            {
                *ilow = ( unsigned int )i;
                *iup = ( unsigned int )( i + 1 );
            }
        }
    }
}

/*

    get pointers to a certain set of tables

*/
void VTableLookup::getTables( unsigned int inoise, unsigned int ize,
                              unsigned int iwoff, unsigned int iaz, unsigned int tel,
                              VTablesToRead* s )
{
    if(!s )
    {
        return;
    }

    unsigned int telX = 0;
    for( unsigned int i = 0; i < fTelType_tables[ize][iwoff][iaz].size(); i++ )
    {
        if( fData->getTelType( tel ) == fTelType_tables[ize][iwoff][iaz][i] )
        {
            telX = i;
            break;
        }
    }
    if( telX == 999999 )
    {
        cout << "VTableLookup::getTables invalid telescope type: " << tel << "\t" << telX << endl;
        cout << "(this means that there is no table in the table file given for the requested telescope type)" << endl;
        exit( EXIT_FAILURE );
    }
    if( fDebug == 2 )
    {
        cout << "DEBUG  getTables() "  << inoise << " " << ize << " " << iwoff << " " << iaz << " " << telX << endl;
        if( ize >= fmscw.size() )
        {
            cout << "ERROR: ze index out of range (max " << fmscw.size() << ")" << endl;
        }
        if( iwoff >= fmscw[ize].size() )
        {
            cout << "ERROR: woff index out of range (max " << fmscw[ize].size() << ")" << endl;
        }
        if( iaz >= fmscw[ize][iwoff].size() )
        {
            cout << "ERROR: iaz index out of range (max " << fmscw[ize][iwoff].size() << ")" << endl;
        }
        if( telX >= fmscw[ize][iwoff][iaz].size() )
        {
            cout << "ERROR: tel index out of range (max " << fmscw[ize][iwoff][iaz].size() << ")" << endl;
        }
        if( inoise >= fmscw[ize][iwoff][iaz][telX].size() )
        {
            cout << "ERROR: noise index out of range (max " << fmscw[ize][iwoff][iaz][telX].size() << ")" << endl;
        }
        cout << "DEBUG dimensions ze " << fmscw.size() << endl;
        cout << "DEBUG dimensions woff " << fmscw[ize].size() << endl;
        cout << "DEBUG dimensions iaz " << fmscw[ize][iwoff].size() << endl;
        cout << "DEBUG dimensions telX " << fmscw[ize][iwoff][iaz].size() << endl;
        cout << "DEBUG dimensions noise " << fmscw[ize][iwoff][iaz][telX].size() << endl;
        cout << "DEBUG  " << fmscw[ize][iwoff][iaz][telX][inoise] << endl;
        cout << "DEBUG  MEDIAN (MSCW) " << inoise << " " << ize << " " << iwoff << " " << iaz << " " << telX << " ";
        if( fmscw[ize][iwoff][iaz][telX][inoise]->getHistoMedian() )
        {
            cout << fmscw[ize][iwoff][iaz][telX][inoise]->getHistoMedian()->GetEntries() << "\t";
            cout << fmscw[ize][iwoff][iaz][telX][inoise]->getHistoMedian()->GetTitle() << "\t";
            cout << fmscw[ize][iwoff][iaz][telX][inoise]->getHistoMedian()->GetDirectory()->GetPath() << endl;
        }
        cout << "DEBUG  MEDIAN (MSCW,2) " << fmscw.size() << endl;
        cout << "DEBUG  MEDIAN (MSCL) " << inoise << " " << ize << " " << iwoff << " " << iaz << " " << telX << " ";
        if( fmscl[ize][iwoff][iaz][telX][inoise]->getHistoMedian() )
        {
            cout << fmscl[ize][iwoff][iaz][telX][inoise]->getHistoMedian()->GetEntries() << "\t";
            cout << fmscl[ize][iwoff][iaz][telX][inoise]->getHistoMedian()->GetTitle() << "\t";
            cout << fmscl[ize][iwoff][iaz][telX][inoise]->getHistoMedian()->GetDirectory()->GetPath() << endl;
        }
        cout << "DEBUG  MEDIAN (MSCL,2) " << fmscl.size() << endl;
    }

    s->hmscwMedian[tel] = fmscw[ize][iwoff][iaz][telX][inoise]->getHistoMedian();
    s->hmsclMedian[tel] = fmscl[ize][iwoff][iaz][telX][inoise]->getHistoMedian();
    s->henergySRMedian[tel] = fenergySizevsRadius[ize][iwoff][iaz][telX][inoise]->getHistoMedian();
}


/*
    calculate mean scaled values and energies from lookup tables
*/
void VTableLookup::calculateMSFromTables( VTablesToRead* s, double esys )
{
    if(!s )
    {
        return;
    }
    // make sure that list of pointers to tables exists
    if(!f_calc_msc || !f_calc_energySR )
    {
        s->reset();
        return;
    }
    double i_dummy = 0.;

    float* i_s = fData->getSize( 1., fTLRunParameter->fUseEvndispSelectedImagesOnly );

    f_calc_msc->setCalculateEnergies( false );
    ///////////////////
    // calculate mscw
    f_calc_msc->setVHistograms( s->hmscwMedian );
    s->mscw = f_calc_msc->calc(( int )fData->getNTel(), fData->getDistanceToCore(),
                               i_s, fData->getWidth(),
                               s->mscw_T, i_dummy, i_dummy, s->mscw_Tsigma );
    ///////////////////
    // calculate mscl
    f_calc_msc->setVHistograms( s->hmsclMedian );
    s->mscl = f_calc_msc->calc(( int )fData->getNTel(), fData->getDistanceToCore(),
                               i_s, fData->getLength(),
                               s->mscl_T, i_dummy, i_dummy, s->mscl_Tsigma );
    ///////////////////
    // calculate energy (method 1)
    f_calc_energySR->setCalculateEnergies( true );
    f_calc_energySR->setVHistograms( s->henergySRMedian );
    s->energySR = f_calc_energySR->calc(( int )fData->getNTel(), fData->getDistanceToCore(),
                                        i_s, 0,
                                        s->energySR_T, s->energySR_Chi2, s->energySR_dE, s->energySR_Tsigma );
}


bool VTableLookup::initialize( VTableLookupRunParameter* iTLRunParameter )
{
    fTLRunParameter = iTLRunParameter;

    if(!fTLRunParameter )
    {
        cout << "VTableLookup::initialize: error: no table lookup run parameters " << endl;
        return false;
    }
    // init data handler
    fData = new VTableLookupDataHandler( fwrite, fTLRunParameter );
    fData->fillTables( fwrite );

    // if false, don't fill events without array trigger into output tree (MC only)
    setNoTriggerEvents( fTLRunParameter->bNoNoTrigger );
    cout << endl;
    // use median or mean value for energy determination
    setUseMedianForEnergyDetermination( fTLRunParameter->fUseMedianEnergy );
    // set input file names
    fTLRunParameter->isMC = setInputFiles( fTLRunParameter->inputfile );
    // weight event to this spectral index
    if( fTLRunParameter->isMC )
    {
        setSpectralIndex( fTLRunParameter->fSpectralIndex );
    }

    ///////////////////////////////////////
    // write mscw,mscl, and energy tables
    if( fTLRunParameter->readwrite == 'W' )
    {
        char ihname[900];
        if( fTLRunParameter->inputfile.size() > 0 )
        {
            sprintf( ihname, "lookup table file (array recid = %d, source files: %s)", fTLRunParameter->rec_method, fTLRunParameter->inputfile[0].c_str() );
        }
        else
        {
            sprintf( ihname, "lookup table file (array recid = %d)", fTLRunParameter->rec_method );
        }

        string iTitle = ihname;
        cout << "Pedvar levels per telescopes: ";
        for( unsigned int i = 0; i < fData->getNoiseLevel().size(); i++ )
        {
            cout << " " << fData->getNoiseLevel()[i];
        }
        cout << ")" << endl;
        setMCTableFiles( fTLRunParameter->tablefile, fTLRunParameter->ze, fTLRunParameter->fWobbleOffset,
                         fData->getNoiseLevel(), "tb", ihname, fTLRunParameter->fWrite1DHistograms );

        // set min/max distance to camera center
        if( fData )
        {
            fData->setMCDistanceToCameraCenter( fTLRunParameter->fMC_distance_to_cameracenter_min, fTLRunParameter->fMC_distance_to_cameracenter_max );
        }
    }
    ///////////////////////////////////////
    // read MC tables
    else
    {
        // read pedvars
        readNoiseLevel( true );
        // read tables from disk
        setMCTableFiles( fTLRunParameter->tablefile, "tb", fTLRunParameter->fInterpolateString );
        // set output files
        setOutputFile( fTLRunParameter->outputfile, fTLRunParameter->writeoption, fTLRunParameter->tablefile );
    }

    return true;
}
