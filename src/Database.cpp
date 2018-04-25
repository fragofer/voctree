//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "Database.h"

#include "KeyPointPersistor.h"
#include "ShootSegmenter.h"


using namespace cv;
using namespace std;


Ptr<Database>
Database::build(
        string &path,
        FeatureMethod &fm,
        bool reuseFeatures,
        int k,
        int h, int maxFiles, int maxFilesVocabulary, bool reuseVocabulary, int pca_dim
) {

    Ptr<Database> ret = new Database(path, fm, reuseFeatures, k, h, maxFiles, maxFilesVocabulary, reuseVocabulary,
                                     pca_dim
    );
    return ret;
}


Ptr<Database>
Database::load(string &path) {
    Ptr<Database> ret = new Database(path, false);
    return ret;
}

Ptr<Database>
Database::update(string &path) {
    Ptr<Database> ret = new Database(path, true);
    return ret;
}


void
checkSize(Mat &img) {

    int MAX_DIM = 640;
    float aspect = (float) img.rows / img.cols;
    Size newSize;

    if (aspect > 1) {

        // portrait image
        if (img.rows < MAX_DIM) {
            return;
        }
        int newWidth = (int) round(MAX_DIM / aspect);
        newSize = Size(newWidth, MAX_DIM);
    } else {
        // landscape image
        if (img.cols < MAX_DIM) {
            return;
        }
        int newHeight = (int) round(MAX_DIM * aspect);
        newSize = Size(MAX_DIM, newHeight);
    }
    resize(img, img, newSize, 0, 0, INTER_LINEAR);

}


bool
Database::extractFeatures(Mat &img,
                          vector<KeyPoint> &outKeypoints,
                          Mat &outDescriptors) {

    unsigned int MIN_FEATURES = 4;

    checkSize(img);

    FeatureMethod fm = _fm;

    fm.detectAndCompute(img, outKeypoints, outDescriptors);

    if (outKeypoints.size() < MIN_FEATURES) {
        // not enough features.
        return false;
    }

    return true;
}

void
Database::flushFeatures(bool forVocabulary) {

    FileManager fm(_path);
    string fileKeypoints;
    string fileDescriptors;

    if (forVocabulary) {

        fileKeypoints = fm.file(FileManager::VOCABULARY_KEYPOINTS);
        fileDescriptors = fm.file(FileManager::VOCABULARY_DESCRIPTORS);

    } else {

        fileKeypoints = fm.file(FileManager::KEYPOINTS);
        fileDescriptors = fm.file(FileManager::DESCRIPTORS);

    }

    cout << endl << "flushing features to disk..." << endl;
    if (_descriptors.rows == 0) {
        return;
    }

    KeyPointPersistor kpp;
    kpp.append(fileKeypoints, _keypoints);

    Mat descriptorsToWrite;
    if (!forVocabulary && _usePCA) {
        _pca->project(_descriptors, descriptorsToWrite);
    } else {
        descriptorsToWrite = _descriptors;
    }


    MatPersistor mp(fileDescriptors);
    if (!mp.exists()) {
        mp.create(descriptorsToWrite);
    } else {
        mp.openWrite();
        mp.append(descriptorsToWrite);
        mp.close();
    }
    //mp.append( file_descs, _descriptors );

    _keypoints.clear();
    _descriptors.create(0, _descriptors.cols, _descriptors.type());

}

void
Database::checkFlushFeatures(bool forVocabulary) {

    int maxDescriptors = 128 * 1024;
    if (_descriptors.rows < maxDescriptors) {
        return;
    }

    flushFeatures(forVocabulary);

}


