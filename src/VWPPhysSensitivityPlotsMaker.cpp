/*!  VWPPhysSensitivityPlotsMaker

     plot sets of sensitivities for CTA/VTS

     Example (executed on the root commane line):

     WPPhysSensitivityPlotsMaker a;
     a.compareDataSets("DataSets.list");

     with < DataSets.list > see example files

*/

#include "VWPPhysSensitivityPlotsMaker.h"

VWPPhysSensitivityPlotsMaker::VWPPhysSensitivityPlotsMaker()
{

	vector< double > iOffAxisValue;
	iOffAxisValue.push_back( 0.5 );
	iOffAxisValue.push_back( 1.5 );
	iOffAxisValue.push_back( 2.5 );
	iOffAxisValue.push_back( 3.25 );
	iOffAxisValue.push_back( 3.75 );
	//    iOffAxisValue.push_back( 4.25 );
	//    iOffAxisValue.push_back( 4.75 );

	cout << "VWPPhysSensitivityPlotsMaker: hardwired offsets from camera center: ";
	for( unsigned int i = 0; i < iOffAxisValue.size(); i++ )
	{
		cout << iOffAxisValue[i] << ", ";
	}
	cout << " [deg]" << endl;

	setOffAxisAngle( iOffAxisValue );
	setEnergyRange_Lin_TeV();
	setObservingTime();
	setAxisUnits();
	setPrintingOptions();
	setPlotRequirements();

	fPlotAllInOneCanvas = 0;
	fSensitivityPad = 0;
	fSensitivityRatioPad = 0;
	fEffAreaPad = 0;
	fBckRatesPad = 0;
	fERes = 0;
	fAngRes = 0;

}

void VWPPhysSensitivityPlotsMaker::plotAllInOneCanvas()
{
	fPlotAllInOneCanvas = new TCanvas( "cSensitivityFullCanvas", "sensitivity", 400, 10, 900, 720 );
	fPlotAllInOneCanvas->SetGridx( 0 );
	fPlotAllInOneCanvas->SetGridy( 0 );

	fSensitivityPad = new TPad( "cSensitivityPad", "sensitivity", 0.01, 0.34, 0.68, 0.99 );
	fSensitivityPad->SetLeftMargin( 0.15 );
	fSensitivityPad->SetRightMargin( 0.03 );
	fSensitivityPad->Draw();

	fSensitivityRatioPad = new TPad( "cSensitivityPadRatio", "sensitivity ratio", 0.02, 0.01, 0.38, 0.34 );
	fSensitivityRatioPad->SetBottomMargin( 0.13 );
	fSensitivityRatioPad->SetTopMargin( 0.05 );
	fSensitivityRatioPad->Draw();

	fEffAreaPad = new TPad( "cEffAreaPad", "effective area", 0.69, 0.67, 0.99, 0.99 );
	fEffAreaPad->SetLeftMargin( 0.15 );
	fEffAreaPad->SetRightMargin( 0.05 );
	fEffAreaPad->Draw();

	fBckRatesPad = new TPad( "cBckRatesPad", "background rates", 0.69, 0.34, 0.99, 0.67 );
	fBckRatesPad->SetLeftMargin( 0.15 );
	fBckRatesPad->SetRightMargin( 0.05 );
	fBckRatesPad->Draw();

	fERes = new TPad( "cERes", "energy resolution", 0.69, 0.01, 0.99, 0.34 );
	fERes->SetLeftMargin( 0.15 );
	fERes->SetRightMargin( 0.05 );
	fERes->SetBottomMargin( 0.13 );
	fERes->SetTopMargin( 0.05 );
	fERes->Draw();

	fAngRes = new TPad( "cAngRes", "angular resolution", 0.40, 0.01, 0.68, 0.34 );
	fAngRes->SetLeftMargin( 0.13 );
	fAngRes->SetRightMargin( 0.05 );
	fAngRes->SetBottomMargin( 0.13 );
	fAngRes->SetTopMargin( 0.05 );
	fAngRes->Draw();
}

