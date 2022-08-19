/**********************************************************
 * Plot spectral energy distribution and TS distributions *
 **********************************************************/

void getSens( TString anasumfile, TString outputbasename )
{
	fstream file;
	// Backup streambuffers of  cout
	streambuf* stream_buffer_cout = cout.rdbuf();
	streambuf* stream_buffer_cin = cin.rdbuf();
	// Get the streambuffer of the file
	streambuf* stream_buffer_file;
	
	gSystem->Load( "$EVNDISPSYS/lib/libVAnaSum.so" );
	VSensitivityCalculator a;
	TCanvas* c;
	// signif, minevts,obstime,minbackgrrate,alpha
	a.setSignificanceParameter( 5, 10, 50, 0.05, 0.2 );
	//a.plotIntegralSensitivityvsEnergyFromCrabSpectrum(c,
	//    anasumfile.Data(),1,"CU",0.03,1e5);
	a.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c,
			anasumfile.Data(), 1, "CU", 0.15 );
			
	TGraphAsymmErrors* tg = ( TGraphAsymmErrors* ) a.getSensitivityGraph();
	// Redirect output to file
	file.open( Form( "%s_intsens50.txt", outputbasename.Data() ), ios::out );
	stream_buffer_file = file.rdbuf();
	// Redirect cout to file
	cout.rdbuf( stream_buffer_file );
	cout << "# Sensitivity (int) VTS from Crab analysis (in CU)" << endl;
	cout << "# X, EXlow, EXhigh, Y, EYlow, EYhigh" << endl;
	for( Int_t k = 0; k < tg->GetN(); k++ )
	{
		cout << Form( "%.3e, %.3e, %.3e, %.3e, %.3e, %.2e",
					  tg->GetX()[k],
					  tg->GetEXlow()[k],
					  tg->GetEXhigh()[k],
					  tg->GetY()[k],
					  tg->GetEYlow()[k],
					  tg->GetEYhigh()[k] ) << endl;
	}
	// Recover standard output
	cout.rdbuf( stream_buffer_cout );
	file.close();
	
	a.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c,
			anasumfile.Data(), 1, "CU", 0.15 );
			
	TGraphAsymmErrors* tg = ( TGraphAsymmErrors* ) a.getSensitivityGraph();
	// Redirect output to file
	file.open( Form( "%s_diffsens50.txt", outputbasename.Data() ), ios::out );
	stream_buffer_file = file.rdbuf();
	// Redirect cout to file
	cout.rdbuf( stream_buffer_file );
	cout << "# Sensitivity (diff) VTS from Crab analysis (in CU)" << endl;
	cout << "# X, EXlow, EXhigh, Y, EYlow, EYhigh" << endl;
	for( Int_t k = 0; k < tg->GetN(); k++ )
	{
		cout << Form( "%.3e, %.3e, %.3e, %.3e, %.3e, %.2e",
					  tg->GetX()[k],
					  tg->GetEXlow()[k],
					  tg->GetEXhigh()[k],
					  tg->GetY()[k],
					  tg->GetEYlow()[k],
					  tg->GetEYhigh()[k] ) << endl;
	}
	// Recover standard output
	cout.rdbuf( stream_buffer_cout );
	file.close();
}

