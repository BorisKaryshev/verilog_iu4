#include <Vmain.h>

#include <memory>
#include <vector>

#include <verilated.h>
#include <verilated_vcd_c.h>

constexpr int CYCLES = 1000;
long long int NUMBER = std::pow(3, 30);
constexpr int CLK_STEP_PS = 1;
constexpr int TEST_ITERATIONS_LIMIT = 1000000;


inline char to_char(int var) {
    return static_cast<char>(var + '0');
}

void generate_vcd_example() {
    auto module = std::make_unique<Vmain>();  // Instantiate module

    // Enable waveform tracing
    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module->trace(tfp.get(), 99);
    tfp->open("main.vcd");


    std::vector<uint8_t> data = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 3, 3, 3, 4, 2, 1
    };

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

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    generate_vcd_example();
    return 0;
}
