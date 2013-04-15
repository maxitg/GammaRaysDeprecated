//
//  GRFermiLAT.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <sstream>
#include <fstream>
#include <algorithm>
#include <map>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <Security/SecTransform.h>
#include <Security/SecEncodeTransform.h>
#else
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#endif

#include <curl/curl.h>
#include <fitsio.h>

#include "GRFermiLAT.h"

size_t GRFermiLAT::handleFermiDataServerResponce(char *ptr, size_t size, size_t nmemb, GRFermiLAT *me) {
    return me->saveFermiDataServerResponce(ptr, size, nmemb);
}

size_t GRFermiLAT::saveFermiDataServerResponce(char *ptr, size_t size, size_t nmemb) {
    fermiDataServerResponce.append(ptr, size*nmemb);
    return size * nmemb;
}

size_t GRFermiLAT::saveFermiDataServerResponceToFile(char *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

char GRFermiLAT::digitHexCode(int digit) {
    if (digit < 10) return '0'+digit;
    else return 'a'+digit-10;
}

string GRFermiLAT::hash(double startTime, double endTime, GRLocation location) {
    double parameters[4];
    parameters[0] = startTime;
    parameters[1] = endTime;
    parameters[2] = location.ra;
    parameters[3] = location.dec;
    int parametersSize = 4*sizeof(double);
    
    string result;
    
    int digestLength;
    
#ifdef __APPLE__
    
    digestLength = CC_SHA256_DIGEST_LENGTH;
    CC_SHA256_CTX context;
    unsigned char md[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256_Init(&context);
    CC_SHA256_Update(&context, (unsigned char*)parameters, parametersSize);
    CC_SHA256_Final(md, &context);
    
#else
    
    digestLength = SHA256_DIGEST_LENGTH;
    SHA256_CTX context;
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256_Init(&context);
    SHA256_Update(&context, (unsigned char*)parameters, parametersSize);
    SHA256_Final(md, &context);
    
#endif
    
    result.reserve(digestLength*2);
    for (int i = 0; i < digestLength; i++) {
        unsigned char currentChar = md[i];
        result.push_back(digitHexCode(currentChar/16));
        result.push_back(digitHexCode(currentChar%16));
    }
    
    return result;
}

enum GRFermiLATDownloadPhotonsException {
    GRFermiLATDownloadPhotonsExceptionTimeBeforeStart = 0,
    };

string GRFermiLAT::downloadPhotons(double startTime, double endTime, GRLocation location) {
    
    string queryHash = hash(startTime, endTime, location);
    if (mkdir(queryHash.c_str(), S_IRWXU ^ S_IRWXG ^ S_IRWXO) == -1) {
        if (errno == EEXIST) {
            return queryHash;
        }
        else {
            perror(queryHash.c_str());
            return "";
        }
    }
    
    cout << "downloading query hash: " << queryHash << endl;
    
     /*
        Input form to send out
     <form method="post" action="/cgi-bin/ssc/LAT/LATDataQuery.cgi" enctype="multipart/form-data">
     <input value="query" name="destination" type="hidden" />
     <input value="Start Search" type="submit" />
     <input value="Reset" type="reset" />
     <table>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#objectNameOrCoordinates"><b>Object name or coordinates:</b></a></td><td><input value name="coordfield" type="text" /></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#coordinateSystem"><b>Coordinate system:</b></a></td><td><select name="coordsystem"><option>J2000</option><option>B1950</option><option>Galactic</option></select></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#searchRadius"><b>Search radius (degrees):</b></a></td><td><input value name="shapefield" type="text" /></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#observationDates"><b>Observation dates:</b></a></td><td><input value name="timefield" type="text" /></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#timeSystem"><b>Time system:</b></a></td><td><select name="timetype"><option>Gregorian</option><option>MET</option><option>MJD</option></select></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#energyRange"><b>Energy range (MeV):</b></a></td><td><input value name="energyfield" type="text" /></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#LATdataType"><b>LAT data type:</b></a></td><td><select name="photonOrExtendedOrNone"><option>Photon</option><option>Extended</option><option>None</option></select></td></tr>
     <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#spacecraftData"><b>Spacecraft data:</b></a></td><td><input checked="checked" name="spacecraft" id="spacecraft" type="checkbox" /></td></tr>
     </table>
     </form>
     */
        
    ostringstream coordfield;
    coordfield << fixed << location.ra << ", " << location.dec;
    ostringstream timefield;
    timefield << fixed << startTime << ", " << endTime;
    
    CURL *curl;
    CURLcode res;
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    curl_global_init(CURL_GLOBAL_ALL);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "destination",
                 CURLFORM_COPYCONTENTS, "query",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "coordfield",
                 CURLFORM_COPYCONTENTS, coordfield.str().c_str(),
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "coordsystem",
                 CURLFORM_COPYCONTENTS, "J2000",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "shapefield",
                 CURLFORM_COPYCONTENTS, "60",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "timefield",
                 CURLFORM_COPYCONTENTS, timefield.str().c_str(),
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "timetype",
                 CURLFORM_COPYCONTENTS, "MET",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "energyfield",
                 CURLFORM_COPYCONTENTS, "30, 300000",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "photonOrExtendedOrNone",
                 CURLFORM_COPYCONTENTS, "Extended",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "spacecraft",
                 CURLFORM_COPYCONTENTS, "on",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "submit",
                 CURLFORM_COPYCONTENTS, "send",
                 CURLFORM_END);
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://fermi.gsfc.nasa.gov/cgi-bin/ssc/LAT/LATDataQuery.cgi");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->handleFermiDataServerResponce);
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
    }
    
    string resultsURLLeft = "The results of your query may be found at <a href=\"";
    string resultsURLRight = "\">";
    if (fermiDataServerResponce.find(resultsURLLeft) == string::npos) {
        string errorMessageLeft = "Unable to handle query";
        string errorMessageRight = "</b>";
        size_t errorMessageIndex = fermiDataServerResponce.find(errorMessageLeft);
        size_t errorMessageSize = fermiDataServerResponce.find(errorMessageRight, errorMessageIndex) - errorMessageIndex;
        string errorMessage = fermiDataServerResponce.substr(errorMessageIndex, errorMessageSize);
        cerr << "Fermi data server error: " << errorMessage << endl;
        throw GRFermiLATDownloadPhotonsExceptionTimeBeforeStart;
    }
    size_t resultsURLIndex = fermiDataServerResponce.find(resultsURLLeft) + resultsURLLeft.size();
    size_t resultsURLSize = fermiDataServerResponce.find(resultsURLRight, resultsURLIndex) - resultsURLIndex;
    string resultsURL = fermiDataServerResponce.substr(resultsURLIndex, resultsURLSize);
    
    bool resultsReady = false;
    vector <string> resultURLs;
    
    if (curl) {
        while (!resultsReady) {
            fermiDataServerResponce.clear();
            curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, resultsURL.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->handleFermiDataServerResponce);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            
            curl_easy_cleanup(curl);
            
            int progressIndex;
            if (fermiDataServerResponce.find("Query complete") != string::npos) {
                cout << "complete!" << endl;
                resultsReady = true;
            }
            else if ((progressIndex = fermiDataServerResponce.rfind("In progress")) != string::npos) {
                cout << "in progress! waiting... (" << fermiDataServerResponce.substr(progressIndex+38, 6) << ")" << endl;
                system("sleep 1");
            }
            else {
                cerr << "Query is in unknown state. Download failed." << endl;
                cerr << "coordfield=" << coordfield.str().c_str() << " " << "timefield=" << timefield.str().c_str() << endl;
                cerr << "Query results URL: " << resultsURL << endl;
                cerr << "--- start of responce ---" << endl;
                cerr << fermiDataServerResponce << endl;
                cerr << "--- end of responce ---" << endl;
                return "";
            }
        }
        
        size_t location = 0;
        resultURLs.clear();
        while ((location = fermiDataServerResponce.find(".fits\">", ++location)) != string::npos) {
            size_t linkIndex = fermiDataServerResponce.rfind("href=\"", location);
            resultURLs.push_back(fermiDataServerResponce.substr(linkIndex+6, (location+5) - (linkIndex+6)));
        }
    }
    
    vector <string> filenames(resultURLs.size());
    
    for (int i = 0; i < resultURLs.size(); i++) {
        filenames[i] = resultURLs[i].substr(resultURLs[i].rfind("/")+1);
        FILE *fits = fopen((queryHash + "/" + filenames[i]).c_str(), "wb");
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, resultURLs[i].c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fits);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->saveFermiDataServerResponceToFile);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fits);
    }
    
    ofstream eventList((queryHash + "/eventList.txt").c_str());
    for (int i = 0; i < filenames.size(); i++) {
        if (filenames[i].find("_EV") != string::npos) eventList << queryHash + "/" + filenames[i] << endl;
        else if (filenames[i].find("_SC") != string::npos) {
            symlink(filenames[i].c_str(), (queryHash + "/spacecraft.fits").c_str());
        }
        else {
            cerr << "unknown file type: " << filenames[i] << endl;
        }
    }
    eventList.close();
    
    if (resultURLs.size() == 0) cout << "zero results !!!" << endl;
    else cout << "files downloaded:" << endl;
    for (int i = 0; i < resultURLs.size(); i++) {
        cout << resultURLs[i] << endl;
    }
    
    return queryHash;
}

