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

class TestLRUCacheModule : public ::testing::TestWithParam<int> {
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

TEST_F(TestLRUCacheModule, TestResetBehavior) {
    // After reset, all cache entries should be zero
    module_->rst = 0;
    module_->clk = 1;
    module_->eval();

    module_->clk = 0;
    module_->rst = 1;
    module_->eval();

    std::array<DataType, CACHE_SIZE> current {};
    std::ranges::copy(module_->cache, current.begin());
    std::array<DataType, CACHE_SIZE> expected = {};
    EXPECT_EQ(expected, current);
}

TEST_F(TestLRUCacheModule, TestRepeatInput) {
    std::vector<DataType> input(100, 42);
    auto expected = GetCorrectSequence(input);
    auto result = RunModule(input);
    ASSERT_EQ(expected, result);
}

TEST_F(TestLRUCacheModule, TestEmptyInput) {
    std::vector<DataType> input;
    auto result = RunModule(input);
    ASSERT_TRUE(result.empty());
}

TEST_F(TestLRUCacheModule, TestMaxValueInput) {
    std::vector<DataType> input(10, std::numeric_limits<DataType>::max());
    auto expected = GetCorrectSequence(input);
    auto result = RunModule(input);
    ASSERT_EQ(expected, result);
}
