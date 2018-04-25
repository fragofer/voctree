//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "KeyPointPersistor.h"
#include "MatPersistor.h"

#include <cv.h>
#include <vector>

using namespace std;
using namespace cv;


void
KeyPointPersistor::copyTo(Mat &aux, vector<KeyPoint> &kps) {
    aux.create(kps.size(), 6, CV_32F);

    for (unsigned int i = 0; i < kps.size(); i++) {

        KeyPoint kp = kps.at(i);

        aux.at<float>(i, 0) = kp.pt.x;
        aux.at<float>(i, 1) = kp.pt.y;
        aux.at<float>(i, 2) = kp.angle;
        aux.at<float>(i, 3) = kp.size;
        aux.at<float>(i, 4) = kp.octave;
        aux.at<float>(i, 5) = kp.response;

    }

}

void
KeyPointPersistor::persist(string file_path, vector<KeyPoint> &kps) {

    Mat aux;
    copyTo(aux, kps);

    MatPersistor mp(file_path);
    mp.create(aux.cols, aux.type());
    mp.openWrite();
    mp.append(aux);
    mp.close();

    //mp.persist(file_path, aux);

    //cout << aux << endl;

}

void
KeyPointPersistor::append(string filePath, vector<KeyPoint> &kps) {

    Mat aux;
    copyTo(aux, kps);

    MatPersistor mp(filePath);
    if (!mp.exists()) {
        mp.create(aux);
    } else {
        mp.openWrite();
        mp.append(aux);
        mp.close();
    }

}


void
KeyPointPersistor::restore(string file_path, vector<KeyPoint> &kps) {

    Mat aux;

    MatPersistor mp(file_path);
    mp.openRead();
    mp.read(aux);

    //cout << aux << endl;

    kps.clear();

    for (int i = 0; i < aux.rows; i++) {

        KeyPoint kp;
        kp.pt.x = aux.at<float>(i, 0);
        kp.pt.y = aux.at<float>(i, 1);
        kp.angle = aux.at<float>(i, 2);
        kp.size = aux.at<float>(i, 3);
        kp.octave = aux.at<float>(i, 4);
        kp.response = aux.at<float>(i, 5);

        kps.push_back(kp);

    }


}


KeyPointPersistor::KeyPointPersistor() {
}

KeyPointPersistor::~KeyPointPersistor() {
}

