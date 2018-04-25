//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.
#include "Configuration.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;


Configuration::Configuration() {

}

void
Configuration::store(const string &fileName) {

    ofstream file(fileName.c_str());

    if (file.is_open()) {

        map<string, string>::iterator it = _dict.begin();
        while (it != _dict.end()) {

            string key = it->first;
            string value = it->second;

            file << key << "=" << value << endl;

            it++;
        }

    } else {

        throw std::runtime_error("Could not write configuration file!");

    }


}


Configuration::Configuration(const string &fileName) {

    ifstream f(fileName.c_str());

    if (f.is_open()) {
        string line;
        while (!f.eof()) {
            getline(f, line);

            line.erase(line.find_last_not_of(" \n\r\t") + 1);

            if (line.size() == 0) {
                continue;
            }

            if (line.substr(0, 1) == "#") {
                // it's a comment line
                continue;
            }

            size_t pos;
            pos = line.find_first_of('=');
            string param = line.substr(0, pos);
            string val = line.substr(pos + 1);
            _dict[param] = val;

        }
    } else {
        throw std::runtime_error("Could not open configuration file!");
    }
}

void
Configuration::put(string key, string value) {

    _dict[key] = value;

}


string &
Configuration::get(string key) {

    return _dict.at(key);

}

bool
Configuration::has(string key) {

    map<string, string>::const_iterator it = _dict.find(key);
    return (it != _dict.end());

}
