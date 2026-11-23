#include "game.h"

void GameScene::start_music() {
    if(music_playing_) {
        return;
    }

    smlt::SoundFlags flags;
    flags.stream_audio = true;
    music_ = assets->load_sound("music/telohub.adx", flags);
    if(!music_) {
        S_WARN("Unable to load background music: music/telohub.adx");
        return;
    }

    music_source_ = create_child<smlt::AudioSource>();
    if(!music_source_) {
        S_WARN("Unable to create the background music source");
        stop_music();
        return;
    }

    music_playing_ = music_source_->play_sound(
        music_, smlt::AUDIO_REPEAT_FOREVER, smlt::DISTANCE_MODEL_AMBIENT);
    if(!music_playing_) {
        S_WARN("Unable to start background music");
        stop_music();
        return;
    }
    apply_music_volume();
}

void GameScene::apply_music_volume() {
    if(music_playing_) {
        music_playing_->set_gain(menu_.music_volume() / 100.0f);
    }
}

void GameScene::stop_music() {
    if(music_playing_) {
        music_playing_->stop();
    }
    music_playing_ = {};
    music_.reset();
    // The scene owns the AudioSource and cleans it up with its other nodes.
    music_source_ = nullptr;
}
