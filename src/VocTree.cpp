//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "VocTree.h"


#include <set>

#include "MatPersistor.h"
#include "VecPersistor.hpp"
#include "ExtKmeans.h"
#include "FileHelper.h"
#include "KMeans.h"

using namespace cv;
using namespace std;


bool VocTree::isLeaf(int idNode) {
    int idxNode = _index[idNode];
    return (_indexLeaves.at(idxNode) != -1);
}


void
VocTree::cluster(

        int K,
        string &file,
        Mat &centers,
        vector<string> &clusters

) {

    Mat labels;
    //int attempts = 1;
    int maxItt = 5;
    long useMem = 512 * MEGA;

    std::cout << "clustering file: " << file << endl;
    extKmeans(_useNorm, K, maxItt, file, useMem, labels, centers);

    std::cout << "splitting subclusters..." << endl << flush;
    distribute(K, file, labels, useMem, clusters);

}


void
VocTree::cluster(
        int K,
        Mat &descriptors,
        Mat &centers,
        vector<Mat> &clusters
) {

    Mat labels;

    //long maxRows = min(_remainingFeatures, descriptors.rows);
    long rows = descriptors.rows;

    if (rows > 5000)
        cout << "clustering " << rows << endl;
    // convetional clustering

    int attempts = 5;
    int flags = KMEANS_PP_CENTERS;
    TermCriteria term(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0);

    // K means parameters:
    //  - data				: data to be clustered (in our case feature descriptors).
    //  - k					: number of clusters
    //  - labels 			: stores in each row the cluster label of the ith descriptor
    //  - term criteria
    //  - attempts
    //  - flags
    //  - centers

    if (_useNorm == NORM_L2) {
        // use statndard kmeans algorithm
        kmeans(descriptors, K, labels, term, attempts, flags, centers);
    } else {
        // for HAMMING uses kmajority
        myKmeans(_useNorm, K, 10, descriptors, labels, centers);
    }


    // Create empty collections to distribute values in clusters
    for (int i = 0; i < K; i++) {
        Mat cluster = Mat(0, descriptors.cols, descriptors.type());
        clusters.push_back(cluster);
    }

    // Distributes values in clusters
    for (int i = 0; i < rows; i++) {

        //int lbl = labels.at<int>(i, 0);
        int lbl = (int) labels.at<uchar>(i, 0);
        Mat descriptor = descriptors.row(i);
        Mat &cluster = clusters[lbl];

        cluster.push_back(descriptor);

    }

}

int
VocTree::getNextIdxNode() {

    int ret = _usedNodes;
    _usedNodes++;
    return ret;
}

int
VocTree::getNextIdxLeaf() {
    int ret = _usedLeaves;
    _usedLeaves++;
    return ret;
}


void
VocTree::expand(Mat &mat, int rows) {
    if (mat.rows < rows) {
        mat.resize(rows);
    }
}


void shrink(Mat &mat, int rows) {
    if (mat.rows > rows) {
        mat.resize(rows);
    }
}

static int lastProgress = -1;

void showProgress(int k, int idNode, int level, int child) {

    if (level == 1) {
        if (idNode == 1 && child == 0) {
            std::cout << "progress: ";
        }

        int progress = 100 * (k * idNode + child + 1) / (k * k + k);
        if (lastProgress != progress) {
            std::cout << progress << "% ";
            lastProgress = progress;
        }
        if (idNode == k && child == (k - 1)) {
            std::cout << endl;
        }
        std::cout << flush;

    }

}


