// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

#include <memory>

#include <verilated.h>
#include <verilated_vcd_c.h>

constexpr int CYCLES = 1000;
long long int NUMBER = std::pow(3, 30);
constexpr int CLK_STEP_PS = 1;

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

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    auto dut = std::make_unique<Vmain>();  // Instantiate DUT

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    dut->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    // Initialize signals
    dut->clk = 1;
    dut->rst = 0;
    int curr_x = 1;
    dut->x = curr_x;

    // Reset cycle
    dut->eval();
    tfp->dump(0);

    // Run simulation for 30 clock cycles
    for (int i = 0; i < CYCLES; i++) {
        if (i > 2) {
            dut->rst = 1;
        }
        dut->clk = (i % 2);
        if(dut->clk && dut->rst) {
            dut->x = curr_x;
            curr_x++;
        }
        dut->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
    }

    // Cleanup
    tfp->close();
    return 0;
}