void
Database::registerElem(string &name, vector<KeyPoint> &kps, Mat &descs, bool forVocabulary) {
//Database::registerElem( FileHelper::Entry & ent, vector<KeyPoint> & kps, Mat & descs ) {

    cout << " feat:"
         << descs.rows
         << " " << descs.cols
         << flush;

    int featuresCount = kps.size();
    DBElem info;
    info.name = name;
    info.featuresCount = featuresCount;

    if (forVocabulary) {

        _vocCatalog.add(info);
        _totalVocFeatures += descs.rows;
        _totalVocDBelems++;

    } else {

        _catalog.add(info);
        _totalFeatures += descs.rows;
        _totalDBelems++;

    }

    _keypoints.insert(_keypoints.end(), kps.begin(), kps.end());
    _descriptors.push_back(descs);

    checkFlushFeatures(forVocabulary);

}


void
Database::processPicture(FileHelper::Entry &ent, bool forVocabulary) {

    string fileName = ent.fullName();
    Mat img = imread(fileName.c_str());
    if (!img.data) {
        cout << " warning! cannot process file." << endl;
        return;
    }

    vector<KeyPoint> keypoints;
    Mat descriptors;

    if (!extractFeatures(img, keypoints, descriptors)) {

        cout << " picture skipped";

    } else {

        //string name = ent.fileName;
        //registerElem( name, keypoints, descriptors );
        string relName = ent.relName();
        registerElem(relName, keypoints, descriptors, forVocabulary);


    }

    return;

}


bool
Database::endsWith(string str, string suffix) {
    return (str.substr(str.length() - suffix.size()) == suffix);
}

bool
Database::isPicture(string fileName) {
    if (endsWith(fileName, ".png")) return true;
    if (endsWith(fileName, ".jpg")) return true;
    return false;
}

bool
Database::isVideo(string fileName) {
    if (endsWith(fileName, ".avi")) return true;
    if (endsWith(fileName, ".mp4")) return true;
    if (endsWith(fileName, ".mkv")) return true;
    if (endsWith(fileName, ".divx")) return true;
    if (endsWith(fileName, ".mov")) return true;
    return false;
}

void
Database::setSegmentVideo(bool mode) {
    _segmentVideo = mode;
    return;
}

void
Database::processVideo(FileHelper::Entry &ent, bool forVocabulary) {

    //bool segment = true;
    bool segment = _segmentVideo;
    ShootSegmenter sm;

    string fileName = ent.fullName();

    cout << " processing video..." << endl << flush;
    VideoCapture vc(fileName.c_str());

    if (!vc.isOpened()) {
        cout << " warning! cannot process video " << ent.fileName << "." << endl;
        return;
    }

    // Register this video in the video catalog.
    VideoInfo vi;
    int idVideo = _videos.size();
    //vi.fileName = ent.fileName;
    vi.fileName = ent.relName();
    _videos.add(vi);

    vector<KeyPoint> keypoints;
    Mat descriptors;

    Mat frame;
    int numFrames = 0;
    while (1) {

        vc >> frame;
        if (frame.empty()) {
            break;
        }


        cout << "\t"
             << ent.fileName
             << " frame:" << numFrames << flush;

        if (segment && !sm.chooseThisFrame(frame)) {

            //cout << " frame skipped. (by segmenter)" << flush;

        } else if (!extractFeatures(frame, keypoints, descriptors)) {

            cout << " frame skipped. (too few features)" << flush;

        } else {

            stringstream ss;
            ss << idVideo << "#" << numFrames;
            string imgName = ss.str();

            registerElem(imgName, keypoints, descriptors, forVocabulary);


        }
        numFrames++;
        cout << endl;

    }

}

bool
Database::hasElement(string fileName, bool inVocabulary) {


    if (isPicture(fileName)) {

        Catalog<DBElem> ctlg;
        if (inVocabulary) {
            ctlg = _vocCatalog;
        } else {
            ctlg = _catalog;
        }

        for (int i = 0; i < ctlg.size(); i++) {
            DBElem elem = ctlg.get(i);
            if (elem.name.compare(fileName) == 0) {
                return true;
            }
        }

    } else if (isVideo(fileName)) {

        Catalog<VideoInfo> ctlg;
        if (inVocabulary) {
            ctlg = _vocVideos;
        } else {
            ctlg = _videos;
        }

        for (int i = 0; i < ctlg.size(); i++) {
            VideoInfo elem = ctlg.get(i);
            if (elem.fileName.compare(fileName) == 0) {
                return true;
            }
        }

    }

    return false;
}

