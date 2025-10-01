// #include "Vctr.h"
// #include "Vcounter.h"
// #include <Vmultiplier.h>
#include <Vmain.h>

#include <memory>

#include <verilated.h>
#include <verilated_vcd_c.h>

constexpr int CYCLES = 1000;
constexpr int CLK_STEP_PS = 1;

inline char to_char(int var) {
    return static_cast<char>(var + '0');
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    auto dut = std::make_unique<Vmain>();  // Instantiate DUT

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    dut->trace(tfp.get(), 99);
    tfp->open("ctr.vcd");

    // Initialize signals
    dut->clk = 0;
    dut->reset = 0;

    // Reset cycle
    dut->eval();
    tfp->dump(0);

    // Run simulation for 30 clock cycles
    uint8_t a = 0;
    uint8_t b = 2;
    uint8_t c = 1;
    for (int i = 0; i < CYCLES; i++) {
        if (i > 4) {
            c = 4;
        }
        dut->reset = 1;
        dut->clk = !dut->clk;
        if(dut->clk && i > 1) {
            dut->a = a;
            dut->b = b;
            dut->c = c;
            a++;
        }
        dut->eval();
        tfp->dump(i * CLK_STEP_PS + CLK_STEP_PS);  // Trace at mid-cycle
    }

    // Cleanup
    tfp->close();
    return 0;
}
