/* \file  atmosphericSounding.C
   \brief plot ballon data from soundings measurements compared with CORSIKA and MODTRAN values

   *** might need some attention ***

   \author Gernot Maier
*/

/*
      compare yearly data with CORSIKA/MODTRAN atmospheres
*/
void plot_years( bool bMODTRAN = false )
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(true);

    if( !bMODTRAN )
    {
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 2 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );
    }
    else
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 1 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/tropical.profile.tp6", "TROPICAL (MODTRAN)", 2 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudesummer.profile.tp6", "midlatitude summer (MODTRAN)", 3 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4 );
    }

    a.setPlottingPeriod( "yearly" );
    a.plotAverages( 1995, 1, 2010, 12 );
}

/*
   plot a 2D distribution and plot values from CORSIKA/MODTRAN on top of it
*/
void plot_2D( bool bMODTRAN = false )
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(false);

    if( !bMODTRAN )
    {
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 2 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );
    }
    else
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 1 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/tropical.profile.tp6", "TROPICAL (MODTRAN)", 2 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudesummer.profile.tp6", "midlatitude summer (MODTRAN)", 3 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4 );
    }

//    a.setPlottingPeriod( "all" );
    a.plot2DProfiles( 1995, 1, 2010, 12 );
}

/*!
      compare monthly data with CORSIKA/MODTRAN atmospheres
*/
void plot_monthly( bool bMODTRAN )
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(true);

    if( !bMODTRAN )
    {
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 2 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );
    }
    else
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 1 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/tropical.profile.tp6", "TROPICAL (MODTRAN)", 2 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudesummer.profile.tp6", "midlatitude summer (MODTRAN)", 3 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4 ); 
    }

    a.setPlottingPeriod( "monthly_all" );
    a.plotAverages( 1995, 1, 2010, 12 );
}

/*!
      compare bi-monthly data with CORSIKA/MODTRAN atmospheres
*/
void plot_bimonthly()
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(true);

    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 2 );
    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );

    a.setPlottingPeriod( "bimonthly_all" );
    a.plotAverages( 1995, 1, 2010, 12 );
}

/*!
     plot atmospheric profiles during Crab Observations
*/
void plot_CrabPeriods( bool bMODTRAN = false, double iMax = 30., bool bUser = false )
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(true);
    a.setPlottingRangeHeight( 0., iMax );

    if( !bUser )
    {
       if( !bMODTRAN )
       {
	  a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 2 );
	  a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
	  a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
	  a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );
       }
       else
       {
	  a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 1 );
	  a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/tropical.profile.tp6", "TROPICAL (MODTRAN)", 2 );
	  a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudesummer.profile.tp6", "midlatitude summer (MODTRAN)", 3 );
	  a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4 );
       }
    }
    else
    {
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard (CORSIKA)", 3, 1 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 4, 1 );
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.50km.VCrab.profile.tp6", "VCrab", 2, 1 );
    }

    a.setPlottingPeriod( "atmoCrab.dat" );
    a.plotAverages( 1995, 1, 2010, 12 );
}


/*
   compare different atmospheres from CORSIKA (possibly with MODTRAN)
*/
void compareCORSIKA( double iHeightMax = 120., bool iModtran = false )
{
   VAtmosphereSoundings a;

   if( !iModtran )
   {
      a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 1 );
      a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof20.dat", "VERITAS Crab", 3 );
      a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof21.dat", "VERITAS Winter", 4 );
      a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof22.dat", "VERITAS Summer", 6 );
/*   a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
   a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
   a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 ); */
   }
   else
   {
      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 1 );
      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/V.Crab.US76.50km.profile.tp6", "VERITAS Crab (MODTRAN)", 3 );
      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/V.Winter.US76.50km.profile.tp6", "VERITAS Winter (MODTRAN)", 4 );
      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/V.Summer.US76.50km.profile.tp6", "VERITAS Summer (MODTRAN)", 6 );
/*      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/tropical.profile.tp6", "TROPICAL (MODTRAN)", 2 );
      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudesummer.profile.tp6", "midlatitude summer (MODTRAN)", 3 );
      a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4 ); */
   }

   a.plotCORSIKA_Density_vs_Heigth(0, 0., iHeightMax );
   a.plotCORSIKA_IndexofRefraction_vs_Heigth( 0, 0., iHeightMax );
   if( iModtran )
   {
      a.plotCORSIKA_Temperature_vs_Heigth( 0, 0., iHeightMax );
      a.plotCORSIKA_Ozone_vs_Heigth( 0, 0., iHeightMax );
      a.plotCORSIKA_RelativeHumidity_vs_Heigth( 0, 0., iHeightMax );
      a.plotCORSIKA_DewPoint_vs_Heigth( 0, 0., iHeightMax );
   }
}

/*!
      compare monthly data with CORSIKA and MODTRAN atmospheres
*/
void plot_monthly( double iMax = 30. ) 
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(true);
    a.setPlottingRangeHeight( 0., iMax );

    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US Standard", 2 );
    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof1.dat", "tropical", 3 );
    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof2.dat", "midlatitude summer", 4 );
    a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );

    a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 1, 2 );
/*    a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/tropical.profile.tp6", "TROPICAL (MODTRAN)", 2, 2 );
    a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudesummer.profile.tp6", "midlatitude summer (MODTRAN)", 3, 2 );
    a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4, 2 );  */
    

    a.setPlottingPeriod( "monthly_all" );