void
Database::processFiles(bool update, bool forVocabulary) {

    FileManager fm(_path);

    vector<FileHelper::Entry> dir;
    string path;
    if (forVocabulary) {
        path = fm.vocabularyDir();
    } else {
        path = fm.inputDir();
    }

    FileHelper::listDir(path, dir, true);

    for (unsigned int i = 0; i < dir.size(); i++) {
        FileHelper::Entry ent = dir.at(i);

        if (ent.type != FileHelper::TYPE_FILE) {
            continue;
        }

        if (forVocabulary && _maxFilesVocabulary > 0 && i > (unsigned int) _maxFilesVocabulary) {
            break;
        }
        //if (_maxFiles > 0 && i >= (unsigned int) _maxFiles) {
        //	break;
        //}

        string fileName = ent.fullName();
        string relName = ent.relName();

        cout << "file " << i << "/" << dir.size() << " " << ent.fileName << flush;

        //if ( update && hasElement( ent.fileName ) ) {
        if (update && hasElement(relName, forVocabulary)) {

            cout << " already in catalog.";

        } else if (isPicture(fileName)) {

            processPicture(ent, forVocabulary);

        } else if (isVideo(fileName)) {

            processVideo(ent, forVocabulary);

        }

        cout << endl << flush;


    }

    flushFeatures(forVocabulary);


}


int
Database::imagesCount() {
    return _catalog.size();
}


Mat
Database::getImage(int idImage) {

    DBElem info = _catalog.get(idImage);

    string path = _path;
    path += "/input";

    string fileName = path + "/" + info.name;
    cout << fileName << endl;

    Mat img = imread(fileName);
    checkSize(img);

    return img;

}

void checkDir(string dirName) {

    if (!FileHelper::exists(dirName)) {
        cout << "creating directory: " << dirName << endl;
        FileHelper::createDir(dirName);
    }

}

void checkDirs(FileManager &fileMgr) {

    checkDir(fileMgr.inputDir());
    checkDir(fileMgr.dataDir());
    checkDir(fileMgr.queryDir());
    checkDir(fileMgr.resultDir());
    checkDir(fileMgr.vocabularyDir());

}


void Database::processInput(bool reuseFeatures, bool forVocabulary) {

    FileManager fileMgr(_path);

    string fileCatalog;
    string fileVideos;
    string fileKeyps;
    string fileDescs;

    string fileMethod = fileMgr.file(FileManager::FEAT_METHOD);

    Catalog<DBElem> *ctlg;
    Catalog<VideoInfo> *ctlgVid;

    if (forVocabulary) {

        fileCatalog = fileMgr.file(FileManager::VOCABULARY_CATALOG);
        fileVideos = fileMgr.file(FileManager::VOCABULARY_CATALOG_VIDEO);
        fileKeyps = fileMgr.file(FileManager::VOCABULARY_KEYPOINTS);
        fileDescs = fileMgr.file(FileManager::VOCABULARY_DESCRIPTORS);

        ctlg = &_vocCatalog;
        ctlgVid = &_vocVideos;
    } else {

        fileCatalog = fileMgr.file(FileManager::CATALOG);
        fileVideos = fileMgr.file(FileManager::CATALOG_VIDEO);
        fileKeyps = fileMgr.file(FileManager::KEYPOINTS);
        fileDescs = fileMgr.file(FileManager::DESCRIPTORS);

        ctlg = &_catalog;
        ctlgVid = &_videos;

    }

    bool canReuseFeatures = reuseFeatures &&
                            FileHelper::exists(fileMethod) &&
                            FileHelper::exists(fileCatalog) &&
                            FileHelper::exists(fileVideos) &&
                            FileHelper::exists(fileKeyps) &&
                            FileHelper::exists(fileDescs);

    if (canReuseFeatures) {

        // loads catalog
        cout << "loading catalog..." << endl;
        ctlg->load(fileCatalog);

        // loads video catalog
        cout << "loading video catalog..." << endl;
        ctlgVid->load(fileVideos);

        // checks that features files exist
        if (!FileHelper::exists(fileKeyps)) {
            cerr << "cannot find keypoint file." << endl;
            exit(-1);
        }

        if (!FileHelper::exists(fileDescs)) {
            cerr << "cannot find descriptors file." << endl;
            exit(-1);
        }

        // checks that file method exist
        if (!FileHelper::exists(fileMethod)) {
            cerr << "cannot find method file." << endl;
            exit(-1);
        }

        cout << "re-using features..." << endl;
        MatPersistor mp(fileDescs);
        mp.openRead();
        _totalFeatures = mp.rows();
        mp.close();

    } else {

        cout << "processing files: " << endl;

        FileHelper::deleteFile(fileCatalog);
        FileHelper::deleteFile(fileVideos);
        FileHelper::deleteFile(fileKeyps);
        FileHelper::deleteFile(fileDescs);

        processFiles(false, forVocabulary);

        cout << "storing catalog..." << endl;
        ctlg->store(fileCatalog);

        cout << "storing video catalog..." << endl;
        ctlgVid->store(fileVideos);

        cout << "storing method..." << endl;
        _fm.store(fileMethod);

    }


}


