//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "FileHelper.h"

#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include <dirent.h>

#include <fstream>
#include <string.h>


#include <algorithm>

using namespace std;

void getInfo(const string path, struct stat &info) {

    if (stat(path.c_str(), &info) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

}


bool
FileHelper::exists(const string path) {

    struct stat info;
    return (stat(path.c_str(), &info) == 0);

}

bool
FileHelper::isFile(const string path) {
    struct stat info;
    getInfo(path, info);
    return S_ISREG(info.st_mode);
}


bool
FileHelper::isDirectory(const string path) {

    struct stat info;
    getInfo(path, info);
    return S_ISDIR(info.st_mode);

}

void
FileHelper::deleteFile(const string path) {
    remove(path.c_str());
}


void
FileHelper::createDir(const string path) {

    struct stat info;
    if (stat(path.c_str(), &info) == -1) {
        mkdir(path.c_str(), 0700);
    }

}

struct alphabetically {
    inline bool operator()(
            const FileHelper::Entry &e1,
            const FileHelper::Entry &e2) {
        return (e1.fileName.compare(e2.fileName) < 0);
    }
};

struct alphabetically_rel {
    inline bool operator()(
            const FileHelper::Entry &e1,
            const FileHelper::Entry &e2) {
        string rel1 = e1.relName();
        string rel2 = e2.relName();
        return (rel1.compare(rel2) < 0);
    }
};

struct type_alpha_rel {
    inline bool operator()(
            const FileHelper::Entry &e1,
            const FileHelper::Entry &e2) {


        if (e1.type == FileHelper::TYPE_FILE && e2.type != FileHelper::TYPE_FILE) {
            return true;
        }
        if (e1.type != FileHelper::TYPE_FILE && e2.type == FileHelper::TYPE_FILE) {
            //cout << "false" << endl;
            return false;
        }

        string rel1 = e1.path_rel;
        string rel2 = e2.path_rel;

        int relCmp = rel1.compare(rel2);
        if (relCmp < 0) {
            return true;
        } else if (relCmp > 0) {
            return false;
        }

        string fn1 = e1.fileName;
        string fn2 = e2.fileName;

        int comp = fn1.compare(fn2);
        if (comp < 0) {
            return true;
        } else if (comp > 0) {
            return false;
        }

        return comp;


    }
};

void
FileHelper::listDir(const string path, vector<Entry> &result, bool recursive) {
    FileHelper::listDir(path, "/", result, recursive);
    std::sort(result.begin(), result.end(), type_alpha_rel());
}

void
FileHelper::listDir(const string path_base, const string path_rel, vector<Entry> &result, bool recursive) {

    // opens the directory
    string path = path_base + path_rel;
    DIR *d = opendir(path.c_str());

    // checks if it was opened correctly
    if (!d) {
        cerr << "can't open directory " << path << endl;
        exit(EXIT_FAILURE);
    }

    struct stat info;
    struct tm *clock;
    while (1) {
        struct dirent *entry;

        // readdir gets subsequent entries from "d"
        entry = readdir(d);
        if (!entry) {
            // There are no more entries in this directory, so break
            // out of the while loop.
            break;
        }

        string fileName = string(entry->d_name);
        if (strcmp(fileName.c_str(), ".") == 0 ||
            strcmp(fileName.c_str(), "..") == 0) {
            //cout << "skipping " << fileName << endl;
            continue;
        }

        Entry ent;
        ent.fileName = fileName;
        ent.path_base = path_base;
        ent.path_rel = path_rel;
        if (entry->d_type & DT_REG) {
            ent.type = TYPE_FILE;
        } else if (entry->d_type & DT_DIR) {
            ent.type = TYPE_DIRECTORY;
        } else {
            ent.type = TYPE_OTHER;
        }

        string fullName = ent.fullName();
        //cout << fullName << endl;
        getInfo(fullName, info);
        clock = gmtime(&(info.st_mtime));
        ent.lastModif = mktime(clock);

        result.push_back(ent);

        if (recursive && ent.type == TYPE_DIRECTORY) {

            string subdir = ent.relName() + "/";
            listDir(path_base, subdir, result, true);

        }


    }

    closedir(d);

}


string
FileHelper::currentPath() {

    char path[_PATH_MAX];
    char *dir = getcwd(path, _PATH_MAX);
    dir++;
    string ret(path);
    return ret;

}

void
FileHelper::copy(const string source, const string target) {
    ifstream src(source.c_str(), ios::binary);
    ofstream dst(target.c_str(), ios::binary);
    dst << src.rdbuf();
}

FileHelper::FileHelper() {
}

FileHelper::~FileHelper() {
}

