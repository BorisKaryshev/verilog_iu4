#include <Vmain.h>

#include <array>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>
#include <iostream>
#include <random>

#include <verilated.h>
#include <verilated_vcd_c.h>
#include <gtest/gtest.h>

constexpr int CYCLES = 1000;
constexpr int CLK_STEP_PS = 1;
constexpr int TEST_ITERATIONS = 10000;
constexpr uint8_t CACHE_SIZE = 7;

using DataType = uint8_t;

namespace {
std::array<DataType, CACHE_SIZE> RunOneIterationOfCache(std::array<DataType, CACHE_SIZE> data, DataType new_value) {
    auto it = std::ranges::find(data, new_value);
    if(it != data.begin()) {
        it++;
    }

    DataType prev = new_value;
    for(auto i = data.begin(); i != it; i = std::next(i)) {
        std::swap(prev, *i);
    }
    return data;
}

std::vector<std::array<DataType, CACHE_SIZE>> GetCorrectSequence(const std::vector<DataType>& data) {
    std::vector<std::array<DataType, CACHE_SIZE>> res;
    std::array<DataType, CACHE_SIZE> current_state {};

    for(auto i : data) {
        current_state = RunOneIterationOfCache(current_state, i);
        res.push_back(current_state);
    }

    return res;
}

std::vector<DataType> GenerateRandomSequence(uint64_t seq_size) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, std::numeric_limits<DataType>::max());

    std::vector<DataType> res;
    for(; seq_size > 0; seq_size--) {
        res.push_back(dist(rng));
    }
    return res;
}
}

void generate_vcd_example(const std::vector<uint8_t> data) {
    auto module = std::make_unique<Vmain>();  // Instantiate module

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    // Initialize signals
    module->clk = 1;
    module->rst = 0;
    auto it = data.begin();

    // Reset cycle
    module->eval();
    tfp->dump(0);

    // Run simulation for 30 clock cycles
    for (int i = 0; i < CYCLES; i++) {
        if(!module->clk && module->rst && it != data.end()) {
            module->x = *it;
            it = std::next(it);
        }
        if (i > 2) {
            module->rst = 1;
        }
        module->clk = (i % 2);
        module->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
    }

    // Cleanup
    tfp->close();
}

class TestLRUCacheModule : public testing::Test {
 public:
    void SetUp() override {
        module_ = std::make_unique<Vmain>();
    };
    void TearDown() override {
        module_ = nullptr;
    };


    std::vector<std::array<DataType, CACHE_SIZE>> RunModuleWithEvalCallback(
            const std::vector<DataType>& data,
            auto callback
    ) {
        std::vector<std::array<DataType, CACHE_SIZE>> res;
        module_->rst = 0;
        module_->clk = 1;
        callback(module_);

        module_->clk = 0;
        module_->rst = 1;
        callback(module_);

        for(auto i : data) {
            module_->clk = 1;
            module_->x = i;
            callback(module_);

            module_->clk = 0;
            callback(module_);

            std::array<DataType, CACHE_SIZE> current {};
            std::ranges::copy(module_->cache, current.begin());
            res.push_back(current);
        }

        return res;
    }

    std::vector<std::array<DataType, CACHE_SIZE>> RunModule(const std::vector<DataType>& data) {
        return RunModuleWithEvalCallback(data, [](auto& module){
            module->eval();
        });
    }

 protected:
    std::unique_ptr<Vmain> module_;

};

TEST_F(TestLRUCacheModule, TestWithOnes) {
    std::vector<DataType> input(1000, 1);
    auto expected = GetCorrectSequence(input);
    auto result = RunModule(input);

    ASSERT_EQ(expected, result);
}

TEST_F(TestLRUCacheModule, TestWithZeros) {
    std::vector<DataType> input(1000, 0);
    auto expected = GetCorrectSequence(input);
    auto result = RunModule(input);

    ASSERT_EQ(expected, result);
}

TEST_F(TestLRUCacheModule, MiniTest) {
    std::vector<DataType> input {1, 2, 3, 4, 4, 2, 3, 1, 1, 4, 20, 43, 1};
    auto expected = GetCorrectSequence(input);
    auto result = RunModule(input);

    ASSERT_EQ(expected, result);
}

TEST_F(TestLRUCacheModule, LargeTestWithRandomNumbers) {
    for(uint64_t i = 0; i < TEST_ITERATIONS; i++) {
        auto input = GenerateRandomSequence(i);
        auto expected = GetCorrectSequence(input);
        auto result = RunModule(input);

        ASSERT_EQ(expected, result);
    }
}

TEST_F(TestLRUCacheModule, CreateMainVcdFile) {
    std::vector<DataType> data = {
        1, 2, 3, 4, 5, 6, 7, 3, 3, 3, 4, 2, 1, 1, 1, 1, 1, 1
    };


    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module_->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    uint64_t simulation_step = 0;
    RunModuleWithEvalCallback(data, [&simulation_step, &tfp](auto& module) {
        module->eval();
        tfp->dump(CLK_STEP_PS * simulation_step);
        simulation_step++;
    });
}
