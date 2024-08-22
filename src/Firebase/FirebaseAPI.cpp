#include "FirebaseAPI.hpp"
#include "../Utility/TempFile.hpp"

namespace Firebase {


StageList load_cached_stage_list() {
    HashTable<String, StageData> result;
    const JSON documents = JSON::Load(stage_list_filepath);
    assert(documents.isObject());
    for (const auto& [key, document]: documents) {
        result.insert({key, StageData::from_firestore(document)});
    }
    return result;
}

void cache_stage_list(const StageList& stage_list) {
    JSON result;
    for (const auto& [key, document]: stage_list) {
        result[key] = document.encode();
    }
    result.save(stage_list_filepath);
}


void API::create_new_account() const {
    TempFile tmp_file;

    const auto request = request_create_new_account(tmp_file.path);
    const JSON payload = extract_json_result(request, tmp_file.path);
    assert(payload[U"idToken"].isString());
    assert(payload[U"refreshToken"].isString());
    assert(payload[U"localId"].isString());
    payload.save(auth_info_filepath);
}

void API::refresh_user_token() const {
    TempFile tmp_file;

    const String refresh_token = load_refresh_key();
    const auto request = request_refresh_token(refresh_token, tmp_file.path);
    JSON payload = extract_json_result(request, tmp_file.path);
    payload[U"idToken"] = payload[U"id_token"];
    payload[U"refreshToken"] = payload[U"refresh_token"];
    payload[U"localId"] = payload[U"user_id"];
    payload.erase(U"id_token");
    payload.erase(U"refresh_token");
    payload.erase(U"user_id");
    assert(payload[U"idToken"].isString());
    assert(payload[U"refreshToken"].isString());
    assert(payload[U"localId"].isString());
    payload.save(auth_info_filepath);
}

HashTable<String, StageData> API::fetch_stage_array() const {
    HashTable<String, StageData> result;
    TempFile tmp_file;

    const String id_token = load_user_key();
    const auto request = request_fetch_stage_list(id_token, tmp_file.path);
    const JSON payload = extract_json_result(request, tmp_file.path);
    const JSON documents = payload[U"documents"];

    assert(documents.isArray());
    for (const auto& [key, document]: documents) {
        assert(document[U"name"].isString());
        const String name = FileSystem::FileName(document[U"name"].getString());
        result.insert({name, StageData::from_firestore(document[U"fields"])});
    }

    return result;
}

void API::fetch_and_save_image(const StageData& stage_data) const {
    const String id_token = load_user_key();
    const String save_location = path_stage_image(stage_data);
    const auto request = request_fetch_and_save_image(stage_data.storageURI, id_token, save_location);
    if (request.isSuccessful()) {
        return;
    } else {
        throw Error(U"[{}] access to {} failed.: {}"_fmt(
            Unicode::FromUTF8(__func__),
            stage_data.storageURI,
            request.getStatusLine().rtrimmed()
        ));
    }
}

HTTPResponse API::request_create_new_account(const FilePath& dst) const {
    const URL url = api_url(auth_id_host, U"/v1/accounts:signUp?key={}"_fmt(API_KEY));
    const Header headers = {
        {U"Content-Type", U"application/json"}
    };
    const std::string data = JSON{
        {U"returnSecureToken", true}
    }.formatUTF8();
    return SimpleHTTP::Post(url, headers, data.data(), data.size(), dst);
}

HTTPResponse API::request_refresh_token(const String& refresh_token, const FilePath& dst) const {
    const URL url = api_url(auth_sectoken_host, U"/v1/token?key={}"_fmt(API_KEY));
    const Header headers = {
        {U"Content-Type", U"application/x-www-form-urlencoded"}
    };
    const std::string data = (U"grant_type=refresh_token&refresh_token={}"_fmt(refresh_token)).toUTF8();
    return SimpleHTTP::Post(url, headers, data.data(), data.size(), dst);
}

HTTPResponse API::request_fetch_stage_list(const String& user_key, const FilePath& dst) const {
    const URL url = api_url(firestore_host, U"/v1/projects/landscape-extenders/databases/(default)/documents/stages");
    const Header headers = {
        {U"Authentication", U"Bearer {}"_fmt(user_key)}
    };
    return SimpleHTTP::Get(url, headers, dst);
}
HTTPResponse API::request_fetch_and_save_image(
    const String& file_name,
    const String& user_key,
    const FilePath& saved_dir
) const {
    const URL url = api_url(storage_host, U"/v0/b/landscape-extenders.appspot.com/o/{}?alt=media"_fmt(file_name));
    const Header headers = {
        {U"Authentication", U"Bearer {}"_fmt(user_key)}
    };
    return SimpleHTTP::Get(url, headers, saved_dir);
}
JSON API::extract_json_result(const HTTPResponse& response, const FilePath& dst) const {
    if (response.isSuccessful()) {
        return JSON::Load(dst);
    } else {
        String context;
        if (FileSystem::Exists(dst)) {
            context = TextReader(dst).readAll();
        }
        throw Error(U"[{}] Response is in {}. / {}"_fmt(
            Unicode::FromUTF8(__func__),
            response.getStatusLine().rtrimmed(),
            context
        ));
    }
}


}