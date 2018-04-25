//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.
#include "ExtKmeans.h"
#include "MatPersistor.h"

#include "KMeans.h"

#include <set>
#include <list>

#include <iostream>

using namespace std;


static void expand(Mat &inMat, int inRow, Mat &outMat, int outRow) {

    // in [ 10, 255, 123, 88 ] --> [
    //			0, 0, 0, 0, 1, 0, 1, 0,
    //			1, 1, 1, 1, 1, 1, 1, 1,
    //			0, 1, 1, 1, 1, 0, 1, 1,
    //			0, 1, 0, 1, 1, 0, 0, 0
    //	]

    for (int i = 0; i < inMat.cols; i++) {

        uchar val = inMat.at<uchar>(inRow, i);

        float b0 = (float) ((0b00000001 & val) >> 0);
        float b1 = (float) ((0b00000010 & val) >> 1);
        float b2 = (float) ((0b00000100 & val) >> 2);
        float b3 = (float) ((0b00001000 & val) >> 3);
        float b4 = (float) ((0b00010000 & val) >> 4);
        float b5 = (float) ((0b00100000 & val) >> 5);
        float b6 = (float) ((0b01000000 & val) >> 6);
        float b7 = (float) ((0b10000000 & val) >> 7);

        outMat.at<float>(outRow, 8 * i + 0) = b7;
        outMat.at<float>(outRow, 8 * i + 1) = b6;
        outMat.at<float>(outRow, 8 * i + 2) = b5;
        outMat.at<float>(outRow, 8 * i + 3) = b4;
        outMat.at<float>(outRow, 8 * i + 4) = b3;
        outMat.at<float>(outRow, 8 * i + 5) = b2;
        outMat.at<float>(outRow, 8 * i + 6) = b1;
        outMat.at<float>(outRow, 8 * i + 7) = b0;

    }

}

static void condense(Mat &inMat, int inRow, Mat &outMat, int outRow) {

    // in [
    //			0.1, 0, 0, 0, 1, 0, 1, 0,
    //			1, 1, 0.8, 1, 1, 1, 1, 1,
    //			0, 1, 1, 1, 1, 0, 1, 1,
    //			0, 1, 0, 1, 1, 0, 0, 0
    //	]
    //  --> out [ 10, 255, 123, 88 ]

    for (int i = 0; i < outMat.cols; i++) {

        float b7 = round(inMat.at<float>(inRow, 8 * i + 0));
        float b6 = round(inMat.at<float>(inRow, 8 * i + 1));
        float b5 = round(inMat.at<float>(inRow, 8 * i + 2));
        float b4 = round(inMat.at<float>(inRow, 8 * i + 3));
        float b3 = round(inMat.at<float>(inRow, 8 * i + 4));
        float b2 = round(inMat.at<float>(inRow, 8 * i + 5));
        float b1 = round(inMat.at<float>(inRow, 8 * i + 6));
        float b0 = round(inMat.at<float>(inRow, 8 * i + 7));


        uchar val =
                ((uchar) b0) << 0 |
                ((uchar) b1) << 1 |
                ((uchar) b2) << 2 |
                ((uchar) b3) << 3 |
                ((uchar) b4) << 4 |
                ((uchar) b5) << 5 |
                ((uchar) b6) << 6 |
                ((uchar) b7) << 7;

        outMat.at<uchar>(outRow, i) = val;

    }

}


void initCentersRandom(int K,
                       MatPersistor &mp,
                       Mat &centers) {


    // Compute random indexes to choose starting points
    // until we have k different

    set<float> distinct;
    pair<set<float>::iterator, bool> ret;

    Mat row;
    centers.create(0, mp.cols(), CV_32F);

    int maxTries = 1000;
    while (maxTries-- >= 0 && (int) distinct.size() < K) {

        int rnd = (rand() % mp.rows());

        mp.setRow(rnd);
        mp.read(row, 1);

        // if norm differs, the point differs.
        float norm = cv::norm(row, NORM_L2);
        ret = distinct.insert(norm);

        if (ret.second) {
            // ret.second == true : (a new element was inserted in the set)
            //int c = distinct.size() - 1;
            centers.push_back(row);
        }


    }

    assert(centers.rows == K);

}