void
VocTree::buildNodeGen(
        int idNode,
        int level,

        bool fromFile,

        // input data to be clustered
        string *pFileName,
        Mat *pDescs,

        int rows
) {


    assert(fromFile == (pFileName != NULL));
    assert(!fromFile == (pDescs != NULL));

    int idxNode = getNextIdxNode();
    _index[idNode] = idxNode;

    if (rows <= _k || level >= _h) {

        // it's a leaf
        //_leaves.at<char>(idxNode) = 1;
        int idxLeaf = getNextIdxLeaf();
        _indexLeaves[idxNode] = idxLeaf;

    } else {

        // not a leaf
        _indexLeaves[idxNode] = -1;

        Mat centers;

        vector<string> fileClusters;
        vector<Mat> matClusters;

        if (fromFile) {

            // Cluster from file

            string fileName = *pFileName;
            cluster(_k, fileName, centers, fileClusters);

            if (idNode != 0) {
                // the input file is not longer necessary.
                //cout << "deleting " << fileName << endl;
                FileHelper::deleteFile(fileName);
            }

        } else {

            // Cluster from matrix
            //cout << "cluster from matrix" << endl;
            cluster(_k, *pDescs, centers, matClusters);


        }

        // for each cluster builds a child recursively
        for (int i = 0; i < _k; i++) {

            int newLevel = level + 1;
            int childId = idChild(idNode, i);

            if (fromFile) {

                string fileCluster = fileClusters[i];
                createNode(childId, newLevel, fileCluster);

            } else {

                Mat cluster = matClusters[i];
                buildNodeFromMat(childId, newLevel, cluster);

            }

            int childIdx = _index[childId];
            centers.row(i).copyTo(_centers.row(childIdx));

            showProgress(_k, idNode, level, i);

        }


    }


}


int
VocTree::idChild(int idNode, int numChild) {

    // --------------------
    // Computing child ids:
    // --------------------
    // Tree Example K = 2
    // the childs of node 5 are nodes (11 and 12):
    // 0 | 1 2 | 3 4 5 6 | 7 8 9 10 11 12 13 14
    //               x              xx xx
    // - node 5 is on 2nd level.
    // - Using geometric sum = (K^n - 1) / (K - 1)
    // - 2nd level starts at: sl := (2^2 - 1)/(2-1) = 3
    // - 3rd level starts at: nl := (2^3 - 1)/(2-1) = 7
    // - each node preceding to 5 has k children,
    //		diff := 5 - sl = 2 nodes
    // - then child(i) of 5 is at: nl + K*diff + i = 11 + i
    // --------------------
    // But,
    // child(i) = nl + K*diff + i = nl + K* [ id - [(K^n - 1) / (K - 1)]] + i =
    //  = nl - (K^(n+1) - K)/(K-1) + K*id + i = [K*id + 1 + i].

    return (_k * idNode) + 1 + numChild;

}


void
VocTree::buildNodeFromFile(int idNode,
                           int level,
                           string &file,
                           int rows) {

    buildNodeGen(idNode, level, true, &file, NULL, rows);

}

void
VocTree::buildNodeFromMat(int idNode,
                          int level,
                          Mat &mat) {

    buildNodeGen(idNode, level, false, NULL, &mat, mat.rows);

}


void
VocTree::createNode(int idNode,
                    int level,
                    string &file) {

    Mat descriptors;
    MatPersistor mp(file);

    mp.openRead();
    int rows = mp.rows();
    int cols = mp.cols();
    int size = mp.elementSize();

    long useMem = 1 * (long) GIGA;
    //long useMem = 1 * (long) MEGA;
    //long useMem = 128 * (long) MEGA;

    long required = rows;
    required *= cols;
    required *= size;

    if (required > useMem) {

        // rows don't fit in memory
        // then process buffering from file.
        mp.close();

        buildNodeFromFile(idNode, level, file, rows);
        //buildNodeGen( idNode, level, true, &file, NULL, rows );

    } else {

        // resuming from RAM
        // rows fit entirely in memory.
        mp.read(descriptors, required);
        mp.close();

        // the file was already clustered,
        // then is is not longer necessary.
        if (idNode != 0) {
            FileHelper::deleteFile(file);
        }

        buildNodeFromMat(idNode, level, descriptors);

    }


}


int
VocTree::getStartingFeatureRow(Catalog<DBElem> &catalog, int startImage) {

    int startingRow = 0;
    for (int idFile = 0; idFile < startImage; idFile++) {
        DBElem info = catalog.get(idFile);
        startingRow += info.featuresCount;
    }
    return startingRow;

}


