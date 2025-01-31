#pragma once
#include <Siv3D.hpp>
#include <set>
#include "StageData.hpp"

namespace Firebase {
const bool IN_DEBUG = false;
using StageList = HashTable<String, StageData>;
const FilePath auth_info_filepath   = U"landscape_extenders/auth_info.json";
const FilePath api_key_filepath     = U"landscape_extenders/api_key.txt";
const FilePath stage_list_filepath  = U"landscape_extenders/stage_list.json";
const FilePath stage_image_folderpath  = U"landscape_extenders/images";

StageList load_cached_stage_list();
void cache_stage_list(const StageList& stage_list);

class API {
public:
    API() {
        assert(FileSystem::Exists(api_key_filepath));
        API_KEY = TextReader{api_key_filepath}.readAll();
    }
    void initialize() {
        if (should_create_account()) {
            refresh_user_token();
        } else {
            create_new_account();
        }
    }

    void create_new_account() const;
    void refresh_user_token() const;
    HashTable<String, StageData> fetch_stage_array() const;
    void fetch_and_save_image(const StageData& stage_data) const;
    
private:
    using Header = HashTable<String, String>;
    String API_KEY;

    const String protocol           = IN_DEBUG ? U"http" : U"https";
    const String auth_id_host       =   IN_DEBUG
                                        ? U"127.0.0.1:9099/identitytoolkit.googleapis.com"
                                        : U"identitytoolkit.googleapis.com";
    const String auth_sectoken_host =   IN_DEBUG
                                        ? U"127.0.0.1:9099/securetoken.googleapis.com"
                                        : U"securetoken.googleapis.com";
    const String storage_host       =   IN_DEBUG
                                        ? U"127.0.0.1:9199"
                                        : U"firebasestorage.googleapis.com";
    const String firestore_host     =   IN_DEBUG
                                        ? U"127.0.0.1:8080"
                                        : U"firestore.googleapis.com";
    
    HTTPResponse request_create_new_account(
        const FilePath& dst
    ) const;
    HTTPResponse request_refresh_token(
        const String& refresh_token,
        const FilePath& dst
    ) const;
    HTTPResponse request_fetch_stage_list(
        const String& user_key,
        const FilePath& dst
    ) const;
    HTTPResponse request_fetch_and_save_image(
        const String& file_name,
        const String& user_key,
        const FilePath& saved_dir
    ) const;
    JSON extract_json_result(
        const HTTPResponse& response,
        const FilePath& dst
    ) const;

    URL base(const String& host) const {
        return U"{}://{}"_fmt(protocol, host);
    }
    URL api_url(const String& host, const String& path) const {
        return FileSystem::PathAppend(base(host), path);
    }

    bool should_create_account() const {
        return FileSystem::Exists(auth_info_filepath);
    }
    String load_refresh_key() const {
        const JSON auth_info = JSON::Load(auth_info_filepath);
        return auth_info[U"refreshToken"].getString();
    }
    String load_user_key() const {
        const JSON auth_info = JSON::Load(auth_info_filepath);
        return auth_info[U"idToken"].getString();
    }

    FilePath path_stage_image(const StageData& stage_data) const {
        return FileSystem::PathAppend(stage_image_folderpath, stage_data.storageURI);
    }
};


}