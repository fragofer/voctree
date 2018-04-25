//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include "Server.h"

#include "FileHelper.h"

#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>


using namespace std;
using namespace cv;


bool startsWith(string str, string prefix) {
    return (strcasecmp(prefix.c_str(), str.substr(0, prefix.size()).c_str()) == 0);
}

string dropPrefix(string str, string prefix) {
    return str.substr(prefix.size());
}

void log(string message) {

    stringstream ss;
    ss << "[" << getpid() << "] " << message << endl;
    cout << ss.str();

}

void sendMessage(int sockfd, string message) {

    log(message);

    stringstream ss;
    ss << message << endl;

    int n = write(sockfd, ss.str().c_str(), ss.str().size());
    if (n < 0) {
        cerr << "SENDING MSG: error writing to socket" << endl;
        exit(1);
    }

}

string
sendCommand(string host, int port, string command) {

    // Create a socket point
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "ERROR opening socket" << endl;
        throw (-1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    struct hostent *server;
    server = gethostbyname(host.c_str());
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // Now connect to the server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        //cerr << "ERROR connecting" << endl;
        throw (-1);
    }

    int n = write(sockfd, command.c_str(), command.length());

    if (n < 0) {
        cerr << "ERROR writing to socket" << endl;
        throw (-1);
    }

    // Now read server response
    char buffer[256];
    bzero(buffer, 256);

    string ret = "";
    while ((n = read(sockfd, buffer, 255)) > 0) {
        string s(buffer, n);
        ret += s;
    }

    if (n < 0) {
        cerr << "ERROR reading from socket" << endl;
        throw (-1);
    }

    //string response(buffer);
    //return response;
    return ret;

}


string
readCommand(int newsockfd) {

    int n;
    char buffer[256];

    bzero(buffer, 256);

    n = read(newsockfd, buffer, 256);
    if (n < 0) {
        cerr << "error reading from socket." << endl;
        exit(1);
    }

    string command(buffer);
    command.erase(command.find_last_not_of(" \n\r\t") + 1);

    return command;

}


void handleQuery(string query, int sockfd, Ptr<Database> &db) {

    //msg()
    string msg;
    msg = "executing query: " + query;
    sendMessage(sockfd, msg);


    string fileQuery;
    if (startsWith(query, "...")) {
        query = dropPrefix(query, "...");
        fileQuery += db->getPath();
    }
    fileQuery += query;

    //int limit = 6; //result.size() > 10 ? 10 : result.size();
    int limit = 16; //result.size() > 10 ? 10 : result.size();

    vector<Matching> result;
    db->query(fileQuery, result, limit);

    vector<Database::ExportInfo> exports = db->exportResults(result);
    assert(result.size() == exports.size());

    cout << "query done." << endl;
    cout << "result size:" << result.size() << endl;

    for (unsigned int i = 0; i < result.size(); i++) {

        Matching m = result.at(i);
        Database::ExportInfo info = exports.at(i);

        //cout << "matching:" << i << ", " << m.id << ", " << m.score << endl;

        float score = m.score;
        //DBElem info = db->getFileInfo( m.id );
        //cout << score << " " << info.name << endl;

        stringstream ss;
        ss << score << "," << m.id << ", " << info.fileName << endl;

        int n = write(sockfd, ss.str().c_str(), ss.str().size());
        if (n < 0) {
            cerr << "writing response: error writing to socket" << endl;
            exit(1);
        }


    }

}


void handleCommand(string command, int sockfd, Ptr<Database> &db) {

    //bool breaks = false;

    if (strcasecmp(command.c_str(), "exit") == 0 ||
        strcasecmp(command.c_str(), "quit") == 0) {
        sendMessage(sockfd, "good bye.");
        //breaks = true;
    } else if (strcasecmp(command.c_str(), "term") == 0) {
        sendMessage(sockfd, "terminating server.");
        kill(getppid(), SIGTERM);
        //breaks = true;
    } else if (startsWith(command, "query ")) {
        string query = dropPrefix(command, "query ");
        handleQuery(query, sockfd, db);
    } else {
        string msg = "unknown command '" + command + "'.";
        sendMessage(sockfd, msg);
    }


}


void processClient(int sockfd, Ptr<Database> &db) {

    string msg = "started";
    log("process start");

    string command = readCommand(sockfd);

    if (strcasecmp(command.c_str(), "hello") == 0) {
        sendMessage(sockfd, "hello :)");
    } else {
        handleCommand(command, sockfd, db);
    }

    close(sockfd);

    log("process end");


}

