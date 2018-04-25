//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.
#include "ShootSegmenter.h"

using namespace cv;

void
ShootSegmenter::shrink(Mat &img, int maxDim, Mat &out) {

    float aspect = (float) img.rows / img.cols;
    Size newSize;

    if (aspect > 1) {

        // portrait image
        if (img.rows < maxDim) {
            return;
        }
        int newWidth = (int) round(maxDim / aspect);
        newSize = Size(newWidth, maxDim);
    } else {
        // landscape image
        if (img.cols < maxDim) {
            return;
        }
        int newHeight = (int) round(maxDim * aspect);
        newSize = Size(maxDim, newHeight);
    }

    cv::resize(img, out, newSize, 0, 0, INTER_LINEAR);

}


bool
ShootSegmenter::chooseThisFrame(Mat &frame) {

    //_numFrames++;
    //return _numFrames % 5 == 0;

    shrink(frame, SHRINK_SIZE, _shf);

    float diff = 0;
    if (_numFrames > 0) {
        diff = norm(_shf, _lshf, NORM_L2);
    }
    _shf.copyTo(_lshf);

    bool chooseThis = false;

    if (diff > THRESH_DIFF) {

        // there's a big jump.

        if (_numFrames - _lastBigJum > 10) {

            // and is stabilized.
            chooseThis = true;

        }
        _lastBigJum = _numFrames;

    } else {

        // the last frame is very similar to the current

        if (_lastChosenPos > -1 && _numFrames - _lastChosenPos > MAX_RESAMPLE) {

            // but it's been a long time since we don't sample anything

            diff = norm(_shf, _lastChosenShk, NORM_L2);
            if (diff > THRESH_DIFF) {

                // the current frame differs by a big jump from the last chosen.
                chooseThis = true;

            }

        }

    }


    if (chooseThis) {
        // remembers the last frame chosen
        _lastChosenPos = _numFrames;
        _shf.copyTo(_lastChosenShk);
    }

    _numFrames++;

    return chooseThis;


}


ShootSegmenter::ShootSegmenter() {

    _numFrames = 0;
    _lastChosenPos = -1;
    _lastBigJum = -1;

}

ShootSegmenter::~ShootSegmenter() {

}

