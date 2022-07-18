/* VSQLTextFileReader
 *
 * simple reader for SQL text files
 *
 */

#include "VSQLTextFileReader.h"

VSQLTextFileReader::VSQLTextFileReader( string iSQLFile )
{
	fisGood = false;
	cout << "Reading SQLText data from " << iSQLFile << endl;
	
	ifstream sql_file;
	sql_file.open( iSQLFile.c_str() );
	
	string line;
	unsigned int z = 0;
	vector< string > temp_string;
	vector< string > key_vector;
	while( sql_file.good() )
	{
		getline( sql_file, line );
		istringstream ss( line );
		if( line.size() ==  0 )
		{
			continue;
		}
		string temp;
		if( z == 0 )
		{
			while( ss.good() )
			{
				getline( ss, temp, '|' );
				fData[temp] = temp_string;
				key_vector.push_back( temp );
			}
		}
		else
		{
			unsigned int counter = 0;
			while( ss.good() )
			{
				getline( ss, temp, '|' );
				if( counter < key_vector.size() && fData.find( key_vector[counter] ) != fData.end() )
				{
					fData[key_vector[counter]].push_back( temp );
				}
				else
				{
					cout << "\t warning: inconsistent column data" << endl;
				}
				counter++;
			}
		}
		z++;
	};
	sql_file.close();
	
	fisGood = true;
}

string VSQLTextFileReader::getValue_from_key( string iKey )
{
	if( fData.find( iKey ) != fData.end() && fData[iKey].size() > 0 )
	{
		return fData[iKey][0];
	}
	return "";
}

vector< string > VSQLTextFileReader::getValueVector_from_key( string iKey )
{
	if( fData.find( iKey ) != fData.end() )
	{
		return fData[iKey];
	}
	return vector< string >();
}

vector< unsigned int > VSQLTextFileReader::getValueVector_from_key_as_integer( string iKey )
{
	vector< unsigned int > itemp;
	if( fData.find( iKey ) != fData.end() )
	{
		for( unsigned int i = 0; i < fData[iKey].size(); i++ )
		{
			if( fData[iKey][i].size() > 0 )
			{
				itemp.push_back( atoi( fData[iKey][i].c_str() ) );
			}
			else
			{
				itemp.push_back( 0 );
			}
		}
	}
	return itemp;
}

vector< double > VSQLTextFileReader::getValueVector_from_key_as_double( string iKey )
{
	vector< double > itemp;
	if( fData.find( iKey ) != fData.end() )
	{
		for( unsigned int i = 0; i < fData[iKey].size(); i++ )
		{
			if( fData[iKey][i].size() > 0 )
			{
				itemp.push_back( atof( fData[iKey][i].c_str() ) );
			}
			else
			{
				itemp.push_back( 0. );
			}
		}
	}
	return itemp;
}

string VSQLTextFileReader::getValue_from_key( string iKey, string iSearchKey, string iValue )
{
	if( fData.find( iSearchKey ) != fData.end() && fData.find( iKey ) != fData.end() )
	{
		for( unsigned int i = 0; i < fData[iSearchKey].size(); i++ )
		{
			if( fData[iSearchKey][i] == iValue )
			{
				return fData[iKey][i];
			}
		}
	}
	return "";
}

bool VSQLTextFileReader::checkDataVectorsForSameLength()
{
	unsigned int iLength = 0;
	for( map<string, vector< string > >::iterator it = fData.begin(); it != fData.end(); ++it )
	{
		if( iLength == 0 )
		{
			iLength = it->second.size();
		}
		else
		{
			if( iLength != it->second.size() )
			{
				return false;
			}
		}
	}
	return true;
}


void VSQLTextFileReader::printData()
{
	for( map<string, vector< string > >::iterator it = fData.begin(); it != fData.end(); ++it )
	{
		cout << it->first << ": ";
		for( unsigned int i = 0; i < it->second.size(); i++ )
		{
			cout << it->second[i] << "   ";
		}
		cout << endl;
	}
}
