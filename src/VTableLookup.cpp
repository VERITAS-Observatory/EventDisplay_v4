/*! \class VTableLookup
    \brief calculation of mean scaled variables and energies using MC filled tables

    \author
    Gernot Maier
    Henric Krawczynski

*/

#include "VTableLookup.h"

/*!
    \param ireadwrite W for table writing, R for reading
*/
VTableLookup::VTableLookup( char ireadwrite, unsigned int iDebug )
{
	fDebug = iDebug;
	
	fNTel = 0;
	
	// look up table file
	fLookupTableFile = 0;
	fDirMSCW = 0;
	fDirMSCL = 0;
	fDirEnergyER = 0;
	fDirEnergySR = 0;
	
	// run parameters
	fTLRunParameter = 0;
	
	fNumberOfIgnoredeEvents = 0;
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
		cout << "making lookup tables" << endl;
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
	fTableAzUpEdge.push_back( -135. );
	fTableAzLowEdge.push_back( -135. );
	fTableAzUpEdge.push_back( -45. );
	fTableAzLowEdge.push_back( -45. );
	fTableAzUpEdge.push_back( 45. );
	fTableAzLowEdge.push_back( 45. );
	fTableAzUpEdge.push_back( 135 );
	// azimuth independent bins
	//    fTableAzLowEdge.push_back(  -1.e3 );   fTableAzUpEdge.push_back( 1.e3 );
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
void VTableLookup::setMCTableFiles( string itablefile, double ize, int woff, int noise, string isuff, string iFileTitle, bool iWrite1DHistograms )
{
	if( fDebug )
	{
		cout << "void VTableLookup::setMCTableFiles" << endl;
	}
	if( !fwrite )
	{
		cout << "VTableLookup::setMCTableFiles warning: setting mc table files makes no sense" << endl;
	}
	if( !fTLRunParameter )
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
		exit( 0 );
	}
	if( fLookupTableFile->TestBit( TFile::kRecovered ) )
	{
		cout << "VTableLookup::setMCTableFiles problems with file (TFile::kRecovered, " << fLookupTableFile->TestBit( TFile::kRecovered ) << "): " << endl;
		cout << itablefile << endl;
		cout << "exiting..." << endl;
		exit( 0 );
	}
	// telescope type
	vector< VTableCalculator* > i_mscw;
	vector< VTableCalculator* > i_mscl;
	vector< VTableCalculator* > i_energySR;
	vector< VTableEnergyCalculator* > i_energy;
	// azimuth
	vector< vector< VTableCalculator* > > ii_mscw;
	vector< vector< VTableCalculator* > > ii_mscl;
	vector< vector< VTableCalculator* > > ii_energySR;
	vector< vector< VTableEnergyCalculator* > > ii_energy;
	
	vector< vector< vector< VTableCalculator* > > > iii_mscw;
	vector< vector< vector< VTableCalculator* > > > iii_mscl;
	vector< vector< vector< VTableCalculator* > > > iii_energySR;
	vector< vector< vector< VTableEnergyCalculator* > > > iii_energy;
	vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscw;
	vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscl;
	vector< vector< vector< vector< VTableCalculator* > > > > iiii_energySR;
	vector< vector< vector< vector< VTableEnergyCalculator* > > > > iiii_energy;
	
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
	// NOISE LEVEL
	sprintf( hname, "NOISE_%05d", noise );
	if( fLookupTableFile->Get( hname ) )
	{
		fLookupTableFile->cd( hname );
	}
	else
	{
		fLookupTableFile->mkdir( hname )->cd();
	}
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
	if( fTLRunParameter->fCTA_MC_offaxisBin_min.size() == 0 )
	{
		i_woff_vector.push_back( woff );
	}
	else
	{
		for( unsigned int i = 0; i < fTLRunParameter->fCTA_MC_offaxisBin_min.size(); i++ )
		{
			if( i < fTLRunParameter->fCTA_MC_offaxisBin_max.size() )
			{
				i_woff_vector.push_back( ( int )( 0.5 * ( fTLRunParameter->fCTA_MC_offaxisBin_min[i]
												  + fTLRunParameter->fCTA_MC_offaxisBin_max[i]
														) * 1000. + 0.5 ) );
			}
		}
	}
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
		ii_energy.clear();
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
			i_energy.clear();
			i_energySR.clear();
			
			//////////////////
			// TELESCOPE TYPES
			TDirectory* i_curDire2 = gDirectory;
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
				
				cout << "create tables in path " << gDirectory->GetPath() << endl;
				
				// DIRECTORIES FOR DIFFERENT TABLES
				fDirMSCW = gDirectory->mkdir( "mscw" );
				fDirMSCL = gDirectory->mkdir( "mscl" );
				fDirEnergyER = gDirectory->mkdir( "energyER" );
				fDirEnergySR = gDirectory->mkdir( "energySR" );
				
				// mean scaled width and length
				i_mscw.push_back( new VTableCalculator( "width", isuff.c_str(), freadwrite, fDirMSCW, false, fTLRunParameter->fPE ) );
				i_mscw.back()->setWrite1DHistograms( fWrite1DHistograms );
				i_mscw.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
				i_mscl.push_back( new VTableCalculator( "length", isuff.c_str(), freadwrite, fDirMSCL, false, fTLRunParameter->fPE ) );
				i_mscl.back()->setWrite1DHistograms( fWrite1DHistograms );
				i_mscl.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
				// energy reconstruction
				i_energy.push_back( new VTableEnergyCalculator( isuff.c_str(), freadwrite, fDirEnergyER, fUseMedianSizeforEnergyDetermination ) );
				i_energy.back()->setCutValues( fTLRunParameter->fminsize, fTLRunParameter->fmaxdist );
				i_energy.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
				i_energy.back()->setWrite1DHistograms( fWrite1DHistograms );
				i_energySR.push_back( new VTableCalculator( "energySR", isuff.c_str(), freadwrite, fDirEnergySR, true, fTLRunParameter->fPE,
									  fTLRunParameter->fUseMedianEnergy ) );
				i_energySR.back()->setWrite1DHistograms( fWrite1DHistograms );
				i_energySR.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
			}   // telescope types
			ii_mscw.push_back( i_mscw );
			ii_mscl.push_back( i_mscl );
			ii_energy.push_back( i_energy );
			ii_energySR.push_back( i_energySR );
		} // azimuth
		iii_mscw.push_back( ii_mscw );
		iii_mscl.push_back( ii_mscl );
		iii_energy.push_back( ii_energy );
		iii_energySR.push_back( ii_energySR );
	} // wobble offsets
	iiii_mscw.push_back( iii_mscw );
	iiii_mscl.push_back( iii_mscl );
	iiii_energy.push_back( iii_energy );
	iiii_energySR.push_back( iii_energySR );
	fmscw.push_back( iiii_mscw );
	fmscl.push_back( iiii_mscl );
	fenergyEnergyvsRadius.push_back( iiii_energy );
	fenergySizevsRadius.push_back( iiii_energySR );
}