void
Database::storePCA() {

    FileManager fm(_path);

    string fileName = fm.file(FileManager::PCA_MODEL);

    FileStorage fs(fileName, FileStorage::WRITE);
    fs << "mean" << _pca->mean;
    fs << "e_vectors" << _pca->eigenvectors;
    fs << "e_values" << _pca->eigenvalues;
    fs.release();

}

void
Database::loadPCA() {

    FileManager fm(_path);
    string fileName = fm.file(FileManager::PCA_MODEL);

    FileStorage fs(fileName, FileStorage::READ);
    Mat mean;
    Mat eigenvectors;
    Mat eigenvalues;
    fs["mean"] >> mean;
    fs["e_vectors"] >> eigenvectors;
    fs["e_values"] >> eigenvalues;
    fs.release();
    _pca = new PCA();
    _pca->mean = mean;
    _pca->eigenvectors = eigenvectors;
    _pca->eigenvalues = eigenvalues;
    //_pca->Flags = PCA::DATA_AS_ROW;

}

Database::Database(
        string &path,
        FeatureMethod &fm,
        bool reuseFeatures,
        int k,
        int h, int maxFiles, int maxFilesVocabulary, bool reuseVocabulary, int pca_dim
        //, int maxTrainingFiles
        //,int kmeansAttempts
        //,TermCriteria & crit
) {


    _path = path;
    _fm = fm;

    _totalVocFeatures = 0;
    _totalVocDBelems = 0;
    _totalFeatures = 0;
    _totalDBelems = 0;
    //_segmentVideo = false;
    _segmentVideo = true;
    _maxFiles = maxFiles;
    _reuseVocabulary = reuseVocabulary;
    _maxFilesVocabulary = maxFilesVocabulary;

    //_kmeansAttempts = kmeansAttempts;
    //_pMpDescs = NULL;

    FileManager fileMgr(_path);
    checkDirs(fileMgr);

    _usePCA = pca_dim > 0;
    _pca_dim = pca_dim;
    storeDBConfig();


    if (!reuseVocabulary) {
        string fileVocabulary = fileMgr.file(FileManager::VOCABULARY_CATALOG);
        FileHelper::deleteFile(fileVocabulary);
    }

    if (!reuseFeatures) {
        string fileMethod = fileMgr.file(FileManager::FEAT_METHOD);
        FileHelper::deleteFile(fileMethod);
    }

    //string fileKeyps = fileMgr.file( FileManager::VOCABULARY_KEYPOINTS );
    //string fileDescs = fileMgr.file( FileManager::VOCABULARY_DESCRIPTORS );

    cout << "processing input for vocabulary..." << endl;
    processInput(reuseFeatures, true);


    if (_usePCA) {
        cout << "Using PCA model for dimensionality reduction..." << endl;
        string fileDescriptors = fileMgr.file(FileManager::VOCABULARY_DESCRIPTORS);
        MatPersistor mp(fileDescriptors);
        mp.openRead();
        Mat vocDescriptors;
        cout << "reading vocabulary descriptors..." << endl;
        mp.read(vocDescriptors);
        mp.close();
        cout << "training PCA model..." << endl;
        _pca = new PCA(vocDescriptors, noArray(), PCA::DATA_AS_ROW, _pca_dim);
        _pca->project(vocDescriptors, vocDescriptors);
        mp.create(vocDescriptors);

        storePCA();
    }


    cout << "processing input for BoFs..." << endl;
    processInput(reuseFeatures, false);

    cout << "total features: " << _totalFeatures << endl;

    cout << "building voctree: " << endl;
    //Ptr<Feature2D> pDM = fm.getDescriptorExtractor();
    //int useNorm = pDM->defaultNorm();
    int useNorm = fm.getDefaultNorm();
    buildtree(k, h, useNorm);


}

