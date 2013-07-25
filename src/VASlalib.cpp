/********************************************************************
 * \file VASlalib.cpp
 * \ingroup common
 * \brief Astronomical tools used by the sp24 analysis package
 *
 * Original Author: Pascal Fortin
 * $Author: gmaier $
 * $Date: 2010/03/08 07:39:53 $
 * $Tag$
 *******************************************************************/

// These deinitions tell the makefile which library the cpp file
// should be included in
// VA_LIBRARY_TAG: libSP24common.a
// VA_LIBRARY_TAG: libSP24commonLite.a

#include "VASlalib.h"

double slaPa ( double ha, double dec, double phi )
/*+
 **  - - - - - -
 **   s l a P a
 **  - - - - - -
 **
 **  HA, Dec to Parallactic Angle.
 **
 **  (double precision)
 **
 **  Given:
 **     ha     d     hour angle in radians (geocentric apparent)
 **     dec    d     declination in radians (geocentric apparent)
 **     phi    d     observatory latitude in radians (geodetic)
 **
 **  The result is in the range -pi to +pi
 **
 **  Notes:
 **
 **  1)  The parallactic angle at a point in the sky is the position
 **      angle of the vertical, i.e. the angle between the direction to
 **      the pole and to the zenith.  In precise applications care must
 **      be taken only to use geocentric apparent HA,Dec and to consider
 **      separately the effects of atmospheric refraction and telescope
 **      mount errors.
 **
 **  2)  At the pole a zero result is returned.
 **
 **  Last revision:   16 August 1994
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double cp, cqsz, sqsz;

    cp = cos ( phi );
    sqsz = cp * sin ( ha );
    cqsz = sin ( phi ) * cos ( dec) - cp * sin ( dec) * cos ( ha );
    return ( ( sqsz != 0.0 || cqsz != 0.0 ) ? atan2 ( sqsz, cqsz ) : 0.0 );
}


void slaCldj ( int iy, int im, int id, double *djm, int *j )
/*
 **  - - - - - - - -
 **   s l a C l d j
 **  - - - - - - - -
 **
 **  Gregorian calendar to Modified Julian Date.
 **
 **  Given:
 **     iy,im,id     int    year, month, day in Gregorian calendar
 **
 **  Returned:
 **     *djm         double Modified Julian Date (JD-2400000.5) for 0 hrs
 **     *j           int    status:
 **                           0 = OK
 **                           1 = bad year   (MJD not computed)
 **                           2 = bad month  (MJD not computed)
 **                           3 = bad day    (MJD computed)
 **
 **  The year must be -4699 (i.e. 4700BC) or later.
 **
 **  The algorithm is derived from that of Hatcher 1984 (QJRAS 25, 53-55).
 **
 **  Last revision:   29 August 1994
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    long iyL, imL;

/* Month lengths in days */
    static int mtab[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Validate year */
    if ( iy < -4699 ) { *j = 1; return; }

/* Validate month */
    if ( ( im < 1 ) || ( im > 12 ) ) { *j = 2; return; }

/* Allow for leap year */
    mtab[1] = ( ( ( iy % 4 ) == 0 ) &&
        ( ( ( iy % 100 ) != 0 ) || ( ( iy % 400 ) == 0 ) ) ) ?
        29 : 28;

/* Validate day */
    *j = ( id < 1 || id > mtab[im-1] ) ? 3 : 0;

/* Lengthen year and month numbers to avoid overflow */
    iyL = (long) iy;
    imL = (long) im;

/* Perform the conversion */
    *djm = (double)
        ( ( 1461L * ( iyL - ( 12L - imL ) / 10L + 4712L ) ) / 4L
        + ( 306L * ( ( imL + 9L ) % 12L ) + 5L ) / 10L
        - ( 3L * ( ( iyL - ( 12L - imL ) / 10L + 4900L ) / 100L ) ) / 4L
        + (long) id - 2399904L );
    return;
}


void slaClyd ( int iy, int im, int id, int *ny, int *nd, int *jstat )
/*
 **  - - - - - - - -
 **   s l a C l y d
 **  - - - - - - - -
 **
 **  Gregorian calendar to year and day in year (in a Julian calendar
 **  aligned to the 20th/21st century Gregorian calendar).
 **
 **  Given:
 **     iy,im,id     int    year, month, day in Gregorian calendar
 **
 **  Returned:
 **     ny          int    year (re-aligned Julian calendar)
 **     nd          int    day in year (1 = January 1st)
 **     jstat       int    status:
 **                          0 = OK
 **                          1 = bad year (before -4711)
 **                          2 = bad month
 **                          3 = bad day (but conversion performed)
 **
 **  Notes:
 **
 **  1  This routine exists to support the low-precision routines
 **     slaEarth, slaMoon and slaEcor.
 **
 **  2  Between 1900 March 1 and 2100 February 28 it returns answers
 **     which are consistent with the ordinary Gregorian calendar.
 **     Outside this range there will be a discrepancy which increases
 **     by one day for every non-leap century year.
 **
 **  3  The essence of the algorithm is first to express the Gregorian
 **     date as a Julian Day Number and then to convert this back to
 **     a Julian calendar date, with day-in-year instead of month and
 **     day.  See 12.92-1 and 12.95-1 in the reference.
 **
 **  Reference:  Explanatory Supplement to the Astronomical Almanac,
 **              ed P.K.Seidelmann, University Science Books (1992),
 **              p604-606.
 **
 **  Last revision:   26 November 1994
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    long i, j, k, l, n, iyL, imL;

/* Month lengths in days */
    static int mtab[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Validate year */
    if ( iy < -4711 ) { *jstat = 1; return; }

/* Validate month */
    if ( ( im < 1 ) || ( im > 12 ) ) { *jstat = 2; return; }

/* Allow for (Gregorian) leap year */
    mtab[1] = ( ( ( iy % 4 ) == 0 ) &&
        ( ( ( iy % 100 ) != 0 ) || ( ( iy % 400 ) == 0 ) ) ) ?
        29 : 28;

/* Validate day */
    *jstat = ( id < 1 || id > mtab[im-1] ) ? 3 : 0;

/* Perform the conversion */
    iyL = (long) iy;
    imL = (long) im;
    i = ( 14 - imL ) /12L;
    k = iyL - i;
    j = ( 1461L * ( k + 4800L ) ) / 4L
        + ( 367L * ( imL - 2L + 12L * i ) ) / 12L
        - ( 3L * ( ( k + 4900L ) / 100L ) ) / 4L + (long) id - 30660L;
    k = ( j - 1L ) / 1461L;
    l = j - 1461L * k;
    n = ( l - 1L ) / 365L - l / 1461L;
    j = ( ( 80L * ( l - 365L * n + 30L ) ) / 2447L ) / 11L;
    i = n + j;
    *nd = 59 + (int) ( l -365L * i + ( ( 4L - n ) / 4L ) * ( 1L - j ) );
    *ny = (int) ( 4L * k + i ) - 4716;
}


void slaCalyd ( int iy, int im, int id, int *ny, int *nd, int *j )
/*
 **  - - - - - - - - -
 **   s l a C a l y d
 **  - - - - - - - - -
 **
 **  Gregorian calendar date to year and day in year (in a Julian
 **  calendar aligned to the 20th/21st century Gregorian calendar).
 **
 **  (Includes century default feature:  use slaClyd for years
 **   before 100AD.)
 **
 **  Given:
 **     iy,im,id   int    year, month, day in Gregorian calendar
 **                       (year may optionally omit the century)
 **  Returned:
 **     *ny        int    year (re-aligned Julian calendar)
 **     *nd        int    day in year (1 = January 1st)
 **     *j         int    status:
 **                         0 = OK
 **                         1 = bad year (before -4711)
 **                         2 = bad month
 **                         3 = bad day (but conversion performed)
 **
 **  Notes:
 **
 **  1  This routine exists to support the low-precision routines
 **     slaEarth, slaMoon and slaEcor.
 **
 **  2  Between 1900 March 1 and 2100 February 28 it returns answers
 **     which are consistent with the ordinary Gregorian calendar.
 **     Outside this range there will be a discrepancy which increases
 **     by one day for every non-leap century year.
 **
 **  3  Years in the range 50-99 are interpreted as 1950-1999, and
 **     years in the range 00-49 are interpreted as 2000-2049.
 **
 **  Called:  slaClyd
 **
 **  Last revision:   22 September 1995
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    int i;

/* Default century if appropriate */
    if ( ( iy >= 0 ) && ( iy <= 49 ) )
        i = iy + 2000;
    else if ( ( iy >= 50 ) && ( iy <= 99 ) )
        i = iy + 1900;
    else
        i = iy;