/*!
    read in zenith angles and file names and construct calculators (one for each angle)

    this function is called in table READING mode

    input file must be sorted in zenith angles (lowest first)

    \param itablefile   root file with all the tables
    \param isuff        histogram suffix
*/
void VTableLookup::setMCTableFiles( string itablefile, string isuff, string iInterpolString )
{
	if( fDebug )
	{
		cout << "void VTableLookup::setMCTableFiles( string itablefile, string isuff )" << endl;
	}
	
	gErrorIgnoreLevel = 20001;
	fLookupTableFile = new TFile( itablefile.c_str() );
	if( fLookupTableFile->IsZombie() )
	{
		fLookupTableFile->Close();
		const char* data_dir = gSystem->Getenv( "OBS_EVNDISP_ANA_DIR" );
		if( data_dir )
		{
			// try to see of file exists in directory ./tables
			string itemp = data_dir;
			itemp += "/Tables/" + itablefile;
			itablefile = itemp;
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
			cout << " (no $OBS_EVNDISP_ANA_DIR defined)" << endl;
			exit( EXIT_FAILURE );
		}
	}
	gErrorIgnoreLevel = 0;
	cout << "reading table file ( may take a while ): " << itablefile << endl;
	
	vector< VTableCalculator* > i_mscw;
	vector< VTableCalculator* > i_mscl;
	vector< VTableCalculator* > i_energySR;
	vector< ULong64_t > i_telType;
	vector< VTableEnergyCalculator* > i_energy;
	vector< vector< VTableCalculator* > > ii_mscw;
	vector< vector< VTableCalculator* > > ii_mscl;
	vector< vector< VTableCalculator* > > ii_energySR;
	vector< vector< VTableEnergyCalculator* > > ii_energy;
	vector< vector< ULong64_t > > ii_telType;
	vector< vector< vector< VTableCalculator* > > > iii_mscw;
	vector< vector< vector< VTableCalculator* > > > iii_mscl;
	vector< vector< vector< VTableCalculator* > > > iii_energySR;
	vector< vector< vector< VTableEnergyCalculator* > > > iii_energy;
	vector< vector< vector< ULong64_t > > > iii_telType;
	vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscw;
	vector< vector< vector< vector< VTableCalculator* > > > > iiii_mscl;
	vector< vector< vector< vector< VTableCalculator* > > > > iiii_energySR;
	vector< vector< vector< vector< VTableEnergyCalculator* > > > > iiii_energy;
	vector< vector< vector< vector< ULong64_t > > > > iiii_telType;
	
	fTableNoiseLevel.clear();
	fTableZe.clear();
	vector< double > i_ze;
	fTableDirectionOffset.clear();
	vector< double > i_DirectionOffset;
	vector< vector< double > > ii_DirectionOffset;
	
	// NOISE LEVEL
	vector< string > iDName = getSortedListOfDirectories( fLookupTableFile );
	for( unsigned int n = 0; n < iDName.size(); n++ )
	{
		fTableNoiseLevel.push_back( atof( iDName[n].substr( 6, 5 ).c_str() ) / 100. );
		
		fLookupTableFile->cd( iDName[n].c_str() );
		if( fDebug == 2 )
		{
			cout << "DEBUG  DIR " << " " << gDirectory->GetPath() << endl;
		}
		// ZENITH ANGLE
		TDirectory* iDirZe = gDirectory;
		vector< string > iDNameZE = getSortedListOfDirectories( iDirZe );
		i_ze.clear();
		ii_DirectionOffset.clear();
		iiii_mscw.clear();
		iiii_mscl.clear();
		iiii_energy.clear();
		iiii_energySR.clear();
		iiii_telType.clear();
		for( unsigned z = 0; z < iDNameZE.size(); z++ )
		{
			i_ze.push_back( atof( iDNameZE[z].substr( 3, 3 ).c_str() ) / 10. );
			iii_mscw.clear();
			iii_mscl.clear();
			iii_energy.clear();
			iii_energySR.clear();
			iii_telType.clear();
			
			iDirZe->cd( iDNameZE[z].c_str() );
			// DIRECTION OFFSET
			TDirectory* iDirWoff = gDirectory;
			vector< string > iDNameWoff  = getSortedListOfDirectories( iDirWoff );
			i_DirectionOffset.clear();
			for( unsigned int w = 0; w < iDNameWoff.size(); w++ )
			{
				i_DirectionOffset.push_back( atof( iDNameWoff[w].substr( 5, 4 ).c_str() ) / 1000. );
				ii_mscw.clear();
				ii_mscl.clear();
				ii_energy.clear();
				ii_energySR.clear();
				ii_telType.clear();
				
				iDirWoff->cd( iDNameWoff[w].c_str() );
				// AZIMUTH ANGLE
				TDirectory* iDirAz = gDirectory;
				vector< string > iDNameAz  = getSortedListOfDirectories( iDirAz );
				for( unsigned int a = 0; a < iDNameAz.size(); a++ )
				{
					i_mscw.clear();
					i_mscl.clear();
					i_energy.clear();
					i_energySR.clear();
					i_telType.clear();
					
					iDirAz->cd( iDNameAz[a].c_str() );
					// TELESCOPE
					TDirectory* iDirTel = gDirectory;
					vector< string > iDNameTel = getSortedListOfDirectories( iDirTel );
					for( unsigned int t = 0; t < iDNameTel.size(); t++ )
					{
						// READ IN ALL TABLES
						iDirTel->cd( iDNameTel[t].c_str() );
						TDirectory* iDir = ( TDirectory* )gDirectory->Get( "mscw" );
						// telescope type
						i_telType.push_back( ( ULong64_t )( atoi )( iDNameTel[t].substr( 4, iDNameTel[t].size() ).c_str() ) );
						// get mscw directory
						i_mscw.push_back( new VTableCalculator( "width", isuff.c_str(), freadwrite, iDir, false ) );
						// get mscl directory
						iDirTel->cd( iDNameTel[t].c_str() );
						iDir = ( TDirectory* )gDirectory->Get( "mscl" );
						i_mscl.push_back( new VTableCalculator( "length", isuff.c_str(), freadwrite, iDir, false ) );
						// get energy directory (inverted tabled)
						iDirTel->cd( iDNameTel[t].c_str() );
						iDir = ( TDirectory* )gDirectory->Get( "energyER" );
						i_energy.push_back( new VTableEnergyCalculator( isuff.c_str(), freadwrite, iDir, true, iInterpolString ) );
						i_energy.back()->setCutValues( fTLRunParameter->fminsize, fTLRunParameter->fmaxdist );
						i_energy.back()->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
						// get energy directory (size vs radius method)
						iDirTel->cd( iDNameTel[t].c_str() );
						iDir = ( TDirectory* )gDirectory->Get( "energySR" );
						i_energySR.push_back( new VTableCalculator( "energySR", isuff.c_str(), freadwrite, iDir, true, fTLRunParameter->fPE, fTLRunParameter->fUseMedianEnergy ) );
					}                             // telescopes
					ii_mscw.push_back( i_mscw );
					ii_mscl.push_back( i_mscl );
					ii_energy.push_back( i_energy );
					ii_energySR.push_back( i_energySR );
					ii_telType.push_back( i_telType );
				}                                 // az
				iii_mscw.push_back( ii_mscw );
				iii_mscl.push_back( ii_mscl );
				iii_energy.push_back( ii_energy );
				iii_energySR.push_back( ii_energySR );
				iii_telType.push_back( ii_telType );
			}                                     // woff
			iiii_mscw.push_back( iii_mscw );
			iiii_mscl.push_back( iii_mscl );
			iiii_energy.push_back( iii_energy );
			iiii_energySR.push_back( iii_energySR );
			iiii_telType.push_back( iii_telType );
			ii_DirectionOffset.push_back( i_DirectionOffset );
		}                                         // ze
		fmscw.push_back( iiii_mscw );
		fmscl.push_back( iiii_mscl );
		fenergyEnergyvsRadius.push_back( iiii_energy );
		fenergySizevsRadius.push_back( iiii_energySR );
		fTelType_tables.push_back( iiii_telType );
		
		fTableDirectionOffset.push_back( ii_DirectionOffset );
		fTableZe.push_back( i_ze );
	}
	
	// apply sanity check to the lookup table file
	if( sanityCheckLookupTableFile() )
	{
		cout << "    ...survived test of table file! " << endl;
	}
	else
	{
		cout << "     ...did not survive test of table file ! There are missing tables in your table file: " << itablefile << endl;
		cout << "     You need to redo your table, or contact Gernot if this is the standard table supplied " << endl ;
		sanityCheckLookupTableFile( true ); // this will print which tables are missing
		exit( -1 );
	}
	
	if( fDebug )
	{
		cout << "END void VTableLookup::setMCTableFiles( string itablefile, string isuff )" << endl;
	}
}


