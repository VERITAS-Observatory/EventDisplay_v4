/*! \file VUtilities.cpp

 */

#include "VUtilities.h"

//! Make a lowercase copy of s:
string VUtilities::lowerCase( string& s)
{
    char* buf = new char[s.length()];
    s.copy(buf, s.length());
    for( unsigned int i = 0; i < s.length(); i++) buf[i] = tolower(buf[i]);
    string r(buf, s.length());
    delete [] buf;
    return r;
}


//! Make an uppercase copy of s:
string VUtilities::upperCase(string& s)
{
    char* buf = new char[s.length()];
    s.copy(buf, s.length());
    for( unsigned int i = 0; i < s.length(); i++)
        buf[i] = toupper(buf[i]);
    string r(buf, s.length());
    delete [] buf;
    return r;
}


/*!
    \brief delete pointers in an STL sequence container
*/
template<class Seq> void VUtilities::purge( Seq& c )
{
    typename Seq::iterator i;
    for( i = c.begin(); i != c.end(); i++ )
    {
        delete *i;
        *i = 0;
    }
}

/*

    test if a certain file exist

*/
string VUtilities::testFileLocation( string iFile, string iDirectory, bool bEVNDISPDATA )
{
    ifstream is;
    is.open( iFile.c_str(), ifstream::in);
    if( !is )
    {
	string itemp;
        if( bEVNDISPDATA )
	{
	   itemp = VGlobalRunParameter::getDirectory_EVNDISPAnaData();
	   itemp += "/" + iDirectory + "/" + iFile;
        }
	else
	{
	   itemp = "./" + iDirectory + "/" + iFile;
        }
        ifstream is2;
        is2.open( itemp.c_str(), ifstream::in );
        if( !is2 )
        {
            cout << "testFileLocation: Error opening file: " << iFile << endl;
            cout << "(not found in current directory and in " << iDirectory << ")" << endl;
            iFile = "";
        }
        iFile = itemp;
    }
    return iFile;
}

/*
    
    remove all white spaces from a string

*/
string VUtilities::removeSpaces( string stringIn )
{
   string::size_type pos = 0;
   bool spacesLeft = true;

   while( spacesLeft )
   {
      pos = stringIn.find(" ");
      if( pos != string::npos ) stringIn.erase( pos, 1 );
      else spacesLeft = false;
   }

   return stringIn;
} 

/*
   
   remove all leading spaces in a string

*/
string VUtilities::remove_leading_spaces( string stringIn )
{
   string::size_type pos = 0;
   bool spacesLeft = true;

   while( spacesLeft )
   {
      pos = stringIn.find(" ");
      if( pos != 0 )
      {
         pos = stringIn.find("\t");
      }
      
      if( pos == 0 && pos != string::npos ) stringIn.erase( pos, 1 );
      else spacesLeft = false;
   }

   return stringIn;
} 

/*

   search and replace a certain letter in a string

*/
string VUtilities::search_and_replace( string i1, string iO, string iN )
{
    size_t j;
    for ( ; (j = i1.find( iO )) != string::npos ; ) 
    {
       i1.replace( j, iO.length(), iN );
    }
    return i1;
}

