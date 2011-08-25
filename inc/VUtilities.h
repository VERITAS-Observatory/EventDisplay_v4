//! VUtilities

#ifndef VUTIL_H
#define VUTIL_H

#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "VGlobalRunParameter.h"

using namespace std;

namespace VUtilities
{
   string lowerCase(string& s);
   string upperCase(string& s);
   string removeSpaces( string );

   template<class Seq> void purge( Seq& c );

   string testFileLocation( string iFile, string iDirectory, bool bEVNDISPDATA );
}
#endif
