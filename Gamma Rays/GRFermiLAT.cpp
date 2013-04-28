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

size_t GRFermiLAT::saveFermiDataServerResponceToString(char *ptr, size_t size, size_t nmemb, string *string) {
    string->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t GRFermiLAT::saveFermiDataServerResponceToFile(char *ptr, size_t size, size_t nmemb, FILE *file) {
    size_t written;
    written = fwrite(ptr, size, nmemb, file);
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

string GRFermiLAT::downloadPhotons(double startTime, double endTime, GRLocation location) {
    
    string queryHash = hash(startTime, endTime, location);
    if (mkdir(queryHash.c_str(), S_IRWXU ^ S_IRWXG ^ S_IRWXO) == -1) {
        if (errno != EEXIST) {
            perror(queryHash.c_str());
            throw GRFermiLATExceptionMkdir;
        }
    }
    
    cout << "query hash: " << queryHash << endl;
    
    if ((!fileExists(queryHash, "eventList.txt")) || (!fileExists(queryHash, "spacecraft.fits"))) {
        cout << "downloading query hash: " << queryHash << endl;
    } else {
        return queryHash;
    }
    
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
    
    string responce;
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://fermi.gsfc.nasa.gov/cgi-bin/ssc/LAT/LATDataQuery.cgi");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responce);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->saveFermiDataServerResponceToString);
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            throw GRFermiLATExceptionCurlPerform;
        }
        
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
    } else {
        throw GRFermiLATExceptionCurlInit;
    }
    
    string resultsURLLeft = "The results of your query may be found at <a href=\"";
    string resultsURLRight = "\">";
    size_t resultsURLIndex = responce.find(resultsURLLeft);
    
    if (resultsURLIndex == string::npos) {
        string errorMessageLeft = "Unable to handle query";
        string errorMessageRight = "</b>";
        size_t errorMessageIndex = responce.find(errorMessageLeft);
        size_t errorMessageSize = responce.find(errorMessageRight, errorMessageIndex) - errorMessageIndex;
        string errorMessage = responce.substr(errorMessageIndex, errorMessageSize);
        cerr << "Fermi data server error: " << errorMessage << endl;
        cerr << "coordfield=" << coordfield.str().c_str() << " " << "timefield=" << timefield.str().c_str() << endl;
        
        if (responce.find("occurs before data start") != string::npos) throw GRFermiLATExceptionFermiDataServerTooEarly;
        else throw GRFermiLATExceptionFermiDataServerUnknown;
    }
    
    resultsURLIndex += resultsURLLeft.size();
    size_t resultsURLSize = responce.find(resultsURLRight, resultsURLIndex) - resultsURLIndex;
    string resultsURL = responce.substr(resultsURLIndex, resultsURLSize);
    
    bool resultsReady = false;
    vector <string> resultURLs;
    
    while (!resultsReady) {
        responce.clear();
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, resultsURL.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responce);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->saveFermiDataServerResponceToString);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
                throw GRFermiLATExceptionCurlPerform;
            }
        
            curl_easy_cleanup(curl);
        } else {
            throw GRFermiLATExceptionCurlInit;
        }
        
        int progressIndex;
        if (responce.find("Query complete") != string::npos) {
            cout << "complete!" << endl;
            resultsReady = true;
        }
        else if ((progressIndex = (int)responce.rfind("In progress")) != string::npos) {
            cout << "in progress! waiting... (" << responce.substr(progressIndex+38, 6) << ")" << endl;
            system("sleep 1");
        }
        else if (responce.rfind("Query pending") != string::npos) {
            cout << "in queue! waiting..." << endl;
            system("sleep 1");
        }
        else if (responce.rfind("Query in progress") != string::npos) {
            cout << "in progress! waiting... (0)" << endl;
            system("sleep 1");
        } else {
            cerr << "Query is in unknown state. Download failed." << endl;
            cerr << "coordfield=" << coordfield.str().c_str() << " " << "timefield=" << timefield.str().c_str() << endl;
            cerr << "Query results URL: " << resultsURL << endl;
            cerr << "--- start of responce ---" << endl;
            cerr << responce << endl;
            cerr << "--- end of responce ---" << endl;
            throw GRFermiLATExceptionFermiDataServerQuiryStateUnknown;
        }
    }
        
    size_t currentPosition = 0;
    resultURLs.clear();
    while ((currentPosition = responce.find(".fits\">", ++currentPosition)) != string::npos) {
        size_t linkIndex = responce.rfind("href=\"", currentPosition);
        resultURLs.push_back(responce.substr(linkIndex+6, (currentPosition+5) - (linkIndex+6)));
    }
    
    if (!resultURLs.size()) {
        throw GRFermiLATExceptionFermiDataServerEmptyResults;
    }
    
    vector <string> filenames(resultURLs.size());
    
    for (int i = 0; i < resultURLs.size(); i++) {
        filenames[i] = resultURLs[i].substr(resultURLs[i].rfind("/")+1);
        FILE *fits = fopen((queryHash + "/" + filenames[i]).c_str(), "wb");
        if (fits == NULL) {
            cerr << "file open for write error: " << queryHash + "/" + filenames[i] << endl;
            throw GRFermiLATExceptionFileOpen;
        }
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, resultURLs[i].c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fits);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->saveFermiDataServerResponceToFile);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
                throw GRFermiLATExceptionCurlPerform;
            }
            curl_easy_cleanup(curl);
        } else {
            throw GRFermiLATExceptionCurlInit;
        }
        fclose(fits);
    }
    
    ofstream eventList((queryHash + "/eventList.txt").c_str());
    if (eventList.fail()) {
        cerr << "file open for write error: " << queryHash + "/eventList.txt" << endl;
        throw GRFermiLATExceptionFileOpen;
    }
    for (int i = 0; i < filenames.size(); i++) {
        if (filenames[i].find("_EV") != string::npos) eventList << queryHash + "/" + filenames[i] << endl;
        else if (filenames[i].find("_SC") != string::npos) {
            if (symlink(filenames[i].c_str(), (queryHash + "/spacecraft.fits").c_str()) == -1) {
                perror((queryHash + "/spacecraft.fits").c_str());
                throw GRFermiLATExceptionSymlink;
            }
        }
        else {
            cerr << "unknown file type: " << filenames[i] << endl;
            throw GRFermiLATExceptionFermiDataServerUnknownFile;
        }
    }
    eventList.close();
    
    cout << "files downloaded:" << endl;
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
    if (!fileExists(queryHash, "eventList.txt")) {
        throw GRFermiLATExceptionNoEventListFile;
    }
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
    if (!fileExists(queryHash, "spacecraft.fits")) {
        throw GRFermiLATExceptionNoSpacecraftFile;
    }
    if (!fileExists(queryHash, "filtered.fits")) {
        throw GRFermiLATExceptionNoFilteredFile;
    }
    ostringstream cmd;
    cmd << fixed << "gtmktime" << " ";
    cmd << "scfile=" << queryHash << "/spacecraft.fits" << " ";
    cmd << "filter=" << "\"DATA_QUAL==1 && LAT_CONFIG==1\"" << " ";
    cmd << "roicut=" << "no" << " ";
    cmd << "evfile=" << queryHash << "/filtered.fits" << " ";
    cmd << "outfile=" << queryHash << "/timed.fits" << " ";
    cout << cmd.str() << endl;
    if (system(cmd.str().c_str()) != 0) {
        throw GRFermiLATExceptionGtmktimeFailed;
    }
    return queryHash + "/timed.fits";
}

