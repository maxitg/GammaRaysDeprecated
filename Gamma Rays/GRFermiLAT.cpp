//
//  GRFermiLAT.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>

#include <sstream>
#include <fstream>
#include <algorithm>

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

void GRFermiLAT::downloadPhotons(double startTime, double endTime, GRLocation location) {
    
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
                 CURLFORM_COPYCONTENTS, "180",
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
                return;
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
        FILE *fits = fopen(filenames[i].c_str(), "wb");
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, resultURLs[i].c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fits);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->saveFermiDataServerResponceToFile);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fits);
    }
    
    ofstream eventList("eventList.txt");
    for (int i = 0; i < resultURLs.size(); i++) {
        eventList << resultURLs[i] << endl;
    }
    eventList.close();
    
    if (resultURLs.size() == 0) cout << "zero results !!!" << endl;
    else cout << "files downloaded:" << endl;
    for (int i = 0; i < resultURLs.size(); i++) {
        cout << resultURLs[i] << endl;
    }
}

vector<GRFermiLATPhoton> GRFermiLAT::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location, GRFermiEventClass worstEventClass) {
    ostringstream gtselect;
    gtselect << "gtselect infile=@eventList.txt outfile=dataSelected.fits " << fixed;
    gtselect << "ra=" << location.ra << " ";
    gtselect << "dec=" << location.dec << " ";
    gtselect << "rad=" << "180" << " ";
    gtselect << "tmin=" << startTime << " ";
    gtselect << "tmax=" << endTime << " ";
    gtselect << "emin=" << minEnergy << " ";
    gtselect << "emax=" << maxEnergy << " ";
    gtselect << "zmax=" << 100. << " ";
    gtselect << "evclass=" << (worstEventClass == GRFermiEventClassTransient ? worstEventClass : worstEventClass+1) << " ";
    gtselect << "convtype=" << -1 << " ";
    gtselect << "evtable=" << "EVENTS" << " ";
    gtselect << "chatter=" << 0 << " ";
    cout << gtselect.str() << endl;
    system(gtselect.str().c_str());
    
}