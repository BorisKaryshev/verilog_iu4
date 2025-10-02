// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>

#include <verilated.h>
#include <verilated_vcd_c.h>

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

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    generate_example_vcd(NUMBER);

    std::uint64_t test_end = std::pow(10, 6);

    std::cout << "Running test for all numbers from 0 to " << test_end << std::endl;

    int barWidth = 70;
    for(std::uint64_t i = 0; i < test_end; i += 1) {
        bool expected = ((i % 3) == 0);
        assert(
            test_number(i) == expected
        );

        float progress = static_cast<float>(i) / static_cast<float>(test_end);

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
    std::cout << "\r" << "Success!" << std::endl << std::endl;

    std::vector<std::uint64_t> numbers_to_print = {0, 1, 2, 3, 4, 9, 127, 128, 300, 30001, 3000003};
    std::cout << "Pringing some test results." << std::endl;
    std::cout << "Format: number, is_dividable_by_three, is_dividable_by_three_verilog" << std::endl;

    for(auto num : numbers_to_print) {
        std::cout
            << std::left << std::setw(10) << std::setfill(' ') << num
            << ' '
            << convert_bool_to_stirng_with_padding((num % 3) == 0)
            << ' '
            << convert_bool_to_stirng_with_padding(test_number(num)) << std::endl;
    }

    return 0;
}
