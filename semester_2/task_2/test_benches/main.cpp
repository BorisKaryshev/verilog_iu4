#include <Vmain.h>

#include <array>
#include <limits>
#include <memory>
#include <vector>
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

void generate_vcd_example(const std::vector<uint8_t> &input_data) {
    auto module = std::make_unique<Vmain>();
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    const uint64_t CLK_IN_HALF_PERIOD_PS = 5000;
    const uint64_t CLK_OUT_HALF_PERIOD_PS = 8333;
    const uint64_t TOTAL_TIME_PS = 500000;
    uint64_t time_ps = 0;

    module->clk_in = 0;
    module->clk_out = 0;
    module->rst_in_n = 0;
    module->rst_out_n = 0;
    module->push = 0;
    module->data_in = 0;
    module->pop = 0;                     // pop initially low

    module->eval();
    tfp->dump(time_ps);

    time_ps += 20000;
    for (int i = 0; i < 2; i++) {
        module->clk_in = 0;
        module->clk_out = 0;
        module->eval();
        tfp->dump(time_ps);
        time_ps += 1000;
    }
    module->rst_in_n = 1;
    module->rst_out_n = 1;
    module->eval();
    tfp->dump(time_ps);

    auto data_it = input_data.begin();
    bool sending = true;
    uint64_t next_clk_in_edge = time_ps + CLK_IN_HALF_PERIOD_PS;
    uint64_t next_clk_out_edge = time_ps + CLK_OUT_HALF_PERIOD_PS;

    while (time_ps < TOTAL_TIME_PS) {
        uint64_t next_edge = std::min(next_clk_in_edge, next_clk_out_edge);
        time_ps = next_edge;
        bool clk_in_rising = false;
        bool clk_out_rising = false;
        if (time_ps == next_clk_in_edge) {
            module->clk_in = !module->clk_in;
            clk_in_rising = (module->clk_in == 1);   // rising edge?
            next_clk_in_edge += CLK_IN_HALF_PERIOD_PS;
        }
        if (time_ps == next_clk_out_edge) {
            module->clk_out = !module->clk_out;
            clk_out_rising = (module->clk_out == 1);
            next_clk_out_edge += CLK_OUT_HALF_PERIOD_PS;
        }

        // Drive push on rising clk_in
        if (clk_in_rising && sending && data_it != input_data.end()) {
            if (module->has_space) {
                module->push = 1;
                module->data_in = *data_it;
                ++data_it;
                if (data_it == input_data.end())
                    sending = false;
            } else {
                module->push = 0;   // no space, wait
            }
        } else if (!clk_in_rising && module->clk_in == 0) {
            // On falling edge, clear push
            module->push = 0;
        }

        // Drive pop on rising clk_out
        if (clk_out_rising) {
            if (module->has_value) {
                module->pop = 1;
                // data_out will be sampled inside eval()
            } else {
                module->pop = 0;
            }
        } else if (!clk_out_rising && module->clk_out == 0) {
            // On falling edge, clear pop
            module->pop = 0;
        }

        module->eval();
        tfp->dump(time_ps);
    }

    tfp->close();
}

class TestAsyncQueue : public ::testing::TestWithParam<int> {
public:
    void SetUp() override {
        module_ = std::make_unique<Vmain>();
    }
    void TearDown() override {
        module_ = nullptr;
    }

    std::vector<uint8_t> RunModuleWithEvalCallback(
            const std::vector<uint8_t>& input_data,
            auto callback
    ) {
        std::vector<uint8_t> output_data;
        module_->rst_in_n = 0;
        module_->rst_out_n = 0;
        module_->clk_in = 1;
        module_->clk_out = 1;
        callback(module_);

        module_->clk_in = 0;
        module_->clk_out = 0;
        module_->rst_in_n = 1;
        module_->rst_out_n = 1;
        callback(module_);

        module_->push = 0;
        module_->pop = 0;   // initially low

        auto send_it = input_data.begin();
        bool sending = true;

        for (int cycle = 0; cycle < 500; ++cycle) {
            bool clk_in_rising = (cycle % 2 == 0);   // cycle 0,2,4... -> rising edge
            bool clk_out_rising = (cycle % 3 == 0);  // cycle 0,3,6... -> rising edge

            module_->clk_in = clk_in_rising ? 1 : 0;
            module_->clk_out = clk_out_rising ? 1 : 0;

            // Drive push on rising clk_in
            if (clk_in_rising && sending && send_it != input_data.end()) {
                if (module_->has_space) {
                    module_->push = 1;
                    module_->data_in = *send_it;
                    ++send_it;
                    if (send_it == input_data.end())
                        sending = false;
                } else {
                    module_->push = 0;
                }
            } else if (!clk_in_rising && module_->clk_in == 0) {
                module_->push = 0;   // clear on falling edge
            }

            // Drive pop on rising clk_out
            if (clk_out_rising) {
                if (module_->has_value) {
                    module_->pop = 1;
                    output_data.push_back(module_->data_out);
                } else {
                    module_->pop = 0;
                }
            } else if (!clk_out_rising && module_->clk_out == 0) {
                module_->pop = 0;
            }

            callback(module_);
        }
        return output_data;
    }

    std::vector<uint8_t> RunModule(const std::vector<uint8_t>& input_data) {
        return RunModuleWithEvalCallback(input_data, [](auto& module) {
            module->eval();
        });
    }

protected:
    std::unique_ptr<Vmain> module_;
};

// TEST_F(TestLRUCacheModule, TestWithOnes) {
//     std::vector<DataType> input(1000, 1);
//     auto expected = GetCorrectSequence(input);
//     auto result = RunModule(input);
//
//     ASSERT_EQ(expected, result);
// }
//
// TEST_F(TestLRUCacheModule, TestWithZeros) {
//     std::vector<DataType> input(1000, 0);
//     auto expected = GetCorrectSequence(input);
//     auto result = RunModule(input);
//
//     ASSERT_EQ(expected, result);
// }
//
// TEST_F(TestLRUCacheModule, MiniTest) {
//     std::vector<DataType> input {1, 2, 3, 4, 4, 2, 3, 1, 1, 4, 20, 43, 1};
//     auto expected = GetCorrectSequence(input);
//     auto result = RunModule(input);
//
//     ASSERT_EQ(expected, result);
// }
//
// TEST_P(TestLRUCacheModule, MultipleIterations) {
//     auto input = GenerateRandomSequence(this->GetParam());
//     auto expected = GetCorrectSequence(input);
//     auto result = RunModule(input);
//
//     ASSERT_EQ(expected, result);
// }
//
// INSTANTIATE_TEST_SUITE_P(
//     IterationTests,
//     TestLRUCacheModule,
//     ::testing::Range(0, 10000),  // Test 100 iterations instead of 1,000,000
//     [](const ::testing::TestParamInfo<TestLRUCacheModule::ParamType>& info) {
//         return "Iteration_" + std::to_string(info.param);
//     }
// );

TEST_F(TestAsyncQueue, CreateMainVcdFile) {
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
