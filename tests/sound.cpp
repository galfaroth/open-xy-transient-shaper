#include "../fx.cpp"
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#define REQUIRE(c) do { if (!(c)) { std::printf("failed line %d\n", __LINE__); return 1; } } while (false)
using opxy::i32;
static i32 left[64], right[64];
static double burst(i32 attack, i32 sustain, unsigned start, unsigned end) {
    transient::State state{}; transient::initialize(state);
    i32 p[12] = {attack,sustain,0,1073741824};
    double energy=0;
    for (unsigned block=0;block<64;++block) {
        for(unsigned i=0;i<64;++i) left[i]=right[i]=107374182;
        transient::process(state,left,right,p);
        for(unsigned i=0;i<64;++i) if(block*64+i>=start && block*64+i<end) energy+=double(left[i])*left[i];
    }
    return energy;
}
int main() {
    REQUIRE(sizeof(transient::State) <= 472);
    transient::State s{}; transient::initialize(s);
    i32 p[12]={1073741824,1073741824,0,1073741824};
    i32 original[64]; std::uint32_t random=3;
    for(unsigned b=0;b<20;++b) {
        for(unsigned i=0;i<64;++i) { random=random*1664525u+1013904223u; original[i]=left[i]=right[i]=static_cast<i32>(random); }
        transient::process(s,left,right,p);
        for(unsigned i=0;i<64;++i) REQUIRE(left[i]==original[i] && right[i]==original[i]);
    }
    const double early=burst(1073741824,1073741824,0,128), late=burst(1073741824,1073741824,2048,4096);
    REQUIRE(burst(2147483647,1073741824,0,128)>early*2);
    REQUIRE(burst(0,1073741824,0,128)<early*.8);
    REQUIRE(burst(1073741824,2147483647,2048,4096)>late*8);
    REQUIRE(burst(1073741824,0,2048,4096)<late*.2);
    REQUIRE(burst(2147483647,1073741824,2048,4096)<late*1.05);
    for(i32 mode: {0,1073741824,2147483647}) {
        transient::initialize(s);p[0]=p[1]=p[2]=2147483647;p[3]=1073741824;p[9]=mode;
        for(unsigned b=0;b<150;++b) {
            for(unsigned i=0;i<64;++i) { left[i]=(i&1)?2147483647:(-2147483647-1);right[i]=left[i]; }
            transient::process(s,left,right,p);
            for(unsigned i=0;i<64;++i) { REQUIRE(left[i]==right[i]); REQUIRE((i&1)?left[i]>=0:left[i]<=0); }
        }
        for(unsigned i=0;i<64;++i)left[i]=right[i]=0;
        transient::process(s,left,right,p);
        for(unsigned i=0;i<64;++i) REQUIRE(left[i]==0 && right[i]==0);
    }
    for(i32 x: {transient::full/2-1,transient::full/2,transient::full+transient::full/2-1}) {
        REQUIRE(std::abs(transient::soft(x+1)-transient::soft(x))<=2);
        REQUIRE(transient::soft(-x)==-transient::soft(x));
    }
    transient::State first{},second{};transient::initialize(first);transient::initialize(second);
    for(unsigned i=0;i<64;++i) left[i]=right[i]=i*1000000;
    transient::process(first,left,right,p);i32 saved[64];for(unsigned i=0;i<64;++i)saved[i]=left[i];
    transient::initialize(first);
    for(unsigned i=0;i<64;++i)left[i]=right[i]=i*1000000;
    transient::process(first,left,right,p);
    for(unsigned i=0;i<64;++i) REQUIRE(saved[i]==left[i]);
    std::puts("transient: neutral, attack/sustain, linked stereo, clipping, silence and reset passed");
}
