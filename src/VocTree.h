// Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
// This program is free software: you can use, modify and/or
// redistribute it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later
// version. You should have received a copy of this license along
// this program. If not, see <http://www.gnu.org/licenses/>.


#ifndef VOCTREE_H_
#define VOCTREE_H_

#include <cv.h>
#include <vector>
#include <list>

#include "Matching.h"
#include "Catalog.h"
#include "FileManager.h"


using namespace cv;
using namespace std;


class VocTree {

public:

    /**
     *  Vocabulary tree constructor.
     *  @param  k branch factor.
     *  @param  h maximum height for the tree.
     *  @param  images catalog of images.
     *  @param  dbPath path to database root.
     *  @param  reuseCenters reuses vocabulary
     *  @param  useNorm norm to compare features
     *
     */

    VocTree(
            int k,
            int h,
            Catalog<DBElem> &images,
            string &dbPath,
            bool reuseCenters,
            int useNorm

    );

    /**
     * Vocabulary tree constructor.
     * Loads a vocabulary tree from the given path
     * @param path path where vocabulary tree is located
     */
    VocTree(string &path);

    /**
     * Vocabulary tree destructor
     */
    virtual ~VocTree();

    /**
     * given a matrix with descriptors, performs a query with these descriptors
     * @param queryDescrs input descriptors
     * @param reslt vector with the resulting scores
     * @param limit maximum number of results
     */
    void query(Mat &queryDescrs,
               vector<Matching> &result,
               int limit);

    /**
     * updates the vocabulary tree with new images
     * @param images images catalog
     */
    void update(Catalog<DBElem> &images);

    /**
     * saves the vocabulary tree to disk
     */
    void store();


    /**
     * displays vocabulary statistics info
     */
    void showInfo();

private:

    // norm used to compare descriptors
    int _useNorm;

    // path where vocabulary tree is stored
    string _path;

    // Branch factor
    int _k;
    // Maximum height for the tree
    int _h;

    // Actual number of used nodes
    int _usedNodes;
    // Number of leaves
    int _usedLeaves;

    // Maximum number of nodes (if tree was complete)
    int _nNodes;

    // Dimension of the visual words (for example if SIFT is used then _centDim = 128)
    int _centDim;
    // Data type of the visual words (see OpenCV data types)
    int _centType;

    // Number of indexed images
    int _dbSize;

    // Number of indexed descriptors
    int _totDescriptors;

    // nodes index
    // since the vocabulary tree is not a <K,H> complete tree, only the used nodes indices are stored
    // in this _index vector. _index vector length will be the number of used nodes.
    vector<int> _index;

    // leafs indices
    // _indexLeaves vector has the same length than _index.
    // It stores in each position a (-1) value if that position corresponds to an internal node and
    // the stores the id of leaf node if that position corresponds to a leaf node.
    vector<int> _indexLeaves;

    // nodes information
    // _centers: Mat in R^(_usedNodes x D), stores the nodes centers (or visual words)
    // _weights: Mat in R^_usedNodes, stores the nodes weights
    Mat _centers;
    Mat _weights;

    // for virtual inverted indexes (IIF: Inverted Index File)
    struct IIFEntry {
        int idFile;
        short featCount;
    };

    // inverted indexes:
    // 	-are stored only on the leafs
    //	-have the ids of the images
    //	-can have duplicates
    vector<vector<int> > _invIdx;

    /**
     * @param idNode node index to test
     * @return true if idNode is a leaf
     */
    bool isLeaf(int idNode);

    /**
     * used to create sequential nodes ids, and increments _usedNodes
     * @return the next id node to be used
     */
    int getNextIdxNode();

    /**
     * used to create sequential leaves ids, and increments _usedLeaves
     * @return the next id leaf to be used
     */
    int getNextIdxLeaf();

    /**
     * given a node id, and a child position 0 <= numChild < K, it returns the id of that child node
     * @param idNode input parent id node
     * @param numChild number of child
     * @return the id node of that child
     */
    int idChild(int idNode, int numChild);


    // used to store the d vectors
    struct DComponent {
        int idFile;
        float value;
    };
    vector<vector<DComponent> > _dVectors;

    /**
     * Traverses the tree moving from the root to the leaves looking for the closest visual word in each step
     * @param descriptor input descriptor
     * @return the index of the leaf found
     */
    int findLeaf(Mat &descriptor);

    /**
     * Traverses the tree moving from the root to the leaves looking for the closest visual word in each step
     * @param descriptor input descriptor
     * @return the path from the root to the leaf found, stored as a list of nodes indexes
     */
    list<int> findPath(Mat &queryDescr);


    /**
     * Stores vocabulary tree internal representation data information to disk
     * @param fileName name where to store data
     */
    void storeInfo(string &fileName);

    /**
     * Loads vocabulary tree internal representation data information from disk
     * @param fileName name where to read data
     */
    void loadInfo(string &fileName);

    /**
     * Stores vocabulary tree nodes data to disk.
     * Nodes data (nodes indices, leaves indices and centers) are in three different output files
     * <prefix>+".index", <prefix>+".leaves", <prefix>+".centers" respectively.
     * @param prefix naming the output files
     */
    void storeNodes(string &prefix);

