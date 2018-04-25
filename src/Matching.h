//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef MATCHING_H_
#define MATCHING_H_

#include <iostream>

using namespace std;


class Matching {
    /**
     * This class represents a scoring result
     */

public:

    // The id of the file element
    // (is a number of the inverted index provided to the tree)
    int id;

    // the resulting score, 0.0 <= score <= 2.0
    double score;

    //For sorting purposes
    bool operator<(const Matching &m) const {
        //cout << "operator<"<< endl;
        return score < m.score;
    }

    /**
     * Default Matching constructor
     */
    Matching();

    /**
     * Matching destructor
     */
    virtual ~Matching();

};

#endif /* MATCHING_H_ */