void
Database::storeDBConfig() {

    FileManager fileMgr(_path);
    string fileConfig = fileMgr.file(FileManager::DB_CONFIG);
    FileStorage fs(fileConfig, FileStorage::WRITE);
    fs << "usePCA" << _usePCA;
    fs << "pcaDIM" << _pca_dim;
    fs.release();

}

void
Database::loadDBConfig() {

    FileManager fm(_path);
    string fileName = fm.file(FileManager::DB_CONFIG);

    FileStorage fs(fileName, FileStorage::READ);
    fs["usePCA"] >> _usePCA;
    fs["pcaDIM"] >> _pca_dim;
    fs.release();

}


Database::Database(string path, bool update) {

    _path = path;

    _totalFeatures = 0;
    _segmentVideo = false;

    FileManager fileMgr(_path);
    checkDirs(fileMgr);

    string fileCatalog = fileMgr.file(FileManager::CATALOG);
    string fileVideos = fileMgr.file(FileManager::CATALOG_VIDEO);
    string fileMethod = fileMgr.file(FileManager::FEAT_METHOD);
    string fileKeyps = fileMgr.file(FileManager::KEYPOINTS);
    string fileDescs = fileMgr.file(FileManager::DESCRIPTORS);

    cout << "loading db config..." << endl;
    loadDBConfig();

    // load feature method
    cout << "loading feature method..." << endl;
    _fm.load(fileMethod);
    cout << "using method "
         << _fm.getDetectorKey() << "/"
         << _fm.getExtractorKey() << endl;

    // load catalog
    cout << "loading catalog..." << endl;
    _catalog.load(fileCatalog);
    _totalDBelems = _catalog.size();

    // load video catalog
    cout << "loading video catalog..." << endl;
    _videos.load(fileVideos);

    // load keypoints
    cout << "loading keypoints..." << endl;
    KeyPointPersistor kpp;

    // load descriptors
    cout << "loading descriptors..." << endl;

    // load voctree
    cout << "loading voctree..." << endl;
    _vt = new VocTree(_path);

    if (_usePCA) {
        cout << "loading PCA model..." << endl;
        loadPCA();
    }


    if (update) {

        cout << "updating database..." << endl;
        processFiles(true, false);
        cout << "storing catalog..." << endl;
        _catalog.store(fileCatalog);
        cout << "storing video catalog..." << endl;
        _videos.store(fileVideos);

        _vt->update(_catalog);
    }


}

Database::~Database() {

}


DBElem
Database::getFileInfo(int idImage) {
    return _catalog.get(idImage);
}


void
Database::buildtree(int k, int h, int useNorm) {


    if (_maxFiles > 0) {
        _catalog.shrink(_maxFiles);
    }
    _vt = new VocTree(k, h, _catalog, _path, _reuseVocabulary, useNorm);


}


