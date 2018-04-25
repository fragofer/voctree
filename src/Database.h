//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.


#ifndef DATABASE_H_
#define DATABASE_H_

#include <cv.h>
#include <string.h>

#include "Catalog.h"
#include "FileHelper.h"
#include "FeatureMethod.h"
#include "Matching.h"
#include "VocTree.h"
#include "MatPersistor.h"

using namespace cv;


/**
 * This class represents a dataset of images (or videos) that can be trained and used to perform queries.
 * It allows to register and index files, statistics, etc.
 * It contains a reference to a vocabulary tree, and it will wrap its functionality
 *
 * DATABASE DIRECTORY STRUCTURE:
 * ****************************
 * Given a database root path <root> its directory structure is:
 *
 * <root>/vocabulary: the path where files used to build the vocabulary are placed (required)
 * <root>/input: the path where files to be indexed are placed (required)
 * <root>/data: the path where internal database data files are stored
 * <root>/queries: a the path where to place files to be queried (might be empty)
 * <root>/results: the path where the database will write output results
 *
 * if these directories does not exist, building the database will create them,
 * but required directories must be filled with files to be processed.
 *
 */
class Database {


public:


    /**
 * builds a database
     * before bulding a database make sure that directory structure is created. see directory structure above.
 * @param path path where database is stored on disk
 * @param fm feature method, method to detect and compute descriptors
 * @param reuseFeatures if true, input features wont be computed
 * @param k branch factor for the vocabulary tree
 * @param h maximum height for the vocabulary tree
 * @param maxFiles maximum number files to index, if 0 then all files will be processed
 * @param maxFilesVocabulary maximum number vocabulary files to process, if 0 then all files will be processed
 * @param reuseVocabulary if true, vocabulary features wont be computed
 * @param pca_dim number of dimensions to reduce features using PCA if 0 then disabled.
 * @return a pointer to the resulting database
 */
    static Ptr<Database> build(
            string &path, FeatureMethod &fm, bool reuseFeatures, int k, int h, int maxFiles, int maxFilesVocabulary,
            bool reuseVocabulary, int pca_dim
            //, int maxTrainingFiles
    );


    /**
     * loads a database from disk
     * @param path path where database is stored on disk
     * @return a pointer to the database
     */
    static Ptr<Database> load(string &path);

    /**
     * loads a database from disk and updates it
     * (looks for new files on the input directory, if it finds new files those files will be added to the database)
     * @param path path where database is stored on disk
     * @return a pointer to the database
     */
    static Ptr<Database> update(string &path);


    /**
     * Database destructor
     */
    virtual ~Database();

    /**
     * @return the path to the database
     */
    string getPath();

    /**
     * @return number of indexed images
     */
    int imagesCount();

    /**
     * returns the specified image
     * @param idImage id of the image to be returned
     * @return a Mat with the idImage'th indexed image
     */
    Mat getImage(int idImage);

    /**
     * returns image info for the idImage
     * @param idImage the id of the image
     * @return DBElem contains the name of the file and the number of features it has
     */
    DBElem getFileInfo(int idImage);

    /**
     * Performs a query for an indexed file
     * It wraps the functionality of the vocabulary tree query
     * @param idFile the id of the indexed file
     * @param result a vector containing the scoring results
     * @param limit maximum number of results
     */
    void query(int idFile,
               vector<Matching> &result,
               int limit);

    /**
     * Performs a query for a given file
     * It wraps the functionality of the vocabulary tree query
     * @param fileName file to be queried
     * @param result a vector containing the scoring results
     * @param limit maximum number of results
     */
    void query(string &fileName,
               vector<Matching> &result,
               int limit);

    /**
     * Performs a query for a given file
     * It wraps the functionality of the vocabulary tree query
     * @param fileName file to be queried
     * @param result a vector containing the scoring results
     * @param limit maximum number of results
     * @param outImage a Mat with the image queried
     * @param qKeypoints the keypoints detected for the query image
     * @param qDescriptors the descriptors computed for the query image
     */
    void query(string &fileName,
               vector<Matching> &result,
               int limit,
               Mat &outImage,
               vector<KeyPoint> &qKeypoints,
               Mat &qDescriptors);


    /**
     * This structure is used for exporting purposes. see exportResults method
     */
    struct ExportInfo {
        int idElem;
        string fileName;
    };

