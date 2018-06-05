Efficient Large-scale Image Search With a Vocabulary Tree
=========================================================
This is the source code used in the IPOL article:
http://www.ipol.im/pub/art/2018/199/

version number: 1.1.1
release date: apr.25.2018

Description
===========
This program is a full C++ implementation of the paper "Scalable Recognition with a Vocabulary Tree" by David Nistér and Henrik Stewénius. Open source code is provided, with a functional demo. 

If the -build option is used, the demo will generate a "database" with the vocabulary tree and indexed images to be queried. It works like a database engine with a client-server model and accepts concurrent incoming queries. 
When no query is received for a while, the demo stops itself and the memory is released.

A database is identified by its path on the file system. Let's call that path the database root.

Given a database root path <root> its directory structure is:
 <root>/vocabulary: the path where files used to build the vocabulary are placed (required)
 <root>/input: the path where files to be indexed are placed (required)
 <root>/data: the path where internal database data files are stored
 <root>/queries: a the path where to place files to be queried (might be empty)
 <root>/results: the path where the database will write output results

The vocabulary will be created from the files on the vocabulary directory, and then images on input directory will be indexed. Make sure to fill those directories before running -build option.

This demo makes use of the well known OpenCV Library <http://opencv.org/>. 
The following basic functionalities provided from OpenCV were used:
	- core matrix support and arithmetic matrix operations
	- standard k-means
	- keypoint detection and descriptor extractor (SIFT, SURF, ORB, KAZE, etc)
	- image and video manipulation


Authors and contact information
===============================
Esteban Uriza <euriza@dc.uba.ar>
Francisco Gómez-Fernández <fgomez@dc.uba.ar>
Martin Rais <mrais@cmla.ens-cachan.fr>


Citing this article
===================
If you use this code in your publication, plase cite our work:

