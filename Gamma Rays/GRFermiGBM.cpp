//
//  GRFermiGBM.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 14.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <fstream>

#include <curl/curl.h>
#include <fitsio.h>

#include "GRFermiGBM.h"

double GRFermiGBM::timeFromUnixTime(double unixTime) {
    tm tm1 = {0};
    tm1.tm_year = 70;
    tm1.tm_mon = 0;
    tm1.tm_mday = 1;
    
    tm tm2 = {0};
    tm2.tm_year = 101;
    tm2.tm_mon = 0;
    tm2.tm_mday = 1;
    
    return unixTime + mktime(&tm1) - mktime(&tm2);
}

double GRFermiGBM::currentTime() {
    return timeFromUnixTime(time(NULL));
}

double GRFermiGBM::latestDownloadTime() {
    ifstream timestamp("timestamp");
    double time;
    timestamp >> time;
    timestamp.close();
    return time;
}

void GRFermiGBM::download() {

    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    FILE *gbmRequestInput;
    FILE *grbCatalog;
    gbmRequestInput = fopen("gbm_request", "rb");
    grbCatalog = fopen("grbCatalog.fits", "w");
    
    fseek(gbmRequestInput, 0, SEEK_END);
    int gbmRequestSize = ftell(gbmRequestInput);
    fseek(gbmRequestInput, 0, SEEK_SET);
    
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://heasarc.gsfc.nasa.gov/db-perl/W3Browse/w3query.pl");
    
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, gbmRequestSize);
    curl_easy_setopt(curl, CURLOPT_READDATA, gbmRequestInput);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, grbCatalog);
    
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    
    curl_easy_cleanup(curl);
    fclose(gbmRequestInput);
    fclose(grbCatalog);
    
    ofstream timestamp("timestamp");
    timestamp << fixed << currentTime() << endl;
    timestamp.close();
    return;
}

vector <GRBurst> GRFermiGBM::bursts(double startTime, double endTime) {
    if (latestDownloadTime() < endTime) download();
    
    vector <GRBurst> bursts;
    fitsfile *catalogFile;
    int status = 0;
    long nrows;
    
    fits_open_table(&catalogFile, "grbCatalog.fits", READONLY, &status);
    fits_movabs_hdu(catalogFile, 2, NULL, &status);
    fits_get_num_rows(catalogFile, &nrows, &status);
    
    char *readNames[nrows];
    for (int i = 0; i < nrows; i++) {
        readNames[i] = new char(32);
    }
    fits_read_col(catalogFile, TSTRING, 1, 1, 1, nrows, 0, readNames, 0, &status);
    
    float readRas[nrows];
    fits_read_col(catalogFile, TFLOAT, 2, 1, 1, nrows, 0, readRas, 0, &status);
    
    float readDecs[nrows];
    fits_read_col(catalogFile, TFLOAT, 3, 1, 1, nrows, 0, readDecs, 0, &status);
    
    double readTimes[nrows];
    fits_read_col(catalogFile, TDOUBLE, 4, 1, 1, nrows, 0, readTimes, 0, &status);
    
    double readDurations[nrows];
    fits_read_col(catalogFile, TDOUBLE, 5, 1, 1, nrows, 0, readDurations, 0, &status);
    
    double readDurationErrors[nrows];
    fits_read_col(catalogFile, TDOUBLE, 6, 1, 1, nrows, 0, readDurationErrors, 0, &status);
    
    double readStartTimes[nrows];
    fits_read_col(catalogFile, TDOUBLE, 7, 1, 1, nrows, 0, readStartTimes, 0, &status);
    
    float readLocationErrors[nrows];
    fits_read_col(catalogFile, TFLOAT, 8, 1, 1, nrows, 0, readLocationErrors, 0, &status);
    
    fits_close_file(catalogFile, &status);
    
    if (status) fits_report_error(stderr, status);
    
    for (int i = 0; i < nrows; i++) {
        double time = timeFromUnixTime((readTimes[i]-40587.)*86400.) + readStartTimes[i];
        if (time < startTime || time > endTime) continue;
        GRLocation location = GRLocation(GRCoordinateSystemJ2000, readRas[i], readDecs[i]);
        bursts.push_back(GRBurst(readNames[i], time, readDurations[i], readDurationErrors[i], location, readLocationErrors[i]));
    }
    
    for (int i = 0; i < nrows; i++) {
        delete readNames[i];
    }
    
    return bursts;
}