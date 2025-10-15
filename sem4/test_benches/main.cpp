// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <iostream>

#include <tuple>
#include <verilated.h>
#include <verilated_vcd_c.h>

constexpr int CYCLES = 1000;
long long int NUMBER = std::pow(3, 30);
constexpr int CLK_STEP_PS = 1;
constexpr int TEST_ITERATIONS_LIMIT = 1000000;


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

void generate_vcd_example(uint8_t cmd, uint16_t addr, uint32_t data) {
    auto module = std::make_unique<Vmain>();  // Instantiate module

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    // Initialize signals
    module->sclk = 1;
    module->rst = 0;
    module->ss = 1;

    auto cmd_vector = decimalToBinaryVector(cmd);
    auto addr_vector = decimalToBinaryVector(addr);
    auto data_vector = decimalToBinaryVector(data);

    decltype(cmd_vector) joined;
    std::ranges::copy(cmd_vector, std::back_inserter(joined));
    std::ranges::copy(addr_vector, std::back_inserter(joined));
    std::ranges::copy(data_vector, std::back_inserter(joined));
    auto it = joined.begin();
    auto it_end = joined.end();

    // Reset cycle
    module->eval();
    tfp->dump(0);

    // Run simulation for 30 clock cycles
    for (int i = 0; i < CYCLES; i++) {
        if (i > 2) {
            module->rst = 1;
        }
        module->sclk = (i % 2);
        if(!module->rst || it == it_end) {
            module->ss = 1;
        } else {
            module->ss = 0;
        }
        if(module->sclk && module->rst && it != it_end) {
            module->mosi = *it;
            it = std::next(it);
        } else if (!module->sclk) {
            module->mosi = 0;
        }
        module->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
    }

    // Cleanup
    tfp->close();
}

std::tuple<uint8_t, uint16_t, uint32_t> run_test(uint8_t cmd, uint16_t addr, uint32_t data) {
    auto module = std::make_unique<Vmain>();  // Instantiate module

    // Initialize signals
    module->sclk = 1;
    module->rst = 0;
    module->ss = 1;

    auto cmd_vector = decimalToBinaryVector(cmd);
    auto addr_vector = decimalToBinaryVector(addr);
    auto data_vector = decimalToBinaryVector(data);

    decltype(cmd_vector) joined;
    std::ranges::copy(cmd_vector, std::back_inserter(joined));
    std::ranges::copy(addr_vector, std::back_inserter(joined));
    std::ranges::copy(data_vector, std::back_inserter(joined));
    auto it = joined.begin();
    auto it_end = joined.end();

    // Reset cycle
    module->eval();
    // Run simulation for 30 clock cycles
    for (int i = 0; i < CYCLES && !module->is_ready; i++) {
        if (i > 2) {
            module->rst = 1;
        }
        module->sclk = (i % 2);
        if(!module->rst || it == it_end) {
            module->ss = 1;
        } else {
            module->ss = 0;
        }
        if(module->sclk && module->rst && it != it_end) {
            module->mosi = *it;
            it = std::next(it);
        } else if (!module->sclk) {
            module->mosi = 0;
        }
        module->eval();
    }
    return std::tuple<uint8_t, uint16_t, uint32_t>(
            module->command,
            module->address,
            module->data
    );
}

template <typename T>
constexpr T get_step() {
    return std::max(
            T(1),
            static_cast<T>(std::numeric_limits<T>::max() / TEST_ITERATIONS_LIMIT)
    );
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    uint8_t cmd = 0;
    uint16_t addr = 0;
    uint32_t data = 0;

    constexpr uint8_t cmd_step = get_step<uint8_t>();
    constexpr uint16_t addr_step = get_step<uint16_t>();
    constexpr uint32_t data_step = get_step<uint32_t>();

    std::cout << "Running " << TEST_ITERATIONS_LIMIT << " iterations of test" << std::endl;

    int barWidth = 70;

    for(int i = 0; i < TEST_ITERATIONS_LIMIT; i++) {
        auto [cmd_res, addr_res, data_res] = run_test(cmd, addr, data);

        assert(cmd == cmd_res);
        assert(addr == addr_res);
        assert(data == data_res);

        cmd += cmd_step;
        addr += addr_step;
        data += data_step;

        float progress = static_cast<float>(i) / static_cast<float>(TEST_ITERATIONS_LIMIT);

        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();

        progress += 0.16; // for demonstration only
    }
    for (int i = 0; i < barWidth + 20; i++) {
        std::cout << ' ';
    }
    std::cout.flush();
    std::cout << "\r" << "Success!" << std::endl;
    std::cout << "Generating example vcd file: main.vcd" << std::endl;
    std::cout << "Input data is: {\"cmd\": 38, \"addr\": 238, \"data\": 10038}" << std::endl;

    generate_vcd_example(38, 238, 10038);
    return 0;
}
