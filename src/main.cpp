//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <cv.hpp>

#include "FileHelper.h"
#include "FeatureMethod.h"
#include "Database.h"


#include "Server.h"


using namespace std;
using namespace cv;

// -------------------------------------------




/**
 * Prints help for building a database
 */
void printHelpBuild(string cmd) {

    cout << "---" << endl;
    cout << "option \"-build\" builds a database: " << endl;
    cout << "parameters: " << endl;
    cout << "\t" << "[-reuse]: reuses features, if not specified features will be extracted from input files" << endl;
    cout << endl;
    cout << "\t" << "[-method <DETECTION>:<EXTRACTION>]: features method." << endl;
    cout << "\t\t" << "where " << endl;
    cout << endl;
    cout << "\t\t" << "<DETECTION> is the method used for features detection and" << endl;
    cout << "\t\t\t" << "available methods are: " << endl;
    cout << "\t\t\t\t" << "FAST, STAR, SIFT, SURF, ORB, BRISK, MSER, GFTT, HARRIS, Dense, SimpleBlob" << endl;
    cout << endl;
    cout << "\t\t" << "<EXTRACTION> is the method used for features extraction" << endl;
    cout << "\t\t\t" << "available methods are: " << endl;
    cout << "\t\t\t\t" << "SIFT, SURF, BRIEF, BRISK, ORB, FREAK" << endl;
    cout << endl;
    cout << "\t\t\t" << "default method is SIFT:SIFT" << endl;
    cout << endl;
    cout << "\t" << "[-vtp <K>:<H>]: vocabulary tree parameters." << endl;
    cout << "\t\t" << "where K is the branch factor, and H is the maximum height for the tree." << endl;
    cout << endl;
    cout << "\t" << "[-pca N]: if specified pca is applied over the extracted descriptors." << endl;
    cout << "\t\t" << "Dimensions are reduced to N." << endl;
    cout << endl;
    cout << "---" << endl;
    cout << endl;
    cout << "\t" << "example:" << endl;
    cout << "\t" << cmd << " -build /home/myuser/mydb -method SURF:SURF -vtp 16:5 -pca 32" << endl;
    cout << endl;
    cout << "---" << endl;

}

/**
 * buildDatabase builds a database
 *
 *      A database is identified by its root path in the filesystem (or dbPath param)
 *      and its directories structure is as follows:
 *
 *      <dbPath>/vocabulary: contains the files that will be used to compute the vocabulary
 *      <dbPath>/input: contains the files that will be indexed
 *      <dbPath>/queries: contains files to be queried
 *      <dbPath>/results: here resulting query images will copied (this is necessary for online demo)
 *      <dbPath>/data: here internal database data is stored (BoF files, file catalogs, etc.)
 *
 *      vocabulary and input files can be images or videos.
 *
 *
 * @param argc parameters count received from command line
 * @param argv parameters for building the database
 *              [-reuse]: reuses features, if not specified features will be extracted from input files
 *              [-method <DETECTION>:<EXTRACTION>]: features method.
 *                      where
 *                          <DETECTION> is the method used for features detection, and
 *                          <EXTRACTION> is the method used for features extraction
 *                      default is SIFT:SIFT
 *                      See FeaturesMethod.h for available feature detector and extractors.
 *              [-vtp <K>:<H>]: vocabulary tree parameters.
 *                      where K is the branch factor, and H is the maximum height for the tree.
 *              [-pca N]: if specified pca is applied over the extracted descriptors.
 *                          Dimensions are reduced to N.
 *
 */
