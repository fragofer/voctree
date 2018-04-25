//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef FEATUREMETHOD_H_
#define FEATUREMETHOD_H_

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <cv.hpp>
#include <opencv2/highgui.hpp>
#include <features.h>

#include "Configuration.h"


using namespace cv;
using namespace std;


class FeatureMethod {
public:

    /**
   *  This class represents the pair (keypoints detection / descriptor extractor) technique
   *  that is going to be used for an algorithm
   */


    /**
    * Available Feature detectors:
    * "FAST" – FastFeatureDetector
    *  "STAR" – StarFeatureDetector
    *  "SIFT" – SIFT (nonfree module)
    *  "SURF" – SURF (nonfree module)
    *  "ORB" – ORB
    *  "BRISK" – BRISK
    *  "MSER" – MSER
    *  "GFTT" – GoodFeaturesToTrackDetector
    *  "HARRIS" – GoodFeaturesToTrackDetector with Harris detector enabled
    *  "Dense" – DenseFeatureDetector
    *  "SimpleBlob" – SimpleBlobDetector
     */

    // ----------------------------
    static const int DETECT_FAST = 0;
    static const int DETECT_AGAST = 1;
    static const int DETECT_GFTT = 2;
    static const int DETECT_MSER = 3;
    static const int DETECT_BRISK = 4;
    static const int DETECT_ORB = 5;
    static const int DETECT_KAZE = 6;
    static const int DETECT_AKAZE = 7;
    static const int DETECT_STAR = 8;
    static const int DETECT_DAISY = 9;
    static const int DETECT_SIFT = 10;
    static const int DETECT_SURF = 11;

    /**
     * 	Available Descriptor Extractors:
     * "SIFT" – SIFT
     * "SURF" – SURF
     * "BRIEF" – BriefDescriptorExtractor
     * "BRISK" – BRISK
     * "ORB" – ORB
     * "FREAK" – FREAK
     */
    // extractors
    static const int EXTRACT_MSER = 1;
    static const int EXTRACT_BRISK = 2;
    static const int EXTRACT_ORB = 3;
    static const int EXTRACT_KAZE = 4;
    static const int EXTRACT_AKAZE = 5;
    static const int EXTRACT_BRIEF = 6;
    static const int EXTRACT_FREAK = 7;
    static const int EXTRACT_DAISY = 8;
    static const int EXTRACT_SIFT = 9;
    static const int EXTRACT_SURF = 10;

    static const int EXTRACT_RootSIFT = 11;


    /**
     * Creates a default FeatureMethod
     */
    FeatureMethod();

    /**
     * Creates a FeatureMethod that uses detectorType for detection and extractorType for extraction
     * @param detectorType detection method. See available Feature detectors
     * @param extractorType extraction method. See available Descriptor Extractors
     */
    FeatureMethod(int detectorType, int extractorType);

    /**
    * The same that the integer parameters constructor, but using string constants
    * @param detectorType detection method. See available Feature detectors
    * @param extractorType extraction method. See available Descriptor Extractors
    */
    FeatureMethod(string detectorType, string extractorType);

    /**
     * Creates a FeatureMethod loading paramters from fileName
     * @param fileName
     */
    FeatureMethod(string fileName);


    virtual ~FeatureMethod();

    /**
     * getDefaultNorm
     * @return returns the norm to be used to compare descriptors extracted with this method
     */
    int getDefaultNorm();

    /**
     * given an image img performs detecton and extraction process
     * @param img the input image
     * @param keypoints the resulting detected keypoints
     * @param descriptors the resulting extracted descriptors
     */
    void detectAndCompute(Mat &img, vector<KeyPoint> &keypoints, Mat &descriptors);

    /**
     * saves this FeatureMethod to fileName
     * @param fileName the name of the file where this FeatureMethod is going to be stored to
     */
    void store(string fileName);

    /**
     * loads a FeatureMethod from fileName
     * @param fileName the name of the file where this FeatureMethod is going to be retrieved from
     */
    void load(string fileName);

    /**
     * loads a FeatureMethod from a Configuration object
     * @param cfg the configuration object
     */
    void load(Configuration &cfg);

    friend ostream &operator<<(ostream &out, FeatureMethod fm);

    string getDetectorKey();

    string getExtractorKey();

    static string getDetectorKey(int dt);

    static string getExtractorKey(int et);

    static int getDetectorType(string key);

    static int getExtractorType(string key);

private:

    void _init(int detectorType, int extractorType);

    int _detectorType;
    int _extractorType;
    Ptr<FeatureDetector> _pfd;
    Ptr<DescriptorExtractor> _pde;

};

#endif /* FEATUREMETHOD_H_ */
