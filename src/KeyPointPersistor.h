#ifndef KEYPOINTPERSISTOR_H_
#define KEYPOINTPERSISTOR_H_

#include <stdlib.h>
#include <cv.h>
#include <vector>

using namespace cv;
using namespace std;

class KeyPointPersistor {
public:

    void persist(string file_path, vector<KeyPoint> &kps);

    void restore(string file_path, vector<KeyPoint> &kps);

    void append(string file_path, vector<KeyPoint> &kps);

    KeyPointPersistor();

    virtual ~KeyPointPersistor();

private:
    void copyTo(Mat &aux, vector<KeyPoint> &kps);
};

#endif /* KEYPOINTPERSISTOR_H_ */
