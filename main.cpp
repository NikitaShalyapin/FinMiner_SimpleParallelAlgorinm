#include "fnv.h"
#include "getNonce.h"
#include "getTrailingZeroesCount.h"
#include "jenkins.h"
#include "pjw.h"
#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <pthread.h>
#include <omp.h>
#include <mpi.h>

#include <boost/circular_buffer.hpp>

static const double BLOCK_DURATION = 2;

static std::string hashrateToString(double hashrate)
{
    std::string hs = "TH/s";
    for (const std::string& s : {"H/s", "KH/s", "MH/s", "GH/s"}) {
        if (hashrate < 1000) {
            hs = s;
            break;
        }
        hashrate /= 1000;
    }
    return std::to_string(hashrate) + " " + hs;
}

static bool isNonceValid(uint64_t nonce, const std::array<uint8_t, 32>& seed,
                         uint32_t difficulty)
{
    std::vector<uint8_t> data(seed.begin(), seed.end());
    data.resize(data.size() + 8);
    memcpy(&data[32], &nonce, sizeof(nonce));
    uint32_t fnvHash = fnv(data.data(), data.size());
    uint32_t jenkinsHash = jenkins(data.data(), data.size());
    uint32_t pjwHash = pjw(data.data(), data.size());

    uint32_t h = fnvHash ^ jenkinsHash ^ pjwHash;
    return getTrailingZeroesCount(h) >= difficulty;
}

int main(int argc, char* argv[])
{

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned int> distribution(0, 255);
    uint32_t difficulty = 0;
    for (;;) {
        std::array<uint8_t, 32> seed;

        for (uint8_t& s : seed) {
            s = distribution(generator);
        }
        std::chrono::time_point<std::chrono::system_clock> before =
                std::chrono::system_clock::now();
        std::cout << "New job, difficulty " << difficulty << std::endl;
        uint64_t checkedNoncesCount = 0;
        uint64_t nonce = getNonce(seed, difficulty, checkedNoncesCount);
        std::chrono::time_point<std::chrono::system_clock> after =
                std::chrono::system_clock::now();
        std::chrono::duration<double> t = after - before;


        if (!isNonceValid(nonce, seed, difficulty)) {
            std::cout << "Nonce rejected\n" << std::endl;
            continue;
        }
        if (t.count() > 2 * BLOCK_DURATION) {
            if (difficulty > 0) {
                difficulty--;
            }
        }
        else if (t.count() < BLOCK_DURATION / 2) {
            if (difficulty < 32) {
                difficulty++;
            }
        }
        std::cout << "Nonce accepted";
        double hashrate = static_cast<double>(checkedNoncesCount) / t.count();
        std::cout << " (" << hashrateToString(hashrate) << " " << t.count()
                  << " seconds)\n" << std::endl;
    }
    return 0;
}