void
VocTree::addElements(Catalog<DBElem> &catalog, int startImage) {

    FileManager fm(_path);
    string file = fm.file(FileManager::DESCRIPTORS);
    MatPersistor mp(file);
    mp.openRead();

    int startingRow = getStartingFeatureRow(catalog, startImage);
    mp.setRow(startingRow);

    _invIdx.resize(_usedLeaves);
    mp.setRow(0);

    // For each image
    for (int idFile = startImage; idFile < catalog.size(); idFile++) {

        DBElem info = catalog.get(idFile);

        cout << "adding image: " << info.name << endl;

        // Read the descriptors
        Mat descriptors;
        mp.read(descriptors, info.featuresCount);

        // add each descriptor, to the inverted file index
        for (int d = 0; d < descriptors.rows; d++) {

            Mat descriptor = descriptors.row(d);
            int idLeaf = findLeaf(descriptor);
            int idxNode = _index[idLeaf];
            int idxLeaf = _indexLeaves[idxNode];

            vector<int> &invIdx = _invIdx.at(idxLeaf);
            invIdx.push_back(idFile);
            _totDescriptors++;


        }

    }

    mp.close();


}

void
VocTree::buildNodes() {

    _usedNodes = 0;
    _usedLeaves = 0;
    _totDescriptors = 0;


    // A kd-tree of L levels has #nodes = [ k^(L+1) - k ] / [ k - 1 ]
    // (see paper section 3) note this equation does not take in account the root
    //_nNodes = (pow(k, _h + 1) - k)/ (k - 1);
    // this equation does consider the root:
    _nNodes = (pow(_k, _h + 1) - 1) / (_k - 1);

    cout << "creating nodes... " << endl;

    FileManager fm(_path);
    //string fileDescriptors = fm.file( FileManager::DESCRIPTORS );
    string fileDescriptors = fm.file(FileManager::VOCABULARY_DESCRIPTORS);

    // creates the matrix for centers using
    // the same type as the descriptors.
    MatPersistor mp(fileDescriptors);
    mp.openRead();
    _centDim = mp.cols();
    _centType = mp.type();
    mp.close();

    // fills the indexes with 0s
    _index.assign(_nNodes, 0);
    _indexLeaves.resize(_nNodes);

    // create center buffer and expands it
    // for example. (K=10, h=6, dim=128x4) => approx. 543 MB
    _centers.create(0, _centDim, _centType);
    expand(_centers, _nNodes);

    // Creates root node.
    createNode(0, 0, fileDescriptors);

    // reduce buffers to gain some memory
    //_index.resize(_usedNodes);
    _indexLeaves.resize(_usedNodes);
    shrink(_centers, _usedNodes);


}


VocTree::VocTree(int k, int h, Catalog<DBElem> &images, string &path, bool reuseVocabulary, int useNorm
        //int kmeansAtt,
        //TermCriteria crit
) {

    cout << "voctree create" << endl;

    bool reuseInvIdx = false;

    _path = path;
    FileManager fileMgr(_path);
    string prefix = fileMgr.mapData("voctree_");
    string fileInfo = prefix + "info.xml";
    string fileInvIdx = prefix + "invIdx.bin";
    string fileWeights = prefix + "weights.bin";
    string fileVectors = prefix + "vectors.bin";
    string nodesPrefix = prefix + "nodes";
    string fileMinDistances = prefix + "minDistances.bin";

    if (reuseVocabulary) {

        cout << "loading info" << endl;
        loadInfo(fileInfo);

        cout << "loading nodes" << endl;
        loadNodes(nodesPrefix);

    } else {

        _k = k;
        _h = h;
        _dbSize = images.size();
        _useNorm = useNorm;

        buildNodes();

        cout << "storing nodes" << endl;
        storeNodes(nodesPrefix);

    }


    if (reuseInvIdx) {

        cout << "loading inverted indexes..." << endl;
        loadInvIdx(fileInvIdx);

    } else {

        cout << "creating inverted indexes..." << endl;
        addElements(images, 0);
        _dbSize = images.size();

        cout << "storing inverted indexes..." << endl;
        storeInvIdx(fileInvIdx);

    }

    computeVectors();

    cout << "storing weights..." << endl;
    storeWeights(fileWeights);

    cout << "storing d-vectors..." << endl;
    storeVectors(fileVectors);

    cout << "storing info" << endl;
    storeInfo(fileInfo);

    cout << "voctree created" << endl;

    showInfo();

}