bool GRFermiLAT::fileExists(string queryHash, string fileName) {
    struct stat buf;
    if (stat((queryHash + "/" + fileName).c_str(), &buf) == 0) return true;
    else return false;
}

string GRFermiLAT::gtselect(string queryHash) {
    if (fileExists(queryHash, "filtered.fits")) return queryHash + "/filtered.fits";
    ostringstream cmd;
    cmd << fixed << "gtselect" << " ";
    cmd << "infile=@" << queryHash << "/eventList.txt" << " ";
    cmd << "outfile=" << queryHash << "/filtered.fits" << " ";
    cmd << "ra=" << "INDEF" << " ";
    cmd << "dec=" << "INDEF" << " ";
    cmd << "rad=" << 180 << " ";
    cmd << "tmin=" << "INDEF" << " ";
    cmd << "tmax=" << "INDEF" << " ";
    cmd << "emin=" << 100. << " ";
    cmd << "emax=" << 300000. << " ";
    cmd << "zmax=" << 100. << " ";
    cmd << "evclass=" << 0 << " ";
    cmd << "convtype=" << -1 << " ";
    cmd << "evtable=" << "EVENTS" << " ";
    cmd << "chatter=" << 0 << " ";
    cout << cmd.str() << endl;
    system(cmd.str().c_str());
    return queryHash + "/filtered.fits";
}