/* Perform the conversion */
    slaClyd ( i, im, id, ny, nd, j );
}


void slaDd2tf ( int ndp, double days, char *sign, int ihmsf[4] )
/*
 **  - - - - - - - - -
 **   s l a D d 2 t f
 **  - - - - - - - - -
 **
 **  Convert an interval in days into hours, minutes, seconds.
 **
 **  (double precision)
 **
 **  Given:
 **     ndp       int      number of decimal places of seconds
 **     days      double   interval in days
 **
 **  Returned:
 **     *sign     char     '+' or '-'
 **     ihmsf     int[4]   hours, minutes, seconds, fraction
 **
 **  Last revision:   31 August 1995
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */

#define D2S 86400.0                               /* Days to seconds */

{
    double rs, rm, rh, a, ah, am, as, af;

/* Handle sign */
    *sign = (char) ( ( days < 0.0 ) ?  '-' : '+' );

/* Field units in terms of least significant figure */
    rs = pow ( 10.0, (double) gmax ( ndp, 0 ) );
    rs = dint ( rs );
    rm = rs * 60.0;
    rh = rm * 60.0;

/* Round interval and express in smallest units required */
    a = rs * D2S * fabs ( days );
    a = dnint ( a );

/* Separate into fields */
    ah = a / rh;
    ah = dint ( ah );
    a  = a - ah * rh;
    am = a / rm;
    am = dint ( am );
    a  = a - am * rm;
    as = a / rs;
    as = dint ( as );
    af = a - as * rs;

/* Return results */
    ihmsf[0] = (int) ah;
    ihmsf[1] = (int) am;
    ihmsf[2] = (int) as;
    ihmsf[3] = (int) af;
}


void slaDjcl ( double djm, int *iy, int *im, int *id, double *fd, int *j)
/*
 **  - - - - - - - -
 **   s l a D j c l
 **  - - - - - - - -
 **
 **  Modified Julian Date to Gregorian year, month, day,
 **  and fraction of a day.
 **
 **  Given:
 **     djm      double     Modified Julian Date (JD-2400000.5)
 **
 **  Returned:
 **     *iy      int        year
 **     *im      int        month
 **     *id      int        day
 **     *fd      double     fraction of day
 **     *j       int        status:
 **                      -1 = unacceptable date (before 4701BC March 1)
 **
 **  The algorithm is derived from that of Hatcher 1984 (QJRAS 25, 53-55).
 **
 **  Defined in slamac.h:  dmod
 **
 **  Last revision:   12 March 1998
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double f, d;
    long jd, n4, nd10;

/* Check if date is acceptable */
    if ( ( djm <= -2395520.0 ) || ( djm >= 1e9 ) )
    {
        *j = -1;
        return;
    }
    else
    {
        *j = 0;

/* Separate day and fraction */
        f = dmod ( djm, 1.0 );
        if ( f < 0.0 ) f += 1.0;
        d = djm - f;
        d = dnint ( d );

/* Express day in Gregorian calendar */
        jd = (long) dnint ( d ) + 2400001;
        n4 = 4L*(jd+((6L*((4L*jd-17918L)/146097L))/4L+1L)/2L-37L);
        nd10 = 10L*(((n4-237L)%1461L)/4L)+5L;
        *iy = (int) (n4/1461L-4712L);
        *im = (int) (((nd10/306L+2L)%12L)+1L);
        *id = (int) ((nd10%306L)/10L+1L);
        *fd = f;
        *j = 0;
    }
}


void slaDr2af ( int ndp, double angle, char *sign, int idmsf[4] )
/*
 **  - - - - - - - - -
 **   s l a D r 2 a f
 **  - - - - - - - - -
 **
 **  Convert an angle in radians to degrees, arcminutes, arcseconds.
 **
 **  (double precision)
 **
 **  Given:
 **     ndp       int          number of decimal places of arcseconds
 **     angle     double       angle in radians
 **
 **  Returned:
 **     sign      char*        '+' or '-'
 **     idmsf     int[4]       degrees, arcminutes, arcseconds, fraction
 **
 **  Called:
 **     slaDd2tf
 **
 **  Defined in slamac.h:  D15B2P
 **
 **  Last revision:   19 November 1995
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
/* Scale then use days to h,m,s routine */
    slaDd2tf ( ndp, angle * D15B2P, sign, idmsf );
}


void slaDr2tf ( int ndp, double angle, char *sign, int ihmsf[4] )
/*
 **  - - - - - - - - -
 **   s l a D r 2 t f
 **  - - - - - - - - -
 **
 **  Convert an angle in radians to hours, minutes, seconds.
 **
 **  (double precision)
 **
 **  Given:
 **     ndp       int          number of decimal places of seconds
 **     angle     double       angle in radians
 **
 **  Returned:
 **     sign      char*        '+' or '-'
 **     ihmsf     int[4]       hours, minutes, seconds, fraction
 **
 **  Called:
 **     slaDd2tf
 **
 **  Defined in slamac.h:  D2PI
 **
 **  Last revision:   18 November 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
/* Scale then use days to h,m,s routine */
    slaDd2tf ( ndp, angle / D2PI, sign, ihmsf );
}


double slaGmst ( double ut1 )
/*
 **  - - - - - - - -
 **   s l a G m s t
 **  - - - - - - - -
 **
 **  Conversion from Universal Time to Sidereal Time.
 **
 **  (double precision)
 **
 **  Given:
 **    ut1    double     Universal Time (strictly UT1) expressed as
 **                      Modified Julian Date (JD-2400000.5)
 **
 **  The result is the Greenwich Mean Sidereal Time (double
 **  precision, radians).
 **
 **  The IAU 1982 expression (see page S15 of the 1984 Astronomical
 **  Almanac) is used, but rearranged to reduce rounding errors.
 **  This expression is always described as giving the GMST at
 **  0 hours UT.  In fact, it gives the difference between the
 **  GMST and the UT, which happens to equal the GMST (modulo
 **  24 hours) at 0 hours UT each day.  In this routine, the
 **  entire UT is used directly as the argument for the
 **  standard formula, and the fractional part of the UT is
 **  added separately;  note that the factor 1.0027379... does
 **  not appear.
 **
 **  See also the routine slaGmsta, which delivers better numerical
 **  precision by accepting the UT date and time as separate arguments.
 **
 **  Called:  slaDranrm
 **
 **  Defined in slamac.h:  D2PI, DS2R, dmod
 **
 **  Last revision:   19 March 1996
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double tu;

/* Julian centuries from fundamental epoch J2000 to this UT */
    tu = ( ut1 - 51544.5 ) / 36525.0;

/* GMST at this UT */
    return slaDranrm ( dmod ( ut1, 1.0 ) * D2PI +
        ( 24110.54841 +
        ( 8640184.812866 +
        ( 0.093104 - 6.2e-6 * tu ) * tu ) * tu ) * DS2R );
}


