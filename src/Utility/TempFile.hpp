#pragma once
#include <Siv3D.hpp>

/**
 * @brief 一時ファイル。使った後、もしそこにファイルが存在すれば削除する。
 */
class TempFile {
public:
    const FilePath path;
    TempFile():
        path(FileSystem::UniqueFilePath())
    {}
    TempFile operator=(const TempFile&) = delete;
    TempFile operator=(const TempFile&&) = delete;
    ~TempFile() {
        if (FileSystem::Exists(path)) {
            FileSystem::Remove(path);
        }
    }
};