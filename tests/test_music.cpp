#define ADXTRACT_IMPLEMENTATION
#include "../libraries/dreamcast-sh4-gcc/include/simulant/loaders/adxtract.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>

namespace {

constexpr std::size_t SAMPLE_RATE = 32000;
constexpr std::size_t CHANNELS = 2;
constexpr std::size_t EXPECTED_FRAMES = 4284632;
constexpr std::size_t BUFFER_FRAMES = 4096;

struct DecodedStats {
    std::size_t frames = 0;
    std::size_t nonzero_samples = 0;
    std::size_t saturated_samples = 0;
    std::int32_t peak = 0;
    // FNV-1a over the decoded PCM, in a fixed little-endian byte order.
    std::uint64_t hash = UINT64_C(14695981039346656037);
};

DecodedStats decode_track(adxtract_stream_t* stream) {
    std::array<std::int16_t, BUFFER_FRAMES * CHANNELS> buffer{};
    DecodedStats stats;

    for(;;) {
        const auto count = adx_stream_read_samples(
            stream, true, BUFFER_FRAMES, buffer.data());
        if(count == 0) {
            break;
        }

        assert(stats.frames <= EXPECTED_FRAMES);
        assert(count == std::min(BUFFER_FRAMES, EXPECTED_FRAMES - stats.frames));
        stats.frames += count;

        for(std::size_t i = 0; i < count * CHANNELS; ++i) {
            const std::int32_t sample = buffer[i];
            stats.nonzero_samples += sample != 0;
            stats.saturated_samples +=
                sample == std::numeric_limits<std::int16_t>::min() ||
                sample == std::numeric_limits<std::int16_t>::max();
            stats.peak = std::max(stats.peak, sample < 0 ? -sample : sample);

            const auto bits = static_cast<std::uint16_t>(buffer[i]);
            stats.hash ^= bits & 0xFF;
            stats.hash *= UINT64_C(1099511628211);
            stats.hash ^= bits >> 8;
            stats.hash *= UINT64_C(1099511628211);
        }
    }

    assert(stats.frames == EXPECTED_FRAMES);
    assert(stats.nonzero_samples > 0);
    assert(stats.saturated_samples == 0);
    return stats;
}

} // namespace

int main(int argc, char** argv) {
    const char* filename = argc > 1 ? argv[1] : "assets/music/telohub.adx";
    auto* file = std::fopen(filename, "rb");
    assert(file);
    auto* stream = adx_load_stream(file);
    assert(stream);
    assert(adx_stream_sample_rate(stream) == SAMPLE_RATE);
    assert(adx_stream_channel_count(stream) == CHANNELS);
    assert(!adx_stream_has_loop(stream));

    const auto first = decode_track(stream);
    adx_stream_rewind(stream);
    const auto second = decode_track(stream);
    assert(first.hash == second.hash);
    assert(first.nonzero_samples == second.nonzero_samples);
    assert(first.peak == second.peak);
    adx_stream_close(stream);

    std::cout << "music tests passed: " << first.frames << " frames, "
              << SAMPLE_RATE << " Hz, " << CHANNELS << " channels; peak="
              << first.peak << ", saturated_samples=" << first.saturated_samples
              << ", PCM FNV-1a=" << std::hex << first.hash << '\n';
}