bool VTableLookup::sanityCheckLookupTableFile( bool iPrint )
{
	if( iPrint == false )
	{
		cout << "lookup table file sanity check..." << endl;
	}
	if( iPrint == true )
	{
		cout << "this screen printing will help you to locate the missing LTs..." << endl;
	}
	
	
	for( unsigned int i = 0; i < fTableNoiseLevel.size(); i++ )
	{
	
		if( iPrint == true )
		{
			cout << " Noise level index  " << i << " has " << fTableZe[i].size() << " zenith angles" << endl;
		}
		if( iPrint == false )
			if( ( i > 0 ) && ( fTableZe[i] != fTableZe[i - 1] ) )
			{
				return false;
			}
			
		for( unsigned int j = 0; j < fTableZe[i].size(); j++ )
		{
			if( iPrint == true )
			{
				cout << "Zenith index " << j <<  " has " << fTableDirectionOffset[i][j].size() << " wobble offsets" << endl;
			}
			if( iPrint == false )
			{
				if( ( j > 0 ) && ( fTableDirectionOffset[i][j] != fTableDirectionOffset[i][j - 1] ) )
				{
					return false;
				}
				if( ( i > 0 ) && ( fTableDirectionOffset[i][j] != fTableDirectionOffset[i - 1][j] ) )
				{
					return false;
				}
			}
			
			for( unsigned int k = 0; k < fTableDirectionOffset[i][j].size(); k++ )
				for( unsigned int l = 0; l < fTableAzBins ; l++ )
				{
					//cout << "Telescopes/Azimuth " << fTelType_tables[i][j][k][l].size() << endl;
					
					if( iPrint == false )
					{
						if( ( i > 0 ) && ( fTelType_tables[i][j][k][l] != fTelType_tables[i - 1][j][k][l] ) )
						{
							return false;
						}
						if( ( j > 0 ) && ( fTelType_tables[i][j][k][l] != fTelType_tables[i][j - 1][k][l] ) )
						{
							return false;
						}
						if( ( k > 0 ) && ( fTelType_tables[i][j][k][l] != fTelType_tables[i][j][k - 1][l] ) )
						{
							return false;
						}
						if( ( l > 0 ) && ( fTelType_tables[i][j][k][l] != fTelType_tables[i][j][k][l - 1] ) )
						{
							return false;
						}
					}
				}
		}
	}
	
	if( iPrint == false )
	{
		cout << "Found " << fTableNoiseLevel.size() << " noise levels, " << fTableZe[0].size() << " zenith angles, " << fTableDirectionOffset[0][0].size() << " wobble offsets, " << fTableAzBins << " azimuth bins and " << fTelType_tables[0][0][0][0].size() << " telescope types" << endl;
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
    fill the tables

    (fmscw,fmscl,fenergyEnergyvsRadius are vectors of size 1)
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
	// (read only parameters which are needed from file, VEvndispData::getNextEvent must be adjusted if additional
	//  parameters are used while writing tables)
	while( fData->getNextEvent( true ) )
	{
		fevent = fData->getEventCounter();
		
		// print progress
		if( ( fevent % 1000000 ) == 0 && fevent != 0 )
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
			unsigned int w = getWobbleBin( fData->getMCWobbleOffset() );
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
				
				double* i_s2 = fData->getSize2( 1., t, fTLRunParameter->fUseSelectedImagesOnly );
				double* i_r = fData->getDistanceToCore( t );
				unsigned int i_type = fData->getNTel_type( t );
				
				////////////////////////////////////////////////
				// for zenith-angle == 0 deg fill all az bins
				if( fabs( fData->getMCZe() ) < 3. )
				{
					// OBS: this provides fTableAzBins times more events in the tables for this zenith angle bin
					for( unsigned int a = 0; a < fTableAzBins; a++ )
					{
						// fill tables (get arrays filled for corresponding telescope type; one table per type)
						fmscw[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s2,
								fData->getWidth( t ), idummy1, iEventWeight, idummy3, idummy1 );
						fmscl[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s2,
								fData->getLength( t ), idummy1, iEventWeight, idummy3, idummy1 );
						fenergySizevsRadius[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s2, fData->getMCEnergyArray(),
								idummy1, iEventWeight, idummy3, idummy1 );
						if( !fTLRunParameter->fLimitEnergyReconstruction )
						{
							fenergyEnergyvsRadius[0][0][w][a][i_Tel_type_counter]->calc( i_type, fData->getMCEnergy(),
									i_r, i_s2, idummy1, iEventWeight, idummy3, 0. );
						}
					}
				}
				////////////////////////////////////////////////
				// for zenith-angle != 0 deg get az bin
				else
				{
					unsigned int a = getAzBin( fData->getMCAz() );
					// fill tables (get arrays filled for corresponding telescope type; one table per type)
					fmscw[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s2,
							fData->getWidth( t ), idummy1, iEventWeight, idummy3, idummy1 );
					fmscl[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s2,
							fData->getLength( t ), idummy1, iEventWeight, idummy3, idummy1 );
					fenergySizevsRadius[0][0][w][a][i_Tel_type_counter]->calc( i_type, i_r, i_s2, fData->getMCEnergyArray(),
							idummy1, iEventWeight, idummy3, idummy1 );
							
					if( !fTLRunParameter->fLimitEnergyReconstruction )
					{
						fenergyEnergyvsRadius[0][0][w][a][i_Tel_type_counter]->calc( i_type, fData->getMCEnergy(),
								i_r, i_s2, idummy1, iEventWeight, idummy3, 0. );
					}
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
	// zenith angle lower and upper limit
	int i_az = 0;
	
	double esys = 0.;
	double ze = 0.;
	double woff = 0.;
	int fevent = 0;
	double imr = 0.;
	double inr = 0.;
	
	// lookup table index for interpolation
	unsigned int inoise_up = 0;
	unsigned int inoise_low = 0;
	unsigned int ize_up = 0;
	unsigned int ize_low = 0;
	unsigned int iwoff_up = 0;
	unsigned int iwoff_low = 0;
	
	s_NupZupWup    = new VTablesToRead( fNTel );
	s_NupZupWlow   = new VTablesToRead( fNTel );
	s_NupZup       = new VTablesToRead( fNTel );
	s_NupZlowWup   = new VTablesToRead( fNTel );
	s_NupZlowWlow  = new VTablesToRead( fNTel );
	s_NupZlow      = new VTablesToRead( fNTel );
	s_Nup          = new VTablesToRead( fNTel );
	s_NlowZupWup   = new VTablesToRead( fNTel );
	s_NlowZupWlow  = new VTablesToRead( fNTel );
	s_NlowZup      = new VTablesToRead( fNTel );
	s_NlowZlowWup  = new VTablesToRead( fNTel );
	s_NlowZlowWlow = new VTablesToRead( fNTel );
	s_NlowZlow     = new VTablesToRead( fNTel );
	s_Nlow         = new VTablesToRead( fNTel );
	s_N            = new VTablesToRead( fNTel );
	
	// first event
	bool bFirst = true;
	if( !fTLRunParameter->isMC )
	{
		bFirst = false;
	}
	
	////////////////////////////////////////////////
	// start event loop
	while( fData->getNextEvent( false ) )
	{
		// print progress
		fevent = fData->getEventCounter();
		if( ( fevent % 1000000 ) == 0 )
		{
			cout << "\t now at event " << fevent << endl;
		}
		
		// eventdisplay is saying that his event should be ignored
		if( fData->getEventNumber() == 99999999 )
		{
			fNumberOfIgnoredeEvents++;
			continue;
		}
		
		// get zenith angle for first valid MC event from MC files
		if( bFirst && fData->getMCEnergy() > 0.001 )
		{
			if( fNTel > 0 )
			{
				fTLRunParameter->ze = TMath::Floor( ( 90. - fData->getTelElevation() ) + 0.5 );
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
		// fill MC energy spectra
		fData->fillMChistograms();
		// if data fails basic cuts, write default values directly to tree
		if( !fData->cut() )
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
			//////////////////////////////////////
			ze = fData->getZe();
			woff = fData->getWobbleOffset();
			// get azimuth angle bin (no interpolation)
			i_az = getAzBin( fData->getAz() );
			
			// get noise level for this event
			readNoiseLevel( false );
			
			if( fDebug == 2 )
			{
				cout << endl << endl << "DEBUG  NEW EVENT " << fData->getEventCounter() << endl;
			}
			/////////////////////////////
			// NOISE (low) ZENITH (low)
			if( fDebug == 2 )
			{
				cout << "DEBUG  NOISE LOW ZENITH LOW" << endl;
			}
			for( int t = 0; t < fNTel; t++ )
			{
				if( fDebug == 2 )
				{
					cout << "DEBUG  TELESCOPE " << t << " (T" << t + 1 << ")" << endl;
					cout << "DEBUG      zenith " << ze << ", noise " << fNoiseLevel[t] << ", woff " << woff << ", az " << fData->getAz() << ", az bin " << i_az << endl;
				}
				// noise (low)
				getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fNoiseLevel[t] );
				// get zenith angle
				getIndexBoundary( &ize_up, &ize_low, fTableZe[inoise_low], ze );
				if( fDebug == 2 )
				{
					cout << "DEBUG  WOFF " << t << " " << inoise_low << " " << inoise_up << " " << fNoiseLevel[t] << "\t" << fTableNoiseLevel.size();
					for( unsigned int ii = 0; ii < fTableNoiseLevel.size(); ii++ )
					{
						cout << "    " << fTableNoiseLevel[ii];
					}
					cout << endl;
				}
				
				// zenith angle (low)
				// get direction offset index
				getIndexBoundary( &iwoff_up, &iwoff_low, fTableDirectionOffset[inoise_low][ize_low], woff );
				getTables( inoise_low, ize_low, iwoff_up, i_az, t, s_NlowZlowWup );
				getTables( inoise_low, ize_low, iwoff_low, i_az, t, s_NlowZlowWlow );
			}
			getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fMeanNoiseLevel );
			calculateMSFromTables( s_NlowZlowWup, esys );
			calculateMSFromTables( s_NlowZlowWlow, esys );
			interpolate( s_NlowZlowWlow, fTableDirectionOffset[inoise_low][ize_low][iwoff_low], s_NlowZlowWup, fTableDirectionOffset[inoise_low][ize_low][iwoff_up], s_NlowZlow, woff );
			if( fDebug == 2 )
			{
				cout << "DEBUG  WOFF INTER 1 ";
				cout << woff << " " << fTableDirectionOffset[inoise_low][ize_low][iwoff_low] << " " << fTableDirectionOffset[inoise_low][ize_low][iwoff_up];
				cout << " " << inoise_low << " " << inoise_up << " " << ize_low << " ";
				cout << s_NlowZlowWlow->mscl << " " << s_NlowZlowWup->mscl << " " << s_NlowZlow->mscl << endl;
			}
			
			///////////////////////////
			// NOISE (low) ZENITH (up)
			for( int t = 0; t < fNTel; t++ )
			{
				// noise (low)
				getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fNoiseLevel[t] );
				// get zenith angle
				getIndexBoundary( &ize_up, &ize_low, fTableZe[inoise_low], ze );
				
				// zenith angle (up)
				// get direction offset index
				getIndexBoundary( &iwoff_up, &iwoff_low, fTableDirectionOffset[inoise_low][ize_up], woff );
				getTables( inoise_low, ize_up, iwoff_up, i_az, t, s_NlowZupWup );
				getTables( inoise_low, ize_up, iwoff_low, i_az, t, s_NlowZupWlow );
			}
			calculateMSFromTables( s_NlowZupWup, esys );
			calculateMSFromTables( s_NlowZupWlow, esys );
			getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fMeanNoiseLevel );
			interpolate( s_NlowZupWlow, fTableDirectionOffset[inoise_low][ize_up][iwoff_low], s_NlowZupWup, fTableDirectionOffset[inoise_low][ize_up][iwoff_up], s_NlowZup, woff );
			if( fDebug == 2 )
			{
				cout << "DEBUG  WOFF INTER 2 ";
				cout << woff << " " << fTableDirectionOffset[inoise_low][ize_up][iwoff_low] << " ";
				cout << fTableDirectionOffset[inoise_low][ize_up][iwoff_up] << " " << inoise_low << " " << inoise_up << " " << ize_up;
				cout << " " << s_NlowZupWlow->mscl << " " << s_NlowZupWup->mscl << " " << s_NlowZup->mscl << endl;
			}
			interpolate( s_NlowZlow, fTableZe[inoise_low][ize_low], s_NlowZup, fTableZe[inoise_low][ize_up], s_Nlow, ze, true );
			if( fDebug == 2 )
			{
				cout << "DEBUG  ZE INTER 1 " << ze << " " << fTableZe[inoise_low][ize_low] << " ";
				cout << fTableZe[inoise_low][ize_up] << " " << inoise_low << " ";
				cout << inoise_up << " " << s_NlowZlow->mscl << " " << s_NlowZup->mscl << " " << s_Nlow->mscl << endl;
			}
			
			///////////////////////////
			// NOISE (up) ZENITH (low)
			if( fDebug == 2 )
			{
				cout << "DEBUG  HIGH NOISE" << endl;
			}
			for( int t = 0; t < fNTel; t++ )
			{
				// noise (up)
				getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fNoiseLevel[t] );
				// get zenith angle
				getIndexBoundary( &ize_up, &ize_low, fTableZe[inoise_up], ze );
				if( fDebug == 2 )
				{
					cout << "DEBUG  WOFF " << t << " " << inoise_low << " " << inoise_up << " " << fNoiseLevel[t] << endl;
				}
				
				// zenith angle (low)
				// get direction offset index
				getIndexBoundary( &iwoff_up, &iwoff_low, fTableDirectionOffset[inoise_up][ize_low], woff );
				getTables( inoise_up, ize_low, iwoff_up, i_az, t, s_NupZlowWup );
				getTables( inoise_up, ize_low, iwoff_low, i_az, t, s_NupZlowWlow );
			}
			calculateMSFromTables( s_NupZlowWup, esys );
			calculateMSFromTables( s_NupZlowWlow, esys );
			getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fMeanNoiseLevel );
			interpolate( s_NupZlowWlow, fTableDirectionOffset[inoise_up][ize_low][iwoff_low], s_NupZlowWup, fTableDirectionOffset[inoise_up][ize_low][iwoff_up], s_NupZlow, woff );
			if( fDebug == 2 )
			{
				cout << "DEBUG  WOFF INTER 1 ";
				cout << woff << " " << fTableDirectionOffset[inoise_up][ize_low][iwoff_low] << " " << fTableDirectionOffset[inoise_up][ize_low][iwoff_up];
				cout <<  " " << inoise_low << " " << inoise_up << " " << s_NupZlowWlow->mscl << " " << s_NupZlowWup->mscl << " " << s_NupZlow->mscl << endl;
			}
			
			///////////////////////////
			// NOISE (up) ZENITH (up)
			for( int t = 0; t < fNTel; t++ )
			{
				// noise (up)
				getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fNoiseLevel[t] );
				// get zenith angle
				getIndexBoundary( &ize_up, &ize_low, fTableZe[inoise_up], ze );
				
				// zenith angle (up)
				// get direction offset index
				getIndexBoundary( &iwoff_up, &iwoff_low, fTableDirectionOffset[inoise_up][ize_up], woff );
				getTables( inoise_up, ize_up, iwoff_up, i_az, t, s_NupZupWup );
				getTables( inoise_up, ize_up, iwoff_low, i_az, t, s_NupZupWlow );
			}
			calculateMSFromTables( s_NupZupWup, esys );
			calculateMSFromTables( s_NupZupWlow, esys );
			getIndexBoundary( &inoise_up, &inoise_low, fTableNoiseLevel, fMeanNoiseLevel );
			interpolate( s_NupZupWlow, fTableDirectionOffset[inoise_up][ize_up][iwoff_low], s_NupZupWup, fTableDirectionOffset[inoise_up][ize_up][iwoff_up], s_NupZup, woff );
			if( fDebug == 2 )
			{
				cout << "DEBUG  WOFF INTER 2 ";
				cout << woff << " " << fTableDirectionOffset[inoise_up][ize_up][iwoff_low] << " ";
				cout << fTableDirectionOffset[inoise_up][ize_up][iwoff_up] << " " << inoise_low << " " << inoise_up << " ";
				cout << s_NupZupWlow->mscl << " " << s_NupZupWup->mscl << " " << s_NupZup->mscl << endl;
			}
			///////////////////////////
			interpolate( s_NupZlow, fTableZe[inoise_up][ize_low], s_NupZup, fTableZe[inoise_up][ize_up], s_Nup, ze, true );
			if( fDebug == 2 )
			{
				cout << "DEBUG  ZE INTER 2 " << ze << " " << inoise_low << " " << inoise_up << " ";
				cout << s_NupZlow->mscl << " " << s_NupZup->mscl << " " << s_Nup->mscl << endl;
			}
			interpolate( s_Nlow, fTableNoiseLevel[inoise_low], s_Nup, fTableNoiseLevel[inoise_up], s_N, fMeanNoiseLevel, false );
			if( fDebug == 2 )
			{
				cout << "DEBUG  NOISE INTER " << fMeanNoiseLevel << " " << fTableNoiseLevel[inoise_low] << " ";
				cout << fTableNoiseLevel[inoise_up] << " " << inoise_low << " " << inoise_up << " ";
				cout << s_Nlow->mscl << " " << s_Nup->mscl << " " << s_N->mscl << endl;
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
			fData->setMSCW( s_N->mscw );
			fData->setMSCL( s_N->mscl );
			
			// calculate mean width ratio (mean scaled variables)
			imr = 0.;
			inr = 0.;
			for( unsigned int j = 0; j < s_N->fNTel; j++ )
			{
				if( s_N->mscw_T[j] > 0. && fData->getWidth() )
				{
					imr += fData->getWidth()[j] / s_N->mscw_T[j];
					inr++;
				}
			}
			if( inr > 0. )
			{
				fData->setMWR( imr / inr );
			}
			else
			{
				fData->setMWR( -99. );
			}
			// calculate mean length ratio (mean scaled variables)
			imr = 0.;
			inr = 0.;
			for( unsigned int j = 0; j < s_N->fNTel; j++ )
			{
				if( s_N->mscl_T[j] > 0. && fData->getLength() )
				{
					imr += fData->getLength()[j] / s_N->mscl_T[j];
					inr++;
				}
			}
			if( inr > 0. )
			{
				fData->setMLR( imr / inr );
			}
			else
			{
				fData->setMLR( -99. );
			}
			
			// set energy values
			fData->setEnergy( s_N->energyER, s_N->energySR );
			fData->setChi2( s_N->energyER_Chi2, s_N->energySR_Chi2 );
			fData->setdE( s_N->energyER_dE, s_N->energySR_dE );
			// set mean reduced scaled widths and energies per telescope
			for( unsigned int j = 0; j < s_N->fNTel; j++ )
			{
				fData->setMSCWT( j, s_N->mscw_T[j], s_N->mscw_Tsigma[j] );
				fData->setMSCLT( j, s_N->mscl_T[j], s_N->mscl_Tsigma[j] );
				fData->setEnergyT( j, s_N->energyER_T[j], s_N->energySR_T[j] );
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
	s->energyER =      VStatistics::interpolate( s1->energyER, w1, s2->energyER, w2, w, iCos, 0.1 );
	s->energyER_Chi2 = VStatistics::interpolate( s1->energyER_Chi2, w1, s2->energyER_Chi2, w2, w, iCos, 0.1 );
	s->energyER_dE =   VStatistics::interpolate( s1->energyER_dE, w1, s2->energyER_dE, w2, w, iCos, 0.1 );
	s->energySR =      VStatistics::interpolate( s1->energySR, w1, s2->energySR, w2, w, iCos, 0.1 );
	s->energySR_Chi2 = VStatistics::interpolate( s1->energySR_Chi2, w1, s2->energySR_Chi2, w2, w, iCos, 0.1 );
	s->energySR_dE =   VStatistics::interpolate( s1->energySR_dE, w1, s2->energySR_dE, w2, w, iCos, 0.1 );
	
	for( unsigned int i = 0; i < s1->fNTel; i++ )
	{
		s->mscw_T[i] =      VStatistics::interpolate( s1->mscw_T[i], w1, s2->mscw_T[i], w2, w, iCos, 1.e-2 );
		s->mscl_T[i] =      VStatistics::interpolate( s1->mscl_T[i], w1, s2->mscl_T[i], w2, w, iCos, 1.e-2 );
		s->energyER_T[i] =  VStatistics::interpolate( s1->energyER_T[i], w1, s2->energyER_T[i], w2, w, iCos, 1.e-2 );
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
							sprintf( hname, "NOISE %d ZE %d WOFF %d AZ %d TEL %d", i, t, u, v, w );
							fmscw[i][t][u][v][w]->terminate( fmscw[i][t][u][v][w]->getOutputDirectory(), hname );
							fmscl[i][t][u][v][w]->terminate( fmscl[i][t][u][v][w]->getOutputDirectory(), hname );
							if( !fTLRunParameter->fLimitEnergyReconstruction )
							{
								fenergyEnergyvsRadius[i][t][u][v][w]->terminate( fenergyEnergyvsRadius[i][t][u][v][w]->getOutputDirectory(), hname );
							}
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
		if( fNumberOfIgnoredeEvents > 0 )
		{
			cout << endl << "\t total number of ignored events: " << fNumberOfIgnoredeEvents << endl;
		}
	}
	
	////////////////////////////////////////////////////////////////////
	// (GM): large amount of objects read from subdirectory of the tablefile might result in
	//       excessive time needed to close the tablefile
	//       tablefile is therefore only close in table writing mode
	if( freadwrite == 'w' || freadwrite == 'W' )
	{
		cout << "closing file..." << endl;
		fLookupTableFile->Close();
	}
	
	cout << "exiting..." << endl;
}


int VTableLookup::getAzBin( double az )
{
	// be sure that az is bin intervall [-180., 180.]
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
	f_calc_energy = new VTableEnergyCalculator( fNTel );
	f_calc_energy->setCutValues( fTLRunParameter->fminsize, fTLRunParameter->fmaxdist );
	f_calc_energy->setMinRequiredShowerPerBin( fTLRunParameter->fMinRequiredShowerPerBin );
	
	return bMC;
}


vector< string > VTableLookup::getSortedListOfDirectories( TDirectory* iDir )
{
	vector< string > iDName;
	
	if( !iDir )
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
			iDName.push_back( iK->GetName() );
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


void VTableLookup::readNoiseLevel( bool bWriteToRunPara )
{
	if( !fData )
	{
		return;
	}
	
	fNoiseLevel     = fData->getNoiseLevel( !bWriteToRunPara );
	fMeanNoiseLevel = fData->getMeanNoiseLevel( !bWriteToRunPara );
	
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
	
	if( ( int )fNoiseLevel.size() != fNTel )
	{
		cout << " VTableLookup::readNoiseLevel ERROR: could not find mean pedestal variation for each telescope" << endl;
		cout << "exiting...." << endl;
		exit( 0 );
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


void VTableLookup::getTables( unsigned int inoise, unsigned int ize, unsigned int iwoff, unsigned int iaz, unsigned int tel, VTablesToRead* s )
{
	if( !s )
	{
		return;
	}
	
	unsigned int telX = 0;
	for( unsigned int i = 0; i < fTelType_tables[inoise][ize][iwoff][iaz].size(); i++ )
	{
		if( fData->getTelType( tel ) == fTelType_tables[inoise][ize][iwoff][iaz][i] )
		{
			telX = i;
			break;
		}
	}
	
	if( telX == 999999 )
	{
		cout << "VTableLookup::getTables invalid telescope type: " << tel << "\t" << telX << endl;
		exit( -1 );
	}
	if( fDebug == 2 )
	{
		cout << "DEBUG  getTables() "  << inoise << " " << ize << " " << iwoff << " " << iaz << " " << telX << endl;
		cout << "DEBUG  " << fmscw[inoise][ize][iwoff][iaz][telX] << endl;
		cout << "DEBUG  MEDIAN (MSCW) " << inoise << " " << ize << " " << iwoff << " " << iaz << " " << telX << " ";
		if( fmscw[inoise][ize][iwoff][iaz][telX]->getHistoMedian() )
		{
			cout << fmscw[inoise][ize][iwoff][iaz][telX]->getHistoMedian()->GetEntries() << "\t";
			cout << fmscw[inoise][ize][iwoff][iaz][telX]->getHistoMedian()->GetTitle() << "\t";
			cout << fmscw[inoise][ize][iwoff][iaz][telX]->getHistoMedian()->GetDirectory()->GetPath() << endl;
		}
		cout << "DEBUG  MEDIAN (MSCW,2) " << fmscw[inoise].size() << endl;
		cout << "DEBUG  MEDIAN (MSCL) " << inoise << " " << ize << " " << iwoff << " " << iaz << " " << telX << " ";
		if( fmscl[inoise][ize][iwoff][iaz][telX]->getHistoMedian() )
		{
			cout << fmscl[inoise][ize][iwoff][iaz][telX]->getHistoMedian()->GetEntries() << "\t";
			cout << fmscl[inoise][ize][iwoff][iaz][telX]->getHistoMedian()->GetTitle() << "\t";
			cout << fmscl[inoise][ize][iwoff][iaz][telX]->getHistoMedian()->GetDirectory()->GetPath() << endl;
		}
		cout << "DEBUG  MEDIAN (MSCL,2) " << fmscl[inoise].size() << endl;
	}
	
	s->hmscwMedian[tel] = fmscw[inoise][ize][iwoff][iaz][telX]->getHistoMedian();
	s->hmsclMedian[tel] = fmscl[inoise][ize][iwoff][iaz][telX]->getHistoMedian();
	s->henergySRMedian[tel] = fenergySizevsRadius[inoise][ize][iwoff][iaz][telX]->getHistoMedian();
	if( !fTLRunParameter->fLimitEnergyReconstruction )
	{
		s->henergyERMedian[tel] = fenergyEnergyvsRadius[inoise][ize][iwoff][iaz][telX]->getHistoMedian();
	}
}


void VTableLookup::calculateMSFromTables( VTablesToRead* s, double esys )
{
	if( !s )
	{
		return;
	}
	if( !f_calc_msc || !f_calc_energy || !f_calc_energySR )
	{
		s->reset();
		return;
	}
	double i_dummy = 0.;
	
	double* i_s2 = fData->getSize2( 1., fTLRunParameter->fUseSelectedImagesOnly );
	
	f_calc_msc->setCalculateEnergies( false );
	// calculate mscw
	f_calc_msc->setVHistograms( s->hmscwMedian );
	s->mscw = f_calc_msc->calc( ( int )fData->getNTel(), fData->getDistanceToCore(),
								i_s2, fData->getWidth(),
								s->mscw_T, i_dummy, i_dummy, s->mscw_Tsigma );
	// calculate mscl
	f_calc_msc->setVHistograms( s->hmsclMedian );
	s->mscl = f_calc_msc->calc( ( int )fData->getNTel(), fData->getDistanceToCore(),
								i_s2, fData->getLength(),
								s->mscl_T, i_dummy, i_dummy, s->mscl_Tsigma );
	// calculate energy (method 1)
	f_calc_energySR->setCalculateEnergies( true );
	f_calc_energySR->setVHistograms( s->henergySRMedian );
	s->energySR = f_calc_energySR->calc( ( int )fData->getNTel(), fData->getDistanceToCore(),
										 i_s2, 0,
										 s->energySR_T, s->energySR_Chi2, s->energySR_dE, s->energySR_Tsigma );
	// calculate energy (method 0)
	f_calc_energy->setVHistograms( s->henergyERMedian );
	s->energyER = f_calc_energy->calc( ( int )fData->getNTel(), fData->getMCEnergy(), fData->getDistanceToCore(),
									   i_s2, s->energyER_T, s->energyER_Chi2, s->energyER_dE, esys );
}


bool VTableLookup::initialize( VTableLookupRunParameter* iTLRunParameter )
{
	fTLRunParameter = iTLRunParameter;
	
	if( !fTLRunParameter )
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
                int i_mean_pedvarlevel = (int)(fData->getMeanNoiseLevel()*100);
                cout << "setting mean pedvar level for table selection to : " << fData->getMeanNoiseLevel() << endl;
                cout << "   (pedvar levels per telescopes are ";
                for( unsigned int i = 0; i < fData->getNoiseLevel().size(); i++ ) cout << " " << fData->getNoiseLevel()[i];
                cout << ")" << endl;
		setMCTableFiles( fTLRunParameter->tablefile, fTLRunParameter->ze, fTLRunParameter->fWobbleOffset,
				 i_mean_pedvarlevel, "tb", ihname, fTLRunParameter->fWrite1DHistograms );
						 
		// set min/max distance to camera center
		if( fData )
		{
			fData->setMCDistanceToCameraCenter( fTLRunParameter->fMC_distance_to_cameracenter_min, fTLRunParameter->fMC_distance_to_cameracenter_max );
		}
	}
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

unsigned int VTableLookup::getWobbleBin( double w )
{
	if( fTLRunParameter->fCTA_MC_offaxisBin_min.size() == 0 )
	{
		return 0;
	}
	
	for( unsigned int i = 0; i < fTLRunParameter->fCTA_MC_offaxisBin_min.size(); i++ )
	{
		if( w >= fTLRunParameter->fCTA_MC_offaxisBin_min[i] && w < fTLRunParameter->fCTA_MC_offaxisBin_max[i] )
		{
			return i;
		}
	}
	
	return 9999;
}

