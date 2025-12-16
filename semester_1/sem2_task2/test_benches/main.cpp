#include <Vmain.h>

#include <memory>
#include <vector>
#include <cmath>
#include <cstdint>

#include <verilated.h>
#include <verilated_vcd_c.h>
#include <gtest/gtest.h>

constexpr int CYCLES = 1000;
constexpr int CLK_STEP_PS = 1;
constexpr long long int NUMBER = 123456; // is dividable by three

inline char to_char(int var) {
    return static_cast<char>(var + '0');
}

std::string_view convert_bool_to_stirng_with_padding(bool value) {
    if(value) return " true";
    return "false";
}

std::vector<int> decimalToBinaryVector(long long int decimal_number) {
    std::vector<int> binary_vector;

    if (decimal_number == 0) {
        binary_vector.push_back(0);
        return binary_vector;
    }

    while (decimal_number > 0) {
        binary_vector.push_back(decimal_number % 2); // Get the remainder (0 or 1)
        decimal_number /= 2;                         // Divide by 2 for the next bit
    }

    std::reverse(binary_vector.begin(), binary_vector.end()); // Reverse to get correct order
    return binary_vector;
}

bool test_number(std::uint64_t num) {
    auto dut = std::make_unique<Vmain>();  // Instantiate DUT

    // Initialize signals
    dut->clk = 0;
    dut->reset = 0;
    dut->signal = 0;

    // Reset cycle
    dut->eval();

    std::vector<int> bits = decimalToBinaryVector(num);
    auto signal = bits.begin();

    dut->reset = 1;
    dut->eval();
    dut->begining = 1;
    int is_finished = 0;
    int is_running = 0;
    // Run simulation for 30 clock cycles
    while(!dut->has_result) {
        dut->clk = !dut->clk;
        if(dut->clk && signal != bits.end() && is_running) {
            dut->signal = *(signal++);
        } else if (is_finished != 1 && signal == bits.end() && dut->clk) {
            dut->finish = 1;
            is_finished += dut->clk;
        } else if(is_finished == 1) {
            dut->finish = 0;
        }
        dut->eval();
        dut->begining = 0;
        if(!is_finished) is_running = 1;
    }

    return dut->is_dividable_by_three;
}

void generate_example_vcd(std::uint64_t num) {
    auto dut = std::make_unique<Vmain>();  // Instantiate DUT

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    dut->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    // Initialize signals
    dut->clk = 0;
    dut->reset = 0;
    dut->signal = 0;

    // Reset cycle
    dut->eval();
    tfp->dump(0);

    std::vector<int> bits = decimalToBinaryVector(NUMBER);
    auto signal = bits.begin();

    dut->reset = 1;
    dut->eval();
    tfp->dump(CLK_STEP_PS);  // Trace at mid-cycle
    dut->begining = 1;
    int is_finished = 0;
    int is_running = 0;
    // Run simulation for 30 clock cycles
    for (int i = 1; i < CYCLES; i++) {
        dut->clk = (i % 2);
        if(dut->clk && signal != bits.end() && is_running) {
            dut->signal = *(signal++);
        } else if (is_finished != 1 && signal == bits.end() && dut->clk) {
            dut->finish = 1;
            is_finished += dut->clk;
        } else if(is_finished == 1) {
            dut->finish = 0;
        }
        dut->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
        dut->begining = 0;
        if(!is_finished) is_running = 1;
    }

    // Cleanup
    tfp->close();
}

class DivisibleByThreeTest : public ::testing::Test {
};

class DivisibleByThreeTestWithParametr : public ::testing::TestWithParam<int> {
};

TEST_F(DivisibleByThreeTest, VcdGeneration) {
    // This test generates the VCD file - we just verify it doesn't crash
    ASSERT_NO_THROW(generate_example_vcd(NUMBER));
}

TEST_P(DivisibleByThreeTestWithParametr, RangeTest) {
    bool expected = ((GetParam() % 3) == 0);
    EXPECT_EQ(test_number(GetParam()), expected)
        << "Failed for number: " << GetParam()
        << ", expected: " << (expected ? "true" : "false")
        << ", got: " << (test_number(GetParam()) ? "true" : "false");
}

INSTANTIATE_TEST_SUITE_P(
    IterationTests,
    DivisibleByThreeTestWithParametr,
    ::testing::Range(0, 1000000, 127),  // Test 1,000,000
    [](const ::testing::TestParamInfo<DivisibleByThreeTestWithParametr::ParamType>& info) {
        return "Iteration_" + std::to_string(info.param);
    }
);

// Optional: Performance test for larger ranges
TEST_F(DivisibleByThreeTest, PerformanceTest) {
    std::vector<std::uint64_t> large_numbers = {1000000, 10000000, 100000000, 1 << 30};

    for(auto num : large_numbers) {
        bool expected = (num % 3) == 0;
        bool actual = test_number(num);

        EXPECT_EQ(actual, expected)
            << "Performance test failed for large number: " << num;
    }
}