double slaGmsta ( double date, double ut )
/*
 **  - - - - - - - - -
 **   s l a G m s t a
 **  - - - - - - - - -
 **
 **  Conversion from Universal Time to Greenwich mean sidereal time,
 **  with rounding errors minimized.
 **
 **  (double precision)
 **
 **  Given:
 *     date   double     UT1 date (MJD: integer part of JD-2400000.5))
 **    ut     double     UT1 time (fraction of a day)
 **
 **  The result is the Greenwich Mean Sidereal Time (double precision,
 **  radians, in the range 0 to 2pi).
 **
 **  There is no restriction on how the UT is apportioned between the
 **  date and ut1 arguments.  Either of the two arguments could, for
 **  example, be zero and the entire date+time supplied in the other.
 **  However, the routine is designed to deliver maximum accuracy when
 **  the date argument is a whole number and the ut argument lies in
 **  the range 0 to 1, or vice versa.
 **
 **  The algorithm is based on the IAU 1982 expression (see page S15 of
 **  the 1984 Astronomical Almanac).  This is always described as giving
 **  the GMST at 0 hours UT1.  In fact, it gives the difference between
 **  the GMST and the UT, the steady 4-minutes-per-day drawing-ahead of
 **  ST with respect to UT.  When whole days are ignored, the expression
 **  happens to equal the GMST at 0 hours UT1 each day.  Note that the
 **  factor 1.0027379... does not appear explicitly but in the form of
 **  the coefficient 8640184.812866, which is 86400x36525x0.0027379...
 **
 **  In this routine, the entire UT1 (the sum of the two arguments date
 **  and ut) is used directly as the argument for the standard formula.
 **  The UT1 is then added, but omitting whole days to conserve accuracy.
 **
 **  See also the routine slaGmst, which accepts the UT1 as a single
 **  argument.  Compared with slaGmst, the extra numerical precision
 **  delivered by the present routine is unlikely to be important in
 **  an absolute sense, but may be useful when critically comparing
 **  algorithms and in applications where two sidereal times close
 **  together are differenced.
 **
 **  Called:  slaDranrm
 **
 **  Defined in slamac.h:  DS2R, dmod
 **
 **  Last revision:   14 October 2001
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double d1, d2, t;

/* Julian centuries since J2000. */
    if ( date < ut )
    {
        d1 = date;
        d2 = ut;
    }
    else
    {
        d1 = ut;
        d2 = date;
    }
    t = ( d1 + ( d2 - 51544.5 ) ) / 36525.0;

/* GMST at this UT1. */
    return slaDranrm ( DS2R * ( 24110.54841
        + ( 8640184.812866
        + ( 0.093104
        - 6.2e-6 * t ) * t ) * t
        + 86400.0 * ( dmod ( d1, 1.0 ) +
        dmod ( d2, 1.0 ) ) ) );
}


double slaDranrm ( double angle )
/*
 **  - - - - - - - - - -
 **   s l a D r a n r m
 **  - - - - - - - - - -
 **
 **  Normalize angle into range 0-2 pi.
 **
 **  (double precision)
 **
 **  Given:
 **     angle     double      the angle in radians
 **
 **  The result is angle expressed in the range 0-2 pi (double).
 **
 **  Defined in slamac.h:  D2PI, dmod
 **
 **  Last revision:   19 March 1996
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double w;

    w = dmod ( angle, D2PI );
    return ( w >= 0.0 ) ? w : w + D2PI;
}


void slaAltaz(double ha, double dec, double phi,
double *az, double *azd, double *azdd,
double *el, double *eld, double *eldd,
double *pa, double *pad, double *padd )
/*
 **  - - - - - - - - -
 **   s l a A l t a z
 **  - - - - - - - - -
 **
 **  Positions, velocities and accelerations for an altazimuth
 **  telescope mount.
 **
 **  (double precision)
 **
 **  Given:
 **     ha          double      hour angle
 **     dec         double      declination
 **     phi         double      latitude
 **
 **  Returned:
 **     *az         double      azimuth
 **     *azd        double         "    velocity
 **     *azdd       double         "    acceleration
 **     *el         double      elevation
 **     *eld        double          "     velocity
 **     *eldd       double          "     acceleration
 **     *pa         double      parallactic angle
 **     *pad        double          "      "   velocity
 **     *padd       double          "      "   acceleration
 **
 **  Notes:
 **
 **  1)  Natural units are used throughout.  HA, DEC, PHI, AZ, EL
 **      and ZD are in radians.  The velocities and accelerations
 **      assume constant declination and constant rate of change of
 **      hour angle (as for tracking a star);  the units of AZD, ELD
 **      and PAD are radians per radian of HA, while the units of AZDD,
 **      ELDD and PADD are radians per radian of HA squared.  To
 **      convert into practical degree- and second-based units:
 **
 **        angles * 360/2pi -> degrees
 **        velocities * (2pi/86400)*(360/2pi) -> degree/sec
 **        accelerations * ((2pi/86400)**2)*(360/2pi) -> degree/sec/sec
 **
 **      Note that the seconds here are sidereal rather than SI.  One
 **      sidereal second is about 0.99727 SI seconds.
 **
 **      The velocity and acceleration factors assume the sidereal
 **      tracking case.  Their respective numerical values are (exactly)
 **      1/240 and (approximately) 1/3300236.9.
 **
q**  2)  Azimuth is returned in the range 0-2pi;  north is zero,
**      and east is +pi/2.  Elevation and parallactic angle are
**      returned in the range +/-pi.  Position angle is +ve
**      for a star west of the meridian and is the angle NP-star-zenith.
**
**  3)  The latitude is geodetic as opposed to geocentric.  The
**      hour angle and declination are topocentric.  Refraction and
**      deficiencies in the telescope mounting are ignored.  The
**      purpose of the routine is to give the general form of the
**      quantities.  The details of a real telescope could profoundly
**      change the results, especially close to the zenith.
 **
 **  4)  No range checking of arguments is carried out.
 **
 **  5)  In applications which involve many such calculations, rather
 **      than calling the present routine it will be more efficient to
 **      use inline code, having previously computed fixed terms such
 **      as sine and cosine of latitude, and (for tracking a star)
 **      sine and cosine of declination.
 **
 **  Defined in slamac.h:  DPI, D2PI
 **
 **  Last revision:   30 November 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */

#define TINY 1e-30                                /* Zone of avoidance around zenith/nadir */

{
    double sh, ch, sd, cd, sp, cp, chcd, sdcp, x, y, z, rsq, r, a, e,
        c, s, q, qd, ad, ed, edr, add, edd, qdd;

/* Useful functions */
    sh = sin ( ha );
    ch = cos ( ha );
    sd = sin ( dec );
    cd = cos ( dec );
    sp = sin ( phi );
    cp = cos ( phi );
    chcd = ch * cd;
    sdcp = sd * cp;
    x = - chcd * sp + sdcp;
    y = - sh * cd;
    z = chcd * cp + sd * sp;
    rsq = x * x + y * y;
    r = sqrt ( rsq );

/* Azimuth and elevation */
    a = rsq != 0.0 ? atan2 ( y, x ) : 0.0;
    if ( a < 0.0 ) a += D2PI;
    e = atan2 ( z, r );

/* Parallactic angle */
    c = cd * sp - ch * sdcp;
    s = sh * cp;
    q = ( s != 0.0 || c != 0.0 ) ? atan2 ( s, c ) : DPI - ha;

/* Velocities and accelerations (clamped at zenith/nadir) */
    if ( rsq < TINY )
    {
        rsq = TINY;
        r = sqrt ( rsq );
    }
    qd = - x * cp / rsq;
    ad = sp + z * qd;
    ed = cp * y / r;
    edr = ed / r;
    add = edr * ( z * sp + ( 2.0 - rsq ) * qd );
    edd = - r * qd * ad;
    qdd = edr * ( sp + 2.0 * z * qd );

/* Results */
    *az = a;
    *azd = ad;
    *azdd = add;
    *el = e;
    *eld = ed;
    *eldd = edd;
    *pa = q;
    *pad = qd;
    *padd = qdd;
}


