#include "Vcounter.h"
#include "verilated.h"
#include <verilated_vcd_c.h>
#include <memory>

constexpr int CYCLES = 100000;
constexpr int CLK_STEP_PS = 1;

inline char to_char(int var) {
    return static_cast<char>(var + '0');
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    auto dut = std::make_unique<Vcounter>();  // Instantiate DUT

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    dut->trace(tfp.get(), 99);
    tfp->open("counter.vcd");

    // Initialize signals
    dut->clk = 0;
    dut->in = 1;

    // Reset cycle
    dut->eval();
    tfp->dump(0);

    // Run simulation for 30 clock cycles
    for (int i = 0; i < CYCLES; i++) {
        dut->clk = !dut->clk;
        // if (i % 4 == 0) {
        //     dut->in = !dut->in;
        // }
        dut->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
    }

    // Cleanup
    tfp->close();
    return 0;
}