void
VocTree::showInfo() {

    std::cout << "-----------------------------" << endl;
    std::cout << "VocTree Info: " << endl;
    std::cout << ">max height (H): " << _h << endl;
    std::cout << ">children by node (K): " << _k << endl;
    std::cout << ">DB file count: " << _dbSize << endl;
    std::cout << ">total nodes: " << _usedNodes << endl;
    std::cout << ">total leaves: " << _usedLeaves << endl;
    std::cout << "-----------------------------" << endl;

}


void
VocTree::update(Catalog<DBElem> &images) {

    if (images.size() == _dbSize) {
        std::cout << "there's no new image in the database." << endl;
        return;
    }

    FileManager fileMgr(_path);
    string prefix = fileMgr.mapData("voctree_");
    string fileInfo = prefix + "info.xml";
    string fileInvIdx = prefix + "invIdx.bin";
    string fileWeights = prefix + "weights.bin";
    string fileVectors = prefix + "vectors.bin";

    std::cout << "loading inverted indexes..." << endl;
    loadInvIdx(fileInvIdx);

    std::cout << "updating inverted indexes..." << endl;

    addElements(images, _dbSize);
    _dbSize = images.size();


    std::cout << "storing inverted indexes..." << endl;
    storeInvIdx(fileInvIdx);

    computeVectors();

    std::cout << "storing weights..." << endl;
    storeWeights(fileWeights);


    std::cout << "storing d-vectors..." << endl;
    storeVectors(fileVectors);

    std::cout << "storing info" << endl;
    storeInfo(fileInfo);

    std::cout << "voctree updated" << endl;

    showInfo();

}

void
VocTree::loadInfo(string &fileName) {

    FileStorage file(fileName, cv::FileStorage::READ);

    _k = (int) file["k"];
    _h = (int) file["h"];
    _useNorm = (int) file["useNorm"];
    _dbSize = (int) file["dbSize"];
    _nNodes = (int) file["nNodes"];
    _usedNodes = (int) file["nextIdNode"];
    _usedLeaves = (int) file["nextIdLeaf"];
    _totDescriptors = (int) file["totDescriptors"];

}


void
VocTree::computeVectors() {

    _weights.create(_usedNodes, 1, CV_32F);
    _dVectors.resize(_usedNodes);

    vector<IIFEntry> invIdx;
    cout << "computing d-vectors..." << endl;
    computeInvertedIndex(0, 0, invIdx);


    // normalize d-vectors
    //L1 Normalization of each row of d vectors
    // see equation 3 on section 4:
    // (computes:  d / || d ||)

    cout << "normalizing d-vectors...";
    // we compute norm L1
    vector<float> sum(_dbSize, 0);
    for (unsigned int idx = 0; idx < _dVectors.size(); idx++) {
        vector<DComponent> &comps = _dVectors.at(idx);
        for (unsigned int pos = 0; pos < comps.size(); pos++) {
            DComponent &dc = comps.at(pos);
            sum.at(dc.idFile) += dc.value; // L1
            //sum.at( dc.idFile ) += (dc.value * dc.value); // L2

        }
    }
    cout << " ... " << endl;
    long count = 0;
    for (unsigned int idx = 0; idx < _dVectors.size(); idx++) {
        vector<DComponent> &comps = _dVectors.at(idx);
        for (unsigned int pos = 0; pos < comps.size(); pos++) {
            DComponent &dc = comps.at(pos);
            //dc.value = sqrt( dc.value / sum.at( dc.idFile )); // Hellinger Kernel
            dc.value /= sum.at(dc.idFile); // L1
            //dc.value /= sqrt( sum.at( dc.idFile ) ); //L2
            count++;
        }
    }

    return;

}