void slaDe2h ( double ha, double dec, double phi, double *az, double *el )
/*
 **  - - - - - - - -
 **   s l a D e 2 h
 **  - - - - - - - -
 **
 **  Equatorial to horizon coordinates:  HA,Dec to Az,El
 **
 **  (double precision)
 **
 **  Given:
 **     ha          double       hour angle
 **     dec         double       declination
 **     phi         double       observatory latitude
 **
 **  Returned:
 **     *az         double       azimuth
 **     *el         double       elevation
 **
 **  Notes:
 **
 **  1)  All the arguments are angles in radians.
 **
 **  2)  Azimuth is returned in the range 0-2pi;  north is zero,
 **      and east is +pi/2.  Elevation is returned in the range
 **      +/-pi/2.
 **
 **  3)  The latitude must be geodetic.  In critical applications,
 **      corrections for polar motion should be applied.
 **
 **  4)  In some applications it will be important to specify the
 **      correct type of hour angle and declination in order to
 **      produce the required type of azimuth and elevation.  In
 **      particular, it may be important to distinguish between
 **      elevation as affected by refraction, which would
 **      require the "observed" HA,Dec, and the elevation
 **      in vacuo, which would require the "topocentric" HA,Dec.
 **      If the effects of diurnal aberration can be neglected, the
 **      "apparent" HA,Dec may be used instead of the topocentric
 **      HA,Dec.
 **
 **  5)  No range checking of arguments is carried out.
 **
 **  6)  In applications which involve many such calculations, rather
 **      than calling the present routine it will be more efficient to
 **      use inline code, having previously computed fixed terms such
 **      as sine and cosine of latitude, and (for tracking a star)
 **      sine and cosine of declination.
 **
 **  Defined in slamac.h:  D2PI
 **
 **  Last revision:   30 November 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double sh, ch, sd, cd, sp, cp, x, y, z, r, a;

/* Useful trig functions */
    sh = sin ( ha );
    ch = cos ( ha );
    sd = sin ( dec );
    cd = cos ( dec );
    sp = sin ( phi );
    cp = cos ( phi );

/* Az,El as x,y,z */
    x = - ch * cd * sp + sd * cp;
    y = - sh * cd;
    z = ch * cd * cp + sd * sp;

/* To spherical */
    r = sqrt ( x * x + y * y );
    a = ( r != 0.0 ) ? atan2 ( y, x ) : 0.0;
    *az = ( a < 0.0 ) ? a + D2PI : a;
    *el = atan2 ( z, r );
}


void slaDh2e ( double az, double el, double phi, double *ha, double *dec )
/*
 **  - - - - - - - -
 **   s l a D h 2 e
 **  - - - - - - - -
 **
 **  Horizon to equatorial coordinates:  Az,El to HA,Dec
 **
 **  (double precision)
 **
 **  Given:
 **     az          double       azimuth
 **     el          double       elevation
 **     phi         double       observatory latitude
 **
 **  Returned:
 **     *ha         double       hour angle
 **     *dec        double       declination
 **
 **  Notes:
 **
 **  1)  All the arguments are angles in radians.
 **
 **  2)  The sign convention for azimuth is north zero, east +pi/2.
 **
 **  3)  HA is returned in the range +/-pi.  Declination is returned
 **      in the range +/-pi/2.
 **
 **  4)  The is latitude is (in principle) geodetic.  In critical
 **      applications, corrections for polar motion should be applied.
 **
 **  5)  In some applications it will be important to specify the
 **      correct type of elevation in order to produce the required
 **      type of HA,Dec.  In particular, it may be important to
 **      distinguish between the elevation as affected by refraction,
 **      which will yield the "observed" HA,Dec, and the elevation
 **      in vacuo, which will yield the "topocentric" HA,Dec.  If the
 **      effects of diurnal aberration can be neglected, the
 **      topocentric HA,Dec may be used as an approximation to the
 **      "apparent" HA,Dec.
 **
 **  6)  No range checking of arguments is done.
 **
 **  7)  In applications which involve many such calculations, rather
 **      than calling the present routine it will be more efficient to
 **      use inline code, having previously computed fixed terms such
 **      as sine and cosine of latitude.
 **
 **  Last revision:   30 November 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double sa, ca, se, ce, sp, cp, x, y, z, r;

/* Useful trig functions */
    sa = sin ( az );
    ca = cos ( az );
    se = sin ( el );
    ce = cos ( el );
    sp = sin ( phi );
    cp = cos ( phi );

/* HA,Dec as x,y,z */
    x = - ca * ce * sp + se * cp;
    y = - sa * ce;
    z = ca * ce * cp + se * sp;

/* To spherical */
    r = sqrt ( x * x + y * y );
    *ha = ( r != 0.0 ) ? atan2 ( y, x ) : 0.0;
    *dec = atan2 ( z, r );
    return;
}


void slaDtp2s ( double xi, double eta, double raz, double decz,
double *ra, double *dec )
/*
 **  - - - - - - - - -
 **   s l a D t p 2 s
 **  - - - - - - - - -
 **
 **  Transform tangent plane coordinates into spherical.
 **
 **  (double precision)
 **
 **  Given:
 **     xi,eta      double   tangent plane rectangular coordinates
 **     raz,decz    double   spherical coordinates of tangent point
 **
 **  Returned:
 **     *ra,*dec    double   spherical coordinates (0-2pi,+/-pi/2)
 **
 **  Called:  slaDranrm
 **
 **  Last revision:   3 June 1995
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double sdecz, cdecz, denom;

    sdecz = sin ( decz );
    cdecz = cos ( decz );
    denom = cdecz - eta * sdecz;
    *ra = slaDranrm ( atan2 ( xi, denom ) + raz );
    *dec = atan2 ( sdecz + eta * cdecz, sqrt ( xi * xi + denom * denom ) );
}


void slaPreces ( const char sys[3], double ep0, double ep1,
double *ra, double *dc )
/*
 **  - - - - - - - - - -
 **   s l a P r e c e s
 **  - - - - - - - - - -
 **
 **  Precession - either FK4 (Bessel-Newcomb, pre-IAU1976) or
 **  FK5 (Fricke, post-IAU1976) as required.
 **
 **  Given:
 **     sys        char[]     precession to be applied: "FK4" or "FK5"
 **     ep0,ep1    double     starting and ending epoch
 **     ra,dc      double     RA,Dec, mean equator & equinox of epoch ep0
 **
 **  Returned:
 **     *ra,*dc    double     RA,Dec, mean equator & equinox of epoch ep1
 **
 **  Called:    slaDranrm, slaPrebn, slaPrec, slaDcs2c,
 **             slaDmxv, slaDcc2s
 **
 **  Notes:
 **
 **  1)  The epochs are Besselian if sys='FK4' and Julian if 'FK5'.
 **      For example, to precess coordinates in the old system from
 **      equinox 1900.0 to 1950.0 the call would be:
 **          slaPreces ( "FK4", 1900.0, 1950.0, &ra, &dc )
 **
 **  2)  This routine will not correctly convert between the old and
 **      the new systems - for example conversion from B1950 to J2000.
 **      For these purposes see slaFk425, slaFk524, slaFk45z and
 **      slaFk54z.
 **
 **  3)  If an invalid sys is supplied, values of -99.0,-99.0 will
 **      be returned for both ra and dc.
 **
 **  Last revision:   15 June 2001
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double pm[3][3], v1[3], v2[3];

/* Validate sys */
    if ( ( toupper ( (int) sys[0] ) != 'F' )
        || ( toupper ( (int) sys[1] ) != 'K' )
        || ( (int) sys[2] != '4' && (int) sys[2] != '5' ) )
    {
        *ra = -99.0;                              /* Error */
        *dc = -99.0;
    }
    else
    {

/* Generate appropriate precession matrix */
        if ( (int) sys[2] == '4' )
            slaPrebn ( ep0, ep1, pm );
        else
            slaPrec ( ep0, ep1, pm );

/* Convert RA,Dec to x,y,z */
        slaDcs2c ( *ra, *dc, v1 );

/* Precess */
        slaDmxv ( pm, v1, v2 );

/* Back to RA,Dec */
        slaDcc2s ( v2, ra, dc );
        *ra = slaDranrm ( *ra );
    }
}


void slaPrebn ( double bep0, double bep1, double rmatp[3][3] )
/*
 **  - - - - - - - - -
 **   s l a P r e b n
 **  - - - - - - - - -
 **
 **  Generate the matrix of precession between two epochs,
 **  using the old, pre-IAU1976, Bessel-Newcomb model, using
 **  Kinoshita's formulation (double precision)
 **
 **  Given:
 **     BEP0    double        beginning Besselian epoch
 **     BEP1    double        ending Besselian epoch
 **
 **  Returned:
 **     RMATP   double[3][3]  precession matrix
 **
 **  The matrix is in the sense   v(bep1)  =  rmatp * v(bep0)
 **
 **  Reference:
 **     Kinoshita, H. (1975) 'Formulas for precession', SAO Special
 **     Report No. 364, Smithsonian Institution Astrophysical
 **     Observatory, Cambridge, Massachusetts.
 **
 **  Called:  slaDeuler
 **
 **  Defined in slamac.h:  DAS2R
 **
 **  Last revision:   30 October 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double bigt, t, tas2r, w, zeta, z, theta;

/* Interval between basic epoch B1850.0 and beginning epoch in TC */
    bigt  = ( bep0 - 1850.0 ) / 100.0;

