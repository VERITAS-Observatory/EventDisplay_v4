/*! \class VImageCleaning

 */

#include "VImageCleaning.h"
#include "TopoTrigger.h"

VImageCleaning::VImageCleaning( VEvndispData *iData )
{
   fData = iData;
}

void VImageCleaning::printDataError( string iFunctionName )
{
   cout << iFunctionName;
   cout << " error: no pointer to data class set" << endl;
   exit( -1 );
}

/*!
  tailcut cleaning with fixed thresholds
   \par hithresh image threshold
   \par lothresh border threshold
*/
void VImageCleaning::cleanImageFixed(double hithresh, double lothresh )
{
    cleanImageFixed( hithresh, lothresh, -999. );
};

/*!
  tailcut cleaning with fixed thresholds
   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
*/
void VImageCleaning::cleanImageFixed(double hithresh, double lothresh, double brightthresh )
{
    if( !fData ) printDataError( "VImageCleaning::cleanImageFixed" );
// calculates the valarray of tubes to be included in the parameterization
    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );
    fData->setImageBorderNeighbour( false );
    unsigned int i_nchannel = fData->getNChannels();

    for ( unsigned int i=0; i < i_nchannel; i++)
    {
        if( fData->getSums()[i] > hithresh )
        {
            if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] )
            {
                fData->setImage( i, true );
                for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                {
                    unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
                    if ( k < fData->getImage().size() && fData->getSums()[k] > lothresh && !fData->getImage()[k] ) fData->setBorder( k, true );
                }
            }
        }
        if( fData->getSums()[i] > brightthresh )
        {
            if( fData->getDetectorGeo()->getAnaPixel()[i] > 0 && !fData->getDead(fData->getHiLo()[i])[i] ) fData->setBrightNonImage( i, true );
        }
    }

// (preli) set the trigger vector in MC case (preli)
// trigger vector are image/border tubes
    if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 || fData->getReader()->getDataFormatNum() == 6 ) fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
// (end of preli)

    if( fData->getRunParameter()->fRemoveIsolatedPixel )       removeIsolatedPixels();
    if( fData->getRunParameter()->fFillImageBorderNeighbours ) fillImageBorderNeighbours();
}


/*!
  signal-to-noise tailcut cleaning
   \par hithresh image threshold
   \par lothresh border threshold
*/
void VImageCleaning::cleanImagePedvars(double hithresh, double lothresh, bool iSmall, bool iPad )
{
    cleanImagePedvars( hithresh, lothresh, lothresh, iSmall, iPad );
}


/*!
  signal-to-noise tailcut cleaning
   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
*/
void VImageCleaning::cleanImagePedvars(double hithresh, double lothresh, double brightthresh, bool iSmall, bool iPad )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::cleanImagePedvars " << fData->getTelID() << endl;
// calculates the vector of tubes to be included in the parameterization
    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );
    fData->setImageBorderNeighbour( false );
    unsigned int i_nchannel = fData->getNChannels();
    double i_pedvars_i = 0.;
    double i_pedvars_k = 0.;
    unsigned int k = 0;

    for ( unsigned int i = 0; i < i_nchannel; i++)
    {
        if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead(fData->getHiLo()[i])[i]) continue;
        i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i])[i];

        if( fData->getSums()[i] > hithresh * i_pedvars_i )
        {
            fData->setImage( i, true );
            fData->setBorder( i, false );
            for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
            {
                k = fData->getDetectorGeo()->getNeighbours()[i][j];
                if( k < i_nchannel )
                {
                    i_pedvars_k = fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k])[k];
                    if( !fData->getImage()[k] && fData->getSums()[k] > lothresh * i_pedvars_k ) fData->setBorder( k, true );
                }
            }
        }
        if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) fData->setBrightNonImage( i, true );
    }

// (preli) set the trigger vector in MC case (preli)
// trigger vector are image/border tubes
    if( fData->getReader() )
    {
       if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 || fData->getReader()->getDataFormatNum() == 6 ) fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
    }
    else cout << "XXXXXXXXXXXXXXXXXXXXXXXXXX " << fData->getReader() << endl;
// (end of preli)

    removeIsolatedPixels();
    fillImageBorderNeighbours();
}




/*!
  Image cleaning routine using pixel timing information
  based on Nepomuks PhD thesis time-cluster cleaning algorithm
   - uses fixed time differences for discrimination of pixels/clusters
   - adjusts time difference according to the time gradient
   - handles single core pixel
   - BrightNonImages not completely implemented yet (needs checks - but who is really using them?)

   \par hithresh image threshold
   \par lothresh border threshold
   \par brightthresh bright pixel threshold
   \par timeCutPixel time diffeence between pixels
   \par timeCutCluster time difference between clusters
   \par minNumPixel minimum number of pixels in a cluster
   \par loop_max number of loops
*/

