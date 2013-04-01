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

#include "GRFermiLAT.h"
#include <curl/curl.h>

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

struct numbers {
    double startTime;
    double endTime;
    float ra;
    float dec;
};

string GRFermiLAT::hash(double startTime, double endTime, GRLocation location) {
    double parameters[4];
    parameters[0] = startTime;
    parameters[1] = endTime;
    parameters[2] = location.ra;
    parameters[3] = location.dec;
    int parametersSize = 4*sizeof(double);
    
    string result;
    
#ifdef __APPLE__
    
    CC_SHA256_CTX context;
    unsigned char md[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256_Init(&context);
    CC_SHA256_Update(&context, (unsigned char*)parameters, parametersSize);
    CC_SHA256_Final(md, &context);
    
    CFDataRef dataToEncode = CFDataCreate(kCFAllocatorDefault, md, CC_SHA256_DIGEST_LENGTH);
    CFErrorRef error = NULL;
    SecTransformRef encodingRef = SecEncodeTransformCreate(kSecBase64Encoding, &error);
    SecTransformSetAttribute(encodingRef, kSecTransformInputAttributeName, dataToEncode, &error);
    CFDataRef resultData = (CFDataRef)SecTransformExecute(encodingRef, &error);
    CFStringRef str = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, resultData, kCFStringEncodingUTF8);
    
    char base64Pointer[256];
    CFStringGetCString(str, base64Pointer, 256, kCFStringEncodingUTF8);
    
    result = string(base64Pointer);
    
#else
    
    SHA256_CTX context;
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256_Init(&context);
    SHA256_Update(&context, (unsigned char*)parameters, parametersSize);
    SHA256_Final(md, &context);
    
    BIO * mem = BIO_new(BIO_s_mem());
    
    BIO * b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    mem = BIO_push(b64, mem);
    
    BIO_write(mem, md, SHA256_DIGEST_LENGTH);
    BIO_flush(mem);
    
    char * base64Pointer;
    long base64Length = BIO_get_mem_data(mem, &base64Pointer);
    
    result = string(base64Pointer, base64Length);
    
    BIO_free_all(mem);
    
#endif
    
    return result;
}

string GRFermiLAT::downloadPhotons(double startTime, double endTime, GRLocation location) {
    
    string queryHash = hash(startTime, endTime, location);
    cout << "query hash: " << queryHash << endl;
    if (mkdir(queryHash.c_str(), S_IRWXU ^ S_IRWXG ^ S_IRWXO) == -1) {
        if (errno == EEXIST) {
            cout << "already downloaded!" << endl;
            return queryHash;
        }
        else {
            perror(queryHash.c_str());
            return "";
        }
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
            curl_easy_cleanup(curl);
            
            if (fermiDataServerResponce.find("Query complete") != string::npos) {
                cout << "complete!" << endl;
                resultsReady = true;
            }
            else if (fermiDataServerResponce.find("Query in progress") != string::npos) {
                cout << "in progress! waiting..." << endl;
                system("sleep 1");
            }
            else {
                cerr << "Query is in unknown state. Download failed." << endl;
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
    cmd << "filter=" << "\"DATA_QUAL==1 && LAT_CONFIG==1 && ABS(ROCK_ANGLE)<52\"" << " ";
    cmd << "roicut=" << "yes" << " ";
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
    cmd << fixed << "evfile=" << queryHash << "/timed.fits" << " ";
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
    
    result += "_V6::";
    
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
    gtltcube(queryHash);
}

GRPsf GRFermiLAT::psf(double startTime, double endTime, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType) {
    string queryHash = downloadPhotons(startTime, endTime, location);
    processPhotons(queryHash);
    string psfFilename = gtpsf(queryHash, location, eventClass, conversionType);
    return GRPsf(psfFilename);
}

vector<GRFermiLATPhoton> GRFermiLAT::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location, GRFermiEventClass worstEventClass) {
    vector <GRFermiLATPhoton> photons;
    return photons;
}