/* Interval over which precession required, in tropical centuries */
    t = ( bep1 - bep0 ) / 100.0;

/* Euler angles */
    tas2r = t * DAS2R;
    w = 2303.5548 + ( 1.39720 + 0.000059 * bigt ) * bigt;
    zeta = (w + ( 0.30242 - 0.000269 * bigt + 0.017996 * t ) * t ) * tas2r;
    z = (w + ( 1.09478 + 0.000387 * bigt + 0.018324 * t ) * t ) * tas2r;
    theta = ( 2005.1125 + ( - 0.85294 - 0.000365* bigt ) * bigt +
        ( - 0.42647 - 0.000365 * bigt - 0.041802 * t ) * t ) * tas2r;

/* Rotation matrix */
    slaDeuler ( "ZYZ", -zeta, theta, -z, rmatp );
}


void slaPrec ( double ep0, double ep1, double rmatp[3][3] )
/*
 **  - - - - - - - -
 **   s l a P r e c
 **  - - - - - - - -
 **
 **  Form the matrix of precession between two epochs (IAU 1976, FK5).
 **
 **  (double precision)
 **
 **  Given:
 **     ep0    double         beginning epoch
 **     ep1    double         ending epoch
 **
 **  Returned:
 **     rmatp  double[3][3]   precession matrix
 **
 **  Notes:
 **
 **  1)  The epochs are TDB (loosely ET) Julian epochs.
 **
 **  2)  The matrix is in the sense   v(ep1)  =  rmatp * v(ep0) .
 **
 **  3)  Though the matrix method itself is rigorous, the precession
 **      angles are expressed through canonical polynomials which are
 **      valid only for a limited time span.  There are also known
 **      errors in the IAU precession rate.  The absolute accuracy
 **      of the present formulation is better than 0.1 arcsec from
 **      1960AD to 2040AD, better than 1 arcsec from 1640AD to 2360AD,
 **      and remains below 3 arcsec for the whole of the period
 **      500BC to 3000AD.  The errors exceed 10 arcsec outside the
 **      range 1200BC to 3900AD, exceed 100 arcsec outside 4200BC to
 **      5600AD and exceed 1000 arcsec outside 6800BC to 8200AD.
 **      The SLALIB routine slaPrecl implements a more elaborate
 **      model which is suitable for problems spanning several
 **      thousand years.
 **
 **  References:
 **     Lieske,J.H., 1979. Astron. Astrophys.,73,282.
 **          equations (6) & (7), p283.
 **     Kaplan,G.H., 1981. USNO circular no. 163, pa2.
 **
 **  Called:  slaDeuler
 **
 **  Defined in slamac.h:  DAS2R
 **
 **  Last revision:   10 July 1994
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double t0, t, tas2r, w, zeta, z, theta;

/* Interval between basic epoch J2000.0 and beginning epoch (JC) */
    t0 = ( ep0 - 2000.0 ) / 100.0;

/* Interval over which precession required (JC) */
    t =  ( ep1 - ep0 ) / 100.0;

/* Euler angles */
    tas2r = t * DAS2R;
    w = 2306.2181 + ( ( 1.39656 - ( 0.000139 * t0 ) ) * t0 );
    zeta = (w + ( ( 0.30188 - 0.000344 * t0 ) + 0.017998 * t ) * t ) * tas2r;
    z = (w + ( ( 1.09468 + 0.000066 * t0 ) + 0.018203 * t ) * t ) * tas2r;
    theta = ( ( 2004.3109 + ( - 0.85330 - 0.000217 * t0 ) * t0 )
        + ( ( -0.42665 - 0.000217 * t0 ) - 0.041833 * t ) * t ) * tas2r;

/* Rotation matrix */
    slaDeuler ( "ZYZ", -zeta, theta, -z, rmatp );
}


void slaDeuler ( const char *order, double phi, double theta,
double psi, double rmat[3][3] )
/*
 **  - - - - - - - - - -
 **   s l a D e u l e r
 **  - - - - - - - - - -
 **
 **  Form a rotation matrix from the Euler angles - three successive
 **  rotations about specified Cartesian axes.
 **
 **  (double precision)
 **
 **  Given:
 **    *order char     specifies about which axes the rotations occur
 **    phi    double   1st rotation (radians)
 **    theta  double   2nd rotation (   "   )
 **    psi    double   3rd rotation (   "   )
 **
 **  Returned:
 **    rmat   double[3][3]  rotation matrix
 **
 **  A rotation is positive when the reference frame rotates
 **  anticlockwise as seen looking towards the origin from the
 **  positive region of the specified axis.
 **
 **  The characters of order define which axes the three successive
 **  rotations are about.  A typical value is 'zxz', indicating that
 **  rmat is to become the direction cosine matrix corresponding to
 **  rotations of the reference frame through phi radians about the
 **  old z-axis, followed by theta radians about the resulting x-axis,
 **  then psi radians about the resulting z-axis.
 **
 **  The axis names can be any of the following, in any order or
 **  combination:  x, y, z, uppercase or lowercase, 1, 2, 3.  Normal
 **  axis labelling/numbering conventions apply;  the xyz (=123)
 **  triad is right-handed.  Thus, the 'zxz' example given above
 **  could be written 'zxz' or '313' (or even 'zxz' or '3xz').  Order
 **  is terminated by length or by the first unrecognized character.
 **
 **  Fewer than three rotations are acceptable, in which case the later
 **  angle arguments are ignored.  Zero rotations leaves rmat set to the
 **  identity matrix.
 **
 **  Last revision:   9 December 1996
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    int j, i, l, n, k;
    double result[3][3], rotn[3][3], angle, s, c , w, wm[3][3];
    char axis;

/* Initialize result matrix */
    for ( j = 0; j < 3; j++ )
    {
        for ( i = 0; i < 3; i++ )
        {
            result[i][j] = ( i == j ) ? 1.0 : 0.0;
        }
    }

/* Establish length of axis string */
    l = strlen ( order );

/* Look at each character of axis string until finished */
    for ( n = 0; n < 3; n++ )
    {
        if ( n <= l )
        {

/* Initialize rotation matrix for the current rotation */
            for ( j = 0; j < 3; j++ )
            {
                for ( i = 0; i < 3; i++ )
                {
                    rotn[i][j] = ( i == j ) ? 1.0 : 0.0;
                }
            }

/* Pick up the appropriate Euler angle and take sine & cosine */
            switch ( n )
            {
                case 0 :
                    angle = phi;
                    break;
                case 1 :
                    angle = theta;
                    break;
                default:
                    angle = psi;
                    break;
            }
            s = sin ( angle );
            c = cos ( angle );

/* Identify the axis */
            axis =  order[n];
            if ( ( axis == 'X' ) || ( axis == 'x' ) || ( axis == '1' ) )
            {

/* Matrix for x-rotation */
                rotn[1][1] = c;
                rotn[1][2] = s;
                rotn[2][1] = -s;
                rotn[2][2] = c;
            }
            else if ( ( axis == 'Y' ) || ( axis == 'y' ) || ( axis == '2' ) )
            {

/* Matrix for y-rotation */
                rotn[0][0] = c;
                rotn[0][2] = -s;
                rotn[2][0] = s;
                rotn[2][2] = c;
            }
            else if ( ( axis == 'Z' ) || ( axis == 'z' ) || ( axis == '3' ) )
            {

/* Matrix for z-rotation */
                rotn[0][0] = c;
                rotn[0][1] = s;
                rotn[1][0] = -s;
                rotn[1][1] = c;
            }
            else
            {

/* Unrecognized character - fake end of string */
                l = 0;
            }

/* Apply the current rotation (matrix rotn x matrix result) */
            for ( i = 0; i < 3; i++ )
            {
                for ( j = 0; j < 3; j++ )
                {
                    w = 0.0;
                    for ( k = 0; k < 3; k++ )
                    {
                        w += rotn[i][k] * result[k][j];
                    }
                    wm[i][j] = w;
                }
            }
            for ( j = 0; j < 3; j++ )
            {
                for ( i= 0; i < 3; i++ )
                {
                    result[i][j] = wm[i][j];
                }
            }
        }
    }

