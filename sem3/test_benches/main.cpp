// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

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
constexpr uint64_t N_OF_TESTS = 100;
constexpr uint64_t MAX_SEQ_SIZE = 1000;
constexpr int CYCLES = 1000;
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


std::vector<uint8_t> run_module_over_numbers(const std::vector<uint8_t>& numbers) {
    auto module = std::make_unique<Vmain>();

    auto x_it = numbers.begin();

    module->clk = 1;
    module->rst = 1;
    module->x = *x_it;

    std::vector<uint8_t> result;
    bool is_finished = false;
    uint64_t cycles = CYCLES;
    while(!is_finished && cycles > 0) {
        module->clk = !module->clk;
        if(module->clk && x_it != numbers.end()) {
            module->x = *x_it;
            x_it = std::next(x_it);
        }
        if(module->clk && module->has_result) {
            result.push_back(module->out);
        }
        is_finished = (result.size() >= numbers.size() + 1);
        cycles--;
    }
    return result;
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_correct_sequence(uint64_t n_of_input_elements) {
    std::vector<uint8_t> input_seq;

    std::default_random_engine generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    uint8_t min = 0;
    uint8_t max = 255;
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
    module->rst = 1;

    auto x_it = input.begin();
    module->x = *x_it;
    module->eval();

    bool is_finished = false;

    std::vector<uint8_t> res;
    uint64_t cycles = CYCLES;
    while(!is_finished && cycles > 0) {
        module->clk = !module->clk;
        module->eval();

        if(module->clk && x_it != input.end()) {
            module->x = *x_it;
            x_it = std::next(x_it);
        }
        is_finished = (res.size() >= input.size() + 1);
    }
    cycles--;
    return res;
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
    module->rst = 0;
    module->x = *x_it;

    module->eval();
    tfp->dump(0);

    std::vector<uint8_t> result;
    bool is_finished = false;
    for (int i = 0; i < CYCLES && !is_finished; i++) {
        module->rst = 1;
        module->clk = (i % 2);
        if(module->clk && module->rst && x_it != numbers.end()) {
            module->x = *x_it;
            x_it = std::next(x_it);
        }
        module->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle

        if(module->has_result && module->clk) {
            result.push_back(module->out);
        }
        is_finished = (result.size() >= numbers.size() + 1);
    }

    // Cleanup
    tfp->close();
    return result;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    {
        auto [in, _] = generate_correct_sequence(10);
        create_example_vcd(in);
    }

    // std::cout << "Running test for " << N_OF_TESTS << " random sequeces with sizes from 1 to " << MAX_SEQ_SIZE << "(step " << MAX_SEQ_SIZE / N_OF_TESTS << ")" << std::endl;
    //
    // int barWidth = 70;
    // uint64_t curr_seq_size = 1;
    // for(std::uint64_t i = 0; i < N_OF_TESTS; i += 1, curr_seq_size += MAX_SEQ_SIZE/N_OF_TESTS) {
    //     auto [in, expected] = generate_correct_sequence(curr_seq_size);
    //     auto result = test_sequence(in);
    //     assert(expected.size() == result.size());
    //     for(uint64_t i = 0; i < expected.size(); i++) {
    //         assert(expected[i] == result[i]);
    //     }
    //
    //     float progress = static_cast<float>(i) / static_cast<float>(N_OF_TESTS);
    //
    //     std::cout << "[";
    //     int pos = barWidth * progress;
    //     for (int i = 0; i < barWidth; ++i) {
    //         if (i < pos) std::cout << "=";
    //         else if (i == pos) std::cout << ">";
    //         else std::cout << " ";
    //     }
    //     std::cout << "] " << int(progress * 100.0) << " %\r";
    //     std::cout.flush();
    //
    //     progress += 0.16; // for demonstration only
    // }
    // for (int i = 0; i < barWidth + 20; i++) {
    //     std::cout << ' ';
    // }
    // std::cout.flush();
    // std::cout << "\r" << "Success!" << std::endl << std::endl;

    // std::vector<uint8_t> input_seq = {0, 1, 2, 3, 5, 7, 12};
    // std::vector<uint8_t> expected = {0};
    // for(auto i : input_seq) {
    //     expected.push_back(
    //         A * expected.back() + B * i
    //     );
    // }
    // auto result = test_sequence(input_seq);
    // std::cout << "Pringing some test results." << std::endl;
    // std::cout << "Format: x, expected, result" << std::endl;
    //
    // for(uint64_t i = 0; i < std::max(expected.size(), result.size()); i++) {
    //     std::cout << vector_get(input_seq, i, "   ") << " "
    //         << vector_get(expected, i, "   ") << " "
    //         << vector_get(result, i, "   ") << std::endl;
    // }

    return 0;
}