string GRFermiLAT::gtltcube(string queryHash) {
    if (fileExists(queryHash, "ltcube.fits")) return queryHash + "/ltcube.fits";
    if (!fileExists(queryHash, "eventList.txt")) {
        throw GRFermiLATExceptionNoEventListFile;
    }
    if (!fileExists(queryHash, "spacecraft.fits")) {
        throw GRFermiLATExceptionNoSpacecraftFile;
    }
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
    if (!fileExists(queryHash, "ltcube.fits")) {
        throw GRFermiLATExceptionNoLtCubeFile;
    }
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
    try {
        gtmktime(queryHash);
    } catch (GRFermiLATException e) {
        if (e == GRFermiLATExceptionGtmktimeFailed) throw GRFermiLATExceptionNoPhotons;
    }
}

vector <vector <GRPsf> > GRFermiLAT::psfsForAllEventClassesAndConversionTypes(double startTime, double endTime, GRLocation location) {
    string queryHash;
    try {
        queryHash = downloadPhotons(startTime, endTime, location);
    } catch (GRFermiLATException e) {
        if (e == GRFermiLATExceptionFermiDataServerTooEarly) throw GRFermiLATExceptionNoPhotons;
        else throw e;
    }
    
    gtltcube(queryHash);
    
    vector <vector <GRPsf> > psfs;
    psfs.resize(GRFermiEventClassesCount);
    for (int i = 0; i < psfs.size(); i++) psfs[i].reserve(GRFermiConversionTypesCount);
    
    for (int i = 0; i < GRFermiEventClassesCount; i++) {
        for (int j = 0; j < GRFermiConversionTypesCount; j++) {
            string filename = gtpsf(queryHash, location, GRFermiEventClasses[i], GRFermiConversionTypes[j]);
            psfs[i].push_back(GRPsf(filename));
        }
    }
    
    return psfs;
}