void
VocTree::computeInvertedIndex(int idNode, int level, vector<IIFEntry> &out) {

    if (isLeaf(idNode)) {

        int idxNode = _index[idNode];
        int idxLeaf = _indexLeaves[idxNode];

        vector<int> &invIdx = _invIdx.at(idxLeaf);

        // converts the format of leaves inverted index
        // to the format of virtual inverted indexes
        // Example: [1,1,1,2,3,3,3,3,3] -> [1:3,2:1,3:5]

        IIFEntry ent;
        ent.idFile = -1;
        for (unsigned int i = 0; i < invIdx.size(); i++) {
            int idFile = invIdx.at(i);
            if (idFile != ent.idFile) {
                if (ent.idFile != -1) {
                    out.push_back(ent);
                }
                ent.idFile = idFile;
                ent.featCount = 1;
            } else {
                ent.featCount++;
            }
        }
        if (ent.idFile != -1) {
            out.push_back(ent);
        }


    } else {

        // not leaf

        out.clear();

        // compute the inverted indexes for all this childs
        vector<vector<IIFEntry> > virtualInvIdx(_k);
        for (int i = 0; i < _k; i++) {
            int childId = idChild(idNode, i);
            computeInvertedIndex(childId, level + 1, virtualInvIdx.at(i));
        }

        // now join the child inverted indexes onto the one on the current node.
        // since, each inverted index is ordered by file id, it is possible to do a sorted merge
        // to do that, use a vector of K "pointers" to the lower id image in each child inverted index.
        vector<int> pointers(_k, 0);
        vector<int> toAdvance;

        while (1) {

            toAdvance.clear();

            IIFEntry ent;
            ent.idFile = INT_MAX;
            for (int i = 0; i < _k; i++) {

                vector<IIFEntry> &invIdx = virtualInvIdx.at(i);
                unsigned int pos = pointers.at(i);
                if (pos < invIdx.size()) {

                    if (invIdx.at(pos).idFile < ent.idFile) {
                        ent = invIdx.at(pos); //does a copy
                        toAdvance.clear();
                        toAdvance.push_back(i);
                    } else if (invIdx.at(pos).idFile == ent.idFile) {
                        ent.featCount += invIdx.at(pos).featCount;
                        toAdvance.push_back(i);
                    }

                }

            }

            if (toAdvance.size() == 0) {
                // there're no more entries
                break;
            }

            // collect the merged entry
            out.push_back(ent);

            // advance the pointers
            for (unsigned int i = 0; i < toAdvance.size(); i++) {
                int idxPointer = toAdvance.at(i);
                pointers.at(idxPointer)++;
            }

        }

        // since we don't need any more the child's inverted indexes
        // let's remove them to free some memory
        for (int i = 0; i < _k; i++) {
            virtualInvIdx.at(i).clear();
        }

    }


    // Ni: the number of images in the database with at least one descriptor vector path through node i
    int Ni = out.size();
    int N = _dbSize;
    float weight = log((double) N / (double) Ni);

    int idxNode = _index[idNode];
    _weights.at<float>(idxNode) = weight;
    vector<DComponent> &comps = _dVectors.at(idxNode);
    comps.resize(Ni);

    for (int pos = 0; pos < Ni; pos++) {

        IIFEntry ent = out.at(pos);

        float mji = ent.featCount;

        DComponent &dc = comps.at(pos);
        dc.idFile = ent.idFile;
        dc.value = weight * mji;

    }


}


void
VocTree::loadNodes(string &filePrefix) {

    string fileIdx = filePrefix + ".index";
    string fileLeaves = filePrefix + ".leaves";
    string fileCenters = filePrefix + ".centers";

    VecPersistor vp;
    vp.restore(fileIdx, _index);
    vp.restore(fileLeaves, _indexLeaves);

    MatPersistor mpc(fileCenters);
    mpc.openRead();
    mpc.read(_centers);
    mpc.close();


}