void VImageCleaning::cleanImageFixedWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max )
{
    cleanImageWithTiming( hithresh, lothresh, brightthresh, timeCutPixel, timeCutCluster, minNumPixel, loop_max, true );
}

void VImageCleaning::cleanImagePedvarsWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max )
{
    cleanImageWithTiming( hithresh, lothresh, brightthresh, timeCutPixel, timeCutCluster, minNumPixel, loop_max, false );
}


void VImageCleaning::cleanImageWithTiming( double hithresh, double lothresh, double brightthresh, double timeCutPixel, double timeCutCluster, int minNumPixel, int loop_max, bool isFixed )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::cleanImageWithTiming " << fData->getTelID() << endl;

    // check if time gradient was already calculated
    if( fData->getImageParameters()->tgrad_x == 0 && fData->getImageParameters()->tint_x == 0 )
      {
	timeCutPixel = 5.0;
	timeCutCluster = fData->getNSamples();         // = number of readout samples
      }
    else 
      {
	float tpix = fData->getImageParameters()->tgrad_x * 0.15; // VERITAS pixel size (HARDCODED)
	if( tpix > timeCutPixel )
	  { 
	    timeCutPixel = tpix + 0.1;                     // added some uncertenties adhoc
	  }
      }
//     cout << "TimeImageCleaning: TC = " << timeCutPixel << " / " << timeCutCluster << " L = " << loop_max << endl;

    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );

    unsigned int i_nchannel = fData->getNChannels();
    
    //////////////////////////////////////////////////////
    // STEP 1: Select all pixels with a signal > hithresh 
    //

    double i_pedvars_i = 0.;
    for ( unsigned int i = 0; i < i_nchannel; i++)
    {
        if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead(fData->getHiLo()[i])[i]) continue;
	
	if( isFixed )
	{
	    if( fData->getSums()[i] > hithresh )     fData->setImage( i, true );
//	    if( fData->getSums()[i] > brightthresh ) fData->setBrightNonImage( i, true );
	}
	else
	{
	    i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i])[i];
	    if( fData->getSums()[i] > hithresh * i_pedvars_i )      fData->setImage( i, true );
