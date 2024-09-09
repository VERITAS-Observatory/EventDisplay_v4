//! VSQLTextFileReader read sql prepared text file into a map

#ifndef VSQLTextFileReader_H
#define VSQLTextFileReader_H

#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VSQLTextFileReader
{
    private:

        bool fIsGood;
        map<string, vector< string >> fData;

        void readSQLFile( string iSQLFile );

    public:

        VSQLTextFileReader( string iSQLFile );
        VSQLTextFileReader(
            string iSQLFileDirectory,
            unsigned irunnumber,
            string iSQLFileType,
            unsigned int iTelID = 9999 );
        ~VSQLTextFileReader() {}

        string getValue_from_key( string iKey );
        vector< string > getValueVector_from_key( string iKey );
        vector< unsigned int > getValueVector_from_key_as_integer( string iKey );
        vector< double > getValueVector_from_key_as_double( string iKey );
        string getValue_from_key( string iKey, string iSearchKey, string iValue );

        bool checkDataVectorsForSameLength();
        bool isGood()
        {
            return fIsGood;
        }
        void printData();
};

#endif
