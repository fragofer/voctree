//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef MATPERSISTOR_H
#define MATPERSISTOR_H

#include <stdlib.h>
#include <cv.hpp>

using namespace std;
using namespace cv;

/**
 * This class is used to persist a Matrix from OpenCV to disk
 * OpenCV built-in persistence to XML/YML has to much overhead and is not fast enough
 */
class MatPersistor {

public:

    /**
     * MatPersistor constructor
     * @param fileName persistence file path. file path where the matrix is going to be persisted
     */
    MatPersistor(string &fileName);

    /**
     * MatPersistor destructor
     */
    virtual ~MatPersistor();

    /**
     * checks if the persistor is open. When it is open, data can be read or written.
     * @return true if it is open
     */
    bool isOpen();

    /**
     * checks whether the persistence file path (given in the constructor) exists
     * @return true if the persistence file path exists
     */
    bool exists();

    /**
     * closes the persistor.
     */
    void close();

    /**
     * persists an empty matrix of the given type.
     * @param cols columns of the matrix
     * @param type data type. See OpenCV data types
     * @return true if empty persistence was created
     */
    bool create(int cols, int type);

    /**
     * Persists the given Mat mat to the persistence file path
     * @param mat the matrix to be persisted
     * @return true if matrix was persisted
     */
    bool create(Mat &mat);

    /**
     * Opens a MatPersistor for reading
     * @return true if persistor could be opened
     */
    bool openRead();

    /**
     * Opens a MatPersistor for writing
     * @return true if persistor could be opened
     */
    bool openWrite();

    /**
     * Appends Mat to the end of the persisted data
     * this positions the current row at the end
     * @param mat the input matrix
     */
    void append(const Mat &mat);

    /**
     * Appends the specified rows of Mat to the end of the persisted data
     * this positions the current row at the end
     * @param mat the input matrix
     */
    void append(const Mat &mat, int rows);

    /**
 * retrieves contents from the persisted data to the output matrix mat
     * data will be loaded from the current positioned row
 * @param mat the output matrix
     */
    void read(Mat &mat);

    /**
 * retrieves rows rows from the persisted data to the output matrix mat
     * data will be loaded from the current positioned row
 * @param mat the output matrix
     * @param row
 */
    int read(Mat &mat, int rows);

    /**
     * sets the current row (where data is going to be read or written)
     * @param row position where to read or write next
     */
    void setRow(int row);

    /**
     * @return number of columns defined
     */
    int cols();

    /**
     * @return number of rows the persisted matrix has
     */
    int rows();

    /**
     * @return data type of matrix element (see OpenCV data types)
     */
    int type();

    /**
     * @return element size of the data type
     */
    int elementSize();


private:

    struct Header {
        int cols;
        int rows;
        int type;
    };

    Header _header;
    FILE *_pFile;
    string _fileName;

    static const int READ = 1;
    static const int WRITE = 2;
    int _mode;
    int _currentRow;

    bool readHeader();

    bool writeHeader();

    bool open(int mode);

};

#endif // MATPERSISTOR_H
