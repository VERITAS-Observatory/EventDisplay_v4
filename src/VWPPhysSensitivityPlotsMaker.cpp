/*!  VWPPhysSensitivityPlotsMaker

     plot sets of sensitivities for CTA/VTS

     Example (executed on the root commane line):

     WPPhysSensitivityPlotsMaker a;
     a.setListOfArrays( "subArray.E.list" );
     a.compareDataSets("DataSets.list");

     with 

     subArray.list:

     E
     I

     and 

     DataSets.list

     DESY.20130424.E1.ID2.ISDC3700m g85  180000 0. 3 1
     DESY.20130424.E1.ID2.v_leeds   g85  180000 0. 1 1

*/

#include "VWPPhysSensitivityPlotsMaker.h"

VWPPhysSensitivityPlotsMaker::VWPPhysSensitivityPlotsMaker()
{

    vector< double > iOffAxisValue;
    iOffAxisValue.push_back( 0.1 );
    iOffAxisValue.push_back( 1.2 );
    iOffAxisValue.push_back( 2.4 );
    iOffAxisValue.push_back( 3.1 );
    iOffAxisValue.push_back( 3.6 );
    iOffAxisValue.push_back( 4.1 );

    cout << "VWPPhysSensitivityPlotsMaker: hardwired offsets from camera center: ";
    for( unsigned int i = 0; i < iOffAxisValue.size(); i++ ) cout << iOffAxisValue[i] << ", ";
    cout << " [deg]" << endl;

    setOffAxisAngle( iOffAxisValue );
    setObservingTime();
    setAxisUnits();
    setPrintingOptions();
}

bool VWPPhysSensitivityPlotsMaker::setListOfArrays( string iSubArrayFileTxt )
{
    return readList( iSubArrayFileTxt, fListOfArrays );
}

bool VWPPhysSensitivityPlotsMaker::setListOfDataSets( string iDataSetFileTxt )
{
    return readList( iDataSetFileTxt, fListofDataSets );
}

bool VWPPhysSensitivityPlotsMaker::readList( string iFileTxt, vector< string >& iList )
{
    iList.clear();

    ifstream is;
    is.open( iFileTxt.c_str(), ifstream::in );
    if( !is ) return false;

    string is_line;
    while( getline( is, is_line ) ) 
    {
      iList.push_back( is_line );
    }

    if( iList.size() == 0 )
    {
        cout << "VWPPhysSensitivityPlotsMaker::readlist: no arrays given" << endl;
	cout << "\t " << iFileTxt << endl;
	return false;
    }

    cout << "found " << iList.size() << " entries" << endl;

    return true;
}

void VWPPhysSensitivityPlotsMaker::compareDataSets( string iDataSetFile )
{
   VPlotWPPhysSensitivity a;
   a.addDataSets( iDataSetFile );
   a.plotIRF( fPrintingOptions );
   a.plotSensitivity( fPrintingOptions, fSensitivity_min, fSensitivity_max, fSensitivity_Unit );
}

/*

    plot different data sets on top of each other

*/
void VWPPhysSensitivityPlotsMaker::compareDataSets( string iSubArray, string iDataSet )
{
    if( iSubArray.size() > 0 )
    {
        fListOfArrays.clear();
	fListOfArrays.push_back( iSubArray );
    }
    if( iDataSet.size() > 0 )
    {
       fListofDataSets.clear();
       fListofDataSets.push_back( iDataSet );
    }
    cout << "Compare " << fListOfArrays.size() << " arrays in " << fListofDataSets.size() << " data set" << endl;

    for( unsigned int i = 0; i < fListOfArrays.size(); i++ )
    {
       VPlotWPPhysSensitivity a;
       for( unsigned int j = 0; j < fListofDataSets.size(); j++ )
       {
	  a.addDataSet( fListofDataSets[j], fListOfArrays[i], fObservingTime_s, 0.0, "", j+1, 1 );
       }
       string iP = "";
       if( fPrintingOptions.size() > 0 ) iP = fPrintingOptions + "-" + fListOfArrays[i];
       a.plotIRF( iP );
       a.plotSensitivity( iP, fSensitivity_min, fSensitivity_max, fSensitivity_Unit );
    }
}

void VWPPhysSensitivityPlotsMaker::compareOffAxisSensitivities( string iSubArray, string iDataSet )
{
    if( iSubArray.size() > 0 )
    {
        fListOfArrays.clear();
	fListOfArrays.push_back( iSubArray );
    }
    if( iDataSet.size() > 0 )
    {
       fListofDataSets.clear();
       fListofDataSets.push_back( iDataSet );
    }
    cout << "Compare " << fListOfArrays.size() << " arrays in " << fListofDataSets.size() << " data set" << endl;

    for( unsigned int i = 0; i < fListOfArrays.size(); i++ )
    {
       VPlotWPPhysSensitivity a;
       for( unsigned int k = 0; k < fOffAxisAngle.size(); k++ )
       {
	  for( unsigned int j = 0; j < fListofDataSets.size(); j++ )
	  {
	     a.addDataSet( fListofDataSets[j], fListOfArrays[i], fObservingTime_s, fOffAxisAngle[k], "", k+1, j+1 );
          }
       }
       string iP = "";
       if( fPrintingOptions.size() > 0 ) iP += fPrintingOptions + "-" + fListOfArrays[i];
       a.plotIRF( iP );
       a.plotSensitivity( iP, fSensitivity_min, fSensitivity_max, fSensitivity_Unit );
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