//	    if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) fData->setBrightNonImage( i, true );
	}
    }

    //////////////////////////////////////////////////////////
    // STEP 2: Make clusters 
    //
    //       - group touching core pixels to clusters 
    //         (only if the pixels are coincident in time!)


    // REALLY NEEDED: in case there is still something in the memory, reset all channels to cluster id 0
    //                needs to be checked for memory leaks!!!
    for( unsigned int i = 0; i < i_nchannel; i++ ) fData->setClusterID( i, 0 );

    int i_cluster=0;
    int c_id=0;

    for( unsigned int i = 0; i < i_nchannel; i++ )
      {
	if( fData->getImage()[i] )
	  {
	    if( fData->getClusterID()[i] != 0 )
	      {
		c_id = fData->getClusterID()[i];
	      }
	    else if( i_cluster==0 || fData->getClusterID()[i] == 0 )
	      {
		i_cluster++;
		c_id = i_cluster;
	      }
	    else
	      cout << "WARNING: Something looks wrong - this should not happen\n" 
		   << "["<<i_cluster << "/" << c_id << "] " << i << endl;

	    fData->setClusterID( i, c_id );

	    for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
	      {
		unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
		if( fData->getImage()[k] && fData->getClusterID()[k] == 0 )
		  {
  		    if ( fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel ) 
 		      {
			fData->setClusterID( k, c_id );
 		      }
		  }
	      }
	  }
      }

    //////////////////////////////////////////////////////////
    // STEP 3: Calculate for each cluster weighted mean time 
    //
    //   - each cluster gets its own "clustertime" and "clustersize"
    //   - the cluster with the largest size is the "main" cluster
    //
    // new: for time difference adjustment calculate "cenx" and "ceny"

    int i_clusterNpix;      // number of pixels in cluster
    double i_clustersize;   // size of cluster 
    double i_clustertime;   // weighted mean time of all pixels in a cluster 

    double i_cenx, i_ceny;
    double i_clustercenx=0.;   // X center of gravity of the cluster
    double i_clusterceny=0.;   // Y center of gravity of the cluster

    double i_mainclustersize=0;   // size of the "main cluster"

    int cluster = 0;
    while( cluster <= i_cluster ){

      i_clusterNpix=0;
      i_clustersize=0.;
      i_clustertime=0.;
      i_clustercenx=0.; i_cenx=0.;
      i_clusterceny=0.; i_ceny=0.;

      for( unsigned int i = 0; i < i_nchannel; i++ )
	{
   	  if( fData->getClusterID()[i] == cluster && fData->getImage()[i] )
	    {
	      i_clusterNpix++; 

	      i_clustersize += fData->getSums()[i]; 
	      i_clustertime += ( fData->getSums()[i] * fData->getTZeros()[i] ); 
	      
	      double xi = fData->getDetectorGeo()->getX()[i];
	      double yi = fData->getDetectorGeo()->getY()[i];
	      
	      i_cenx += ( fData->getSums()[i] * xi );
	      i_ceny += ( fData->getSums()[i] * yi );
	    }
	}
      if( i_clustersize != 0 ) 
	{
	  i_clustertime = i_clustertime / i_clustersize;
	  i_clustercenx = i_cenx / i_clustersize;
	  i_clusterceny = i_ceny / i_clustersize;
	} 
      else 
	{ 
	  i_clustertime = -99; 
	  i_clustersize = -99; 
	  i_clustercenx = -99; 
	  i_clusterceny = -99;	
	}

      fData->setClusterNpix( cluster, i_clusterNpix );
      fData->setClusterSize( cluster, i_clustersize );
      fData->setClusterTime( cluster, i_clustertime );
      fData->setClusterCenx( cluster, i_clustercenx );
      fData->setClusterCeny( cluster, i_clusterceny );

//       cout << "### " << cluster << " Npix=" << i_clusterNpix << " size=" << i_clustersize << " time=" << i_clustertime 
// 	   << " cenx=" << i_clustercenx << " ceny=" << i_clusterceny << endl;

      if( i_clustersize == -99 ) fData->setMainClusterID( 0 );
      else if( i_clustersize >= i_mainclustersize )
        {
          i_mainclustersize = i_clustersize;
          fData->setMainClusterID( cluster );
        }
      cluster++;
    }
