//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "FeatureMethod.h"
#include <opencv2/xfeatures2d.hpp>


using namespace cv;
using namespace std;


ostream &
operator<<(ostream &out, FeatureMethod fm) {

    out << "FEATURE METHOD:" << endl;

    //out << "Detector:" << fm._detectorType << endl;
    //printParams( out, *fm.getFeatureDetector() );
    //out << "Extractor:" << fm._extractorType << endl;
    //printParams( out, *fm.getDescriptorExtractor() );

    return out;

}


int
FeatureMethod::getDefaultNorm() {
    return _pde->defaultNorm();
}


static void
applyRootSIFT(Mat &descriptors, double eps = 1e-7) {

    vector<float> sums;
    Mat absDescs = abs(descriptors);
    for (int r = 0; r < descriptors.rows; r++) {
        double s = sum(absDescs.row(r))[0];
        for (int c = 0; c < descriptors.cols; c++) {
            descriptors.at<float>(r, c) = sqrt(descriptors.at<float>(r, c) / (s + eps));
        }
    }

}

void
FeatureMethod::detectAndCompute(Mat &img, vector<KeyPoint> &keypoints, Mat &descriptors) {

    _pfd->detect(img, keypoints);

    _pde->compute(img, keypoints, descriptors);

    if (_extractorType == EXTRACT_RootSIFT) {
        applyRootSIFT(descriptors);
    }

}


string
FeatureMethod::getDetectorKey(int dt) {

    switch (dt) {
        case (DETECT_AGAST):
            return "AGAST";
        case (DETECT_AKAZE):
            return "AKAZE";
        case (DETECT_BRISK):
            return "BRISK";
        case (DETECT_DAISY):
            return "DAISY";
        case (DETECT_FAST):
            return "FAST";
        case (DETECT_GFTT):
            return "GFFT";
        case (DETECT_KAZE):
            return "KAZE";
        case (DETECT_MSER):
            return "MSER";
        case (DETECT_ORB):
            return "ORB";
        case (DETECT_SIFT):
            return "SIFT";
        case (DETECT_SURF):
            return "SURF";
    }

    return "not supported";

}

int
FeatureMethod::getDetectorType(string key) {

    if (key == "AGAST") return DETECT_AGAST;
    if (key == "AKAZE") return DETECT_AKAZE;
    if (key == "BRISK") return DETECT_BRISK;
    if (key == "DAISY") return DETECT_DAISY;
    if (key == "FAST") return DETECT_FAST;
    if (key == "GFFT") return DETECT_GFTT;
    if (key == "KAZE") return DETECT_KAZE;
    if (key == "MSER") return DETECT_MSER;
    if (key == "ORB") return DETECT_ORB;
    if (key == "SIFT") return DETECT_SIFT;
    if (key == "STAR") return DETECT_STAR;
    if (key == "SURF") return DETECT_SURF;

    return -1;

}


string
FeatureMethod::getExtractorKey(int et) {

    switch (et) {
        case (EXTRACT_AKAZE):
            return "AKAZE";
        case (EXTRACT_BRIEF):
            return "BRIEF";
        case (EXTRACT_BRISK):
            return "BRISK";
        case (EXTRACT_DAISY):
            return "DAISY";
        case (EXTRACT_FREAK):
            return "FREAK";
        case (EXTRACT_KAZE):
            return "KAZE";
        case (EXTRACT_MSER):
            return "MSER";
        case (EXTRACT_ORB):
            return "ORB";
        case (EXTRACT_SIFT):
            return "SIFT";
        case (EXTRACT_SURF):
            return "SURF";

        case (EXTRACT_RootSIFT):
            return "RootSIFT";
    }

    return "not supported";

}

string
FeatureMethod::getExtractorKey() {

    return FeatureMethod::getExtractorKey(_extractorType);

}

string
FeatureMethod::getDetectorKey() {

    return FeatureMethod::getDetectorKey(_detectorType);

}


