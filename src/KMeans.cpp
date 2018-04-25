
#include "KMeans.h"

#include <iostream>

using namespace std;


bool matIsEqual(const cv::Mat &mat1, const cv::Mat &mat2) {
    // treat two empty mat as identical as well
    if (mat1.empty() && mat2.empty()) {
        return true;
    }
    // if dimensionality of two mat is not identical, these two mat is not identical
    if (mat1.cols != mat2.cols || mat1.rows != mat2.rows || mat1.dims != mat2.dims) {
        return false;
    }
    cv::Mat diff;
    cv::compare(mat1, mat2, diff, cv::CMP_NE);
    int nz = cv::countNonZero(diff);
    return nz == 0;
}

void initCenters(int normType, int K, Mat &inpData, Mat &outCenters) {

    assert(inpData.rows >= K);

    outCenters.create(0, inpData.cols, inpData.type());

    int chosen = 0;
    int tries = 1000;
    while (tries-- > 0 && chosen < K) {

        int rndRow = random() % inpData.rows;

        Mat selectedRow = inpData.row(rndRow);
        bool dup = false;
        for (int r = 0; r < chosen; r++) {
            if (matIsEqual(outCenters.row(r), selectedRow)) {
                dup = true;
                break;
            }
        }
        if (!dup) {
            outCenters.push_back(selectedRow);
            //selectedRow.copyTo(outCenters.row(chosen));
            chosen++;
        }

    }

    while (chosen < K) {
        int rndRow = random() % inpData.rows;
        Mat selectedRow = inpData.row(rndRow);
        outCenters.push_back(selectedRow);
        chosen++;
    }


}

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


static int setLabels(
        int normType,
        int K,
        Mat &data,
        Mat &centers,
        Mat &labels,

        Mat &acum,
        Mat &count
) {

    int changes = 0;

    Mat expanded(acum.rows, acum.cols, acum.type());

    for (int lbl = 0; lbl < data.rows; lbl++) {

        int closest = 0;
        double minDistance = -1;

        // looks for the closest center
        for (int c = 0; c < centers.rows; c++) {

            //cout << "distance ";
            //cout << data.type() << " vs " << centers.type() << endl;
            double distance = norm(data.row(lbl), centers.row(c), normType);
            //cout << distance << endl;
            //cout << "distance" << distance << endl;

            if (c == 0 || distance < minDistance) {

                minDistance = distance;
                closest = c;

            }

        }

        long pos = lbl;


        if ((int) labels.at<uchar>(pos) != closest) {
            labels.at<uchar>(pos) = closest;
            changes++;
            //cout << "changed!" << (int) labels.at<uchar>(pos) << "." << endl;
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


double computeCompactness(
        int normType,
        int K,
        Mat &data,
        long useRows,
        Mat &centers,
        Mat &labels) {

    Mat acum(K, 1, CV_64F);
    Mat count(K, 1, CV_32S);

    acum = Scalar(0);
    count = Scalar(0);

    for (int lbl = 0; lbl < useRows; lbl++) {

        uchar c = labels.at<uchar>(lbl);
        double distance = norm(data.row(lbl), centers.row(c), normType);
        acum.at<double>(c, 0) += (distance * distance);
        count.at<int>(c, 0)++;

    }

    //cout << "distances: ";
    double total = 0;
    for (int c = 0; c < K; c++) {
        //acum.at<double>(c) /= count.at<int>(c);
        //cout << c << ":" << acum.at<double>(c) << ",";
        total += acum.at<double>(c);
    }
    //cout << endl;
    //cout << "total: " << total << endl;
    return total;
}


static void handleEmpty(
        int normType,
        int K,
        Mat &data,
        Mat &centers,
        Mat &labels,
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
        for (int lbl = 0; lbl < data.rows; lbl++) {

            //long pos = lbl + lblOffset;

            if (labels.at<uchar>(lbl) == biggClust) {

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


        labels.at<uchar>(farthest) = c;


    }

}

static void tryKmeans(int normType,
                      int K,

                      int maxItt,

                      Mat &inpData,

                      Mat &outLabels,
                      Mat &outCenters
) {


    int featDim = inpData.cols;
    int rows = inpData.rows;

    outLabels.create(rows, 1, CV_8U);
    outLabels = Scalar(0);

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

    //int minAccepted = (int) max(0.005 * totRows, 1.0);
    int minAccepted = 1;

    //cout << "init centers" << endl;
    initCenters(normType, K, inpData, outCenters);

    for (int itt = 0; itt < maxItt; itt++) {

        //cout << "iteration: " << (itt + 1) << "/" << maxItt << endl;

        acum = Scalar(0);
        count = Scalar(0);

        long changes = 0;

        //cout << "labeling" << endl;
        changes = setLabels(
                normType,
                K,
                inpData,
                outCenters,
                outLabels,
                acum,
                count
        );

        //count.at<int>(0) = 0;


//			    if (done + read >= totRows) {
        //cout << "handling empty" << endl;
        handleEmpty(
                normType,
                K,
                inpData,
                outCenters,
                outLabels,
                acum,
                count
        );
//			    }

        // update step
//			// recompute centers.
        //cout << "update" << endl;
        for (int c = 0; c < outCenters.rows; c++) {

            assert(count.at<float>(c) != 0);
//				assert(acum.at<float>(c) != 0);

            float factor = 1.0 / count.at<int>(c);
            acum.row(c) *= factor;

        }

        if (normType == NORM_HAMMING || normType == NORM_HAMMING2) {
            for (int c = 0; c < outCenters.rows; c++) {
                condense(acum, c, outCenters, c);
            }
        } else {
            acum.copyTo(outCenters);
        }


        if (changes <= minAccepted) {
            break;
        }

    }


}


void myKmeans(

        int normType,
        int K,

        int maxItt,

        Mat &inpData,

        Mat &outLabels,
        Mat &outCenters

) {


    long useRows = inpData.rows;


    int numAttempts = 10;
    double bestCompactness = -1;
    Mat centers;
    Mat labels;

    for (int att = 0; att < numAttempts; att++) {

        if (useRows > 20000)
            cout << "attempt. " << att << endl;

        tryKmeans(normType, K, maxItt, inpData, labels, centers);


        double compactness = computeCompactness(normType, K, inpData, useRows, centers, labels);

        if (att == 0 || compactness < bestCompactness) {
            if (inpData.rows > 20000)
                cout << "better result was found. " << compactness << endl;
            bestCompactness = compactness;
            centers.copyTo(outCenters);
            labels.copyTo(outLabels);
        }

    }


}

