#include "core.h"
#include <opxy/q31.hpp>
namespace transient {
namespace {
using opxy::i32;
const i32 full = 8388607;
i32 clamp(i32 p) { return p < 0 ? 0 : p; }
i32 smooth(i32 x, i32 target, i32 shift) {
    const i32 d = target - x;
    const i32 mask = (1 << shift) - 1;
    return x + (d > 0 ? (d + mask) >> shift : -((-d + mask) >> shift));
}
i32 magnitude(i32 x) { return x < 0 ? -x : x; }
i32 gain_sample(i32 x, i32 gain) {
    if (gain == 32768) return x;
    return opxy::q31::multiply(x, gain * 16384) * 4;
}
// Unity below half scale; a parabolic knee reaches full scale with zero slope.
i32 soft(i32 x) {
    const i32 a = magnitude(x);
    if (a <= full / 2) return x;
    if (a >= full + full / 2) return x < 0 ? -full : full;
    const i32 d = (a - full / 2) * 256;
    const i32 y = a - opxy::q31::multiply(d, d) / 512;
    return x < 0 ? -y : y;
}
i32 output(i32 x) {
    if (x > full) return opxy::q31::maximum;
    if (x < -8388608) return -opxy::q31::maximum - 1;
    return x * 256;
}
}
void initialize(State& s) { s = State{}; s.gain = 32768; }
void process(State& s, opxy::Channel left, opxy::Channel right, opxy::Parameters p) {
    const i32 targets[4] = {clamp(p[0]) / 32768 - 32768, clamp(p[1]) / 32768 - 32768,
                           clamp(p[2]) / 65536, clamp(p[3]) / 32768};
    const i32 response = ((clamp(p[8]) / 65536) * 3) / 32768;
    const i32 mode = ((clamp(p[9]) / 65536) * 3) / 32768;
    if (!s.started) { for (unsigned k = 0; k < 4; ++k) s.controls[k] = targets[k]; s.started = 1; }
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        for (unsigned k = 0; k < 4; ++k) s.controls[k] = smooth(s.controls[k], targets[k], 7);
        const i32 l = left[i] / 256, r = right[i] / 256;
        const i32 level = magnitude(l) > magnitude(r) ? magnitude(l) : magnitude(r);
        s.fast = smooth(s.fast, level, level > s.fast ? 2 + response : 6 + response);
        s.slow = smooth(s.slow, level, level > s.slow ? 7 + response : 10 + response);
        const i32 peak = s.fast / 256;
        const i32 excess = s.fast > s.slow ? (s.fast - s.slow) / 256 : 0;
        const i32 attack = excess * 32768 / (peak + 1), sustain = 32768 - attack;
        const i32 a = s.controls[0], b = s.controls[1];
        const i32 shaped = (a * attack / 32768) + (b * sustain / 32768);
        i32 gain = 32768 + (shaped >= 0 ? shaped * 3 : shaped * 3 / 4);
        if (gain > 131071) gain = 131071;
        s.gain = smooth(s.gain, gain, 4);
        const bool neutral = !s.controls[0] && !s.controls[1] && !s.controls[2] && s.controls[3] == 32768 && !mode && s.gain == 32768;
        if (neutral) continue;
        i32 channels[2] = {l, r};
        for (unsigned c = 0; c < 2; ++c) {
            i32 x = gain_sample(channels[c], s.gain);
            x += 7 * opxy::q31::multiply(x, s.controls[2] * 65536);
            if (mode == 1) x = soft(x);
            else if (mode == 2) x = x > full ? full : x < -full ? -full : x;
            x = opxy::q31::multiply(x, s.controls[3] * 32768) * 2;
            channels[c] = output(x);
        }
        left[i] = channels[0]; right[i] = channels[1];
    }
}
}