//     cout << "MAIN CLUSTER " << getMainClusterID()
// 	 << ": Npix=" << getClusterNpix()[ getMainClusterID() ] 
// 	 << " Size=" << getClusterSize()[ getMainClusterID() ] 
// 	 << " Time=" << getClusterTime()[ getMainClusterID() ] << endl;

    //////////////////////////////////////////////////////////////////////////////////////////
    // STEP 4: eliminate all clusters with time differences > Tcluster to the main cluster 
    //
    //         NEW: use tgrad_x to calculate Tcluster


    int i_ID = 0;

    float i_mainX=0.;      // c.o.g. (main cluster)
    float i_mainY=0.;      // c.o.g. (main cluster)
    float i_mainXpos=0.;   // position on major axis
    float i_mainXtime=0.;  // time on major axis

    float i_clusterX, i_clusterY;  // c.o.g. (cluster)
    float i_clusterXpos;           // position on major axis
    float i_clusterXtime;          // time on major axis

    if( fData->getImageParameters()->tgrad_x != 0 && fData->getImageParameters()->tint_x != 0 ) 
      {   
	i_mainX = fData->getClusterCenx()[ fData->getMainClusterID() ];
	i_mainY = fData->getClusterCeny()[ fData->getMainClusterID() ];
	i_mainXpos = i_mainX * fData->getImageParameters()->cosphi + i_mainY * fData->getImageParameters()->sinphi;
	i_mainXtime = fData->getImageParameters()->tgrad_x * i_mainXpos;
	
	i_clusterX = 0.;
	i_clusterY = 0.;
	i_clusterXpos = 0.;
	i_clusterXtime = 0.;

	cluster = 1;
	while( cluster <= i_cluster )
	  {
	    i_clusterX = fData->getClusterCenx()[ cluster ];
	    i_clusterY = fData->getClusterCeny()[ cluster ];
	    i_clusterXpos = i_clusterX*fData->getImageParameters()->cosphi + i_clusterY*fData->getImageParameters()->sinphi;
	    i_clusterXtime = fData->getImageParameters()->tgrad_x*i_clusterXpos;
	    
	    for( unsigned int i = 0; i < i_nchannel; i++ )
	      {
		i_ID = fData->getClusterID()[i];
		if( i_ID == 0 || i_ID == fData->getMainClusterID() ) continue;

		if( i_ID == cluster && fData->getImage()[i] )
		  {
// 		    cout<< "  " <<i_ID << "  delta_T  = " << fabs(xtime - xtimemain) << endl;
// 		    cout<< "  " <<i_ID << "  time diff= " << fabs( getClusterTime()[i_ID] - getClusterTime()[getMainClusterID()]) << endl;
// 		    cout<< i_ID << " " << i << ": " << getClusterTime()[i_ID] << "-" << getClusterTime()[getMainClusterID()] << "=" 
// 			<< getClusterTime()[i_ID] - getClusterTime()[getMainClusterID()]  
// 			<< " fabs(" << i_clusterXtime << "-" << i_mainXtime << "=" << i_clusterXtime - i_mainXtime << ") = " 
// 			<< fabs( (getClusterTime()[i_ID]-getClusterTime()[getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) << endl;
		    
		    if( fabs( (fData->getClusterTime()[i_ID]-fData->getClusterTime()[fData->getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) > timeCutCluster )
		      {
// 			cout << " --> "<< i << " cluster time cleaning : " 
// 			     << fabs( (getClusterTime()[i_ID]-getClusterTime()[getMainClusterID()]) - (i_clusterXtime - i_mainXtime) ) <<  endl;
			fData->setImage( i, false );
			fData->setClusterID( i, -99 );
		      }
		  }
	      }
	    cluster++;
	  }
      }
    else 
      {
	for( unsigned int i = 0; i < i_nchannel; i++ )
	  {
	    if( fData->getClusterID()[i] == 0 ) continue;
	    
	    if( fData->getImage()[i] )
	      {
		i_ID = fData->getClusterID()[i];
		i_pedvars_i = fData->getPedvars( fData->getCurrentSumWindow()[i], fData->getHiLo()[i])[i];
		
		if( i_ID != fData->getMainClusterID() && fabs( fData->getClusterTime()[i_ID] - fData->getClusterTime()[fData->getMainClusterID()] ) > timeCutCluster )
		  {
// 		    cout << "----> "<< i << " was removed with cluster time cleaning" << endl;
		    fData->setImage( i, false );
		    fData->setClusterID( i, -99 );
	      
// 	      if( fData->getSums()[i] > brightthresh  * i_pedvars_i ) setBrightNonImage( i, true );
//  	      if( fData->getSums()[i] > hithresh  * i_pedvars_i ) setBrightNonImage( i, true );
		  }
	      }
	  }
      }

    /////////////////////////////////////////////////////////////////    
    // STEP 5: find boundary pixels (and add them to the cluster) 
    // 
    //  selection criteria:
    //  - signal above lothresh
    //  - loop through already cleaned direct neighbors with time difference < Tpixel
    //
    //  this step can be reiterated more than once (number of loops)
    //
    
    for( int loop=0; loop < loop_max; loop++ )
      {
	int counter=0;
	int tmp_border[i_nchannel];
	int tmp_cluster[i_nchannel];

	double i_pedvars_k = 0.;
	for ( unsigned int i = 0; i < i_nchannel; i++)
	  {
// 	    if (fData->getDetectorGeo()->getAnaPixel()[i] < 1 || 
//  		fData->getClusterID()[i] == 0 || fData->getClusterID()[i] == -99 ) continue;
	    
	    i_ID = fData->getClusterID()[i];
	    if( fData->getImage()[i] || fData->getBorder()[i] )
	      {
		for( unsigned int j = 0; j < fData->getDetectorGeo()->getNNeighbours()[i]; j++ )
		  {
		    unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];

		    if( isFixed )
		    {
			if( !fData->getImage()[k] && !fData->getBorder()[k] && fData->getSums()[k] > lothresh 
			    && fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
			{
			    tmp_border[counter]=k;
			    tmp_cluster[counter]=i_ID;
			    counter++;
			}
		    }
		    else 
		    {
			i_pedvars_k = fData->getPedvars( fData->getCurrentSumWindow()[k], fData->getHiLo()[k])[k];
			if( !fData->getImage()[k] && !fData->getBorder()[k] 
			    && fData->getSums()[k] > lothresh * i_pedvars_k 
			    && fabs( fData->getTZeros()[i] - fData->getTZeros()[k] ) < timeCutPixel )
			{
			    tmp_border[counter]=k;
			    tmp_cluster[counter]=i_ID;
			    counter++;
			}
		    }
		  }
	      }
	  }
	if( fData->getMainClusterID() != 0 )
	  {
	    //  	    cout << "setBorder: ";
	    for( int pixel = 0; pixel < counter; pixel++ )
	      {
  		//cout << tmp_border[pixel] << "(" << tmp_cluster[pixel] << ") , ";
		fData->setBorder( tmp_border[pixel], true );
 		fData->setClusterID( tmp_border[pixel], tmp_cluster[pixel] );
	      }
  	    //cout << endl;
	  }
      }

    /////////////////////////////////////////////////////////////////////
    // STEP 6: merge touching clusters and reset cluster parameters 
    //

    mergeClusters();

//     for( unsigned int i = 0; i < i_nchannel; i++ ) setClusterID( i, 0 );
    for( int x=0; x <= i_cluster; x++ )
      {
	fData->setClusterNpix( x, 0 ); 
	fData->setClusterSize( x, 0 );
	fData->setClusterTime( x, 0 );
      } 
   
    i_mainclustersize=0;
    cluster = 1;

    while( cluster <= i_cluster ){

      i_clusterNpix=0;
      i_clustersize=0.;
      i_clustertime=0.;

      for( unsigned int i = 0; i < i_nchannel; i++ )
	{
 	  if( fData->getClusterID()[i] == cluster )
	    {
	      if( fData->getImage()[i] || fData->getBorder()[i] ) 
		{
		  i_clusterNpix++;
		  i_clustersize += fData->getSums()[i];
		  i_clustertime += fData->getSums()[i] * fData->getTZeros()[i];
		} 
	      else fData->setClusterID( i, -99 );
	    }
	}
      if( i_clustersize != 0 ) i_clustertime = i_clustertime/i_clustersize;
      else { i_clustertime = -99.; i_clustersize = -99; }

      fData->setClusterNpix( cluster, i_clusterNpix );
      fData->setClusterSize( cluster, i_clustersize );
      fData->setClusterTime( cluster, i_clustertime );

//       if( i_clusterNpix > 0 )
// 	cout << "     CLUSTER " <<cluster<< ": Npix=" <<i_clusterNpix<< " Size=" <<i_clustersize<< " Time=" << i_clustertime << endl;

      if( i_clustersize == -99 ) fData->setMainClusterID( 0 );
      else if( i_clustersize >= i_mainclustersize )
        {
          i_mainclustersize = i_clustersize;
          fData->setMainClusterID( cluster );
        }
      cluster++;
    }

    //  count number of clusters before removing (useful for hadron rejection?)
    set< int > tmp_counter_uncleaned;
    for( unsigned int i = 0; i < fData->getNChannels(); i++)
      {  
	if ( fData->getImage()[i] || fData->getBorder()[i] )
	  {
	    i_ID = fData->getClusterID()[i]; 
	    if ( i_ID != 0 || i_ID != -99 ) 
	      {
		tmp_counter_uncleaned.insert( i_ID );
	      }
	  }
      }
    fData->setNcluster_uncleaned( tmp_counter_uncleaned.size() );

    /////////////////////////////////////////////////////////////
    // STEP 7: eliminate clusters with less then XX pixels 
    //         & clusters where one core pixel has less then 2 direct border pixels
    
    removeSmallClusters( minNumPixel );


    /////////////////////////////////////////////////////////////
    // FINAL STEP: 
    //
    //  - count number of clusters (useful for hadron rejection?)

    set< int > tmp_counter_cleaned;
    for( unsigned int i = 0; i < fData->getNChannels(); i++)
      {  
	if ( fData->getImage()[i] || fData->getBorder()[i] )
	  {
	    i_ID = fData->getClusterID()[i]; 
	    if ( i_ID != 0 || i_ID != -99 ) 
	      {
		tmp_counter_cleaned.insert( i_ID );
	      }
	  }
      }
    fData->setNcluster_cleaned( tmp_counter_cleaned.size() );



//       cout << "BRIGHT NON IMAGES : ";
//       for( unsigned int i = 0; i < i_nchannel; i++ )
//       {
//       // check for bright non images
//       if( fData->getBrightNonImage()[i] )
//       {
//       if( fData->getImage()[i] ) setBrightNonImage( i, false );
//       else if( fData->getBorder()[i] ) setBrightNonImage( i, false );
//       }
      
//       if( fData->getBrightNonImage()[i] )
//       cout << i << ", ";
//       }
//       cout << endl;


    if( fData->getReader()->getDataFormatNum() == 1 || fData->getReader()->getDataFormatNum() == 4 || fData->getReader()->getDataFormatNum() == 6 ) 
      fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
    

    fillImageBorderNeighbours();
}


void VImageCleaning::mergeClusters()
{
  int i_clusterID;
  int k_clusterID;

  for( unsigned int i = 0; i < fData->getNChannels(); i++)
    {  
      if ( fData->getImage()[i] || fData->getBorder()[i] ) 
	{
	  i_clusterID = fData->getClusterID()[i];
	  unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
	  
	  for( unsigned int j = 0; j < i_neighbour_size; j++ )
	    {
	      unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
	      k_clusterID = fData->getClusterID()[k];
	      
	      if ( (fData->getImage()[k]||fData->getBorder()[k]) && k_clusterID !=0 && k_clusterID != fData->getClusterID()[i] ) 
		{
		  for( unsigned int n = 0; n < fData->getNChannels(); n++)
		    {
		      if ( (fData->getImage()[n]||fData->getBorder()[n]) && fData->getClusterID()[n] == fData->getClusterID()[k] ) 
			{
			  fData->setClusterID( n, i_clusterID );
			}
		    }
		}
	    }
	}
    }
}


void VImageCleaning::removeSmallClusters( int minPix )
{
  int i_cluster = 0;

  for( unsigned int i = 0; i < fData->getNChannels(); i++ )
    {
      i_cluster = fData->getClusterID()[i];
      if( i_cluster == 0 || i_cluster == -99 ) continue;
      
// remove clusters with less then minPix
      if( fData->getClusterNpix()[i_cluster] < minPix )
	{
// 	  if( fData->getImage()[i] || fData->getBorder()[i] ) cout << "ELIMINATOR ACTIVE for cluster " << i_cluster << " : ";
	  
	  if( fData->getImage()[i] )
	    {
// 	      cout << i << ", ";
	      fData->setImage( i, false );
	      fData->setClusterID( i, -99 );
	    }
	  else if( fData->getBorder()[i] )
	    {
// 	      cout << i << ", ";
	      fData->setBorder( i, false );
	      fData->setClusterID( i, -99 );
	    }
// 	  cout << endl;
	}
	  
// remove single core pixels with less than two direct border pixels
      int c1=0;
      int c2=0;

      bool dont_remove = false;
      
      if( fData->getImage()[i] )
	{
	  unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
	  for( unsigned int j = 0; j < i_neighbour_size; j++ )
	    {
	      unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
	      if( fData->getImage()[k] ) 
		{
		  dont_remove=true;
		}
	      else if ( fData->getBorder()[k] )
		{
		  c1++;
		}
	      else if( k < fData->getDead().size() && fData->getDead(fData->getHiLo()[k])[k] )
		{
		  for( unsigned l = 0; l < i_neighbour_size; l++ )
		    {
		      unsigned int m = fData->getDetectorGeo()->getNeighbours()[i][l];
		      if ( m != i && m < fData->getBorder().size() && (fData->getBorder()[m] || fData->getImage()[m]) )
			{
			  c2++;
			}
		    }
		}
	    }
	  if( dont_remove ) continue;
	}
      
      if( c1+c2 < 2 && fData->getImage()[i] )
	{
// 	  cout << "----> "<< i << " was removed as single core pixel (incl ";
	  fData->setImage( i, false );
	  fData->setBorder( i, false );
	  fData->setBrightNonImage( i, true );
	  fData->setClusterID( i, -99 );
	  
// remove the rest of the single core cluster (if it exists)
	  unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
	  for( unsigned int j = 0; j < i_neighbour_size; j++ )
	    {
	      unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
	      if ( fData->getBorder()[k] )
		{
		  fData->setBorder( k, false );
// 		  cout << k << " ";
		  
		  for( unsigned l = 0; l < fData->getDetectorGeo()->getNNeighbours()[k]; l++ )
		    {
		      unsigned int m = fData->getDetectorGeo()->getNeighbours()[k][l];
		      if ( fData->getBorder()[m] )
			{
			  fData->setBorder( m, false );
// 			  cout << m << " ";
			}
		    }
		}
	    }
// 	  cout << ")" << endl;
	}
    }
  
}


/*
  HP END
*/


void VImageCleaning::fillImageBorderNeighbours()
{
    fData->setImageBorderNeighbour( false );
// assume fData->getImageBorderNeighbour() are all false
    for( unsigned int i = 0; i < fData->getNChannels(); i++)
    {
        if ( fData->getImage()[i] || fData->getBorder()[i] )
        {
// a pixel is its own neighbour :-)
            fData->getImageBorderNeighbour()[i] = true;

            unsigned int i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
            for( unsigned int j = 0; j < i_neighbour_size; j++ )
            {
                unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
                if( k < fData->getImageBorderNeighbour().size() && !fData->getDead()[k] ) fData->getImageBorderNeighbour()[k] = true;
            }
        }
    }
}


void VImageCleaning::removeIsolatedPixels()
{
// loop again to remove isolated image pixels
// if neighbour is dead, check neighbours of this dead channel (see e.g. run 329 event 709)
    bool i_neigh = false;
    unsigned int i_neighbour_size = 0;
    unsigned int k = 0;

    for( unsigned int i = 0; i < fData->getNChannels(); i++)
    {
        if ( fData->getImage()[i])
        {
            i_neigh = false;
            i_neighbour_size = fData->getDetectorGeo()->getNNeighbours()[i];
            for( unsigned int j = 0; j < i_neighbour_size; j++ )
            {
                k = fData->getDetectorGeo()->getNeighbours()[i][j];
                if ( k < fData->getBorder().size() && ( fData->getBorder()[k] ||fData->getImage()[k] ) )
                {
                    fData->setImage( i, true );
                    i_neigh = true;
                    break;
                }
                else if( k < fData->getDead().size() && fData->getDead(fData->getHiLo()[k])[k] )
                {
                    for( unsigned l = 0; l < i_neighbour_size; l++ )
                    {
                        unsigned int m = fData->getDetectorGeo()->getNeighbours()[i][l];
                        if ( m != i && m < fData->getBorder().size() && (fData->getBorder()[m] || fData->getImage()[m]) )
                        {
                            fData->setImage( i, true );
                            i_neigh = true;
                            break;
                        }
                    }
                }
                if( !i_neigh ) fData->setImage( i, false );
            }
        }
        if( fData->getBrightNonImage()[i] )
        {
            if( fData->getImage()[i] ) fData->setBrightNonImage( i, false );
            else if( fData->getBorder()[i] ) fData->setBrightNonImage( i, false );
        }
    }

// check if any channels are user enabled/disabled
// (GM) don't know what this does
/*  if( fData->getRunParameter()->frunmode != 4 )
  {
     for (unsigned int i=0; i<fData->getNChannels(); i++)
     {
     if( fData->getImageUser()[i] == 1 )
     {
         setImage( i, true );
         setBorder( i, false );
     }
     else if( fData->getImageUser()[i] == -1 )
     {
setImage( i, false );
setBorder( i, false );
}
}
} */
}


void VImageCleaning::cleanImage_clusterCleaning( double threshold_clustersize )
{
// calculates the valarray of tubes to be included in the parameterization
    fData->setImage( false );
    fData->setBorder( false );
    fData->setBrightNonImage( false );
    fData->setImageBorderNeighbour( false );
    unsigned int i_nchannel = fData->getNChannels();

    for ( unsigned int i=0; i < i_nchannel; i++)
    {
        if( fData->getDetectorGeo()->getAnaPixel()[i] < 1 || fData->getDead(fData->getHiLo()[i])[i] ) continue;

// calculate size of this cluster
        double iClusterSize = fData->getSums()[i];

        for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
        {
            unsigned int k = fData->getDetectorGeo()->getNeighbours()[i][j];
            if( fData->getDetectorGeo()->getAnaPixel()[k] < 1 || fData->getDead(fData->getHiLo()[k])[k] ) continue;

            if( k < fData->getImage().size() ) iClusterSize += fData->getSums()[k];
        }
        if( iClusterSize > threshold_clustersize ) fData->setImage( i, true );
    }
// (preli) set the trigger vector in MC case (preli)
// trigger vector are image/border tubes
    if( fData->getReader()->getDataFormat() == "grisu" || fData->getReader()->getDataFormat() == "DST" ) fData->getReader()->setTrigger( fData->getImage(), fData->getBorder() );
// (end of preli)

    removeIsolatedPixels();
    fillImageBorderNeighbours();
}


//
// MS: produce a trigger map for calculation of  binary-image Hillas parameters and apply some sort of cleaning algorithm to reject noisy pixels
//
void VImageCleaning::cleanTriggerFixed( double hithresh, double lothresh )
{
// MS: produce a trigger-level cleaned image. The logic here is that one might want to require
// only patches of 3 nearest neighbors to constitute a valid center pixel.
    fData->setTrigger( false );
    unsigned int i_nchannel = fData->getNChannels();

    for( unsigned int i=0; i< i_nchannel; i++ )
    {
        if( fData->getDetectorGeo()->getAnaPixel()[i] > 0  && !fData->getDead(fData->getHiLo()[i])[i] )
        {
                                                  // use the CFD hits
            if( fData->getRunParameter()->fPWmethod == 0 || fData->getRunParameter()->fPWmethod == 1 )
            {
                if( fData->getReader()->getFullTrigVec()[i]  )
                {
// ensure that at least XX neighbors are above this threshold
                    int local_neighbors = 0;
                    for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                    {
                        unsigned int k=fData->getDetectorGeo()->getNeighbours()[i][j];
                        if( k < fData->getTrigger().size()  && fData->getReader()->getFullTrigVec()[k]  )
                            local_neighbors++;
                    }
                    if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors )
                        fData->setTrigger( i, true );
                }
            }

                                                  // use a software cleaning algorithm for determining the trigger hits from the FADCs
            if( fData->getRunParameter()->fPWmethod == 2 || fData->getRunParameter()->fPWmethod == 3 )
            {
                if( fData->getSums()[i] > fData->getRunParameter()->fPWcleanThreshold )
                {
// ensure that at least XX neighbors are above this threshold
                    int local_neighbors = 0;
                    for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                    {
                        unsigned int k=fData->getDetectorGeo()->getNeighbours()[i][j];
                        if( k < fData->getTrigger().size()  && fData->getSums()[k] > fData->getRunParameter()->fPWcleanThreshold  )
                            local_neighbors++;
                    }
                    if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors )
                        fData->setTrigger( i, true );
                }
            }
        }
    }

                                                  // then include the next row of pixels that "made" the center pixel alive
    if( fData->getRunParameter()->fPWmethod == 1 || fData->getRunParameter()->fPWmethod == 3 )
    {
        vector< bool > tmp_trigger( i_nchannel, 0 );

        for( unsigned int i=0; i< i_nchannel; i++ )
        {
            if( fData->getDetectorGeo()->getAnaPixel()[i] > 0  && !fData->getDead(fData->getHiLo()[i])[i] )
            {

                if( fData->getReader()->getFullTrigVec()[i]  )
                {
                    int local_neighbors = 0;
                    for( unsigned int j=0; j < fData->getDetectorGeo()->getNeighbours()[i].size(); j++ )
                    {
                        unsigned int k=fData->getDetectorGeo()->getNeighbours()[i][j];
                        if( k < fData->getTrigger().size()  &&  fData->getTrigger()[k]  )
                            local_neighbors++;
                    }
                    if( local_neighbors >= fData->getRunParameter()->fPWcleanNeighbors - 1 )
                        tmp_trigger[i] = 1;
                }
            }
        }

        for( unsigned int i=0; i< i_nchannel; i++ )
        {
            fData->setTrigger( i, (tmp_trigger[i] | fData->getTrigger()[i]) );
        }

    }