Configuration
readConfig(string dbPath) {

    string configFile;
    configFile = dbPath + "/config.txt";

    if (!FileHelper::exists(configFile)) {
        cerr << "could not find database configuration file " << configFile << "." << endl;
        exit(-1);
    }

    return Configuration(configFile);

}

void listenForClients(int port, Ptr<Database> db) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "error opening socket." << endl;
        exit(1);
    }

    struct sockaddr_in serv_addr;

    //Initialize socket structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // re-use socket when has been killed recently
    int val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    // sets socket timeout.
    // if it doesn't receive queries for a while, it auto-terminates
    struct timeval timeout;
    timeout.tv_sec = 5 * 60;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
        cerr << "set socket option failed" << endl;
        exit(1);
    }


    //Now bind the host address using bind() call
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "cannot bind socket." << endl;
        exit(1);
    }

    struct sockaddr_in cli_addr;
    socklen_t clilen;

    clilen = sizeof(cli_addr);
    cout << "database listening on port: " << port << endl;

    listen(sockfd, 5);

    bool term = false;
    while (!term) {

        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {

            cout << "terminating server due to inactivity of (" << timeout.tv_sec << ") secs." << endl;

            term = true;
            continue;

        }


        //Create child process
        int pid = fork();
        if (pid < 0) {
            cerr << "error creating new process (fork)." << endl;
            exit(1);
        }

        if (pid == 0) {
            //This is the client process
            close(sockfd);

            processClient(newsockfd, db);

            cout << "process query" << endl;
            exit(0);

        } else {

            close(newsockfd);

        }

    }


}


int getPort(string dbPath) {

    // Reads database configuration
    Configuration cfg = readConfig(dbPath);

    if (!cfg.has("port")) {
        cerr << "no port configured" << endl;
        exit(1);
    }

    int port = atoi(cfg.get("port").c_str());

    return port;

}


bool isStarted(int port) {

    string host = "localhost";
    string command = "hello";

    try {
        string ret = sendCommand(host, port, command);
        return true;
    }
    catch (int ex) {
        return false;
    }

}

string startingLockName(string dbPath) {
    return dbPath + "/starting.lock";
}

bool isStarting(string dbPath) {
    string lock = startingLockName(dbPath);
    return (FileHelper::exists(lock));
}

void setStartingLock(string dbPath) {
    string lock = startingLockName(dbPath);
    ofstream file(lock.c_str());
}

void delStartingLock(string dbPath) {
    string lock = startingLockName(dbPath);
    FileHelper::deleteFile(lock);
}

void startDatabase(string dbPath) {

    int port = getPort(dbPath);

    if (isStarted(port)) {
        cout << "database is STARTED" << endl;
        return;
    }

    if (isStarting(dbPath)) {
        cout << "database is STARTING" << endl;
        return;
    }

    // Ok, the server is stopped.

    setStartingLock(dbPath);

    cout << "starting server..." << endl;

    Ptr<Database> db;
    cout << "loading database " << dbPath << "..." << endl << flush;
    db = Database::load(dbPath);
    cout << "load done." << endl << flush;

    delStartingLock(dbPath);


    listenForClients(port, db);


}


string getState(string dbPath) {

    int port = getPort(dbPath);

    if (isStarted(port)) {
        return "STARTED";
    }

    if (isStarting(dbPath)) {
        return "STARTING";
    }

    return "STOPPED";

}


void runQuery(string dbPath, string query) {

    int port = getPort(dbPath);

    string host = "localhost";
    string command = "query " + query;
    try {

        string ret = sendCommand(host, port, command);
        cout << ret << endl;

    }
    catch (int ex) {

        // can't run command.
        if (isStarting(dbPath)) {
            cout << "database is STARTING" << endl;
        } else {
            cout << "database is STOPPED" << endl;
        }


    }

}


void stopDatabase(string dbPath) {

    int port = getPort(dbPath);

    if (!isStarted(port)) {

        cout << "database is not STARTED" << endl;
        return;

    }

    string host = "localhost";
    string command = "term";
    try {
        string ret = sendCommand(host, port, command);
        cout << ret << endl;
    }
    catch (int ex) {
        cout << "can't stop database" << endl;
    }

}