void splitPathFile(string fileName, string &path, string &file) {
    int pos = fileName.find_last_of("/");
    path = fileName.substr(0, pos + 1);
    file = fileName.substr(pos + 1);
}

vector<Database::ExportInfo>
Database::exportResults(vector<Matching> &result) {

    vector<ExportInfo> ret;

    FileManager fileMgr(_path);

    for (unsigned int i = 0; i < result.size(); i++) {

        Matching m = result.at(i);

        int idImage = m.id;
        DBElem info = _catalog.get(idImage);

        string resName = fileMgr.inputDir() + "/" + info.name;
        ResourceInfo res = getResourceInfo(resName);

        string path, file;
        splitPathFile(res.fileName, path, file);
        string outName = fileMgr.resultDir() + "/" + file;
        if (isVideo(outName)) {
            stringstream ss;
            ss << "#" << res.frameNumber << ".jpg";
            outName += ss.str();
        }
        ExportInfo ei;
        ei.idElem = idImage;
        ei.fileName = outName.substr(_path.size());
        ret.push_back(ei);

        string outKeypName = fileMgr.resultDir() + "/keyp_" + ei.fileName;

        cout << "exporting result: " << outName << endl;

        if (FileHelper::exists(outName)) {

            // file was already exported.

        } else {

            if (res.type == TYPE_PICTURE) {

                FileHelper::copy(resName, outName);

            } else {

                // this result is within a video
                Mat img = readResource(resName);
                if (!img.data) {
                    cerr << resName << " can not be read" << endl;
                } else {

                    cout << "writing output" << outName << endl;
                    if (!imwrite(outName, img)) {
                        cerr << resName << " can not be exported" << endl;
                    }

                }

                // TODO: to avoid overhead when exporting would be better to:
                // - group results by video id.
                // - for each different video, sort results by frame id
                // - export frames in order

            }

        }

        exportFeaturesImage(outName);

    }

    return ret;

}


Mat
Database::readResource(string &fileName) {

    ResourceInfo info = getResourceInfo(fileName);
    Mat img;
    if (info.type == TYPE_VIDEO) {

        string file = info.fileName;
        //  open video, and extract the requested frame
        VideoCapture vc(file.c_str());
        if (!vc.isOpened()) {
            cout << " warning! cannot process video " << file << "." << endl;
            return img;
        }

        vc.set(CV_CAP_PROP_POS_FRAMES, info.frameNumber);
        if (!vc.read(img)) {
            cerr << "cannot read frame" << endl;
            return img;
        }

    } else {
        // its an image.
        img = imread(fileName.c_str());
    }

    return img;

}


Database::ResourceInfo
Database::getResourceInfo(int idElem) {

    DBElem info = _catalog.get(idElem);
    ResourceInfo ret = getResourceInfo(info.name);
    ret.id = idElem;
    return ret;

}

Database::ResourceInfo
Database::getResourceInfo(string fileName) {

    ResourceInfo ret;
    int pos = fileName.find('#');
    if (pos > 0) {

        ret.type = TYPE_VIDEO;

        // it's from a video

        // split string in fileName and frame
        // <video file name or id>#<frame number>
        int frame = atoi(fileName.substr(pos + 1).c_str());
        ret.frameNumber = frame;

        fileName = fileName.substr(0, pos);
        if (!isVideo(fileName)) {
            // it's the id of the file.
            string file, path;
            splitPathFile(fileName, path, file);
            int idVideo = atoi(file.c_str());
            fileName = path + _videos.get(idVideo).fileName;
            ret.idVideo = idVideo;

        }
        ret.fileName = fileName;

    } else {

        ret.type = TYPE_PICTURE;
        ret.fileName = fileName;

    }

    return ret;

}

