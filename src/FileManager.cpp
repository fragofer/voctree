//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "FileManager.h"

FileManager::FileManager(string &path) {
    _path = path;
}

FileManager::~FileManager() {

}


string
FileManager::dataDir() {
    return _path + DIRBAR + "data";
}

string
FileManager::inputDir() {
    return _path + DIRBAR + "input";
}

string
FileManager::queryDir() {
    return _path + DIRBAR + "queries";
}

string
FileManager::resultDir() {
    return _path + DIRBAR + "results";
}

string
FileManager::vocabularyDir() {
    return _path + DIRBAR + "vocabulary";
}


string
FileManager::root() {
    return _path;
}

string
FileManager::mapData(string prefix) {
    return dataDir() + DIRBAR + prefix;
}


string
FileManager::name(int idFile) {

    if (idFile == DB_CONFIG) return "config.txt";
    if (idFile == CATALOG) return "catalog.txt";
    if (idFile == CATALOG_VIDEO) return "catalog_videos.txt";
    if (idFile == FEAT_METHOD) return "method.txt";
    if (idFile == DESCRIPTORS) return "descriptors.bin";
    if (idFile == KEYPOINTS) return "keypoints.bin";
    if (idFile == PCA_MODEL) return "pca.fs";

    if (idFile == VOCABULARY_CATALOG) return "vocabulary_catalog.txt";
    if (idFile == VOCABULARY_CATALOG_VIDEO) return "vocabulary_catalog_video.txt";
    if (idFile == VOCABULARY_DESCRIPTORS) return "vocabulary_descriptors.bin";
    if (idFile == VOCABULARY_KEYPOINTS) return "vocabulary_keypoints.bin";

    return "";

}

string
FileManager::file(int idFile) {

    return dataDir() + DIRBAR + name(idFile);

}