/* Copy the result */
    for ( j = 0; j < 3; j++ )
    {
        for ( i = 0; i < 3; i++ )
        {
            rmat[i][j] = result[i][j];
        }
    }
}


void slaDmxv ( double dm[3][3], double va[3], double vb[3] )
/*
 **  - - - - - - - -
 **   s l a D m x v
 **  - - - - - - - -
 **
 **  Performs the 3-d forward unitary transformation:
 **     vector vb = matrix dm * vector va
 **
 **  (double precision)
 **
 **  Given:
 **     dm       double[3][3]    matrix
 **     va       double[3]       vector
 **
 **  Returned:
 **     vb       double[3]       result vector
 **
 **  Note:  va and vb may be the same array.
 **
 **  Last revision:   6 November 1999
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    int i, j;
    double w, vw[3];

/* Matrix dm * vector va -> vector vw */
    for ( j = 0; j < 3; j++ )
    {
        w = 0.0;
        for ( i = 0; i < 3; i++ )
        {
            w += dm[j][i] * va[i];
        }
        vw[j] = w;
    }

/* Vector vw -> vector vb */
    for ( j = 0; j < 3; j++ )
    {
        vb[j] = vw[j];
    }
}


void slaDcc2s ( double v[3], double *a, double *b )
/*
 **  - - - - - - - - -
 **   s l a D c c 2 s
 **  - - - - - - - - -
 **
 **  Direction cosines to spherical coordinates.
 **
 **  (double precision)
 **
 **  Given:
 **     v      double[3]   x,y,z vector
 **
 **  Returned:
 **     *a,*b  double      spherical coordinates in radians
 **
 **  The spherical coordinates are longitude (+ve anticlockwise
 **  looking from the +ve latitude pole) and latitude.  The
 **  Cartesian coordinates are right handed, with the x axis
 **  at zero longitude and latitude, and the z axis at the
 **  +ve latitude pole.
 **
 **  If v is null, zero a and b are returned.
 **  At either pole, zero a is returned.
 **
 **  Last revision:   31 October 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double x, y, z, r;

    x = v[0];
    y = v[1];
    z = v[2];
    r = sqrt ( x * x + y * y );

    *a = ( r != 0.0 ) ? atan2 ( y, x ) : 0.0;
    *b = ( z != 0.0 ) ? atan2 ( z, r ) : 0.0;
}


float slaSep ( float a1, float b1, float a2, float b2 )
/*
 **  - - - - - - -
 **   s l a S e p
 **  - - - - - - -
 **
 **  Angle between two points on a sphere.
 **
 **  (single precision)
 **
 **  Given:
 **     a1,b1     float     spherical coordinates of one point
 **     a2,b2     float     spherical coordinates of the other point
 **
 **  (The spherical coordinates are [RA,Dec], [Long,Lat] etc, in radians.)
 **
 **  The result is the angle, in radians, between the two points.  It is
 **  always positive.
 **
 **  Called:  slaDsep
 **
 **  Last revision:   7 May 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{

/* Use double precision version. */
    return (float) slaDsep( (double) a1, (double) b1,
        (double) a2, (double) b2 );
}


double slaDsep ( double a1, double b1, double a2, double b2 )
/*
 **  - - - - - - - -
 **   s l a D s e p
 **  - - - - - - - -
 **
 **  Angle between two points on a sphere.
 **
 **  (double precision)
 **
 **  Given:
 **     a1,b1    double    spherical coordinates of one point
 **     a2,b2    double    spherical coordinates of the other point
 **
 **  (The spherical coordinates are [RA,Dec], [Long,Lat] etc, in radians.)
 **
 **  The result is the angle, in radians, between the two points.  It
 **  is always positive.
 **
 **  Called:  slaDcs2c, slaDsepv
 **
 **  Last revision:   7 May 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double v1[3], v2[3];

/* Convert coordinates from spherical to Cartesian. */
    slaDcs2c ( a1, b1, v1 );
    slaDcs2c ( a2, b2, v2 );

/* Angle between the vectors. */
    return slaDsepv ( v1, v2 );
}


void slaDcs2c ( double a, double b, double v[3] )
/*
 **  - - - - - - - - -
 **   s l a D c s 2 c
 **  - - - - - - - - -
 **
 **  Spherical coordinates to direction cosines.
 **
 **  (double precision)
 **
 **  Given:
 **     a,b       double      spherical coordinates in radians
 **                           (RA,Dec), (long,lat) etc
 **
 **  Returned:
 **     v         double[3]   x,y,z unit vector
 **
 **  The spherical coordinates are longitude (+ve anticlockwise
 **  looking from the +ve latitude pole) and latitude.  The
 **  Cartesian coordinates are right handed, with the x axis
 **  at zero longitude and latitude, and the z axis at the
 **  +ve latitude pole.
 **
 **  Last revision:   31 October 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double cosb;

    cosb = cos ( b );
    v[0] = cos ( a ) * cosb;
    v[1] = sin ( a ) * cosb;
    v[2] = sin ( b );
    return;
}


void slaCs2c ( float a, float b, float v[3] )
/*
 **  - - - - - - - -
 **   s l a C s 2 c
 **  - - - - - - - -
 **
 **  Spherical coordinates to direction cosines.
 **
 **  (single precision)
 **
 **  Given:
 **     a,b      float     spherical coordinates in radians
 **                        (RA,Dec), (long,lat) etc
 **
 **  Returned:
 **     v        float[3]  x,y,z unit vector
 **
 **  The spherical coordinates are longitude (+ve anticlockwise
 **  looking from the +ve latitude pole) and latitude.  The
 **  Cartesian coordinates are right handed, with the x axis
 **  at zero longitude and latitude, and the z axis at the
 **  +ve latitude pole.
 **
 **  Last revision:   31 October 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    float cosb;

    cosb = (float) cos ( b );
    v[0] = (float) cos ( a ) * cosb;
    v[1] = (float) sin ( a ) * cosb;
    v[2] = (float) sin ( b );
}


float slaSepv ( float v1[3], float v2[3] )
/*
 **  - - - - - - - -
 **   s l a S e p v
 **  - - - - - - - -
 **
 **  Angle between two vectors.
 **
 **  (single precision)
 **
 **  Given:
 **     v1     float[3]     first vector
 **     v2     float[3]     second vector
 **
 **  The result is the angle, in radians, between the two vectors.  It
 **  is always positive.
 **
 **  Notes:
 **
 **  1  There is no requirement for the vectors to be unit length.
 **
 **  2  If either vector is null, zero is returned.
 **
 **  3  The simplest formulation would use dot product alone.  However,
 **     this would reduce the accuracy for angles near zero and pi.  The
 **     algorithm uses both cross product and dot product, which maintains
 **     accuracy for all sizes of angle.
 **
 **  Called:  slaDsepv
 **
 **  Last revision:   7 May 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    int i;
    double dv1[3], dv2[3];

/* Use double precision version. */
    for ( i = 0; i < 3; i++ )
    {
        dv1[i] = (double) v1[i];
        dv2[i] = (double) v2[i];
    }
    return (float) slaDsepv ( dv1, dv2 );
}


double slaDsepv ( double v1[3], double v2[3] )
/*
 **  - - - - - - - - -
 **   s l a D s e p v
 **  - - - - - - - - -
 **
 **  Angle between two vectors.
 **
 **  (double precision)
 **
 **  Given:
 **     v1      double[3]    first vector
 **     v2      double[3]    second vector
 **
 **  The result is the angle, in radians, between the two vectors.  It
 **  is always positive.
 **
 **  Notes:
 **
 **  1  There is no requirement for the vectors to be unit length.
 **
 **  2  If either vector is null, zero is returned.
 **
 **  3  The simplest formulation would use dot product alone.  However,
 **     this would reduce the accuracy for angles near zero and pi.  The
 **     algorithm uses both cross product and dot product, which maintains
 **     accuracy for all sizes of angle.
 **
 **  Called:  slaDvxv, slaDvn, slaDvdv
 **
 **  Last revision:   7 May 2000
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double v1xv2[3], wv[3], s, c;

/* Modulus of cross product = sine multiplied by the two moduli. */
    slaDvxv ( v1, v2, v1xv2 );
    slaDvn ( v1xv2, wv, &s );