int
FeatureMethod::getExtractorType(string key) {

    if (key == "AKAZE") return EXTRACT_AKAZE;
    if (key == "BRIEF") return EXTRACT_BRIEF;
    if (key == "BRISK") return EXTRACT_BRISK;
    if (key == "DAISY") return EXTRACT_DAISY;
    if (key == "FREAK") return EXTRACT_FREAK;
    if (key == "KAZE") return EXTRACT_KAZE;
    if (key == "MSER") return EXTRACT_MSER;
    if (key == "ORB") return EXTRACT_ORB;
    if (key == "SIFT") return EXTRACT_SIFT;
    if (key == "SURF") return EXTRACT_SURF;

    if (key == "RootSIFT") return EXTRACT_RootSIFT;


    return -1;

}


Ptr<Feature2D> create(string key) {

    Ptr<Feature2D> ret;

//	// detectors
//	ret = FastFeatureDetector::create();
//	ret = AgastFeatureDetector::create();
//	ret = GFTTDetector::create();
//	ret = MSER::create();
//	ret = BRISK::create();
//	ret = ORB::create();
//	ret = KAZE::create();
//	ret = AKAZE::create();
//	// --
//	ret = xfeatures2d::StarDetector::create();
//	ret = xfeatures2d::FREAK::create();
//	ret = xfeatures2d::DAISY::create();
//	ret = xfeatures2d::SIFT::create();
//	ret = xfeatures2d::SURF::create();
//
//	// extractors
//	ret = MSER::create();
//	ret = BRISK::create();
//	ret = ORB::create();
//	ret = KAZE::create();
//	ret = AKAZE::create();
//	// --
//	ret = xfeatures2d::BriefDescriptorExtractor::create();
//	ret = xfeatures2d::FREAK::create();
//	ret = xfeatures2d::DAISY::create();
//	ret = xfeatures2d::SIFT::create();
//	ret = xfeatures2d::SURF::create();




    return ret;

}

void
FeatureMethod::_init(int detectorType, int extractorType) {

    //initModule_nonfree();

    _detectorType = detectorType;
    _extractorType = extractorType;

    switch (_detectorType) {
        case (DETECT_AGAST):
            _pfd = AgastFeatureDetector::create();
            break;
            //case (DETECT_AKAZE): _pfd = AKAZE::create(AKAZE::DESCRIPTOR_MLDB, 0, 3, 0.0001f, 4, 4, KAZE::DIFF_PM_G2);;					break;
        case (DETECT_AKAZE):
            _pfd = AKAZE::create();
            break;
        case (DETECT_BRISK):
            _pfd = BRISK::create();
            break;
        case (DETECT_DAISY):
            _pfd = xfeatures2d::DAISY::create();
            break;
        case (DETECT_FAST):
            _pfd = FastFeatureDetector::create();
            break;
        case (DETECT_GFTT):
            _pfd = GFTTDetector::create();
            break;
            //case (DETECT_KAZE):  _pfd = KAZE::create(); 					break;
        case (DETECT_KAZE):
            _pfd = KAZE::create(true, false, 0.0001f, 4, 4, KAZE::DIFF_PM_G2);
            break;
        case (DETECT_MSER):
            _pfd = MSER::create();
            break;
        case (DETECT_ORB):
            _pfd = ORB::create(2500);
            break;
        case (DETECT_SIFT):
            _pfd = xfeatures2d::SIFT::create();
            break;
        case (DETECT_SURF):
            _pfd = xfeatures2d::SURF::create();
            break;
    }

    int d = detectorType;
    int e = extractorType;
    if (
            (d == DETECT_AKAZE && e == EXTRACT_AKAZE) ||
            (d == DETECT_BRISK && e == EXTRACT_BRISK) ||
            (d == DETECT_BRISK && e == EXTRACT_DAISY) ||
            (d == DETECT_KAZE && e == EXTRACT_KAZE) ||
            (d == DETECT_MSER && e == EXTRACT_MSER) ||
            (d == DETECT_ORB && e == EXTRACT_ORB) ||
            (d == DETECT_SIFT && e == EXTRACT_SIFT) ||
            (d == DETECT_SURF && e == EXTRACT_SURF)) {
        _pde = _pfd;
    } else {

        switch (extractorType) {
            case (EXTRACT_AKAZE):
                _pde = AKAZE::create();
                break;
            case (EXTRACT_BRIEF):
                _pde = xfeatures2d::BriefDescriptorExtractor::create();
                break;
            case (EXTRACT_BRISK):
                _pde = BRISK::create();
                break;
            case (EXTRACT_DAISY):
                _pde = xfeatures2d::DAISY::create();
                break;
            case (EXTRACT_FREAK):
                _pde = xfeatures2d::FREAK::create();
                break;
            case (EXTRACT_KAZE):
                _pde = KAZE::create();
                break;
            case (EXTRACT_MSER):
                _pde = MSER::create();
                break;
            case (EXTRACT_ORB):
                _pde = ORB::create();
                break;
            case (EXTRACT_SIFT):
                _pde = xfeatures2d::SIFT::create();
                break;
            case (EXTRACT_SURF):
                _pde = xfeatures2d::SURF::create();
                break;

            case (EXTRACT_RootSIFT):
                _pde = xfeatures2d::SIFT::create();
                break;
        }


    }

    //string keyDT = getDetectorKey(detectorType);
    //string keyET = getExtractorKey(extractorType);
    //_pfd = create(keyDT.c_str());
    //cout << "Detector: " << keyDT << endl;
    //_pfd = FeatureDetector::create(keyDT.c_str());

    //cout << "Extractor: " << keyET << endl;
    //_pde = DescriptorExtractor::create(keyET.c_str());

    // use this to fine-tune parameters
    //pfd->set("contrastThreshold", 0.1);
    //Algorithm& algorithm = (Algorithm&) *pfd;
    //printParams( *pfd );

}

