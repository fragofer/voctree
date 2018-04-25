//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.
/**
 * 	This file provides External K-means clustering implementation
 *
 */

#ifndef EXTKMEANS_H_
#define EXTKMEANS_H_


#include <cv.hpp>

using namespace std;
using namespace cv;

const int KILO = 1024;
const int MEGA = KILO * KILO;
const int GIGA = KILO * MEGA;

/**
 * extKmeans: This method performs K-clustering external clustering
 * Use this implementation when the memory required to store the elements to be clustered does not fit in RAM.
 * Running k-means from RAM will be much faster.
 *
 * @param normType norm type to be used to compute distances.
 * 								NORM_L2 and NORM_HAMMING OpenCV constants are supported.
 *								When NORM_HAMMING is used K-Majority variant is computed.
 * @param K The number of clusters to create
 * @param maxItt Maximum number of iterations to do in the algorithm
 * @param inpDataFile OpenCV Matrix ( N x D ) with the data to be clustered. Elements arranged by row.
 * 										N is number of elements to be clustered.
 * 										D is the dimension of each element.
 * @param useMemBytes The maximum number of bytes that the algorithm is allowed to use to create a
 * 										buffer for clustering
 * @param outLabels size (N x 1). Indicates which cluster each element belongs to.
 * @param outCenters size: (K x D). Contains the K centers of each cluster.
 */

void extKmeans(

        int normType,
        int K,
        //int attempts,
        int maxItt,

        std::string inpDataFile,

        long useMemBytes,

        Mat &outLabels,
        Mat &outCenters


);

/**
 * distribute: This method takes a file that contains a matrix of elements with data,
 * and the labels for each element, and then it splits
 * the data according with the labels into K different files.
 *  Use this method in combination with extKmeans.
 * @param K The number of clusters
 * @param inpDataFile 	Name of the file containing the Matrix with the elements to be split.
 * @param labels Indicates which cluster each element belongs to.
 * @param useMemBytes The maximum number of bytes to create buffers.
 * @param outClusters A vector containing the names of each file where the split data was written.
 */

void distribute(

        int K,
        string &inpDataFile,
        Mat &labels,
        long useMemBytes,
        vector<string> &outClusters

);

#endif /* EXTKMEANS_H_ */