    /**
     * Given a vector with scoring results it writes the resulting files to the results directory
     * and returns a vector with the export info. For example, if a scoring result is from a video, it will
     * open the video, position on the resulting frame, and write that frame as an image to the results directory.
     * @param result a vector with scoring results
     * @return a vector with the exporting results
     */
    vector<ExportInfo> exportResults(vector<Matching> &result);

    /**
     * It loads the specified image,
     * computes features for that image,
     * draws keypoints detected,
     * and writes the image to the results directory adding the "keyp_" prefix.
     * @param fileName the specified image to be processed. Image must be indexed.
     */
    void exportFeaturesImage(string fileName);

    /**
     * Given an image
     * it computes features for that image,
     * draws keypoints detected,
     * and writes the image to the results directory as results/"keyp_" + <fileName>.
     * @param fileName the file name suffix to be written
     * @param imgIn the input image on which decriptors will be computed
     * @param keypoints the output keypoints
     */
    void exportFeaturesImage(string fileName, Mat &imgIn, vector<KeyPoint> &keypoints);

    /**
     * sets if the database must export files from results to the results directory
     * @param value if true it will export files
     */
    void setExports(bool value) {
        _exports = value;
    }

    /**
     * @return true if the database must export files from results to the results directory
     */
    bool getExports() {
        return _exports;
    }

    /**
     * @return the indexed files catalog
     */
    const Catalog<DBElem> &getCatalog() {
        return _catalog;
    }

private:

    bool _exports;
    bool _segmentVideo;
    long _totalFeatures;
    int _totalDBelems;

    int _maxFiles;
    int _maxFilesVocabulary;
    bool _reuseVocabulary;
    int _kmeansAttempts;
    TermCriteria _term;

    // BoFs Catalog
    Catalog<DBElem> _catalog;
    Catalog<VideoInfo> _videos;

    // Vocabulary Catalog
    Catalog<DBElem> _vocCatalog;
    Catalog<VideoInfo> _vocVideos;
    long _totalVocFeatures;
    int _totalVocDBelems;


    FeatureMethod _fm;
    string _path;

    vector<KeyPoint> _keypoints;
    Mat _descriptors;

    Ptr<VocTree> _vt;

    bool endsWith(string str, string suffix);

    bool isPicture(string fileName);

    bool isVideo(string fileName);

    void store(bool reuseFeatures);


    // maxFiles: maximum number of files to process (for features generation)
    // maxTrainingFiles: maximum number of files to include in vocabulary
    Database(string &path, FeatureMethod &fm, bool reuseFeatures, int k, int h, int maxFiles, int maxFilesVocabulary,
             bool reuseVocabulary, int pca_dim
            //,int kmeansAttempts
            //,TermCriteria & term
    );

    Database(string path, bool update);

    void buildtree(int k, int h, int useNorm);

    void processInput(bool reuseFeatures, bool forVocabulary);

    void processFiles(bool update, bool forVocabulary);

    void setSegmentVideo(bool mode);

    void processVideo(FileHelper::Entry &ent, bool forVocabulary);

    void processPicture(FileHelper::Entry &ent, bool forVocabulary);

    bool hasElement(string fileName, bool inVocabulary);

    bool extractFeatures(Mat &img,
                         vector<KeyPoint> &outKeypoints,
                         Mat &outDescriptors);

    void
    registerElem(string &name,
                 vector<KeyPoint> &kps,
                 Mat &descs,
                 bool forVocabulary);

    void checkFlushFeatures(bool forVocabulary);

    void flushFeatures(bool forVocabulary);

    Mat readResource(string &fileName);

    static const int TYPE_PICTURE = 0;
    static const int TYPE_VIDEO = 1;
    struct ResourceInfo {
        int id;
        int type;
        string fileName;
        int idVideo;
        int frameNumber;
    };

    ResourceInfo
    getResourceInfo(int idElem);

    ResourceInfo
    getResourceInfo(string fileName);

    Ptr<MatPersistor> _pMpDescs;

    bool _usePCA;
    int _pca_dim;
    Ptr<PCA> _pca;

    void storePCA();

    void loadPCA();

    void storeDBConfig();

    void loadDBConfig();

};

#endif /* DATABASE_H_ */
