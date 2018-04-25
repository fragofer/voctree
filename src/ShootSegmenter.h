//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include <cv.hpp>

using namespace cv;
using namespace std;

#ifndef SHOOTSEGMENTER_H_
#define SHOOTSEGMENTER_H_

class ShootSegmenter {

    /**
     * Since a video can have many pictures almost duplicated,
     * it is interesting to have a mechanism to choose a subset of that frames.
     *
     * This class receives a sequence of frames (from a video) and
     * decides which frames to consider (trying discarding almost duplicated frames).
     *
     */

public:

    /**
     * Constructor
     */
    ShootSegmenter();

    /**
    * Destructor
    */
    virtual ~ShootSegmenter();

    /**
     * Decides if this frame has to be considered
     * @param frame input frame
     * @return true if this frame has to be considered
     */
    bool
    chooseThisFrame(Mat &frame);


private:

    static const int SHRINK_SIZE = 16;
    static const int THRESH_DIFF = 200;
    static const int STAB_ = 800;
    static const int MAX_RESAMPLE = 12;

    int _numFrames;
    int _lastBigJum;
    int _lastChosenPos;
    Mat _shf;
    Mat _lshf;
    Mat _lastChosenShk;

    void
    shrink(Mat &img, int maxDim, Mat &out);


};

#endif /* SHOOTSEGMENTER_H_ */