VocTree::VocTree(string &path) {

    bool loadInvertedIndexes = false;

    std::cout << "voctree create" << endl;

    _path = path;
    FileManager fileMgr(_path);
    string prefix = fileMgr.mapData("voctree_");
    string fileInfo = prefix + "info.xml";
    string fileInvIdx = prefix + "invIdx.bin";
    string fileWeights = prefix + "weights.bin";
    string fileVectors = prefix + "vectors.bin";
    string nodesPrefix = prefix + "nodes";

    std::cout << "loading info" << endl;
    loadInfo(fileInfo);

    std::cout << "loading nodes" << endl;
    loadNodes(nodesPrefix);

    if (loadInvertedIndexes) {

        std::cout << "loading inverted indexes..." << endl;
        loadInvIdx(fileInvIdx);

    }

    std::cout << "loading weights..." << endl;
    loadWeights(fileWeights);

    std::cout << "loading d-vectors..." << endl;
    loadVectors(fileVectors);

    std::cout << "voctree loaded" << endl;

    showInfo();

}


VocTree::~VocTree() {
    cout << "voctree delete" << endl;
}


list<int>
VocTree::findPath(Mat &descriptor) {

    list<int> path;
    int idNode = 0;
    unsigned int numCh = _k;

    path.push_back(idNode);

    while (!isLeaf(idNode)) {

        //Search the closest sub-cluster
        int idClosest = 0;
        double minDist = numeric_limits<int>::max();

        for (size_t i = 0; i < numCh; i++) {

            int childId = idChild(idNode, i);
            int idxChild = _index[childId];
            double d = norm(descriptor, _centers.row(idxChild), _useNorm);

            if (i == 0 || d < minDist) {
                minDist = d;
                idClosest = childId;
            }

        }

        idNode = idClosest;
        path.push_back(idNode);
    }

    return path;

}


int
VocTree::findLeaf(Mat &descriptor) {

    int idNode = 0;

    while (!isLeaf(idNode)) {

        // Search the closest center
        int idClosest = 0;
        float minDist = -1;

        for (int i = 0; i < _k; i++) {

            int childId = idChild(idNode, i);

            int idxChild = _index[childId];

            float d = norm(descriptor, _centers.row(idxChild), _useNorm);
            if (i == 0 || d < minDist) {
                minDist = d;
                idClosest = childId;
            }

        }

        idNode = idClosest;
    }


    return idNode;

}


void
VocTree::query(Mat &descriptors, vector<Matching> &result, int limit) {

    vector<float> q(_usedNodes, 0);
    double sum = 0;
    for (int i = 0; i < descriptors.rows; i++) {

        Mat qDescr = descriptors.row(i);

        list<int> path = findPath(qDescr);

        // computes qi = ni * wi (see paper 4.1)
        list<int>::iterator it = path.begin();
        for (; it != path.end(); it++) {

            int idNode = (*it);
            int idxNode = _index[idNode];

            float weight = _weights.at<float>(idxNode);
            if (!(isinf(weight))) {

                q[idxNode] += weight; // L1
                //q[ idxNode ] += (weight * weight); // L2
                sum += weight;

            }

        }

    }

    //Now normalize q vector
    for (unsigned int i = 0; i < q.size(); i++) {
        //q[i] = sqrt( q[i] / sum ); // Hellinger Kernel?
        q[i] /= sum; // L1
        //q[i] /= sum*sum; // L2
    }

    //Now perform |q - d| for every d database element
    result.resize(_dbSize);

    //for (int m = 0; m < _dbSize; m++) {
    //	//cout << "setting score for " << m << endl;
    //	Matching& match = result.at( m );
    //	DBElem elem = _catalog.get(m);
    //	match.id = elem.id;
    //	match.score = 2;
    //}

    //cout << "non-zero count:" << _d_vectors.nzCount() << endl;
    for (unsigned int idxNode = 0; idxNode < q.size(); idxNode++) {
        float qi = q[idxNode];
        if (qi > 0) {

            vector<DComponent> &comps = _dVectors.at(idxNode);
            for (unsigned int pos = 0; pos < comps.size(); pos++) {

                DComponent &dc = comps.at(pos);
                float di = dc.value;
                float diff = abs(qi - di);

                Matching &match = result.at(dc.idFile);
                match.id = dc.idFile;

                match.score += (diff - di - qi); // L1
                //match.score += (diff*diff - di*di - qi*qi); // L2
                //match.score -= (2 * qi * di); // L2


            }

        }
    }


    //Now, sort the matching vector from highest to lowest score
    //int limit = 100;
    //	Ptr< vector<Matching> > res = new vector<Matching>( min(_dbSize, limit); );
    //	partial_sort_copy(result.begin(), result.end(), res->begin(), res->end());

    sort(result.begin(), result.end());
    if ((unsigned int) limit < result.size()) {
        result.resize(limit);
    }


    double zeroEps = 1e-03;
    int shrink = 0;
    for (unsigned int i = 0; i < result.size(); i++) {
        if (result[i].score < zeroEps) {
            result[i].score = 0;
        } else if (result[i].id == -1) {
            shrink++;
        }
    }
    if (shrink > 0) {
        //cout << "shrinking" << shrink << endl;
        result.resize(result.size() - shrink);
    }


}