void
FeatureMethod::store(string fileName) {

    Configuration cfg;

    cfg.put("detector", getDetectorKey(_detectorType));
    //Algorithm& algoFD = (Algorithm&) *_pfd;
    //storeParams(algoFD, cfg, "detector.");

    cfg.put("extractor", getExtractorKey(_extractorType));
    //Algorithm& algoET = (Algorithm&) *_pde;
    //storeParams(algoET, cfg, "extractor.");

    cfg.store(fileName);

}

//bool startsWith( string str, string prefix ) {
//	return ( str.substr(0, prefix.size()) == prefix );
//}

//string dropPrefix( string str, string prefix ) {
//	return str.substr( prefix.size() );
//}


void
FeatureMethod::load(string fileName) {

    Configuration cfg(fileName);
    load(cfg);

}


void
FeatureMethod::load(Configuration &cfg) {

    string dKey = cfg.get("detector");
    string eKey = cfg.get("extractor");

    int dt = getDetectorType(dKey);
    int et = getExtractorType(eKey);

    _init(dt, et);


    //Algorithm& algoFD = (Algorithm&) *_pfd;
    //readParams(algoFD, cfg, "detector.");

    //Algorithm& algoET = (Algorithm&) *_pde;
    //readParams(algoET, cfg, "extractor.");


}


FeatureMethod::FeatureMethod() {

    _init(DETECT_SIFT, EXTRACT_SIFT);

}

FeatureMethod::FeatureMethod(int detectorType, int extractorType) {

    _init(detectorType, extractorType);

}

FeatureMethod::FeatureMethod(string detectorType, string extractorType) {

    int dt = getDetectorType(detectorType);
    int et = getExtractorType(extractorType);
    _init(dt, et);

}


FeatureMethod::FeatureMethod(string fileName) {

    load(fileName);

}


FeatureMethod::~FeatureMethod() {

}