string GRFermiLAT::gtmktime(string queryHash) {
    if (fileExists(queryHash, "timed.fits")) return queryHash + "/timed.fits";
    ostringstream cmd;
    cmd << fixed << "gtmktime" << " ";
    cmd << "scfile=" << queryHash << "/spacecraft.fits" << " ";
    cmd << "filter=" << "\"DATA_QUAL==1 && LAT_CONFIG==1\"" << " ";
    cmd << "roicut=" << "no" << " ";
    cmd << "evfile=" << queryHash << "/filtered.fits" << " ";
    cmd << "outfile=" << queryHash << "/timed.fits" << " ";
    cout << cmd.str() << endl;
    system(cmd.str().c_str());
    return queryHash + "/timed.fits";
}

string GRFermiLAT::gtltcube(string queryHash) {
    if (fileExists(queryHash, "ltcube.fits")) return queryHash + "/ltcube.fits";
    ostringstream cmd;
    cmd << fixed << "gtltcube" << " ";
    cmd << fixed << "evfile=@" << queryHash << "/eventList.txt" << " ";
    cmd << fixed << "evtable=" << "EVENTS" << " ";
    cmd << fixed << "scfile=" << queryHash << "/spacecraft.fits" << " ";
    cmd << fixed << "sctable=" << "SC_DATA" << " ";
    cmd << fixed << "outfile=" << queryHash << "/ltcube.fits" << " ";
    cmd << fixed << "dcostheta=" << 0.025 << " ";
    cmd << fixed << "binsz=" << 1 << " ";
    cout << cmd.str() << endl;
    system(cmd.str().c_str());
    return queryHash + "/ltcube.fits";
}

