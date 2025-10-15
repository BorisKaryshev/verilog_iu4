// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

#include <algorithm>
#include <iterator>
#include <memory>

#include <verilated.h>
#include <verilated_vcd_c.h>

constexpr int CYCLES = 1000;
long long int NUMBER = std::pow(3, 30);
constexpr int CLK_STEP_PS = 1;

inline char to_char(int var) {
    return static_cast<char>(var + '0');
}

template <typename T>
std::vector<unsigned int> decimalToBinaryVector(T value) {
    constexpr std::size_t N = std::numeric_limits<T>::digits;
    std::vector<unsigned int> bits(N);

    for (std::size_t i = 0; i < N; ++i)
    {
        // Grab bit i (0 = LSB)
        unsigned int bit = (value >> i) & 1u;
        // Store it MSB first
        bits[N - 1 - i] = bit;
    }
    // std::ranges::reverse(bits);
    return bits;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    auto dut = std::make_unique<Vmain>();  // Instantiate DUT

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    dut->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    // Initialize signals
    dut->sclk = 1;
    dut->rst = 0;
    dut->ss = 1;

    auto cmd = decimalToBinaryVector(uint8_t(38));
    auto addr = decimalToBinaryVector(uint16_t(238));
    auto data = decimalToBinaryVector(uint32_t(10038));

    decltype(cmd) joined;
    std::ranges::copy(cmd, std::back_inserter(joined));
    std::ranges::copy(addr, std::back_inserter(joined));
    std::ranges::copy(data, std::back_inserter(joined));
    auto it = joined.begin();
    auto it_end = joined.end();

    // Reset cycle
    dut->eval();
    tfp->dump(0);

    // Run simulation for 30 clock cycles
    for (int i = 0; i < CYCLES; i++) {
        if (i > 2) {
            dut->rst = 1;
        }
        dut->sclk = (i % 2);
        if(!dut->rst || it == it_end) {
            dut->ss = 1;
        } else {
            dut->ss = 0;
        }
        if(dut->sclk && dut->rst && it != it_end) {
            dut->mosi = *it;
            it = std::next(it);
        } else if (!dut->sclk) {
            dut->mosi = 0;
        }
        dut->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
    }

    // Cleanup
    tfp->close();
    return 0;
}