//    a.setPlottingPeriod( "day_night" );
    a.plotAverages( 1995, 1, 2010, 12 );
}

/*!
     function to create MODTRAN input for user defined atmosphere (card 2C1 in .tp5 file)



*/
void write_and_plot_MODTRAN( unsigned int iPeriod = 0, double iHeightMaxData = -1., double iMax = 30., string iName = "userprof" )
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");

    int iDefaultModel = 0;
    if( iPeriod == 0 )
    {
        a.setPlottingPeriod( "atmoCrab.dat" );
	if( iHeightMaxData < 0. ) iHeightMaxData = 30.;
	iDefaultModel = 6;                                   // Crab period: default model is 6 (US 76)
    }
    else if( iPeriod == 1 )
    {
        a.setPlottingPeriod( "winter" );
	if( iHeightMaxData < 0. ) iHeightMaxData = 37.5;
	iDefaultModel = 6;                                   // winter period: default model is 6 (US 76)
    }
    else if( iPeriod == 2 )
    {
        a.setPlottingPeriod( "summer" );
	if( iHeightMaxData < 0. ) iHeightMaxData = 35.0;
	iDefaultModel = 6;                                   // summer period: default model is 6 (US 76)
    }

    if( iPeriod == 0 )
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 4 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US 76  (CORSIKA)", 6 );
    }
    else if( iPeriod == 1 )
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter (MODTRAN)", 4 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6 );
    }
    else if( iPeriod == 2 )
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76 (MODTRAN)", 4 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof6.dat", "US 76  (CORSIKA)", 6 );
    }

// create and print user atmosphere
    a.add_user_Atmosphere( 0, iHeightMaxData, iName );
    a.write_MODTRAN_UserProfile( 0, iDefaultModel, false );

// user created atmosphere
//    a.add_MODTRAN_Atmosphere( "../../../simulation/showerSimulation/data/V.Winter.US76.50km.profile.tp6", "USER", 2, 2 );
    a.add_MODTRAN_Atmosphere( "../../../simulation/showerSimulation/data/V.Summer.US76.50km.profile.tp6", "USER", 2, 2 );

// now plot everything
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(true);
    a.setPlottingRangeHeight( 0., iMax );

// plotting
    a.plotAverages( 1995, 1, 2010, 12 );

// index of refraction
    TCanvas *cIndexOfRefraction = a.plotCORSIKA_IndexofRefraction_vs_Heigth( 0, 0., iMax );

//    a.plotUserAtmosphere_IndexofRefraction_vs_Heigth( cIndexOfRefraction, 0., iMax );

}

void write_and_plot_CORSIKA_user_profiles( unsigned int iPeriod = 0, double iMax = 30. )
{
    VAtmosphereSoundings a("$VERITAS_DATA_DIR/atmosphere/ballonDataTucson_199501_201007.root");

    string iName = "";
    unsigned int iAtmProf = 0;
    if( iPeriod == 0 )
    {
        a.setPlottingPeriod( "atmoCrab.dat" );
	iAtmProf = 20;
	iName = "V.Crab.US76.50km";
    }
    else if( iPeriod == 1 )
    {
        a.setPlottingPeriod( "winter" );
	iAtmProf = 21;
	iName = "V.Winter.US76.50km";
    }
    else if( iPeriod == 2 )
    {
        a.setPlottingPeriod( "summer" );
	iAtmProf = 22;
	iName = "V.Summer.US76.50km";
    }

    if( iPeriod == 0 )
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76", 4, 2 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof20.dat", "VERITAS Crab (atmprof)", 6, 2 );
       a.add_MODTRAN_Atmosphere( "../../../simulation/showerSimulation/data/V.Crab.US76.50km.profile.tp6", "VERITAS Crab", 2, 1 );
    }
    else if( iPeriod == 1 )
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/midlatitudewinter.profile.tp6", "midlatitude winter", 4, 2 );
//       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof3.dat", "midlatitude winter", 6, 2 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof21.dat", "VERITAS winter", 6, 2 );
       a.add_MODTRAN_Atmosphere( "../../../simulation/showerSimulation/data/V.Winter.US76.50km.profile.tp6", "VERITAS Winter", 2 );
    }
    else if( iPeriod == 2 )
    {
       a.add_MODTRAN_Atmosphere("../../../simulation/showerSimulation/data/us76.profile.tp6", "US 76", 4, 2 );
       a.add_CORSIKA_Atmosphere("../../../simulation/showerSimulation/data/atmprof22.dat", "VERITAS Summer", 6, 2 );
       a.add_MODTRAN_Atmosphere( "../../../simulation/showerSimulation/data/V.Summer.US76.50km.profile.tp6", "VERITAS Summer", 2 );
    }

// now plot everything
    a.plotAttributes_ColorChange(true);
    a.plotAttributes_PlotLegend(false);
    a.setPlottingRangeHeight( 0., iMax );

// plotting
    a.plotAverages( 1995, 1, 2010, 12 );

// index of refraction
    TCanvas *cIndexOfRefraction = a.plotCORSIKA_IndexofRefraction_vs_Heigth( 0, 0., iMax );

// write corsika atmosphere
    a.write_CORSIKA_UserProfile( 2, iAtmProf, iName );


}

   