    /**
     * Loads vocabulary tree nodes data from disk.
     * Nodes data (nodes indices, leaves indices and centers) are in three different output files
     * <prefix>+".index", <prefix>+".leaves", <prefix>+".centers" respectively.
     * @param prefix naming the input files
     */
    void loadNodes(string &prefix);


    /**
     * Stores nodes weights data to disk
     * @param fileName output file name
     */
    void storeWeights(string &fileName);

    /**
     * Load nodes weights data from disk
     * @param fileName input file name
     */
    void loadWeights(string &fileName);

    /**
     * Stores inverted indices data to disk
     * @param fileName output file name
     */
    void storeInvIdx(string &fileName);

    /**
     * Load inverted indices data to disk
     * @param fileName output file name
     */
    void loadInvIdx(string &fileName);

    /**
     * Stores d-vectors data to disk
     * @param fileName output file name
     */
    void storeVectors(string &fileName);

    /**
     * Loads d-vectors data from disk
     * @param fileName output file name
     */
    void loadVectors(string &fileName);


    /**
     * Given an input file (containing a Mat with descriptors), it builds the node idNode, on the level level
     * @param idNode id node to be create (for example 0 is the root node)
     * @param level  level of the node that is being created
     * @param descriptorsFile name of the input file containing the descriptors to be used to build the node
     * @param rows number of descriptors to use
     */
    void buildNodeFromFile(int idNode,
                           int level,
                           string &descriptorsFile,
                           int rows
    );


    /**
     * Given an input Mat with descriptors, it builds the node idNode, on the level level
     * @param idNode  id node to be create (for example 0 is the root node)
     * @param level level of the node that is being created
     * @param descriptors input file containing the descriptors to be used to build the node
     */
    void buildNodeFromMat(int idNode,
                          int level,
                          Mat &descriptors
    );

    /**
     * Auxiliar generalization function used to build nodes from descriptors
     * @param idNode  id node to be create (for example 0 is the root node)
     * @param level level of the node that is being created
     * @param fromFile true if a file is used
     * @param pFileName name of the input file containing the descriptors (only used if fromFile is true)
     * @param pDescs pointer to Mat with descriptors (only used if fromFile is false)
     * @param rows number of descriptors to use
     */
    void buildNodeGen(int idNode,
                      int level,
                      bool fromFile,
                      string *pFileName,
                      Mat *pDescs,
                      int rows);

    /**
     * Given a file where descriptors are stored, performs K-clustering
     * distributing the input elements into K output files,
     * Returns a matrix with the K centers, and the name of the K files where
     * the descriptors where distributed.
     * @param K number of clusters
     * @param inputFile file where descriptor to be clustered are stored
     * @param centers the K output centers
     * @param clusters vector containing the name of the files with the elements of the output clusters
     */
    void cluster(
            int K,
            string &inputFile,
            Mat &centers,
            vector<string> &clusters
    );


    /**
     * Given a matrix with descriptors, performs K-clustering
     * Returns a matrix with the K centers, a vector containing K matrices
     * where the descriptors have been distributed.
     * @param K number of clusters
     * @param descs matrix with input descriptors
     * @param centers matrix containing the K output centers
     * @param clusters vector with K matrices with the elements of the output clusters
     */
    void cluster(
            int K,
            Mat &descs,
            Mat &centers,
            vector<Mat> &clusters
    );


    /**
     * Given an input file containing descriptores, it creates the node idNode, at level level for that descriptors.
     * @param idNode id of the node that is being created
     * @param level level of the tree node
     * @param descrFile input file name with the descriptors
     */
    void createNode(int idNode, int level, string &descrFile);

    /**
     * Enlarges a given Mat, if it has less than rows rows, preserving its contents
     * @param mat the Mat to be enlarged
     * @param rows the resulting number of rows that Mat will have
     */
    void expand(Mat &mat, int rows);

    /**
     * Indexes image elements into the tree starting from startImage within the catalog
     * @param catalog the input catalog with the elements to be indexed
     * @param startImage the starting position for indexing
     */
    void addElements(Catalog<DBElem> &catalog, int startImage);

    /**
     * Computes d-vectors
     */
    void computeVectors();

    /**
     * Computes inverted index for the node idNode
     * @param idNode id of the node where to compute the inverted index
     * @param level level of the node where to compute the inverted index
     * @param outInvIdx resulting inverted index
     */
    void computeInvertedIndex(int idNode, int level, vector<IIFEntry> &outInvIdx);

    /**
     * builds all the nodes of the Vocabulary Tree
    */
    void buildNodes();

    /**
     * Given the catalog of indexed images, and a specified position within the catalog (startImage),
     * it accumulates the number descriptors for all the images indexed from 0 to the specified position.
     * This function is used for adding new files to the vocabulary
     * @param catalog catalog of indexed images
     * @param startImage specified position
     * @return the total accumulated number of descriptors indexed from 0 to startImage
     */
    int getStartingFeatureRow(Catalog<DBElem> &catalog, int startImage);

};

#endif /* VOCTREE_H_ */