void initCentersKMeans(
        int normType,
        int K,
        int numSamples,
        int maxRows,
        MatPersistor &mp,
        Mat &centers) {

    // chooses random samples,
    // and with them does K-means in the traditional way.

    int tries = numSamples;
    set<int> distinct;
    pair<set<int>::iterator, bool> ret;
    Mat indexes(0, 1, CV_32S);
    while (tries-- > 0 && (int) distinct.size() < numSamples) {

        //int rndRow = random() % mp.rows();
        int rndRow = random() % maxRows;
        ret = distinct.insert(rndRow);

        if (ret.second) {
            // ret.second == true : (a new element was inserted in the set)
            indexes.push_back(rndRow);
        }

    }

    // now, we sort the indexes to access to disk in order.
    int flags = CV_SORT_EVERY_COLUMN | CV_SORT_ASCENDING;
    cv::sort(indexes, indexes, flags);

    // now collect samples.
    Mat row;
    //Mat samples(0, mp.cols(), CV_32F);
    Mat samples(0, mp.cols(), mp.type());
    for (int i = 0; i < indexes.rows; i++) {

        int rowNo = indexes.at<int>(i);
        mp.setRow(rowNo);
        mp.read(row, 1);
        samples.push_back(row);

    }

    // now compute k-means.
//	int attempts = 5;
//	int maxItt = 20;
//	int kmflags = KMEANS_PP_CENTERS;
//	TermCriteria term( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, maxItt, 0.5);

    //kmeans(samples, K, labels, term, attempts, kmflags, centers);
    Mat labels;
    myKmeans(normType, K, 10, samples, labels, centers);


}


void handleEmptyClusters(
        int normType,
        int K,
        Mat &data,
        int dataRows,
        Mat &centers,
        Mat &labels,
        long lblOffset,
        Mat &acum,
        Mat &count) {


    // checks that all centers have at least one point.
    for (int c = 0; c < K; c++) {

        if (count.at<int>(c) > 0) {
            continue;
        }

        //cout << "fixing empty cluster " << c << endl;

        // cluster i is empty, then:
        //   1. find the biggest cluster
        int biggCount = 0;
        char biggClust = K;
        for (int c2 = 0; c2 < K; c2++) {
            if (count.at<int>(c2) > biggCount) {
                biggClust = c2;
                biggCount = count.at<int>(c2);
            }
        }
        assert(biggClust < K);


        //   2. find the farthest from the center point in the biggest cluster
        float maxDist = -1;
        int farthest = -1;
        for (int lbl = 0; lbl < dataRows; lbl++) {

            long pos = lbl + lblOffset;

            if (labels.at<char>(pos) == biggClust) {

                // this element belongs to the biggest cluster
                float distance = norm(data.row(lbl), centers.row(biggClust), normType);

                if (farthest == -1 || distance > maxDist) {
                    farthest = lbl;
                    maxDist = distance;
                }

            }
        }

        //   3. exclude the farthest point from the biggest cluster and form a new 1-point cluster.
        // elem = data.row(farthest);

        count.at<int>(biggClust) -= 1;
        count.at<int>(c) = 1;

        if (normType == NORM_HAMMING || normType == NORM_HAMMING2) {
            Mat expanded(1, acum.cols, acum.type());
            expand(data, farthest, expanded, 0);
            acum.row(biggClust) -= expanded.row(0);
            acum.row(c) = expanded.row(0);
        } else {
            acum.row(biggClust) -= (data.row(farthest));
            acum.row(c) = (data.row(farthest));
        }


        labels.at<char>(farthest + lblOffset) = c;


    }

}


int setLabels(
        int normType,
        int K,
        Mat &data,
        int dataRows,
        Mat &centers,
        Mat &labels,
        long lblOffset,
        Mat &acum,
        Mat &count
) {

    int changes = 0;

    Mat expanded(acum.rows, acum.cols, acum.type());


    for (int lbl = 0; lbl < dataRows; lbl++) {

        int closest = 0;
        float minDistance = 0;

        // looks for the closest center
        for (int c = 0; c < centers.rows; c++) {

            float distance = norm(data.row(lbl), centers.row(c), normType);

            if (c == 0 || distance < minDistance) {

                minDistance = distance;
                closest = c;

            }

        }

        long pos = lbl + lblOffset;

//		// remembers the K farthest points
//		rememberFarthests(K,
//						minDistance,
//						pos,
//						distFarthests,
//						idxFarthests);


        if (labels.at<char>(pos) != closest) {
            labels.at<char>(pos) = closest;
            changes++;
        }

        count.at<int>(closest) += 1;
        if (normType == NORM_HAMMING || normType == NORM_HAMMING2) {
            expand(data, lbl, expanded, closest);
            acum.row(closest) += expanded.row(closest);
        } else {
            acum.row(closest) += data.row(lbl);
        }

    }


    return changes;

}


