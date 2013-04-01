//
//  main.cpp
//  grspread
//
//  Created by Maxim Piskunov on 01.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "GRFermiLAT.h"

struct QueryParameters {
    bool success;
    double startTime;
    double endTime;
    GRLocation location;
    GRFermiEventClass eventClass;
    GRFermiConversionType conversionType;
    float probability;
    string filename;
    
    void printUsage() {
        cout << "Usage:" << endl;
        cout << "grspread ";
        cout << "startTime(MET) ";
        cout << "endTime(MET) ";
        cout << "ra ";
        cout << "dec ";
        cout << "eventClass(transient,source,clean,ultraclean ";
        cout << "conversionType(front,back) ";
        cout << "probability ";
        cout << "outfile ";
        cout << endl << endl;
        cout << "Example (for GRB100728A):" << endl;
        cout << "grspread 275631128 275633628 88.761 -15.2561 source back 0.95 psf95";
        cout << endl;
    }
    
    QueryParameters(int argc, const char * argv[]) {
        if (argc != 9) {
            success = false;
            printUsage();
        }
        else {
            startTime = atof(argv[1]);
            endTime = atof(argv[2]);
            location.ra = atof(argv[3]);
            location.dec = atof(argv[4]);
            probability = atof(argv[7]);
            
            if (strcmp(argv[5], "transient") == 0) {
                eventClass = GRFermiEventClassTransient;
            } else if (strcmp(argv[5], "source") == 0) {
                eventClass = GRFermiEventClassSource;
            } else if (strcmp(argv[5], "clean") == 0) {
                eventClass = GRFermiEventClassClean;
            } else if (strcmp(argv[5], "ultraclean") == 0) {
                eventClass = GRFermiEventClassUltraclean;
            } else {
                printUsage();
                success = false;
                return;
            }
            
            if (strcmp(argv[6], "front") == 0) {
                conversionType = GRFermiConversionTypeFront;
            } else if (strcmp(argv[6], "back") == 0) {
                conversionType = GRFermiConversionTypeBack;
            } else {
                printUsage();
                success = false;
                return;
            }
            
            filename = argv[8];
            
            success = true;
        }
    }
};

int main(int argc, const char * argv[])
{
    QueryParameters parameters(argc, argv);
    if (parameters.success) {
        
        GRFermiLAT fermiLAT;
        GRPsf psf = fermiLAT.psf(parameters.startTime, parameters.endTime, parameters.location, parameters.eventClass, parameters.conversionType);
        
        ofstream out(parameters.filename.c_str());
        psf.writeSpreads(parameters.probability, out);
        out.close();
        
        return 0;
    } else {
        return -1;
    }
}

