//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef KMEANS_H_
#define KMEANS_H_

#include <cv.h>

using namespace cv;

/**
 * Provides a K-Clustering implementation
 * if normType is L2, then K-means is performed
 * if normType is Hamming then K-majority is performed
 *
 * @param normType NORM_L2 and NORM_HAMMING are supported. see OpenCV NormTypes definitions
 * @param K number of clusters to be formed
 * @param maxItt maximum number of iterations
 * @param inpData OpenCV matrix with the data to be clustered. Arranged by rows.
 * @param outLabels output labels
 * @param outCenters output cluster centers
 */
void myKmeans(

        int normType,
        int K,
        int maxItt,

        Mat &inpData,
        Mat &outLabels,
        Mat &outCenters

);


#endif /* KMEANS_H_ */
