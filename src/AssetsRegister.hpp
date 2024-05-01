#pragma once
#include <Siv3D.hpp>

static FilePath asset_path(const String& path) {
    const String assets_path = U"./assets";
    return FileSystem::PathAppend(assets_path, path);
}

static void RegisterAssets() {
    TextureAsset::Register(U"instructions/page1", asset_path(U"howto/page1.JPG"));
    TextureAsset::Register(U"instructions/page2", asset_path(U"howto/page2.JPG"));
    TextureAsset::Register(U"instructions/page3", asset_path(U"howto/page3.JPG"));

    TextureAsset::Register(U"player/run",   asset_path(U"sprites/stickfigure_walk.png"));
    TextureAsset::Register(U"player/jump",  asset_path(U"sprites/stickfigure_jump.png"));

    AudioAsset::Register(U"se/run",     asset_path(U"se/running.wav"));
    AudioAsset::Register(U"se/jump",    asset_path(U"se/jump.wav"));
    AudioAsset::Register(U"se/land",    asset_path(U"se/land.wav"));
    AudioAsset::Register(U"se/sliding", asset_path(U"se/sliding.mp3"));
    AudioAsset::Register(U"se/rocket",  asset_path(U"se/rocket.mp3"));    
    AudioAsset::Register(U"se/bighit",  asset_path(U"se/hit.mp3"));
    AudioAsset::Register(U"se/gravity",     asset_path(U"se/gravity_sound.mp3"));
    
    AudioAsset::Register(U"bgm/game",   asset_path(U"music/reflectable.mp3"));
    AudioAsset(U"bgm/game").setLoop(true);
    AudioAsset::Register(U"bgm/walk-demo",    asset_path(U"music/Clarity_of_My_Sight.mp3"));
    AudioAsset(U"bgm/walk-demo").setLoop(true);
    AudioAsset::Register(U"bgm/instruction",  asset_path(U"music/予兆.mp3"));
    AudioAsset(U"bgm/instruction").setLoop(true);
    

    FontAsset::Register(U"UIFont", 36, Typeface::Heavy);
}