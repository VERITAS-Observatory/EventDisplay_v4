//! VDBTools VERITAS DB tools
// Revision $Id: VDBTools.h,v 1.1.2.1 2011/03/16 15:48:18 gmaier Exp $

#ifndef VDBTools_H
#define VDBTools_h

#include <TMath.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TSQLServer.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class VDB_ObservingSources_Data : public TObject
{
    public:

    string SourceID;
    float fDec;
    float fRA;
    float fEpoch;

    VDB_ObservingSources_Data();
   ~VDB_ObservingSources_Data() {}

    ClassDef(VDB_ObservingSources_Data,1);
};

class VDB_ObservingSources : public TObject
{
    private:

    bool   fDebug;

    map< string, VDB_ObservingSources_Data* > fVDB_ObservingSources_Data;

    public:

    VDB_ObservingSources();
   ~VDB_ObservingSources() {}
    bool fill( TSQLServer *i_db = 0 );
    VDB_ObservingSources_Data* get_ObservingSources_Data( string iSource );
    void list();
    void setDebug( bool iB = true ) { fDebug = iB; }

    ClassDef(VDB_ObservingSources,1);
};

#endif
