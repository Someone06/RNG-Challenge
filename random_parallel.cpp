#include <algorithm>
#include <bit>
#include <cstdint>
#include <iostream>
#include<omp.h>

using Int=std::uint32_t;

static inline constexpr std::size_t bitSize {sizeof(Int) * 8};
static inline constexpr std::size_t halfBitSize {bitSize / 2};
static inline constexpr Int lowerHalfBitMask {(1 << halfBitSize) - 1};

struct State {
    Int u;
    Int v;
};

/*
 * The peudo-random number generating function
 */ 
[[nodiscard]] inline constexpr Int nextRandomNumber(State& state) noexcept {
    // FIXME: Only works if sizeof(Int) == 4
    state.v = 36969 * (state.v & lowerHalfBitMask) + (state.v >> halfBitSize);
    state.u = 18000 * (state.u & lowerHalfBitMask) + (state.u >> halfBitSize);
    return (state.v << halfBitSize) | (state.u & lowerHalfBitMask);
}

[[nodiscard]] inline constexpr State deriveNewState(State& state) noexcept {
    Int u { nextRandomNumber(state) };
    Int v { nextRandomNumber(state) };
    return State {.u = u, .v = v};
}

// FIXME: Only works if sizeof(Int) == 4
static inline constexpr Int alternatingBitmask {0xAAAAAAAA};

/*
 * A 32-bit number has 16 pairs bits. If every bit has a 50/50 chance of being 0 
 * or 1, than the probability of a pair of bits being 11 is 1/4. Thus we can 
 * extract 16 1/4 chances from a 32-bit number.
 */
[[nodiscard]] inline constexpr Int countPairwiseZeroBits(Int n) noexcept {
    return std::popcount(n & (n << 1) & alternatingBitmask);
}


static inline constexpr Int attempts {231};

static inline constexpr Int numberOfExtractedPairs {halfBitSize};
static inline constexpr Int completeAttempts {attempts / numberOfExtractedPairs};
static inline constexpr Int remainingAttempts {attempts % numberOfExtractedPairs};
static inline constexpr Int remainingAttemptsBitmask {(1 << (remainingAttempts * 2)) - 1};


/*
 * Counts the number of time a 1/4 change is hit when doing 'attempts' attempts.
 */ 
[[nodiscard]] Int calculateRound(State state) noexcept {
    Int count {0};
    
    // Note: Explicily requesting simd instruction decreased performance
    // slightly on a RPI 5.
    //#pragma omp simd
    for(Int i = 0; i < completeAttempts; ++i) {
        Int pseudoRandomNumber { nextRandomNumber(state) };
        Int hits { countPairwiseZeroBits(pseudoRandomNumber) };
        count += hits;
    }

    Int pseudoRandomNumber {nextRandomNumber(state) & remainingAttemptsBitmask};
    Int hits = countPairwiseZeroBits(pseudoRandomNumber);
    count += hits;

    return count;
}


static inline constexpr Int rounds  {1'000'000'000};

struct Init {
    State state;

    [[nodiscard]] constexpr Init(State s) noexcept: state{s} {}

    [[nodiscard]] Init(const Init& other) noexcept {
        State s {other.state};
        int ithread = omp_get_thread_num();
        for(int i {0}; i < 2*ithread; ++i) {
            s = deriveNewState(s);
        }

        state = s;
    }
};

/*
 * Run the simulation for 'rounds' rounds and return the maximum number number
 * of hits that have occurred in any attempt.
 */
[[nodiscard]] Int runSimulation(State state) noexcept {
    Int maxCount {0};
    Init init {state};

    # pragma omp parallel for firstprivate(init) reduction(max:maxCount)
    for(Int i = 0; i < rounds; ++i) {
        State newState { deriveNewState(init.state) };
        Int count {calculateRound(newState)};
        maxCount = std::max(maxCount, count);
    }

    return maxCount;
}

/*
 * The values u and v are used for seeding. Change them at will to get different
 * results.
 */
static inline constexpr Int u {0xc0de15af};
static inline constexpr Int v {~u};

int main() {
    std::cerr << "Starting calculation for with " << rounds << " rounds"
        << std::endl;
    Int maxHits {runSimulation(State{.u=u, .v=v})};
    std::cerr << "Found at max " << maxHits << " hits" << std::endl;
    return 0;
}
