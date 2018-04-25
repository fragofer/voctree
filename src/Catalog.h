//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.
/*
 *
 * Catalog
 * 		this file provides the Catalog<T> class
 * 		as a generic collection of items.
 *
 * 		Catalog<DBElem>, Catalog<VideoInfo> and Catalog<> are
 * 		used in the database.
 *
 */


#ifndef CATALOG_H_
#define CATALOG_H_

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <map>


using namespace std;

/**
 * DBElem: identifies an indexed image file or video frame
 */
struct DBElem {
    string name;
    int featuresCount;
};


/**
 * VideoInfo: identifies a video file
 */
struct VideoInfo {
    int id;
    string fileName;
};

/**
 * Group: identifies a group where several image files belong.
 * for example DBElems (0, 3, 4) belongs to Group 0, and (1, 2) belongs to Group 1
 * used for Query metrics evaluation
 */
struct Group {
    int id;
    int objCount;
    string description;
};

template<class T>
class Catalog {

public:

    /**
     * size of the Catalog
     * @return the number of elements
     */
    int size();

    /**
     * get retrieves an element from the Catalog
     * @param index
     * @return the element at index position
     */
    T get(int index) const;

    /**
     * puts an element into the Catalog
     * @param index the position where the element is going to be placed
     * @param info the element to be put
     */
    void put(int index, T info);

    /**
     * adds an element to the Catalog
     * @param info the element to be added
     */
    void add(T info);

    /**
     * saves the Catalog to catalogFile
     * @param catalogFile the output file name
     */
    void store(string catalogFile);

    /**
     * loads the Catalog from catalogFile
     * @param catalogFile the input file name
     */
    void load(string catalogFile);

    /**
     * shrinks the Catalog (truncates, dropping the elements with positions >= size)
     * @param size new size for the Catalog
     */
    void shrink(int size);

private:
    vector<T> _elems;

};

#endif /* CATALOG_H_ */
