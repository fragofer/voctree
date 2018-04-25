//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef FILEHELPER_H_
#define FILEHELPER_H_

#define _PATH_MAX 4096

#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace std;


class FileHelper {

public:

    /**
     * FileHelper class provides basic functionality for dealing with files and filesystem operations
     */


    /**
     * FileHelper constructor
     */
    FileHelper();

    /**
   * FileHelper destructor
   */
    virtual ~FileHelper();

    static const int TYPE_FILE = 0;
    static const int TYPE_DIRECTORY = 1;
    static const int TYPE_OTHER = 3;

    struct Entry {

        // base path
        string path_base;
        // relative path
        string path_rel;
        // file name without path
        string fileName;
        // file type
        int type;
        time_t lastModif;

        string relName() const {
            string ret;
            ret += path_rel + fileName;
            return ret;
        }

        string fullName() const {
            string ret(path_base);
            ret += relName();
            return ret;
        }

    };

    /**
     * Checks whether a file or path exist
     * @param path path to check
     * @return true if file or path exist
     */
    static bool exists(const string path);

    /**
     * Checks if a given path is a file
     * @param path path to check
     * @return true if path is a file
     */
    static bool isFile(const string path);

    /**
     * Checks if path is a directory
     * @param path path to check
     * @return true if path is a directory
     */
    static bool isDirectory(const string path);

    /**
     * Creates a directory
     * @param path with the directory to be created
     */
    static void createDir(const string path);

    /**
     * Given a path retrieves a vector with file entries. see Entry struct
     * @param path path to be retrieved
     * @param result the vector with the file entries
     * @param recursive if true subdirectories will be explored
     */
    static void listDir(const string path, vector<Entry> &result, bool recursive);

    /**
     * Copies a file from source to target
     * @param source path to the source file
     * @param target path to the target file
     */
    static void copy(const string source, const string target);

    /**
     * deletes a file file
     * @param path path to the file to be deleted
     */
    static void deleteFile(const string path);

    /**
     * Returns the path where application is currently running
     * @return the current path
     */
    static string currentPath();

private:
    static void listDir(const string path, const string path_rel, vector<Entry> &result, bool recursive);

};

#endif /* FILEHELPER_H_ */
