//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "Catalog.h"

template<class T>
void Catalog<T>::add(T info) {
    _elems.push_back(info);
}

template<class T>
int Catalog<T>::size() {
    return _elems.size();
}

template<class T>
T Catalog<T>::get(int index) const {
    T info = _elems.at(index);
    return info;
}

template<class T>
void Catalog<T>::put(int index, T info) {
    _elems[index] = info;
}

template<class T>
void Catalog<T>::shrink(int size) {
    if (size < 0 || (unsigned int) size > _elems.size()) {
        return;
    }
    _elems.resize(size);
}


void storeInfo(ofstream &file, int i, DBElem info) {
    file
            << i << "\t"
            << info.featuresCount << "\t"
            << info.name
            << endl;

}

void storeInfo(ofstream &file, int i, Group grp) {

    file
            << i << "\t"
            << grp.id << "\t"
            << grp.objCount << "\t"
            << grp.description
            << endl;

}

void storeInfo(ofstream &file, int i, VideoInfo info) {
    file
            << i << "\t"
            << info.fileName
            << endl;
}

template<class T>
void Catalog<T>::store(string fileCatalog) {

    ofstream file(fileCatalog.c_str(), ios::out);
    if (file.is_open()) {

        for (unsigned int i = 0; i < _elems.size(); i++) {

            T info = _elems.at(i);
            storeInfo(file, i, info);

        }

        file.close();
    }

}


string
readNextString(string line, int &startPos) {

    int pos = line.find("\t", startPos);
    if (pos < startPos) {
        return line.substr(startPos);
    }

    string ret = line.substr(startPos, pos - startPos);
    startPos = pos + 1;

    return ret;

}

int
readNextInt(string line, int &startPos) {

    string strValue = readNextString(line, startPos);
    return atoi(strValue.c_str());

}


void readInfo(string &line, DBElem &info) {
    int startPos = 0;
    readNextInt(line, startPos);
    int numFeatures = readNextInt(line, startPos);
    string fileName = readNextString(line, startPos);

    info.featuresCount = numFeatures;
    info.name = fileName;

}

void readInfo(string &line, VideoInfo &info) {
    int startPos = 0;
    int idVideo = readNextInt(line, startPos);
    string fileName = readNextString(line, startPos);

    info.id = idVideo;
    info.fileName = fileName;


}


void readInfo(string &line, Group &ret) {

    int startPos = 0;

    readNextInt(line, startPos);
    int id = readNextInt(line, startPos);
    int count = readNextInt(line, startPos);
    string descr = readNextString(line, startPos);

    ret.id = id;
    ret.objCount = count;
    ret.description = descr;

}


template<class T>
void
Catalog<T>::load(string fileCatalog) {

    ifstream file(fileCatalog.c_str(), ios::in);
    if (file.is_open()) {

        string line;
        while (getline(file, line, '\n')) {

            T info;
            readInfo(line, info);
            _elems.push_back(info);

        }

        file.close();
    }

}

template
class Catalog<DBElem>;

template
class Catalog<VideoInfo>;

template
class Catalog<Group>;
