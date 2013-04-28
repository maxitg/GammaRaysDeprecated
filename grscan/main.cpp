//
//  main.cpp
//  grscan
//
//  Created by Maxim Piskunov on 15.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

//#include </usr/llvm-gcc-4.2/lib/gcc/i686-apple-darwin11/4.2.1/include/omp.h>

#include <math.h>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

#include "GRBurst.h"
#include "GRFermiLAT.h"
#include "GRFermiGBM.h"
#include "GRPhotonStorage.h"
#include "GRBurstStorage.h"

using namespace std;

void drawPicture(GRBurst burst, double probability, string filename, bool chat = false, double lengtheningSteps = 100, double shiftSteps = 100) {
    struct stat buf;
    if (stat(filename.c_str(), &buf) == 0) return;
    
    bool success;
    double minL = burst.minLengtheningAllowed(probability, &success);
    double maxL = burst.maxLengtheningAllowed(probability, &success);
    double sprL = exp((log(maxL) - log(minL)) * 0.1);
    minL = minL/sprL;
    maxL = maxL*sprL;
    double stepL = (maxL - minL)/lengtheningSteps;
    double minS = burst.minShiftAllowed(probability, &success);
    double maxS = burst.maxShiftAllowed(probability, &success);
    double sprS = (maxS - minS) * 0.1;
    minS = minS - sprS;
    maxS = maxS + sprS;
    double stepS = (maxS - minS)/shiftSteps;
    
    if (minL == -INFINITY || minS == -INFINITY || maxL == INFINITY || maxS == INFINITY) {
        if (chat) cout << "infinite ranges. cannot draw" << endl;
        return;
    }
    
    if (!success) {
        if (chat) cout << "too low to draw" << endl;
        return;
    }
    
    ofstream dens(filename.c_str());
    
    for (double lengthening = minL; lengthening <= maxL; lengthening += stepL) {
        if (chat) cout << "drawing " << lengthening << " ..." << endl;
        for (double shift = minS; shift <= maxS; shift += stepS) {
            double prob = burst.gevTransformHypothesisProbability(shift, lengthening);
            dens << lengthening << " " << shift << " " << (prob > probability ? prob : 0) << endl;
        }
    }
    dens.close();
}

void drawCDF(GRBurst burst, float minEnergy, float maxEnergy, string filename) {
    ofstream cdfFile(filename.c_str());
    vector <GRDistributionCDFPoint> points = burst.photonDistributionFromStart(minEnergy, maxEnergy).cdf();
    for (int i = 0; i < points.size(); i++) {
        cdfFile << points[i].value << " " << points[i].probability << endl;
    }
    cdfFile.close();
}

void printException(GRFermiLATException e) {
    if (e == GRFermiLATExceptionMkdir) cerr << "mkdir failed" << endl;
    else if (e == GRFermiLATExceptionFileOpen) cerr << "file open failed" << endl;
    else if (e == GRFermiLATExceptionSymlink) cerr << "symlink failed" << endl;
    else if (e == GRFermiLATExceptionCurlInit) cerr << "curl initialization failed" << endl;
    else if (e == GRFermiLATExceptionCurlPerform) cerr << "curl request failed" << endl;
    else if (e == GRFermiLATExceptionFermiDataServerTooEarly) cerr << "photons requested are before Fermi LAT launch date" << endl;
    else if (e == GRFermiLATExceptionFermiDataServerUnknown) cerr << "unknown error from Fermi data server" << endl;
    else if (e == GRFermiLATExceptionFermiDataServerQuiryStateUnknown) cerr << "unknown state of Fermi data server query" << endl;
    else if (e == GRFermiLATExceptionFermiDataServerEmptyResults) cerr << "empty results from Fermi data server" << endl;
    else if (e == GRFermiLATExceptionFermiDataServerUnknownFile) cerr << "unknown file type recieved from Fermi data server" << endl;
    else if (e == GRFermiLATExceptionNoEventListFile) cerr << "eventList.txt not found" << endl;
    else if (e == GRFermiLATExceptionNoFilteredFile) cerr << "filtered.fits not found" << endl;
    else if (e == GRFermiLATExceptionNoSpacecraftFile) cerr << "spacecraft.fits not found" << endl;
    else if (e == GRFermiLATExceptionNoLtCubeFile) cerr << "ltcube.fits not found" << endl;
    else if (e == GRFermiLATExceptionNoPhotons) cerr << "no photons match the query" << endl;
    else cerr << "unknown exception occured" << endl;
}

