// Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
// This program is free software: you can use, modify and/or
// redistribute it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later
// version. You should have received a copy of this license along
// this program. If not, see <http://www.gnu.org/licenses/>.

/**
 * This class is used to persist (and retrieve) a generic standard C++ vector to disk
 */

#ifndef VECPERSISTOR_H_
#define VECPERSISTOR_H_

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;


class VecPersistor {

public:

    /**
     * Persists the given a vector vec to a file filePath
     * @tparam T data type
     * @param filePath output file path
     * @param vec input vector
     */
    template<typename T>
    void
    persist(string filePath, vector<T> &vec);

    /**
 * Given a file path, restores the file to a vector
 * @tparam T data type
 * @param filePath input file path
 * @param vec output vector
 */
    template<typename T>
    void
    restore(string filePath, vector<T> &vec);


private:
    struct Header {
        int elemCount;
    };


};

//#include "VecPersistor.cpp"

template<typename T>
void
VecPersistor::persist(string filePath, vector<T> &vec) {

    Header hdr;
    hdr.elemCount = vec.size();

    ofstream file(filePath.c_str(), ios::out | ios::binary);

    // writes header
    size_t hdr_size = sizeof(Header);
    char *pHdr = reinterpret_cast<char *>(&hdr);
    file.write(pHdr, hdr_size);

    // writes vector data
    long bytes = hdr.elemCount * sizeof(T);

    file.write((char *) &vec[0], bytes);

    file.close();

}


template<typename T>
void
VecPersistor::restore(string filePath, vector<T> &vec) {


    ifstream file(filePath.c_str(), ios::in | ios::binary);

    Header hdr;
    char *pHdr = reinterpret_cast<char *>(&hdr);

    size_t hdr_size = sizeof(Header);
    file.read(pHdr, hdr_size);

    vec.resize(hdr.elemCount);

    // reads data
    long bytes = hdr.elemCount * sizeof(T);

    file.read((char *) &vec[0], bytes);

    file.close();

}


#endif /* VECPERSISTOR_H_ */