/* Dot product = cosine multiplied by the two moduli. */
    c = slaDvdv ( v1, v2 );

/* Angle between the vectors. */
    return s != 0.0 ? atan2 ( s, c ) : 0.0;
}


void slaDvxv ( double va[3], double vb[3], double vc[3] )
/*
 **  - - - - - - - -
 **   s l a D v x v
 **  - - - - - - - -
 **
 **  Vector product of two 3-vectors.
 **
 **  (double precision)
 **
 **  Given:
 **     va      double[3]     first vector
 **     vb      double[3]     second vector
 **
 **  Returned:
 **     vc      double[3]     vector result
 **
 **  Note:  the same vector may be specified more than once.
 **
 **  Last revision:   6 November 1999
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double vw[3];
    int i;

/* Form the vector product va cross vb */
    vw[0] = va[1] * vb[2] - va[2] * vb[1];
    vw[1] = va[2] * vb[0] - va[0] * vb[2];
    vw[2] = va[0] * vb[1] - va[1] * vb[0];

/* Return the result */
    for ( i = 0; i < 3; i++ )
    {
        vc[i] = vw[i];
    }
    return;
}


void slaDvn ( double v[3], double uv[3], double *vm )
/*
 **  - - - - - - -
 **   s l a D v n
 **  - - - - - - -
 **
 **  Normalizes a 3-vector also giving the modulus.
 **
 **  (double precision)
 **
 **  Given:
 **     v       double[3]      vector
 **
 **  Returned:
 **     uv      double[3]      unit vector in direction of v
 **     *vm     double         modulus of v
 **
 **  Note:  v and uv may be the same array.
 **
 **  If the modulus of v is zero, uv is set to zero as well.
 **
 **  Last revision:   6 December 2001
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    int i;
    double w1, w2;

/* Modulus */
    w1 = 0.0;
    for ( i = 0; i < 3; i++ )
    {
        w2 = v[i];
        w1 += w2 * w2;
    }
    w1 = sqrt ( w1 );
    *vm = w1;

/* Normalize the vector */
    w1 = ( w1 > 0.0 ) ? w1 : 1.0;

    for ( i = 0; i < 3; i++ )
    {
        uv[i] = v[i] / w1;
    }
    return;
}


float slaVdv ( float va[3], float vb[3] )
/*
 **  - - - - - - -
 **   s l a V d v
 **  - - - - - - -
 **
 **  Scalar product of two 3-vectors.
 **
 **  (single precision)
 **
 **  Given:
 **      va      float[3]     first vector
 **      vb      float[3]     second vector
 **
 **  The result is the scalar product va.vb  (single precision).
 **
 **  Last revision:   15 July 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    return va[0] * vb[0] + va[1] * vb[1] + va[2] * vb[2];
}


double slaDvdv ( double va[3], double vb[3] )
/*
 **  - - - - - - - -
 **   s l a D v d v
 **  - - - - - - - -
 **
 **  Scalar product of two 3-vectors.
 **
 **  (double precision)
 **
 **
 **  Given:
 **      va      double(3)     first vector
 **      vb      double(3)     second vector
 **
 **
 **  The result is the scalar product va.vb (double precision)
 **
 **
 **  Last revision:   31 October 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    return va[0] * vb[0] + va[1] * vb[1] + va[2] * vb[2];
}


void slaPda2h ( double p, double d, double a,
double *h1, int *j1, double *h2, int *j2 )
/*
 **  - - - - - - - - -
 **   s l a P d a 2 h
 **  - - - - - - - - -
 **
 **  Hour Angle corresponding to a given azimuth
 **
 **  (double precision)
 **
 **  Given:
 **     p           double      latitude
 **     d           double      declination
 **     a           double      azimuth
 **
 **  Returned:
 **     *h1         double      hour angle:  first solution if any
 **     *j1         int         flag: 0 = solution 1 is valid
 **     *h2         double      hour angle:  first solution if any
 **     *j2         int         flag: 0 = solution 2 is valid
 **
 **  Defined in slamac.h:  DPI, DPIBY2
 **
 **  Last revision:   24 November 1994
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */

{
    double pn, an, dn, sa, ca, sasp, qt, qb, hpt, t;

/* Preset status flags to OK */
    *j1 = 0;
    *j2 = 0;

/* Adjust latitude, azimuth, declination to avoid critical values */
    pn = slaDrange ( p );
    if ( fabs ( fabs ( pn ) - DPIBY2 ) < TINY )
    {
        pn -= dsign ( TINY, pn);
    }
    else if ( fabs ( pn ) < TINY )
    {
        pn = TINY;
    }
    an = slaDrange ( a );
    if ( fabs ( fabs ( an ) - DPI ) < TINY )
    {
        an -= dsign ( TINY, an );
    }
    else if ( fabs ( an ) < TINY )
    {
        an = TINY;
    }
    dn = slaDrange ( d );
    if ( fabs ( fabs ( dn ) - fabs ( p ) ) < TINY )
    {
        dn -= dsign ( TINY, dn );
    }
    else if ( fabs ( fabs ( dn ) - DPIBY2 ) < TINY )
    {
        dn -= dsign ( TINY, dn );
    }
    else if ( fabs ( dn ) < TINY )
    {
        dn = TINY;
    }

/* Useful functions */
    sa = sin ( an );
    ca = cos ( an );
    sasp = sa * sin ( pn );

/* Quotient giving sin(h+t) */
    qt = sin ( dn ) * sa * cos ( pn );
    qb = cos ( dn ) * sqrt ( ca * ca + sasp * sasp );

/* Any solutions? */
    if ( fabs ( qt ) <= qb )
    {

/* Yes: find h+t and t */
        hpt = asin ( qt / qb );
        t = atan2 ( sasp, - ca );

/* The two solutions */
        *h1 = slaDrange ( hpt - t );
        *h2 = slaDrange ( - hpt - ( t + DPI ) );

/* Reject unless h and A different signs */
        if ( *h1 * an > 0.0 ) *j1 = - 1;
        if ( *h2 * an > 0.0 ) *j2 = - 1;
    }
    else
    {
        *j1 = - 1;
        *j2 = - 1;
    }
}


double slaDrange ( double angle )
/*
 **  - - - - - - - - - -
 **   s l a D r a n g e
 **  - - - - - - - - - -
 **
 **  Normalize angle into range +/- pi.
 **
 **  (double precision)
 **
 **  Given:
 **     angle     double      the angle in radians
 **
 **  The result is angle expressed in the +/- pi (double precision).
 **
 **  Defined in slamac.h:  DPI, D2PI, dmod
 **
 **  Last revision:   19 March 1996
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double w;

    w = dmod ( angle, D2PI );
    return ( fabs ( w ) < DPI ) ? w : w - dsign ( D2PI, angle );
}


void slaDs2tp ( double ra, double dec, double raz, double decz,
double *xi, double *eta, int *j )
/*
 **  - - - - - - - - -
 **   s l a D s 2 t p
 **  - - - - - - - - -
 **
 **  Projection of spherical coordinates onto tangent plane
 **  ('gnomonic' projection - 'standard coordinates').
 **
 **  (double precision)
 **
 **  Given:
 **     ra,dec      double   spherical coordinates of point to be projected
 **     raz,decz    double   spherical coordinates of tangent point
 **
 **  Returned:
 **     *xi,*eta    double   rectangular coordinates on tangent plane
 **     *j          int      status:   0 = OK, star on tangent plane
 **                                    1 = error, star too far from axis
 **                                    2 = error, antistar on tangent plane
 **                                    3 = error, antistar too far from axis
 **
 **  Last revision:   18 July 1996
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
#define TINYX 1e-6
{
    double sdecz, sdec, cdecz, cdec, radif, sradif, cradif, denom;

/* Trig functions */
    sdecz = sin ( decz );
    sdec = sin ( dec );
    cdecz = cos ( decz );
    cdec = cos ( dec );
    radif = ra - raz;
    sradif = sin ( radif );
    cradif = cos ( radif );