int main(int argc, const char * argv[])
{    
    try {
        
        ofstream log("log");
        ofstream interestingLog("interestingLog");
        /*
        tm tm1 = {0};
        tm1.tm_year = 70;
        tm1.tm_mon = 0;
        tm1.tm_mday = 1;
        
        tm tm2 = {0};
        tm2.tm_year = 101;
        tm2.tm_mon = 0;
        tm2.tm_mday = 1;
        
        double now = time(NULL) + mktime(&tm1) - mktime(&tm2);
        
        GRBurstStorage *storage = NULL;
        storage->getInstance();
        vector <GRBurst> burstCatalog = storage->bursts(0., now);
        */
        
        vector <GRBurst> burstCatalog;
        ifstream burstsFile("bursts");
        while (!burstsFile.eof()) {
            string name;
            double time;
            float ra;
            float dec;
            float error;
            double startOffset;
            double endOffset;
            
            burstsFile >> name >> time >> ra >> dec >> error >> startOffset >> endOffset;
            
            double duration = endOffset - startOffset;
            startOffset -= duration*0.2;
            endOffset += duration*0.2;
            
            if (startOffset != 0) {
                GRBurst burst = GRBurst(name, time, GRLocation(GRCoordinateSystemJ2000, ra, dec, error));
                burst.startOffset = startOffset;
                burst.endOffset = endOffset;
                burstCatalog.push_back(burst);
                cout << name << " " << time << " " << GRLocation(GRCoordinateSystemJ2000, ra, dec, error).description() << endl;
            }
        }
                
        //#pragma omp parallel for
        for (int i = 0; i < burstCatalog.size(); i++) {
                        
            cout << "processing " << burstCatalog[i].name << "..." << endl;
            //int gevCount = burstCatalog[i].gevCount();
            //log << burstCatalog[i].name << " : " << gevCount << " @ " << burstCatalog[i].location.error << endl;
            
            log << burstCatalog[i].info() << endl;
            
            if (burstCatalog[i].gevCount() < 10) continue;
            
            //interestingLog << burstCatalog[i].name << " : " << gevCount << " @ " << burstCatalog[i].location.error << endl;
            //continue;
            
            interestingLog << burstCatalog[i].info() << endl;
            
            vector <GRPhoton> photons = burstCatalog[i].photons();
            for (int i = 0; i < photons.size(); i++) {
                if (photons[i].energy < 1000.) cout << photons[i].description() << endl;
            }
            for (int i = 0; i < photons.size(); i++) {
                if (photons[i].energy >= 1000.) cout << photons[i].description() << endl;
            }
            
            drawCDF(burstCatalog[i], 0., 1000., burstCatalog[i].name + ".MeV.cdf");
            drawCDF(burstCatalog[i], 1000., INFINITY, burstCatalog[i].name + ".GeV.cdf");
            
            int sigmaValues[5] = {1, 2, 3, 4, 5};
            double probabilityValues[5];
            for (int j = 0; j < 5; j++) {
                probabilityValues[j] = 1-erf((double)sigmaValues[j]/sqrt(2.));
            }
            for (int j = 0; j < 5; j++) {
                ostringstream filename;
                filename << burstCatalog[i].name << "." << sigmaValues[j] << "s.slm";
                drawPicture(burstCatalog[i], probabilityValues[j], filename.str(), true);
            }
        }
        
        log.close();
        interestingLog.close();
        
    } catch (GRFermiLATException e) {
        printException(e);
    }
    
    return 0;
}