string GRFermiLAT::instrumentResponceFunctionName(GRFermiEventClass eventClass, GRFermiConversionType conversionType) {
    string result = "P7";
    
    if (eventClass == GRFermiEventClassUltraclean) result += "ULTRACLEAN";
    else if (eventClass == GRFermiEventClassClean) result += "CLEAN";
    else if (eventClass == GRFermiEventClassSource) result += "SOURCE";
    else result += "TRANSIENT";
    
    result += (string)"_V6" + (eventClass == GRFermiEventClassSource ? "MC" : "") + "::";
    
    if (conversionType == GRFermiConversionTypeBack) result += "BACK";
    else result += "FRONT";
    
    return result;
}

string GRFermiLAT::gtpsf(string queryHash, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType) {
    string psfFilename = "psf_" + instrumentResponceFunctionName(eventClass, conversionType) + ".fits";
    if (fileExists(queryHash, psfFilename)) return queryHash + "/" + psfFilename;
    ostringstream cmd;
    cmd << fixed << "gtpsf" << " ";
    cmd << "expcube=" << queryHash << "/ltcube.fits" << " ";
    cmd << "outfile=" << queryHash << "/" << psfFilename << " ";
    cmd << "outtable=" << "PSF" << " ";
    cmd << "irfs=" << instrumentResponceFunctionName(eventClass, conversionType) << " ";
    cmd << "ra=" << location.ra << " ";
    cmd << "dec=" << location.dec << " ";
    cmd << "emin=" << 100. << " ";
    cmd << "emax=" << 1000000. << " ";
    cmd << "nenergies=" << 41 << " ";
    cmd << "thetamax=" << 30 << " ";
    cmd << "ntheta=" << 300 << " ";
    cmd << "chatter=" << 2 << " ";
    cout << cmd.str() << endl;
    system(cmd.str().c_str());
    return queryHash + "/" + psfFilename;
}

void GRFermiLAT::processPhotons(string queryHash) {
    gtselect(queryHash);
    gtmktime(queryHash);
}

GRPsf GRFermiLAT::psf(double startTime, double endTime, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType) {
    string queryHash;
    try {
        queryHash = downloadPhotons(startTime, endTime, location);
    } catch (GRFermiLATDownloadPhotonsException) {
        cerr << "too early" << endl;
    }
    
    gtltcube(queryHash);
    string psfFilename = gtpsf(queryHash, location, eventClass, conversionType);
    return GRPsf(psfFilename);
}

// source: http://fermi.gsfc.nasa.gov/ssc/data/analysis/documentation/Cicerone/Cicerone_Data/LAT_Data_Columns.html#PhotonFile
GRFermiEventClass GRFermiLAT::eventClassFromPsfInt(int psfInt) {
    if ((psfInt/16)%2) return GRFermiEventClassUltraclean;
    else if ((psfInt/8)%2) return GRFermiEventClassClean;
    else if ((psfInt/4)%2) return GRFermiEventClassSource;
    else return GRFermiEventClassTransient;
}

