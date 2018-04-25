//Copyright (C) 2016, Esteban Uriza <estebanuri@gmail.com>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <string>
#include <map>

using namespace std;

class Configuration {

    /**
     *  This class provides a way to store and retrieve settings to and from files on disk
     *  It wraps a standard map functionality.
     */

public:

    /**
     * Creates an empty configuration
     */
    Configuration();

    /**
     * Loads a configuration from fileName
     * @param fileName the file where the configuration will be loaded from
     */
    //
    Configuration(const std::string &fileName);

    /**
     * retrieves a configuration entry
     * @param key the key where this entry is stored
     * @return the configuration entry value
     */
    string &get(string key);

    /**
     * tests if an entry is defined
     * @param key the key for the entry to be tested
     * @return true if this key is defined
     */
    bool has(string key);

    /**
    * defines value for the key key
    * @param key the key where to store the value
    * @param value the value to be set
    */
    void put(string key, string value);


    /**
     * saves this configuration to a file
     * @param fileName the name where to save the configuration
     */
    void store(const std::string &fileName);


private:
    std::map<std::string, std::string> _dict;
};

#endif /* _CONFIGURATION_H_ */