void extKmeans(

        int normType,
        int K,
        //int attempts,
        int maxItt,

        string inpDataFile,
        long useMemBytes,

        Mat &labels,
        Mat &centers


) {


    MatPersistor mp(inpDataFile);
    mp.openRead();

    int featDim = mp.cols();
    long useRows;
    useRows = mp.rows();


    //long useMem 	 = 256 * MEGA;
    long rowSize = featDim * mp.elementSize();
    long bufferRows = useMemBytes / rowSize;

    labels.create(useRows, 1, CV_8U);

    // this will force to change every label in the first iteration.
    labels = Scalar(K);

    cout << "choosing initial centers..." << endl << flush;

    // Method1: chooses K random rows.
    //initCentersRandom(K, mp, centers);

    // Method2: chooses S samples, and aplies k-means
    long samples = useRows < bufferRows ? useRows : bufferRows;
    samples = samples < K ? K : samples;
    initCentersKMeans(normType, K, samples, useRows, mp, centers);

    //cout << centers << endl;

    int acumType;
    int acumCols;
    if (normType == NORM_HAMMING || normType == NORM_HAMMING2) {
        acumType = CV_32F;
        acumCols = featDim * 8;
    } else {
        acumType = CV_32F;
        acumCols = featDim;
    }
    Mat acum(K, acumCols, acumType);
    Mat count(K, 1, CV_32S);
    int minAccepted = (int) max(0.005 * useRows, 1.0);

    Mat buffer(bufferRows, featDim, CV_32F);

    list<float> distFarthests;
    list<long> idxFarthests;

    for (int itt = 0; itt < maxItt; itt++) {

        cout << "iteration: " << (itt + 1) << "/" << maxItt << endl;

        acum = Scalar(0);
        count = Scalar(0);

        long done = 0;
        long changes = 0;
        mp.setRow(0);


        while (done < useRows) {

            long toRead;
            if (useRows - done < bufferRows) {
                toRead = useRows - done;
            } else {
                toRead = bufferRows;
            }
            long read = mp.read(buffer, toRead);

            changes += setLabels(
                    normType,
                    K,
                    buffer,
                    read,
                    centers,
                    labels,
                    done,
                    acum,
                    count
            );


            if (done + read >= useRows) {
                handleEmptyClusters(normType, K, buffer, read, centers, labels, done, acum, count);
            }

            done += read;
            cout << "feats labeled: " << done << endl << flush;

        }


        // update step
        // recompute centers.
        for (int c = 0; c < centers.rows; c++) {

            assert(count.at<float>(c) != 0);
            assert(acum.at<float>(c) != 0);

            float factor = 1.0 / count.at<int>(c);
            acum.row(c) *= factor;

        }

        if (normType == NORM_HAMMING || normType == NORM_HAMMING2) {
            for (int c = 0; c < centers.rows; c++) {
                condense(acum, c, centers, c);
            }
        } else {
            acum.copyTo(centers);
        }

        if (changes <= minAccepted) {
            break;
        }

    }


}

void
distribute(int K,
           string &fileInput,
           Mat &labels,
           long useMemBytes,
           vector<string> &outClusters
) {


    MatPersistor mp(fileInput);
    mp.openRead();

    long useRows = mp.rows();


    outClusters.clear();
    //outIdxs.clear();

    vector<Mat> clusts;
    for (int i = 0; i < K; i++) {
        Mat mat(0, mp.cols(), mp.type());
        clusts.push_back(mat);

        stringstream ss;
        ss << fileInput << "." << i;
        string outFileName = ss.str();
        outClusters.push_back(outFileName);
        MatPersistor mp(outFileName);
        mp.create(mat.cols, mat.type());

        vector<int> idxs;
        //outIdxs.push_back(idxs);

    }

    long done = 0;
    Mat buffer;
    long useMem = 256 * MEGA;
    long rowSize = mp.cols() * mp.elementSize();
    long bufferRows = useMem / rowSize;

    while (done < useRows) {

        long toRead;
        if (useRows - done < bufferRows) {
            toRead = useRows - done;
        } else {
            toRead = bufferRows;
        }
        long read = mp.read(buffer, toRead);

        // empties mats
        for (int i = 0; (unsigned) i < clusts.size(); i++) {
            Mat &clust = clusts.at(i);
            clust.create(0, clust.cols, clust.type());
        }


        // distributes data in clusters
        for (int i = 0; i < read; i++) {

            long lbl = i + done;
            char clust = labels.at<char>(lbl);

            Mat &mat = clusts.at(clust);
            mat.push_back(buffer.row(i));

        }

        cout << "feats read: " << read << endl << flush;


        // writes to disk data of each cluster
        for (int i = 0; (unsigned) i < clusts.size(); i++) {
            string outFileName = outClusters.at(i);
            MatPersistor out(outFileName);
            out.openWrite();
            Mat &clust = clusts.at(i);
            out.append(clust);
            out.close();
        }


        done += read;
        cout << "feats distributed: " << done << endl << flush;

    }


}