// source: http://fermi.gsfc.nasa.gov/ssc/data/analysis/documentation/Cicerone/Cicerone_Data/LAT_Data_Columns.html#PhotonFile
GRFermiConversionType GRFermiLAT::conversinoTypeFromPsfInt(int psfInt) {
    if (psfInt == 0) return GRFermiConversionTypeFront;
    else return GRFermiConversionTypeBack;
}

vector<GRFermiLATPhoton> GRFermiLAT::allPhotons(double startTime, double endTime, GRLocation location) {
    string queryHash;
    try {
        queryHash = downloadPhotons(startTime, endTime, location);
    } catch (GRFermiLATDownloadPhotonsException) {
        cerr << "too early" << endl;
    }
    processPhotons(queryHash);
    
    vector <GRFermiLATPhoton> photons;
    
    string filename = queryHash + "/timed.fits";
    fitsfile *timedFile;
    int status = 0;
    long nrows;
    
    fits_open_table(&timedFile, filename.c_str(), READONLY, &status);
    fits_movabs_hdu(timedFile, 2, NULL, &status);
    fits_get_num_rows(timedFile, &nrows, &status);
    photons.reserve(nrows);
    
    float readEnergies[nrows];
    fits_read_col(timedFile, TFLOAT, 1, 1, 1, nrows, 0, readEnergies, 0, &status);
    
    float readRas[nrows];
    fits_read_col(timedFile, TFLOAT, 2, 1, 1, nrows, 0, readRas, 0, &status);
    
    float readDecs[nrows];
    fits_read_col(timedFile, TFLOAT, 3, 1, 1, nrows, 0, readDecs, 0, &status);
    
    double readTimes[nrows];
    fits_read_col(timedFile, TDOUBLE, 10, 1, 1, nrows, 0, readTimes, 0, &status);
    
    int readEventClasses[nrows];
    fits_read_col(timedFile, TINT, 15, 1, 1, nrows, 0, readEventClasses, 0, &status);
    
    int readConversionTypes[nrows];
    fits_read_col(timedFile, TINT, 16, 1, 1, nrows, 0, readConversionTypes, 0, &status);
    
    fits_close_file(timedFile, &status);
    
    if (status) fits_report_error(stderr, status);
    
    for (int i = 0; i < nrows; i++) {
        GRLocation location = GRLocation(GRCoordinateSystemJ2000, readRas[i], readDecs[i]);
        photons.push_back(GRFermiLATPhoton(readTimes[i], location, readEnergies[i], conversinoTypeFromPsfInt(readConversionTypes[i]), eventClassFromPsfInt(readEventClasses[i])));
    }
    
    return photons;
}

vector<GRPhoton> GRFermiLAT::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location, float locationError, GRFermiEventClass worstEventClass, float confidence) {
    vector <vector <GRPsf> > psfs;
    psfs.resize(GRFermiEventClassesCount);
    for (int i = 0; i < psfs.size(); i++) psfs[i].reserve(GRFermiConversionTypesCount);
    for (int i = 0; i < GRFermiEventClassesCount; i++) {
        for (int j = 0; j < GRFermiConversionTypesCount; j++) {
            psfs[i].push_back(psf(startTime, endTime, location, GRFermiEventClasses[i], GRFermiConversionTypes[j]));
        }
    }
    vector <GRFermiLATPhoton> photons = allPhotons(startTime, endTime, location);
    
    vector <GRPhoton> filteredPhotons;
    for (int i = 0; i < photons.size(); i++) {
        if (photons[i].eventClass < worstEventClass) continue;
        if (location.separation(photons[i].location) > psfs[photons[i].eventClass][photons[i].conversionType].spread(photons[i].energy, 1-confidence) + locationError) continue;
        filteredPhotons.push_back(GRPhoton(photons[i].time, photons[i].location, photons[i].energy));
    }

    return filteredPhotons;
}