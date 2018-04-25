//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.


#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_

#include <string>

// if WINDOWS
// #define DIRBAR "\\"
// else
#define DIRBAR "/"
// endif

using namespace std;

class FileManager {

public:

    /**
     * FileManager class provides access to the database directories structure as well as the files used to
     * store database processed data.
     */

    // File Id constants
    // the internal files used by this application are:
    static const int DB_CONFIG = 0;
    static const int CATALOG = 1;
    static const int CATALOG_VIDEO = 2;
    static const int FEAT_METHOD = 3;
    static const int DESCRIPTORS = 4;
    static const int KEYPOINTS = 5;
    static const int PCA_MODEL = 6;

    static const int VOCABULARY_CATALOG = 7;
    static const int VOCABULARY_CATALOG_VIDEO = 8;
    static const int VOCABULARY_DESCRIPTORS = 9;
    static const int VOCABULARY_KEYPOINTS = 10;

    /**
     * FileManager constructor
     * @param path path to the root directory where database is defined
     */
    FileManager(string &path);

    /**
     * FileManager destructor
     */
    virtual ~FileManager();

    /**
     * @return the path used in the constructor
     */
    string root();

    /**
     * @return the path where data files are stored
     */
    string dataDir();

    /**
     * @return the database input directory path
     */
    string inputDir();

    /**
     * @return the database query directory path
     */
    string queryDir();

    /**
     * @return the database results directory path
     */
    string resultDir();

    /**
     * @return the database vocabulary directory path
     */
    string vocabularyDir();


    string mapData(string prefix);

    /**
     * name: retrieves the file name for the given idfile
     * @param idFile see file id constants
     * @return the file name to be used for the given idfile
     */
    string name(int idFile);

    /**
     * file: retrieves the file path for the given idfile
     * @param idFile see file id constants
     * @return the file path to be used for the given idfile
     */
    string file(int idFile);


private:
    string _path;

};

#endif /* FILEMANAGER_H_ */
