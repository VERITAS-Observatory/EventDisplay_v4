//! VUtilities

#ifndef VUTIL_H
#define VUTIL_H

#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "TMath.h"
#include "TSystem.h"

#include "VGlobalRunParameter.h"

using namespace std;

namespace VUtilities
{
    unsigned int count_number_of_textblocks( string );
    string lowerCase( string& s );
    string upperCase( string& s );
    string removeSpaces( string );
    string remove_leading_spaces( string );
    string trim_spaces( string, string whitespace = " \t" );

    template<class Seq> void purge( Seq& c );

    string search_and_replace( string i1, string iO, string iN );
    string testFileLocation( string iFile, string iDirectory, bool bEVNDISPDATA );

    double line_point_distance( double x1, double y1, double z1,  double alt, double az, double x, double y, double z );

    // from http://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int
    inline bool isInteger( const std::string& s )
    {
        if( s.empty() || ((!isdigit( s[0] ) ) && ( s[0] != '-' ) && ( s[0] != '+' ) ) )
        {
            return false ;
        }

        char* p ;
        strtol( s.c_str(), &p, 10 ) ;

        return (*p == 0 ) ;
    }

    // friendlier colors
    inline int color_id(unsigned int index)
    {
        // Array containing the specified ROOT color ID
        int colors[] = {12, 633, 9, 418, 801, 881, 900, 32, 423, 393, 798, 616, 409};
        unsigned int size = sizeof(colors) / sizeof(colors[0]);

        if (index >= size)
        {
            return index;
        }
        return colors[index];
    }

}
#endif