void printParams(ostream &out, Algorithm &algorithm) {
    std::vector<std::string> parameters;

//    algorithm.getParams(parameters);
//
//    for (int i = 0; i < (int) parameters.size(); i++) {
//        std::string param = parameters[i];
//        int type = algorithm.paramType(param);
//        std::string helpText = algorithm.paramHelp(param);
//        std::string typeText;
//
//        stringstream ss;
//        ss << " value=";
//        switch (type) {
//        case cv::Param::BOOLEAN: {
//				typeText = "bool";
//				bool val = algorithm.getBool(param);
//				ss << boolalpha << val;
//			}
//			break;
//        case cv::Param::INT: {
//				typeText = "int";
//				ss << algorithm.getInt(param);
//			}
//			break;
//        case cv::Param::REAL: {
//				typeText = "real (double)";
//				ss << algorithm.getDouble(param);
////				value += val;
//			}
//			break;
//        case cv::Param::STRING: {
//				typeText = "string";
//				ss << algorithm.getString(param);
//			}
//			break;
//        case cv::Param::MAT: {
//				typeText = "Mat";
//				ss << "<mat>";
//			}
//			break;
//        case cv::Param::ALGORITHM:
//            typeText = "Algorithm";
//            ss << "<algorithm>";
//            break;
//        case cv::Param::MAT_VECTOR:
//            typeText = "Mat vector";
//            ss << "<vector>";
//            break;
//        }

//        out << "Parameter '" << param << "' type=" << typeText << " help=" << helpText << ss.str() << std::endl;

//    }

}

void storeParams(Algorithm &algorithm, Configuration &cfg, string prefix) {
    std::vector<std::string> parameters;
//    algorithm.getParams(parameters);
//
//    for (int i = 0; i < (int) parameters.size(); i++) {
//        std::string param = parameters[i];
//        int type = algorithm.paramType(param);
//        std::string helpText = algorithm.paramHelp(param);
//        std::string typeText;
//
//        stringstream ss;
//
//        switch (type) {
//        case cv::Param::BOOLEAN: {
//				typeText = "bool";
//				bool val = algorithm.getBool(param);
//				ss << boolalpha << val;
//			}
//			break;
//        case cv::Param::INT: {
//				typeText = "int";
//				ss << algorithm.getInt(param);
//			}
//			break;
//        case cv::Param::REAL: {
//				typeText = "double";
//				ss << algorithm.getDouble(param);
//			}
//			break;
//        case cv::Param::STRING: {
//				typeText = "string";
//				ss << algorithm.getString(param);
//			}
//			break;
//        case cv::Param::MAT: {
//				typeText = "Mat";
//				ss << algorithm.getMat(param);
//			}
//			break;
//        case cv::Param::ALGORITHM:
//            typeText = "Algorithm";
//            ss << algorithm.getAlgorithm(param);
//            break;
//        case cv::Param::MAT_VECTOR:
//            typeText = "MatVector";
//            //ss << algorithm.getMatVector(param);
//            break;
//        }
//
//        //out << "Parameter '" << param << "' type=" << typeText << " help=" << helpText << ss.str() << std::endl;
//
//        //out << prefix << param << "=" << ss.str() << endl;
//        string key = prefix + param;
//        string value = ss.str();
//        cfg.put(key, value);
//
//
//    }
}


void readParams(Algorithm &algorithm, Configuration &cfg, string prefix) {
    std::vector<std::string> parameters;
//    algorithm.getParams(parameters);
//
//    for (int i = 0; i < (int) parameters.size(); i++) {
//        std::string param = parameters[i];
//        int type = algorithm.paramType(param);
//
//        string key = prefix + param;
//        if (!cfg.has(key)) {
//        	continue;
//        }
//        string value = cfg.get(key);
//
//        switch (type) {
//        case cv::Param::BOOLEAN: {
//				bool val = atoi(value.c_str());
//				algorithm.set(param, val);
//			}
//			break;
//        case cv::Param::INT: {
//				int val = atoi(value.c_str());
//				algorithm.set(param, val);
//			}
//			break;
//        case cv::Param::REAL: {
//				double val = atof(value.c_str());
//				algorithm.set(param, val);
//			}
//			break;
//        case cv::Param::STRING: {
//				algorithm.set(param, value.c_str());
//			}
//			break;
//        case cv::Param::MAT: {
//        	  // not supported
//			}
//			break;
//        case cv::Param::ALGORITHM:
//        	// not supported
//        	break;
//        case cv::Param::MAT_VECTOR:
//        	// not supported
//        	break;
//        }
//
//
//    }
}