GRPsf GRFermiLAT::psf(double startTime, double endTime, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType) {
    string queryHash;
    try {
        queryHash = downloadPhotons(startTime, endTime, location);
    } catch (GRFermiLATException e) {
        if (e == GRFermiLATExceptionFermiDataServerTooEarly) throw GRFermiLATExceptionNoPhotons;
        else throw e;
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
    vector <GRFermiLATPhoton> photons;
    string queryHash;
    try {
        queryHash = downloadPhotons(startTime, endTime, location);
    } catch (GRFermiLATException e) {
        if (e == GRFermiLATExceptionFermiDataServerTooEarly) return photons;
        else throw e;
    }
    
    try {
        processPhotons(queryHash);
    } catch (GRFermiLATException e) {
        if (e == GRFermiLATExceptionNoPhotons) return photons;
        else throw e;
    }
    
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

vector <GRLocation> GRFermiLAT::pointSources() {    
    vector <GRLocation> sources;
    
    fitsfile *sourcesFile;
    
    int status = 0;
    long nrows;
    fits_open_table(&sourcesFile, "gll_psc_v08.fit", READONLY, &status);
    fits_movabs_hdu(sourcesFile, 2, NULL, &status);
    fits_get_num_rows(sourcesFile, &nrows, &status);
    sources.reserve(nrows);
    float readRas[nrows];
    fits_read_col(sourcesFile, TFLOAT, 2, 1, 1, nrows, 0, readRas, 0, &status);
    float readDecs[nrows];
    fits_read_col(sourcesFile, TFLOAT, 3, 1, 1, nrows, 0, readDecs, 0, &status);
    float readErrors[nrows];
    fits_read_col(sourcesFile, TFLOAT, 9, 1, 1, nrows, 0, readErrors, 0, &status);
    
    fits_close_file(sourcesFile, &status);
    
    if (status) fits_report_error(stderr, status);
    
    for (int i = 0; i < nrows; i++) {
        GRLocation location = GRLocation(GRCoordinateSystemJ2000, readRas[i], readDecs[i], readErrors[i]);
        sources.push_back(location);
    }
    
    return sources;
}

vector<GRPhoton> GRFermiLAT::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location, GRFermiEventClass worstEventClass, float confidence, bool filterPointSources) {
    vector <vector <GRPsf> > psfs;
    try {
        psfs = psfsForAllEventClassesAndConversionTypes(startTime, endTime, location);
    } catch (GRFermiLATException e) {
        if (e == GRFermiLATExceptionNoPhotons) {
            vector <GRPhoton> emptyResult;
            return emptyResult;
        }
        else throw e;
    }
    
    vector <GRFermiLATPhoton> photons = allPhotons(startTime, endTime, location);
    
    for (int i = 0; i < photons.size(); i++) {
        photons[i].location.error = psfs[photons[i].eventClass][photons[i].conversionType].spread(photons[i].energy, 1-confidence);
    }
    
    vector <GRLocation> sources;
    if (filterPointSources) {
        vector <GRLocation> sources = pointSources();
    }
    
    vector <GRPhoton> filteredPhotons;
    for (int i = 0; i < photons.size(); i++) {
        if (photons[i].eventClass < worstEventClass) continue;
        if (location.isSeparated(photons[i].location)) continue;
        
        bool fromSource = false;
        if (filterPointSources) for (int j = 0; j < sources.size(); j++) {
            if (!sources[j].isSeparated(photons[i].location)) {
                fromSource = true;
                break;
            }
        }
        
        if (!filterPointSources || !fromSource) filteredPhotons.push_back(GRPhoton(photons[i].time, photons[i].location, photons[i].energy));
    }

    return filteredPhotons;
}