void
Database::exportFeaturesImage(string fileName) {

    string path, file;
    splitPathFile(fileName, path, file);
    FileManager fileMgr(_path);
    string outName = fileMgr.resultDir() + "/keyp_" + file;
    if (FileHelper::exists(outName)) {
        // file was already exported.
        return;
    }

    Mat img = imread(fileName.c_str());
    if (!img.data) {
        return;
    }

    vector<KeyPoint> kps;
    Mat desc;
    // It is possible to read features from preprocessed file.
    if (!extractFeatures(img, kps, desc)) {
        return;
    }

    Mat imgOut;
    //drawKeypoints( imgIn, keypoints, imgOut, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
    drawKeypoints(img, kps, imgOut, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    imwrite(outName, imgOut);

}


void
Database::exportFeaturesImage(string fileName, Mat &imgIn, vector<KeyPoint> &keypoints) {

    string path, file;
    splitPathFile(fileName, path, file);
    FileManager fileMgr(_path);
    //string outName = fileMgr.resultDir() + "/keyp_" + file;
    string outName = path + "/keyp_" + file;
    if (!endsWith(outName, ".jpg")) {
        outName += ".jpg";
    }
    if (FileHelper::exists(outName)) {
        // file was already exported.
        return;
    }

    Mat imgOut;
    //drawKeypoints( imgIn, keypoints, imgOut, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
    drawKeypoints(imgIn, keypoints, imgOut, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);


    imwrite(outName, imgOut);

}


void
Database::query(int idFile, vector<Matching> &result, int limit) {

    Mat qDescriptors;

    DBElem fileInfo = _catalog.get(idFile);


    if (_pMpDescs == NULL) {
        FileManager fm(_path);
        string fileDescriptors = fm.file(FileManager::DESCRIPTORS);
        //MatPersistor mp( fileDescriptors );
        _pMpDescs = new MatPersistor(fileDescriptors);
        assert(_pMpDescs->exists());
        _pMpDescs->openRead();
    }

    //_pMpDescs->setRow(1234);
    _pMpDescs->read(qDescriptors, fileInfo.featuresCount);

    //cout << "db:running query..." << endl;
    _vt->query(qDescriptors, result, limit);

//	 // Re-rank
//	bool _re_rank = false;
//	if (_re_rank) {
//		FileManager fm(_path);
//		string fileName = fm.inputDir() + "/" + fileInfo.name;
//		Mat img = readResource( fileName );
//		vector<KeyPoint> kps;
//		re_rank( img, kps, qDescriptors, result );
//		//re_rank( img, qKeypoints, qDescriptors, result );
//		sort(result.begin(), result.end());
//	}



}

void
Database::query(string &fileName, vector<Matching> &result, int limit) {

    Mat img;
    vector<KeyPoint> qKeypoints;
    Mat qDescriptors;
    query(fileName, result, limit, img, qKeypoints, qDescriptors);

}


void
Database::query(string &fileName,
                vector<Matching> &result,
                int limit,
                Mat &outImg,
                vector<KeyPoint> &qKeypoints,
                Mat &qDescriptors) {

    cout << "query: " << fileName << endl;

    Mat img = readResource(fileName);

    if (!img.data) {
        cerr << fileName << " can not be read" << endl;
        return;
    }


    cout << "extracting features..." << endl;
    if (!extractFeatures(img, qKeypoints, qDescriptors)) {
        cerr << fileName << " can not be processed" << endl;
        return;
    }


    if (qDescriptors.rows == 0) {
        cerr << "error processing (no descriptors) " << endl;
        return;
    }

    if (_usePCA) {
        cout << "Projecting PCA..." << endl;
        _pca->project(qDescriptors, qDescriptors);
    }

    cout << " feat:"
         << qDescriptors.rows
         << " " << qDescriptors.cols
         << " " << qDescriptors.type()
         << endl << flush;


    cout << "db:running query..." << endl;
    _vt->query(qDescriptors, result, limit);


    if (_exports) {

        cout << "exporting results..." << endl;
        exportResults(result);

        cout << "exporting features image..." << endl;
        exportFeaturesImage(fileName, img, qKeypoints);

    }


}

string
Database::getPath() {
    return _path;
}