void
VocTree::storeInfo(string &fileName) {

    FileStorage file(fileName, cv::FileStorage::WRITE);
    file << "k" << _k;
    file << "h" << _h;
    file << "useNorm" << _useNorm;
    file << "dbSize" << _dbSize;
    file << "nNodes" << _nNodes;
    file << "nextIdNode" << _usedNodes;
    file << "nextIdLeaf" << _usedLeaves;
    file << "totDescriptors" << _totDescriptors;
    //---

}


void VocTree::storeVectors(string &fileName) {

    FILE *pFile = fopen(fileName.c_str(), "wb");

    if (pFile == 0) {
        cerr << "can't write d-vectors file." << endl;
        exit(-1);
    }

    int elemSize = 4;
    int bufferLen = 16 * MEGA;
    void *pBuffer = malloc(bufferLen);
    if (pBuffer == NULL) {
        fclose(pFile);
        cerr << "can't allocate memory" << endl;
        exit(-1);
    }

    int *pInt = (int *) pBuffer;
    float *pFloat = (float *) pBuffer;

    vector<DComponent> *pComps = NULL;
    int pos = 0;
    int cursor = 0;
    bool onIdFile;
    int size;
    long written;
    long bytes;

    unsigned int idx = 0;

    while (idx < _dVectors.size()) {

        if (pComps == NULL) {
            pComps = &(_dVectors.at(idx));
            pos = 0;
            size = pComps->size();
            pInt[cursor++] = size;
            onIdFile = true;
        } else {
            DComponent dc = pComps->at(pos);
            if (onIdFile) {
                pInt[cursor++] = dc.idFile;
                onIdFile = false;
            } else {
                pFloat[cursor++] = dc.value;
                onIdFile = true;
                pos++;
            }
        }

        bytes = elemSize * cursor;
        if (bytes >= bufferLen) {
            written = fwrite(pBuffer, 1, bytes, pFile);
            assert(written == bytes);
            cursor = 0;
        }

        if (pos >= size) {
            // advance to the next d-components
            pComps = NULL;
            idx++;
        }

    }

    bytes = elemSize * cursor;
    if (bytes > 0) {
        written = fwrite(pBuffer, 1, bytes, pFile);
        assert(written == bytes);
    }

    free(pBuffer);
    fclose(pFile);


}


void
VocTree::loadVectors(string &fileName) {

    FILE *pFile = fopen(fileName.c_str(), "rb");

    if (pFile == 0) {
        cerr << "can't read d-vectors file." << endl;
        exit(-1);
    }

    _dVectors.resize(_usedNodes);

    int elemSize = 4;
    int bufferLen = 16 * MEGA;
    void *pBuffer = malloc(bufferLen);
    if (pBuffer == NULL) {
        fclose(pFile);
        cerr << "can't allocate memory" << endl;
        exit(-1);
    }
    int *pInt = (int *) pBuffer;
    float *pFloat = (float *) pBuffer;
    vector<DComponent> *pComps = NULL;
    unsigned int idx = 0;
    int pos = 0;
    bool onIdFile;
    int size;
    long read;

    while ((read = fread(pBuffer, 1, bufferLen, pFile)) > 0) {

        assert(read % elemSize == 0);

        int cursor = 0;
        while ((cursor * elemSize) < read) {

            if (pComps == NULL) {
                pComps = &(_dVectors.at(idx));
                pos = 0;
                size = pInt[cursor++];
                pComps->resize(size);
                onIdFile = true;
            } else {

                DComponent &dc = pComps->at(pos);
                if (onIdFile) {
                    dc.idFile = pInt[cursor++];
                    onIdFile = false;
                } else {
                    dc.value = pFloat[cursor++];
                    onIdFile = true;
                    pos++;
                }
            }

            if (pos >= size) {
                // advance to the next d-components
                pComps = NULL;
                idx++;
            }

        }


    }

    free(pBuffer);
    fclose(pFile);


}