void VWPPhysSensitivityPlotsMaker::compareDataSets( string iDataSetFile, string iDirectionString, bool iUseIntegratedSensitivityForOffAxisPlots )
{
	VPlotWPPhysSensitivity a;
	a.setUseIntegratedSensitivityForOffAxisPlots( iUseIntegratedSensitivityForOffAxisPlots );
	a.setPlotCTARequirements( fPlotCTARequirements, fPlotCTARequirementGoals );
	a.setEnergyRange_Lin_TeV( fMinEnergy_TeV, fMaxEnergy_TeV );
	a.addDataSets( iDataSetFile, iDirectionString );
	a.plotIRF( fPrintingOptions, 50., 5.e7, 0.3, fEffAreaPad, fAngRes, fERes );
	a.plotSensitivity( fPrintingOptions, fSensitivity_min, fSensitivity_max, fSensitivity_Unit, fSensitivityPad, fSensitivityRatioPad, fBckRatesPad );
	a.plotProjectedSensitivities( 0, 5. );
	a.plotSensitivityRatio( fPrintingOptions, 0.4, 4.1, false, fSensitivityRatioPad );
	a.plotSensitivityRatio( fPrintingOptions, 0.2, 1.8, true );
	// Southern sites: 30 GeV - 100 TeV
	if( fPlotCTARequirements < 3 )
	{
		a.printSensitivityFigureOfMerit( 0.03, 100. );
	}
	// Northern sites: 30 GeV - 20 TeV
	else
	{
		a.printSensitivityFigureOfMerit( 0.03, 20. );
	}
}

void VWPPhysSensitivityPlotsMaker::printPlotCTARequirementsIDs()
{
	cout << "requirements IDs: " << endl;
	cout << "0 \t South, 50h, FOM for energy range [0.03,100]" << endl;
	cout << "1 \t South, 5h, FOM for energy range [0.03,100]" << endl;
	cout << "2 \t South, 0.5h, FOM for energy range [0.03,100]" << endl;
	cout << "3 \t North, 50h, FOM for energy range [0.03,20]" << endl;
	cout << "4 \t North, 5h, FOM for energy range [0.03,20]" << endl;
	cout << "5 \t North, 0.5h, FOM for energy range [0.03,20]" << endl;
	cout << "(current settings is " << fPlotCTARequirements << ")" << endl;
	cout << endl;
}

void VWPPhysSensitivityPlotsMaker::compareOffAxisSensitivities( string iSubArray, string iDataSet )
{
	vector< string > iD;
	iD.push_back( iDataSet );
	compareOffAxisSensitivities( iSubArray, iD );
}

/*

   compare off axis sensitivities

   note that some values are hardwired in plotProjectedSensitivities

*/
void VWPPhysSensitivityPlotsMaker::compareOffAxisSensitivities( string iSubArray, vector< string > iDataSet )
{
	if( iSubArray.size() > 0 )
	{
		fListOfArrays.clear();
		fListOfArrays.push_back( iSubArray );
	}
	if( iDataSet.size() > 0 )
	{
		fListofDataSets.clear();
		fListofDataSets = iDataSet;
	}
	cout << "Compare " << fListOfArrays.size() << " array(s) in " << fListofDataSets.size() << " data set(s)" << endl;

	TCanvas* c = 0;
	for( unsigned int j = 0; j < fListofDataSets.size(); j++ )
	{
		for( unsigned int i = 0; i < fListOfArrays.size(); i++ )
		{
			VPlotWPPhysSensitivity a;
			a.setEnergyRange_Lin_TeV( fMinEnergy_TeV, fMaxEnergy_TeV );
			for( unsigned int k = 0; k < fOffAxisAngle.size(); k++ )
			{
				cout << "\t adding " << fListofDataSets[j] << ", " << fListOfArrays[i] << ", " << fObservingTime_s << ", " << fOffAxisAngle[k] << ", " << k << ", " << j << endl;
				a.addDataSet( fListofDataSets[j], fListOfArrays[i], fObservingTime_s, fOffAxisAngle[k], "", k + 1, j + 1 );
			}
			string iP = "";
			if( fPrintingOptions.size() > 0 )
			{
				iP += fPrintingOptions + "-" + fListOfArrays[i];
			}
			a.plotIRF( iP );
			a.plotSensitivity( iP, fSensitivity_min, fSensitivity_max, fSensitivity_Unit );

			c = a.plotProjectedSensitivities( c, fOffAxisAngle.back(), j + 1 );
		}
	}
}

