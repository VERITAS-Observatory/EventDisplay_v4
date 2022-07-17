//! VSQLTextFileReader read sql prepared text file into a map

#ifndef VSQLTextFileReader_H
#define VSQLTextFileReader_H

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VSQLTextFileReader
{
    private:

        bool fisGood;
        map<string, vector< string >> fData;

    public:

        VSQLTextFileReader( string iSQLFile );
        ~VSQLTextFileReader() {}

        string getValue_from_key( string iKey );
        vector< string > getValueVector_from_key( string iKey );
        vector< unsigned int > getValueVector_from_key_as_integer(string iKey);
        string getValue_from_key(string iKey, string iSearchKey, string iValue );

        bool isGood()
        {
            return fisGood;
        }
        void printData();
};

#endif
