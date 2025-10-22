// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

#include <execution>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <iterator>
#include <memory>

#include <random>
#include <sstream>
#include <verilated.h>
#include <verilated_vcd_c.h>

constexpr uint8_t A = 1;
constexpr uint8_t B = 1;
constexpr uint64_t N_OF_TESTS = 10;
constexpr uint64_t N_OF_SEQ_SIZES = std::pow(10, 3);
constexpr uint64_t MAX_SEQ_SIZE = std::pow(10, 5);
constexpr int CYCLES_LIMIT = std::pow(10, 7);
constexpr int CYCLES_FOR_VCD = 20 * std::pow(10, 3);
constexpr int CLK_STEP_PS = 1;

template <typename T>
std::string vector_get(const std::vector<T>& arr, uint64_t index, std::string if_not_found = "") {
    if(index > 0 && index < arr.size()) {
        std::stringstream stream;
        stream << std::setw(3) << std::setfill(' ') << arr[index];
        return stream.str();
    }
    return if_not_found;
}

inline char to_char(int var) {
    return static_cast<char>(var + '0');
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


std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_correct_sequence(uint64_t n_of_input_elements) {
    std::vector<uint8_t> input_seq;

    std::default_random_engine generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    uint8_t min = 0;
    uint8_t max = 10;
    std::uniform_int_distribution<uint8_t> distribution(min, max);

    for(uint64_t i = 0; i < n_of_input_elements; i++) {
        input_seq.push_back(distribution(generator));
    }
    std::vector<uint8_t> res = {0};
    for(auto i : input_seq) {
        res.push_back(
            A*res.back() + B * i
        );
    }
    return std::make_pair(input_seq, res);
}

std::vector<uint8_t> test_sequence(const std::vector<uint8_t>& input) {
    auto module = std::make_unique<Vmain>();
    module->clk = 1;
    module->rst_n = 1;

    auto x_it = input.begin();
    module->clk = 1;
    module->rst_n = 1;
    module->x = *x_it;

    module->eval();
    module->rst_n = 0;

    std::vector<uint8_t> result {0};
    bool is_finished = false;
    for (int i = 0; i < CYCLES_LIMIT && !is_finished; i++) {
        if(i > 1) {
            module->rst_n = 1;
        }
        module->clk = (i % 2);
        if(module->clk && module->rst_n && x_it != input.end()) {
            module->x = *x_it;
            x_it = std::next(x_it);
        }
        module->eval();

        if(module->has_result && module->clk) {
            result.push_back(module->out);
        }
        is_finished = (result.size() == (input.size() + 1));
    }
    return result;
}

std::vector<uint8_t> create_example_vcd(const std::vector<uint8_t>& numbers) {
    auto module = std::make_unique<Vmain>();

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    auto x_it = numbers.begin();
    module->clk = 1;
    module->rst_n = 1;
    module->x = *x_it;

    module->eval();
    tfp->dump(0);
    module->rst_n = 0;

    std::vector<uint8_t> result;
    for (int i = 0; i < CYCLES_FOR_VCD; i++) {
        if(i > 1) {
            module->rst_n = 1;
        }
        module->clk = (i % 2);
        if(module->clk && module->rst_n && x_it != numbers.end()) {
            module->x = *x_it;
            x_it = std::next(x_it);
        }
        module->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle

        if(module->has_result && module->clk && result.size() < (numbers.size())) {
            result.push_back(module->out);
        }
    }

    // Cleanup
    tfp->close();
    return result;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    const uint64_t seq_size_step = std::max(static_cast<uint64_t>(1), static_cast<uint64_t>(MAX_SEQ_SIZE / N_OF_SEQ_SIZES));
    for(std::uint64_t i = 1; i < N_OF_TESTS; i++) {
        for(std::uint64_t j = i; j < MAX_SEQ_SIZE; j += seq_size_step) {
            auto [in, expected] = generate_correct_sequence(j);
            auto result = test_sequence(in);
            assert(expected.size() == result.size());
            for(uint64_t i = 0; i < expected.size(); i++) {
                assert(expected[i] == result[i]);
            }
        }

    }
    std::cout << "Running test for large sequence with 1.000.000 elements" << std::endl;
    {
        auto [in, expected] = generate_correct_sequence(1000000);
        auto result = test_sequence(in);
        assert(expected.size() == result.size());
        for(uint64_t i = 0; i < expected.size(); i++) {
            assert(expected[i] == result[i]);
        }
    }
    std::cout << "Success!" << std::endl << std::endl;

    create_example_vcd(input_seq);

    return 0;
}