void VWPPhysSensitivityPlotsMaker::setAxisUnits( double iMinSensitivity, double iMaxSensitivity, string iUnit )
{
	fSensitivity_min = iMinSensitivity;
	fSensitivity_max = iMaxSensitivity;
	fSensitivity_Unit = iUnit;
}

void VWPPhysSensitivityPlotsMaker::resetVectors()
{
	fListOfArrays.clear();
	fListofDataSets.clear();
	fOffAxisAngle.clear();
}


bool VWPPhysSensitivityPlotsMaker::writeTexFileBody( string iTexFile, string iTexFileTitle )
{
	// tex file
	cout << "Writing tex file: " << iTexFile << endl;
	ofstream os;
	os.open( iTexFile.c_str() );
	if( !os )
	{
		cout << "VWPPhysSensitivityPlotsMaker::writeTexFileBody: failed writing to " << iTexFile << endl;
		return false;
	}

	// intro
	os << "\\documentclass[11pt]{scrartcl}" << endl;
	os << "\\usepackage[a4paper,landscape,scale=0.9]{geometry}" << endl;
	os << "\\usepackage{graphicx}" << endl;
	os << "\\usepackage{epstopdf}" << endl;
	os << "\\usepackage[pdftex,colorlinks=true,bookmarks=false,bookmarksopen=false]{hyperref}" << endl;

	os << "\\title{CTA sensitivities with EVNDISP \\\\ ";
	os << iTexFileTitle << "}" << endl;
	os << "\\author{Gernot Maier \\\\ DESY}" << endl;
	os << "\\date{\\today}" << endl;

	os << "\\begin{document}" << endl;
	os << "\\maketitle" << endl;
	os << "\\tableofcontents" << endl;

	os << "\\newpage" << endl;

	// images
	if( fListOfArrays.size() > 0 )
	{
		for( unsigned int i = 0; i < fListOfArrays.size(); i++ )
		{
			os << "\\begin{figure}" << endl;
			os << "\\centering\\includegraphics[width=0.69\\linewidth]{" << fPrintingOptions << "-" << fListOfArrays[i] << "-Sensitivity.pdf}" << endl;
			os << "\\centering\\includegraphics[width=0.29\\linewidth]{ArrayLayout-" << fListOfArrays[i] << ".pdf}" << endl;
			os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-" << fListOfArrays[i] << "-EffArea.pdf}" << endl;
			os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-" << fListOfArrays[i] << "-BRates.pdf}" << endl;
			os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-" << fListOfArrays[i] << "-AngRes.pdf}" << endl;
			os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-" << fListOfArrays[i] << "-ERes.pdf}" << endl;
			os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-" << fListOfArrays[i] << "-EBias.pdf}" << endl;
			os << "\\end{figure}" << endl;
			os << "\\clearpage" << endl;
			os << endl << endl;
		}
	}
	else
	{
		os << "\\begin{figure}" << endl;
		os << "\\centering\\includegraphics[width=0.69\\linewidth]{" << fPrintingOptions << "-Sensitivity.pdf}" << endl;
		os << "\\centering\\includegraphics[width=0.29\\linewidth]{ArrayLayout-" << fPrintingOptions << ".pdf}" << endl;
		os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-EffArea.pdf}" << endl;
		os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-BRates.pdf}" << endl;
		os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-AngRes.pdf}" << endl;
		os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-ERes.pdf}" << endl;
		os << "\\centering\\includegraphics[width=0.18\\linewidth]{" << fPrintingOptions << "-EBias.pdf}" << endl;
		os << "\\end{figure}" << endl;
		os << "\\clearpage" << endl;
		os << endl << endl;
	}
	// tex file closing
	os << endl;
	os << "\\end{document}" << endl;

	os.close();

	return true;
}
