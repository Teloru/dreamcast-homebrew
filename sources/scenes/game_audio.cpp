#include "game.h"

namespace {

constexpr float SOUND_EFFECT_GAIN = 0.5f;

} // namespace

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

void GameScene::load_sound_effects() {
    const char* const paths[] = {
        "sfx/question.wav", "sfx/star.wav",
        "sfx/slider-tick-1.wav", "sfx/back-1.wav", "sfx/scene-change.wav"
    };
    static_assert(sizeof(paths) / sizeof(paths[0]) ==
                  static_cast<std::size_t>(SoundEffect::COUNT),
                  "Every sound effect needs an asset path");

    smlt::SoundFlags flags;
    flags.stream_audio = false; // Short PCM clips stay in memory, away from CD I/O.
    for(std::size_t i = 0; i < sound_effects_.size(); ++i) {
        sound_effects_[i] = assets->load_sound(paths[i], flags);
        if(!sound_effects_[i]) {
            S_WARN("Unable to load sound effect: {0}", paths[i]);
        }
    }
    sound_effects_source_ = create_child<smlt::AudioSource>();
    if(!sound_effects_source_) {
        S_WARN("Unable to create the sound effects source");
    }
}

void GameScene::play_sound_effect(SoundEffect effect) {
    const auto index = static_cast<std::size_t>(effect);
    if(!sound_effects_source_ || index >= sound_effects_.size() ||
       !sound_effects_[index]) {
        return;
    }

    const auto playing = sound_effects_source_->play_sound(
        sound_effects_[index], smlt::AUDIO_REPEAT_NONE, smlt::DISTANCE_MODEL_AMBIENT);
    // The source owns each instance; short clips may finish on the audio thread.
    smlt::thread::Lock<smlt::thread::Mutex> lock(sound_effects_source_->mutex_);
    if(auto* instance = playing.operator->()) {
        instance->set_gain(SOUND_EFFECT_GAIN);
    }
}

void GameScene::unload_sound_effects() {
    if(sound_effects_source_) {
        sound_effects_source_->destroy();
        sound_effects_source_ = nullptr;
    }
    sound_effects_.fill(smlt::SoundPtr{});
}