void buildDatabase(string dbPath, int argc, char **argv) {

    bool reuseFeatures = false;

    string method = "SIFT:SIFT";
    string vtParams = "10:6";
    string strPCA = "0";

    if (argc >= 4) {

        if (strcasecmp(argv[3], "-reuse") == 0) {
            reuseFeatures = true;
        }
        else if (strcasecmp(argv[3], "-method") == 0) {
            if (argc >= 5) {
                method = argv[4];
            }

            if (argc >= 7) {
                if (strcasecmp(argv[5], "-vtp") == 0) {
                    vtParams = argv[6];
                }
            }

            if (argc >= 9) {
                if (strcasecmp(argv[7], "-pca") == 0) {
                    strPCA = argv[8];
                }
            }
        }

    }


    int pos;
    pos = method.find(":");
    if (pos == -1) {
        cerr << "invalid method" << endl;
        return;
    }
    string dt = method.substr(0, pos);
    string et = method.substr(pos + 1);
    int detectorType = FeatureMethod::getDetectorType(dt);
    int extractorType = FeatureMethod::getExtractorType(et);


    pos = vtParams.find(":");
    if (pos == -1) {
        cerr << "invalid voctree parameters" << endl;
        return;
    }
    int k = atoi(vtParams.substr(0, pos).c_str());
    int h = atoi(vtParams.substr(pos + 1).c_str());
    int pca = atoi(strPCA.c_str());

    cout << "building database " << dbPath << "..." << endl << flush;
    cout << "feature method: " << method << endl << flush;
    cout << "voctree: k:" << k << " h: " << h << endl << flush;

    FeatureMethod fm(detectorType, extractorType);
    int maxFiles = 0;
    int maxFilesVocabulary = 0;
    bool reuseVocabulary = reuseFeatures;

    Database::build(dbPath, fm, reuseFeatures, k, h, maxFiles, maxFilesVocabulary, reuseVocabulary, pca);
    cout << "build done." << endl << flush;


    return;
}


/**
 * Prints help for updating a database
 */
void printHelpUpdate() {

    cout << "---" << endl;
    cout << "Updating a database: " << endl;
    cout << "parameters: " << endl;
    cout << "\t" << "[-reuse]: reuses features, if not specified features will be extracted from input files" << endl;
    cout << endl;
    cout << "\t" << "[-method <DETECTION>:<EXTRACTION>]: features method." << endl;
    cout << "\t\t" << "where " << endl;
    cout << endl;
    cout << "\t\t" << "<DETECTION> is the method used for features detection and" << endl;
    cout << "\t\t\t" << "available methods are: " << endl;
    cout << "\t\t\t\t" << "FAST, STAR, SIFT, SURF, ORB, BRISK, MSER, GFTT, HARRIS, Dense, SimpleBlob" << endl;
    cout << endl;
    cout << "\t\t" << "<EXTRACTION> is the method used for features extraction" << endl;
    cout << "\t\t\t" << "available methods are: " << endl;
    cout << "\t\t\t\t" << "SIFT, SURF, BRIEF, BRISK, ORB, FREAK" << endl;
    cout << endl;
    cout << "\t\t\t" << "default method is SIFT:SIFT" << endl;
    cout << endl;
    cout << "\t" << "[-vtp <K>:<H>]: vocabulary tree parameters." << endl;
    cout << "\t\t" << "where K is the branch factor, and H is the maximum height for the tree." << endl;
    cout << endl;
    cout << "\t" << "[-pca N]: if specified pca is applied over the extracted descriptors." << endl;
    cout << "\t\t" << "Dimensions are reduced to N." << endl;
    cout << endl;
    cout << "---" << endl;

}

/**
 * updateDatabase: checks if there are new files in the dbPath/input directory.
 *  New files are then indexed in the database
 *
 * @param dbPath path where database root is placed in the filesystem
 */

int updateDatabase(string dbPath) {

    cout << "updating database " << dbPath << "..." << endl << flush;

    Database::update(dbPath);
    cout << "update done." << endl << flush;
    return 0;

}



/**
 * Prints usage
 * @param cmd command line name
 */

void printHelpOptions(string cmd) {

    cout << "---" << endl;
    cout << "Usage: " << endl;
    cout << "\t" << cmd << " <option> dbPath [params]" << endl;
    cout << endl;
    cout << "\t" << "dbPath: path where database is placed. "
            "A database is identified by its path on disk." << endl;
    cout << endl;
    cout << "\t" << "available options: " << endl;
    cout << endl;
    cout << "\t" << "-help: displays help" << endl;
    cout << "\t" << "-build: builds a new database" << endl;
    cout << "\t" << "-update: updates a database" << endl;
    cout << "\t" << "-state: returns the server state (STOPPED, STARTING or STARTED)" << endl;
    cout << "\t" << "-start: starts server for receiving queries" << endl;
    cout << "\t" << "-stop: stops server" << endl;
    cout << "\t" << "-query: does a query" << endl;
    cout << "\t" << "-unlock: unlocks server" << endl;
    cout << endl;
    cout << "\t" << "for specific option parameters run:" << endl;
    cout << "\t" << cmd << " -help <option>" << endl;
    cout << "---" << endl;


}