void
VocTree::storeInvIdx(string &fileName) {

    MatPersistor mp(fileName);
    mp.create(1, CV_32S);
    mp.openWrite();

    int elemSize = 4;
    int useMem = (128 * MEGA);
    int bufferRows = useMem / elemSize;
    Mat buffer(bufferRows, 1, mp.type());

    int buffered = 0;
    int totalRows = 0;

    for (unsigned int idx = 0; idx < _invIdx.size(); idx++) {

        vector<int> &invIdx = _invIdx.at(idx);

        int rowsToWrite = elemSize + (elemSize * invIdx.size());
        if (buffered + rowsToWrite > bufferRows) {
            // flush content to disk.
            mp.append(buffer, buffered);
            buffered = 0;
        }

        buffer.at<int>(buffered++) = invIdx.size();
        totalRows++;

        for (unsigned int pos = 0; pos < invIdx.size(); pos++) {
            int idFile = invIdx.at(pos);
            buffer.at<int>(buffered++) = idFile;
            totalRows++;
        }

    }

    cout << _invIdx.size() << " total rows:" << totalRows << endl;

    mp.append(buffer, buffered);
    mp.close();

}


void
VocTree::loadInvIdx(string &fileName) {

    MatPersistor mp(fileName);
    mp.openRead();

    _invIdx.resize(_usedLeaves);

    int elemSize = 4;
    int useMem = (16 * MEGA);
    //int useMem = (1 * KILO);
    int bufferRows = useMem / elemSize;
    //int bufferRows = 16;

    Mat buffer(bufferRows, 1, mp.type());

    int totalRows = 0;
    int rowsRead;
    int idx = 0, pos, size;

    vector<int> *pInvIdx = NULL;

    // read a chunk of data
    while ((rowsRead = mp.read(buffer, bufferRows)) > 0) {

        totalRows += rowsRead;

        int cursor = 0;
        while (cursor < rowsRead) {

            // read one element from chunk
            int data = buffer.at<int>(cursor);
            cursor++;

            if (pInvIdx == NULL) {

                // the inverted file indexes are stored as follows:
                // N1 , value1, value2, ..., valueN1,
                // N2 , value1, value2, ..., valueN2,
                // then, if (pInvIdx == NULL)
                // means that "data" is the size of the inverted file index

                // there's no inverted index started
                size = data;
                pInvIdx = &(_invIdx.at(idx));
                pInvIdx->resize(size);
                pos = 0;

            } else {

                // one inverted file is being currently parsed
                pInvIdx->at(pos) = data;
                pos++;

            }

            if (pos >= size) {
                // advance to the next inverted file index
                pInvIdx = NULL;
                idx++;
            }


        }

    }

    mp.close();

}


void
VocTree::storeNodes(string &filePrefix) {

    string fileIdx = filePrefix + ".index";
    string fileLeaves = filePrefix + ".leaves";
    string fileCenters = filePrefix + ".centers";

    VecPersistor vp;
    vp.persist(fileIdx, _index);
    vp.persist(fileLeaves, _indexLeaves);

    MatPersistor mpc(fileCenters);
    mpc.create(_centers);

}

void
VocTree::storeWeights(string &fileName) {

    MatPersistor mp(fileName);
    mp.create(_weights);

}

void
VocTree::loadWeights(string &fileName) {

    MatPersistor mp(fileName);
    mp.openRead();
    mp.read(_weights);
    mp.close();

}
