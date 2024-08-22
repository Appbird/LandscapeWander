#pragma once
#include <Siv3D.hpp>

namespace Firebase {

struct StageData {
    bool is_ready;
    String name;
    RectF area;
    FilePath storageURI;

    bool includes(Vec2 pos) const {
        Vec2 tl = area.tl(); Vec2 br = area.br();
        return InRange(pos.x, tl.x, br.x) and InRange(pos.y, tl.y, br.y);
    }

    static StageData from_firestore(const JSON& json){
        StageData s;
        
        s.is_ready      = getBoolean(json[U"is_ready"]);
        s.name          = getString(json[U"name"]);
        {
            Vec2 position; Vec2 size;
            position.x    = getNumberAsDouble(getMap(json[U"position"])[U"x"]);
            position.y    = getNumberAsDouble(getMap(json[U"position"])[U"y"]);
            size.x        = getNumberAsDouble(getMap(json[U"size"])[U"x"]);
            size.y        = getNumberAsDouble(getMap(json[U"size"])[U"y"]);
            s.area = RectF{Arg::center = position, size};
        }
        s.storageURI    = getString(json[U"storageURI"]);
        return s;
    }
    static StageData from_savedata(const JSON& json) {
        StageData s;
        s.is_ready = json[U"is_ready"].get<bool>();
        s.name = json[U"name"].getString();
        {
            Vec2 position; Vec2 size;
            position.x = json[U"position"][U"x"].get<double>(); 
            position.y = json[U"position"][U"y"].get<double>();
            size.x = json[U"size"][U"x"].get<double>(); 
            size.y = json[U"size"][U"y"].get<double>();
            s.area = RectF{Arg::center = position, size};
        }
        s.storageURI = json[U"storageURI"].getString();
        return s;
    }
    JSON encode() const {
        JSON json;
        json[U"is_ready"] = is_ready;
        json[U"name"] = name;
        json[U"position"][U"x"] = area.centerX(); 
        json[U"position"][U"y"] = area.centerY();
        json[U"size"][U"x"] = area.w; 
        json[U"size"][U"y"] = area.h;
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
            return value[key_double].get<double>();
        } else { 
            assert(0);
        }
    }
};
}