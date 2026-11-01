#pragma once
#include <opxy/effect.hpp>
namespace transient {
struct State { opxy::i32 controls[4], fast, slow, gain; opxy::u32 started; };
void initialize(State&);
void process(State&, opxy::Channel, opxy::Channel, opxy::Parameters);
}