/* Reciprocal of star vector length to tangent plane */
    denom = sdec * sdecz + cdec * cdecz * cradif;

/* Handle vectors too far from axis */
    if ( denom > TINYX )
    {
        *j = 0;
    }
    else if ( denom >= 0.0 )
    {
        *j = 1;
        denom = TINYX;
    }
    else if ( denom > -TINYX )
    {
        *j = 2;
        denom = -TINYX;
    }
    else
    {
        *j = 3;
    }

/* Compute tangent plane coordinates (even in dubious cases) */
    *xi = cdec * sradif / denom;
    *eta = ( sdec * cdecz - cdec * sdecz * cradif ) / denom;
}


double slaDbear ( double a1, double b1, double a2, double b2 )
/*
 **  - - - - - - - - -
 **   s l a D b e a r
 **  - - - - - - - - -
 **
 **  Bearing (position angle) of one point on a sphere relative
 **  to another.
 **
 **  (double precision)
 **
 **  Given:
 **     a1,b1    double    spherical coordinates of one point
 **     a2,b2    double    spherical coordinates of the other point
 **
 **  (The spherical coordinates are RA,Dec, Long,Lat etc, in radians.)
 **
 **  The result is the bearing (position angle), in radians, of point
 **  a2,b2 as seen from point a1,b1.  It is in the range +/- pi.  The
 **  sense is such that if a2,b2 is a small distance east of a1,b1,
 **  the bearing is about +pi/2.  Zero is returned if the two points
 **  are coincident.
 **
 **  If either b-coordinate is outside the range +/- pi/2, the
 **  result may correspond to "the long way round".
 **
 **  The routine slaDpav performs an equivalent function except
 **  that the points are specified in the form of Cartesian unit
 **  vectors.
 **
 **  Last revision:   8 December 1996
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double da, x, y;

    da = a2 - a1;
    y = sin ( da ) * cos ( b2 );
    x = sin ( b2 ) * cos ( b1 ) - cos ( b2 ) * sin ( b1 ) * cos ( da );
    return ( x != 0.0 || y != 0.0 ) ? atan2 ( y, x ) : 0.0;
}


void slaGaleq ( double dl, double db, double *dr, double *dd )
/*
 **  - - - - - - - - -
 **   s l a G a l e q
 **  - - - - - - - - -
 **
 **  Transformation from IAU 1958 Galactic coordinates to
 **  J2000.0 equatorial coordinates.
 **
 **  (double precision)
 **
 **  Given:
 **     dl,db       double      galactic longitude and latitude l2,b2
 **
 **  Returned:
 **     *dr,*dd     double      J2000.0 RA,dec
 **
 **  (all arguments are radians)
 **
 **  Called:
 **     slaDcs2c, slaDimxv, slaDcc2s, slaDranrm, slaDrange
 **
 **  Note:
 **     The equatorial coordinates are J2000.0.  Use the routine
 **     slaGe50 if conversion to B1950.0 'FK4' coordinates is
 **     required.
 **
 **  Reference:
 **     Blaauw et al, Mon.Not.R.astron.Soc.,121,123 (1960)
 **
 **  Last revision:   21 September 1998
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double v1[3], v2[3];

/*
 **  l2,b2 system of Galactic coordinates
 **
 **  p = 192.25       RA of Galactic north pole (mean B1950.0)
 **  q =  62.6        inclination of Galactic to mean B1950.0 equator
 **  r =  33          longitude of ascending node
 **
 **  p,q,r are degrees
 **
 **  Equatorial to Galactic rotation matrix (J2000.0), obtained by
 **  applying the standard FK4 to FK5 transformation, for zero proper
 **  motion in FK5, to the columns of the B1950 equatorial to
 **  Galactic rotation matrix:
 */
    static double rmat[3][3] =
    {
        { -0.054875539726, -0.873437108010, -0.483834985808 },
        {  0.494109453312, -0.444829589425,  0.746982251810 },
        { -0.867666135858, -0.198076386122,  0.455983795705 }
    };

/* Spherical to Cartesian */
    slaDcs2c ( dl, db, v1 );

/* Galactic to equatorial */
    slaDimxv ( rmat, v1, v2 );

/* Cartesian to spherical */
    slaDcc2s ( v2, dr, dd );

/* Express in conventional ranges */
    *dr = slaDranrm ( *dr );
    *dd = slaDrange ( *dd );
}


void slaDimxv ( double dm[3][3], double va[3], double vb[3] )
/*
 **  - - - - - - - - -
 **   s l a D i m x v
 **  - - - - - - - - -
 **
 **  Performs the 3-d backward unitary transformation:
 **
 **     vector vb = (inverse of matrix dm) * vector va
 **
 **  (double precision)
 **
 **  (n.b.  The matrix must be unitary, as this routine assumes that
 **   the inverse and transpose are identical)
 **
 **
 **  Given:
 **     dm       double[3][3]   matrix
 **     va       double[3]      vector
 **
 **  Returned:
 **     vb       double[3]      result vector
 **
 **  Last revision:   31 October 1993
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    long i, j;
    double w, vw[3];

/* Inverse of matrix dm * vector va -> vector vw */
    for ( j = 0; j < 3; j++ )
    {
        w = 0.0;
        for ( i = 0; i < 3; i++ )
        {
            w += dm[i][j] * va[i];
        }
        vw[j] = w;
    }

/* Vector vw -> vector vb */
    for ( j = 0; j < 3; j++ )
    {
        vb[j] = vw[j];
    }
}


void slaEqgal ( double dr, double dd, double *dl, double *db )
/*
 **  - - - - - - - - -
 **   s l a E q g a l
 **  - - - - - - - - -
 **
 **  Transformation from J2000.0 equatorial coordinates to
 **  IAU 1958 Galactic coordinates.
 **
 **  (double precision)
 **
 **  Given:
 **     dr,dd       double       J2000.0 RA,Dec
 **
 **  Returned:
 **     *dl,*db     double       Galactic longitude and latitude l2,b2
 **
 **  (all arguments are radians)
 **
 **  Called:
 **     slaDcs2c, slaDmxv, slaDcc2s, slaDranrm, slaDrange
 **
 **  Note:
 **     The equatorial coordinates are J2000.0.  Use the routine
 **     slaEg50 if conversion from B1950.0 'FK4' coordinates is
 **     required.
 **
 **  Reference:
 **     Blaauw et al, Mon.Not.R.astron.Soc.,121,123 (1960)
 **
 **  Last revision:   21 September 1998
 **
 **  Copyright P.T.Wallace.  All rights reserved.
 */
{
    double v1[3], v2[3];

/*
 **  l2,b2 system of Galactic coordinates
 **
 **  p = 192.25       RA of Galactic north pole (mean B1950.0)
 **  q =  62.6        inclination of Galactic to mean B1950.0 equator
 **  r =  33          longitude of ascending node
 **
 **  p,q,r are degrees
 **
 **  Equatorial to Galactic rotation matrix (J2000.0), obtained by
 **  applying the standard FK4 to FK5 transformation, for zero proper
 **  motion in FK5, to the columns of the B1950 equatorial to
 **  Galactic rotation matrix:
 */
    static double rmat[3][3];

    rmat[0][0] = -0.054875539726;
    rmat[0][1] = -0.873437108010;
    rmat[0][2] = -0.483834985808;
    rmat[1][0] =  0.494109453312;
    rmat[1][1] = -0.444829589425;
    rmat[1][2] =  0.746982251810;
    rmat[2][0] = -0.867666135858;
    rmat[2][1] = -0.198076386122;
    rmat[2][2] =  0.455983795705;

/* Spherical to Cartesian */
    slaDcs2c ( dr, dd, v1 );

/* Equatorial to Galactic */
    slaDmxv ( rmat, v1, v2 );

/* Cartesian to spherical */
    slaDcc2s ( v2, dl, db );

/* Express in conventional ranges */
    *dl = slaDranrm ( *dl );
    *db = slaDrange ( *db );
}