// assume that the image has been cleaned according to spec, then a new parameter will decide
// wether  to apply a reduction in the number of pixels transmitted:
// This will mask all pixels past the cut-off to be ZERO

    if( fData->getRunParameter()->fPWlimit > 0 )                   // then we'll transmit up to a fixed number of pixels per sector
    {
        const int n_sectors = 3;
        const int sector_max = 182;               // there are 192 channels per trigger sector, but not all are used
        int sector_count[n_sectors] = {0};

        for( int i=0; i<n_sectors; i++ )
        {
            for( int j=0; j<sector_max; j++ )
            {
                int current_pixel = sector_channel_to_pixel[j][i+1]  -1 ;

                if(    fData->getTrigger()[ current_pixel  ]    )
                    sector_count[ i  ]++;

                if( sector_count[i] > fData->getRunParameter()->fPWlimit )
                {
                                                  // then turn the pixel OFF, regardless wether or not it was ON
                    fData->setTrigger( current_pixel, 0 );
                }
            }

        }

    }

}


void VImageCleaning::addImageChannel( unsigned int i_channel )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::addImageChannel" << endl;
    if( i_channel < fData->getImage().size() ) fData->setImageUser( i_channel, 1 );
}


void VImageCleaning::removeImageChannel( unsigned int i_channel )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::removeImageChannel" << endl;
    if( i_channel < fData->getImage().size() ) fData->setImageUser( i_channel, -1 );
}


void VImageCleaning::resetImageChannel( unsigned int i_channel )
{
    if( fData->getDebugFlag() ) cout << "VImageCleaning::resetImageChannel" << endl;
    if( i_channel < fData->getImage().size() ) fData->setImageUser( i_channel, 0 );
}

