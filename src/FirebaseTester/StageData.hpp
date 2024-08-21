#pragma once
#include <Siv3D.hpp>

namespace LandscapeExtenders::Firebase {

struct StageData {
    bool is_ready;
    String name;
    Vec2 position;
    FilePath storageURI;
    static StageData from_firestore(const JSON& json){
        StageData s;
        
        s.is_ready    = getBoolean(json[U"is_ready"]);
        s.name        = getString(json[U"name"]);
        s.position.x  = getNumberAsDouble(getMap(json[U"position"])[U"x"]);
        s.position.y  = getNumberAsDouble(getMap(json[U"position"])[U"y"]);
        s.storageURI  = getString(json[U"storageURI"]);
        return s;
    }
    static StageData from_savedata(const JSON& json) {
        StageData s;
        s.is_ready = json[U"is_ready"].get<bool>();
        s.name = json[U"name"].getString();
        s.position.x = json[U"position"][U"x"].get<double>(); 
        s.position.y = json[U"position"][U"y"].get<double>();
        s.storageURI = json[U"storageURI"].getString();
        return s;
    }
    JSON encode() const {
        JSON json;
        json[U"is_ready"] = is_ready;
        json[U"name"] = name;
        json[U"position"][U"x"] = position.x; 
        json[U"position"][U"y"] = position.y;
        json[U"storageURI"] = storageURI;
        return json;
    }
private:
    static String getString(JSON value) {
        return value[U"stringValue"].getString();
    }
    static bool getBoolean(JSON value) {
        return (value[U"booleanValue"].get<bool>());
    }
    static JSON getMap(JSON value) {
        return (value[U"mapValue"][U"fields"]);
    }
    static double getNumberAsDouble(JSON value) {
        const String key_int = U"integerValue";
        const String key_double = U"doubleValue";
        if (value.contains(key_int)) {
            return Parse<double>(value[key_int].getString());
        } else if (value.contains(key_double)) {
            return Parse<double>(value[key_double].getString());
        } else { 
            assert(0);
        }
    }
};
}