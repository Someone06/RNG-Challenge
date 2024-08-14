#include <algorithm>
#include <bit>
#include <cstdint>
#include <iostream>

static inline constexpr std::uint32_t lowerHalfSet
    {(static_cast<std::uint32_t>(1) << 16) - 1};

/*
 * The peudo-random number generating function
 */ 
[[nodiscard]] constexpr std::uint32_t nextRandomNumber(
        std::uint32_t& u, std::uint32_t& v) noexcept {
    v = 36969 * (v & lowerHalfSet) + (v >> 16);
    u = 18000 * (u & lowerHalfSet) + (u >> 16);
    return (v << 16) + (u & lowerHalfSet);
}

static inline constexpr std::uint32_t alternatingBitmask {0xAAAAAAAA};

/*
 * A 32-bit number has 16 pairs bits. If every bit has a 50/50 chance of being 0 
 * or 1, than the probability of a pair of bits being 11 is 1/4. Thus we can 
 * extract 16 1/4 chances from a 32-bit number.
 */
[[nodiscard]] constexpr std::uint32_t countPairwiseZeroBits(std::uint32_t n) 
        noexcept {
    return std::popcount(n & (n << 1) & alternatingBitmask);
}


static inline constexpr std::uint32_t attempts {231};

static inline constexpr std::uint32_t numberOfExtractedPairs
    {sizeof(std::uint32_t) * 4};

static inline constexpr std::uint32_t completeAttempts 
    {attempts / numberOfExtractedPairs};

static inline constexpr std::uint32_t remainingAttempts 
    {attempts % numberOfExtractedPairs};

static inline constexpr std::uint32_t remainingAttemptsBitmask 
    {(1 << remainingAttempts * 2) - 1};

/*
 * Counts the number of time a 1/4 change is hit when doing 'attempts' attempts.
 */ 
[[nodiscard]] constexpr std::uint32_t calculateRound(
        std::uint32_t u, std::uint32_t v) noexcept {
    std::uint32_t count {0};
    for(std::uint32_t i {0}; i < completeAttempts; ++i) {
        std::uint32_t pseudoRandomNumber { nextRandomNumber(u, v) };
        std::uint32_t hits { countPairwiseZeroBits(pseudoRandomNumber)};
        count += hits;
    }

    std::uint32_t pseudoRandomNumber
        {nextRandomNumber(u, v) & remainingAttemptsBitmask};
    std::uint32_t hits = countPairwiseZeroBits(pseudoRandomNumber);
    count += hits;

    return count;
}


static inline constexpr std::uint32_t rounds  {1'000'000'000};

/*
 * Run the simulation for 'rounds' rounds and return the maximum number number
 * of hits that have occurred in any attempt.
 */
[[nodiscard]] constexpr std::uint32_t runSimulation(
        std::uint32_t u, std::uint32_t v) noexcept {

    std::uint32_t maxCount {0};
    for(std::uint32_t i {0}; i < rounds; ++i) {
        std::uint32_t uu { nextRandomNumber(u, v) };
        std::uint32_t vv { nextRandomNumber(u, v) };
        std::uint32_t count {calculateRound(uu, vv)};
        maxCount = std::max(maxCount, count);
    }

    return maxCount;
}

/*
 * The values u and v are used for seeding. Change them at will to get different
 * results.
 */
static inline constexpr std::uint32_t u {0xC0de15af};
static inline constexpr std::uint32_t v {~u};

int main() {
    std::cerr << "Starting calculation for with " << rounds << " rounds"
        << std::endl;
    std::uint32_t maxHits {runSimulation(u, v)};
    std::cerr << "Found at max " << maxHits << " hits" << std::endl;
    return 0;
}