@article{ipol.2018.199,
  title   = {Efficient Large-scale Image Search with a Vocabulary Tree},
  author  = {Uriza, Esteban and G{\'o}mez-Fern{\'a}ndez, Francisco and Rais, Mart{\'i}n},
  journal = {Image Processing On Line},
  volume  = {8},
  pages   = {7--98},
  year    = {2018},
  doi     = {10.5201/ipol.2018.199}, 
}

% if your bibliography style doesn't support doi fields:
    note    = {\url{https://doi.org/10.5201/ipol.2018.199}}


License
=======
This program is free software: you can use, modify and/or
redistribute it under the terms of the GNU General Public
License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later
version. You should have received a copy of this license along
this program. If not, see <http://www.gnu.org/licenses/>.


Patent warning
==============
This source code uses algorithms possibly linked to the following patents:
- D.G. Lowe. Method and apparatus for identifying scale invariant features in an image and use
of same for locating an object in an image, March 23 2004. US Patent 6,711,293
- R. Funayama, H. Yanagihara, L. Van Gool, T. Tuytelaars, and H. Bay. Robust interest point
detector and descriptor, September 24 2009. US Patent App. 12/298,879.
This code is made available for the exclusive aim of serving as
scientific tool to verify the soundness and completeness of the
algorithm description. Compilation, execution and redistribution
of this file may violate patents rights in certain countries.
The situation being different for every country and changing
over time, it is your responsibility to determine which patent
rights restrictions apply to you before you compile, use,
modify, or redistribute this file. A patent lawyer is qualified
to make this determination.
If and only if they don't conflict with any patent terms, you
can benefit from the following license terms attached to this
source code.


Tools and libraries needed to compile and use the program
=========================================================
In order to compile, it requires to install OpenCV 3.1.0 wich can be downloaded from the official OpenCV web site <http://opencv.org/>.
It is also required to compile the contrib module.

Installing OpenCV
-----------------

The standard way to install OpenCV is to install it in the /usr/local directory, but in this way can't coexist different versions of OpenCV in the same machine. Thus, the way recommend to install OpenCV is to install it in your home directory.
The following terminal commands shows the installation process:

$ mkdir ~/opencv
$ mkdir ~/opencv/installed
$ mkdir ~/opencv/installed/3.1.0

$ cd ~/opencv
$ git clone --branch 3.1.0 --depth 1 https://github.com/opencv/opencv.git ./opencv3.1.0
$ cd opencv3.1.0
$ git clone --branch 3.1.0 --depth 1 https://github.com/opencv/opencv_contrib.git ./contrib
$ mkdir release
$ cd release
$ cmake -D OPENCV_EXTRA_MODULES_PATH=~/opencv/opencv3.1.0/contrib/modules -D CMAKE_INSTALL_PREFIX=~/opencv/installed/3.1.0 ..
$ make -j 8
$ make install

Required modules:
	- opencv_core
	- opencv_highgui
	- opencv_imgproc
	- opencv_imgcodecs
	- opencv_features2d
	- opencv_xfeatures2d
	- opencv_video
	- opencv_videoio
	- opencv_flann
	- opencv_calib3d


Build error for python bindings with opencv_contrib modules
-----------------------------------------------------------

There is an issue when compiling contrib modules with python support in some systems. 
Check this link for a workaround: https://github.com/opencv/opencv/issues/6016
Otherwise, simply add this flag to cmake command: -D BUILD_opencv_python2=OFF


Compiling voctree
-----------------

Unzip the source code into a workspace directory, and compile it with make command.
Also, CMakeLists.txt based project is provided.

$ tar -xvf voctree_1.1.1.tar.gz
$ cd voctree_1.1.1
$ make


Exporting library path
----------------------
You will need to export the path to the OpenCV libraries.

$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/opencv/installed/3.1.0/lib


Usage mode
------------

$ ./vt <option> <params>

	if <option> == '-build': build a new database
	if <option> == '-start': starts server
	if <option> == '-stop': stops server
	if <option> == '-update': updates database
		params := <database path>

	if <option> == '-query': does a query
		params := <database path> <file to query>


Running the demo
================

CREATING A NEW DATABASE:
 a) create a new directory for example "/home/mydb", 
 b) create a new vocabulary directory "/home/mydb/vocabulary"
 c) copy all files you want to use to train vocabulary into the vocabulary directory (files can be images or videos).
 d) create a new directory input for files to be indexed "/home/mydb/input"
 e) copy all files you want to index to input directory (the files can be images or videos).
 f) create a text file "/home/mydb/config.txt"
 g) edit config.txt to specify the port where the database will be listening
 contents of config.txt :
	# settings for database
	port=64003
	
 we are now ready to build the database, execute the command 
 $ ./vt -build /home/mydb

 ... this could take some time depending on the number of images ...
 ... and if we don't get any error, we have built ok the database.


STARTING DATABASE:
 to start the database simply run the command:
 $ vt -start /home/mydb

STOPPING DATABASE:
 to start the database simply run the command:
 $ vt -stop /home/mydb

PERFORMING A QUERY:
 must specify the database and the query image file, for example:
 $ vt -query /home/mydb /home/images/img1.png

UPDATE INDEX:
 to re-index all the files under /home/mydb/input run the command:
 $ vt -update /home/mydb


Source files
============

List of source files provided:

Catalog.cpp        ExtKmeans.h            KeyPointPersistor.h  Server.cpp
Catalog.h          FeatureMethod.cpp      KMeans.cpp           Server.h
CMakeLists.txt     FeatureMethod.h        KMeans.h             ShootSegmenter.cpp
Configuration.cpp  FileHelper.cpp         main.cpp             ShootSegmenter.h
Configuration.h    FileHelper.h           Matching.cpp         VecPersistor.hpp
Database.cpp       FileManager.cpp        Matching.h           VocTree.cpp
Database.h         FileManager.h          MatPersistor.cpp     VocTree.h
ExtKmeans.cpp      KeyPointPersistor.cpp  MatPersistor.h


Changes in the software since it was first published
====================================================
none.

List of known defects
=====================
none.

Credits and acknowledgments
===========================
none.
