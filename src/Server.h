//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.


#ifndef SERVER_H_
#define SERVER_H_

#include <string.h>
#include <cv.hpp>

#include "Database.h"

/**
 * A database is a big data structure. Loading a database in memory is time consuming.
 * A client-server mechanism is needed, where the database is loaded in memory, and then queries are accepted.
 * This file provides this needed server functionalities, using sockets.
 * It works as Database engine, it can be started and it will accept concurrent incoming queries.
 */

using namespace std;
using namespace cv;


/**
 * Starts database
 * @param dbPath path to the database
 */
void startDatabase(string dbPath);


/**
 * Stops the database
 * @param dbPath path to the database
 */
void stopDatabase(string dbPath);


/**
 * Returns database state
 * @param dbPath path to the database
 * @return can be "STARTED", "STARTING", or "STOPPED"
 */
string getState(string dbPath);


bool isStarted(int port);

void log(string message);

void sendMessage(int sockfd, string message);

string sendCommand(string host, int port, string command);

string readCommand(int newsockfd);

void handleQuery(string query, int sockfd, Ptr<Database> &db);

void handleCommand(string command, int sockfd, Ptr<Database> &db);

void processClient(int sockfd, Ptr<Database> &db);

Configuration readConfig(string dbPath);

void listenForClients(int port, Ptr<Database> db);

int getPort(string dbPath);

string startingLockName(string dbPath);

bool isStarting(string dbPath);

void setStartingLock(string dbPath);

void delStartingLock(string dbPath);

void runQuery(string dbPath, string query);


#endif /* SERVER_H_ */
