#pragma once

#include <string>
#include <map>
#ifndef _WIN32
#include <stdint.h>
#include <arpa/inet.h>
#else
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
#endif

// include shared_ptr
#include <cstddef> // for __GLIBCXX__
#ifdef __GLIBCXX__
#  include <tr1/memory>
using std::tr1::shared_ptr;
#else
#  ifdef __IBMCPP__
#    define __IBMCPP_TR1__
#  endif
#  include <memory>
#endif

struct SyncKey {
    enum Type {
        STEP,
        LINEAR,
        SMOOTH,
        RAMP,
    };

    uint32_t row;
    float value;
    Type type;
};

const uint8_t CMD_SET_KEY = 0;
const uint8_t CMD_DEL_KEY = 1;
const uint8_t CMD_GET_TRACK = 2;
const uint8_t CMD_SET_ROW = 3;
const uint8_t CMD_PAUSE = 4;
const uint8_t CMD_SAVE_TRACKS = 5;

class SyncTrack {
public:
    SyncTrack(std::string name);

    std::string getName();

    float getValue(double row);

    SyncKey getPrevKey(int row);
    SyncKey getExactKey(int row);
    void setKey(SyncKey key);
    void delKey(int row);
    bool isKeyFrame(int row);

    void loadFromFile(std::string path);

    std::map<int,SyncKey> getKeyMap();

    // Iterator
    typedef std::map<int,SyncKey>::iterator iterator;
    typedef std::map<int,SyncKey>::const_iterator const_iterator;

    iterator begin() { return keys.begin(); }
    const_iterator begin() const { return keys.begin(); }

    iterator end() { return keys.end(); }
    const_iterator end() const { return keys.end(); }

private:
    std::string name;
    std::map<int, SyncKey> keys;
};

//typedef shared_ptr<SyncTrack> SyncTrackPtr;