void printHelpStart() {

    cout << "---" << endl;
    cout << "option \"-start\" starts the server: " << endl;
    cout << "In order to perform queries, the server must be started." << endl;
    cout << "When server is started, the vocabulary tree is loaded in memory." << endl;
    cout << "If no query is received for 5 minutes, then the server stops itself, and memory is released." << endl;

}

void printHelpStop() {

    cout << "---" << endl;
    cout << "option \"-stop\" stops the server: " << endl;

}

void printHelpUnlock() {

    cout << "---" << endl;
    cout << "option \"-unlock\" unlocks the server: " << endl;
    cout << "When starting a database, a lock file is created." << endl;
    cout << "After server is correctly started, lock is deleted." << endl;
    cout << "If an unexpected problem occurs, then the lock won't be deleted." << endl;
    cout << "In that case, use this command to delete that file and allow server to start." << endl;

}

void printHelpState() {

    cout << "---" << endl;
    cout << "option \"-state\": returns the server state (STOPPED, STARTING or STARTED)" << endl;

}

void printHelpQuery(string cmd) {

    cout << "---" << endl;
    cout << "option \"-query\": performs a query" << endl;
    cout << "parameters: " << endl;
    cout << "\t" << "<query file>: file to be queried" << endl;
    cout << "\t" << "resulting columns are: score, file id, exported file relative path" << endl;
    cout << "\t" << "resulting files will be copied to the folder <dbPath>/results" << endl;
    cout << "---" << endl;
    cout << endl;
    cout << "\t" << "example:" << endl;
    cout << "\t" << cmd << " -query /home/myuser/mydb /home/myuser/mydb/queries/image1.jpg" << endl;
    cout << endl;
    cout << "---" << endl;


}


void printHelp(string cmd, string option) {


    if (strcasecmp(option.c_str(), "build") == 0) {
        printHelpBuild(cmd);
    }
    else
    if (strcasecmp(option.c_str(), "update") == 0) {
        printHelpUpdate();
    }
    else
    if (strcasecmp(option.c_str(), "start") == 0) {
        printHelpStart();
    }
    else
    if (strcasecmp(option.c_str(), "stop") == 0) {
        printHelpStop();
    }
    else
    if (strcasecmp(option.c_str(), "unlock") == 0) {
        printHelpUnlock();
    }
    else
    if (strcasecmp(option.c_str(), "state") == 0) {
        printHelpState();
    }
    else
    if (strcasecmp(option.c_str(), "query") == 0) {
        printHelpQuery(cmd);
    }
    else {

        cerr << "unknown option" << endl;
        printHelpOptions(cmd);

    }



}

/**
 * main entry point for the application
 */

int main(int argc, char **argv) {


    string cmd = argv[0];

    if (argc < 2) {

        cerr << "must specify an option" << endl;
        printHelpOptions(cmd);
        return -1;
    }

    string option = argv[1];

    if (strcasecmp(option.c_str(), "-help") == 0) {

        if (argc < 3) {
            printHelpOptions(cmd);
            return 0;
        }

        string opt = argv[2];
        printHelp(cmd, opt);

        return 0;
    }


    if (argc < 3) {
        cerr << "must specify database path" << endl;
        printHelpOptions(cmd);
        return -1;
    }

    string dbPath = argv[2];

        // Checks that exists the database path
    if (!FileHelper::exists(dbPath)) {
        cerr << "could not find database " << dbPath << "." << endl;
        exit(-1);
    }

    if (strcasecmp(option.c_str(), "-build") == 0) {
        buildDatabase(dbPath, argc, argv);
    }
    else
    if (strcasecmp(option.c_str(), "-update") == 0) {
        updateDatabase(dbPath);
    }
    else
    if (strcasecmp(option.c_str(), "-start") == 0) {
        startDatabase(dbPath);
    }
    else
    if (strcasecmp(option.c_str(), "-stop") == 0) {
        stopDatabase(dbPath);
    }
    else
    if (strcasecmp(option.c_str(), "-unlock") == 0) {
        delStartingLock(dbPath);
    }
    else
    if (strcasecmp(option.c_str(), "-state") == 0) {
        cout << getState(dbPath) << endl;
    }
    else
    if (strcasecmp(option.c_str(), "-query") == 0) {

        if (argc < 4) {
            cerr << "ERR: must specify the query" << endl;
            printHelpQuery(cmd);

            return -1;
        }
        string query = argv[3];

        runQuery(dbPath, query);

    }
    else {

        cerr << "unknow option" << endl;
        printHelpOptions(cmd);

    }


    return 0;

}

