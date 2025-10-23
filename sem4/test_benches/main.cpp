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
#include <fstream>

#include <verilated.h>
#include <verilated_vcd_c.h>
#include <gtest/gtest.h>

constexpr int CYCLES = 1000;
long long int NUMBER = std::pow(3, 30);
constexpr int CLK_STEP_PS = 1;
constexpr int TEST_ITERATIONS_LIMIT = 1000;  // Reduced for faster testing

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

    // Run simulation for clock cycles
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
    // Run simulation for clock cycles
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

// Test single example case
TEST(MainTest, SingleExample) {
    uint8_t cmd = 38;
    uint16_t addr = 238;
    uint32_t data = 10038;

    auto [cmd_res, addr_res, data_res] = run_test(cmd, addr, data);

    EXPECT_EQ(cmd, cmd_res);
    EXPECT_EQ(addr, addr_res);
    EXPECT_EQ(data, data_res);
}

// Test with zero values
TEST(MainTest, ZeroValues) {
    uint8_t cmd = 0;
    uint16_t addr = 0;
    uint32_t data = 0;

    auto [cmd_res, addr_res, data_res] = run_test(cmd, addr, data);

    EXPECT_EQ(cmd, cmd_res);
    EXPECT_EQ(addr, addr_res);
    EXPECT_EQ(data, data_res);
}

// Test with maximum values
TEST(MainTest, MaxValues) {
    uint8_t cmd = std::numeric_limits<uint8_t>::max();
    uint16_t addr = std::numeric_limits<uint16_t>::max();
    uint32_t data = std::numeric_limits<uint32_t>::max();

    auto [cmd_res, addr_res, data_res] = run_test(cmd, addr, data);

    EXPECT_EQ(cmd, cmd_res);
    EXPECT_EQ(addr, addr_res);
    EXPECT_EQ(data, data_res);
}

// Parameterized test for multiple iterations
class MainParameterizedTest : public ::testing::TestWithParam<int> {};

TEST_P(MainParameterizedTest, MultipleIterations) {
    const int iteration = GetParam();

    constexpr uint8_t cmd_step = get_step<uint8_t>();
    constexpr uint16_t addr_step = get_step<uint16_t>();
    constexpr uint32_t data_step = get_step<uint32_t>();

    uint8_t cmd = static_cast<uint8_t>(iteration * cmd_step);
    uint16_t addr = static_cast<uint16_t>(iteration * addr_step);
    uint32_t data = static_cast<uint32_t>(iteration * data_step);

    auto [cmd_res, addr_res, data_res] = run_test(cmd, addr, data);

    EXPECT_EQ(cmd, cmd_res);
    EXPECT_EQ(addr, addr_res);
    EXPECT_EQ(data, data_res);
}

// Instantiate the parameterized test with a subset of iterations
INSTANTIATE_TEST_SUITE_P(
    IterationTests,
    MainParameterizedTest,
    ::testing::Range(0, 100000),  // Test 100 iterations instead of 1,000,000
    [](const ::testing::TestParamInfo<MainParameterizedTest::ParamType>& info) {
        return "Iteration_" + std::to_string(info.param);
    }
);

// Test VCD generation
TEST(MainTest, GenerateVcd) {
    EXPECT_NO_THROW({
        generate_vcd_example(38, 238, 10038);
    });

    // You could also add a check to verify the file was created
    std::ifstream file("main.vcd");
    EXPECT_TRUE(file.good()) << "VCD file was not created successfully";
}
