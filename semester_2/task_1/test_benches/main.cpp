#include <Vmain.h>
#include <memory>
#include <vector>
#include <random>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <gtest/gtest.h>

constexpr int CLK_STEP_PS = 1;
using DataType = uint8_t;

void generate_vcd_example(const std::vector<uint8_t> data) {
    auto module = std::make_unique<Vmain>();

    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedVcdC>();
    module->trace(tfp.get(), 99);
    tfp->open("main.vcd");

    module->clk = 0;
    module->rst = 1;
    module->s_axis_tvalid = 0;
    module->s_axis_tdata = 0;
    module->m_axis_tready = 1;
    module->eval();
    tfp->dump(0);

    const int RESET_CYCLES = 4;
    for (int i = 0; i < RESET_CYCLES * 2; i++) {
        module->clk = !module->clk;
        module->eval();
        tfp->dump(static_cast<vluint64_t>(i * CLK_STEP_PS));
    }
    module->rst = 0;

    auto it = data.begin();
    bool sending_done = false;
    bool receiving_done = false;
    std::vector<uint8_t> received;

    int backpressure_counter = 0;
    const int BACKPRESSURE_INTERVAL = 5;
    const int BACKPRESSURE_DURATION = 2;

    for (int i = 0; i < 1000 && !(sending_done && receiving_done); i++) {
        module->clk = 1;
        module->eval();
        tfp->dump(static_cast<vluint64_t>(i * 2 * CLK_STEP_PS));

        if (!sending_done && module->s_axis_tready) {
            if (it != data.end()) {
                module->s_axis_tdata = *it;
                module->s_axis_tvalid = 1;
                ++it;
            } else {
                module->s_axis_tvalid = 0;
                sending_done = true;
            }
        }

        if (backpressure_counter % 5 < 2) {
            module->m_axis_tready = 0;
        } else {
            module->m_axis_tready = 1;
        }
        backpressure_counter++;

        module->clk = 0;
        module->eval();
        tfp->dump(static_cast<vluint64_t>(i * 2 * CLK_STEP_PS + CLK_STEP_PS));

        if (module->m_axis_tvalid && module->m_axis_tready) {
            uint8_t out = module->m_axis_tdata;
            received.push_back(out);
            if (received.size() == data.size())
                receiving_done = true;
        }
    }

    tfp->close();
    std::cout << "VCD generated. Sent " << data.size() << " bytes, got "
              << received.size() << " bytes." << std::endl;
}

class TestSkidBufferCacheModule : public ::testing::Test {
public:
    void SetUp() override {
        module_ = std::make_unique<Vmain>();
        reset();
    }

    void TearDown() override {
        module_ = nullptr;
    }

    void reset() {
        module_->rst = 1;
        for (int i = 0; i < 4; i++) tick();
        module_->rst = 0;
        tick();
        module_->s_axis_tvalid = 0;
        module_->s_axis_tdata = 0;
        module_->m_axis_tready = 1;
    }

    void tick() {
        module_->clk = 0;
        module_->eval();
        module_->clk = 1;
        module_->eval();
    }

    // Run one cycle with given input and output ready.
    // Returns true if output data was transferred this cycle.
    bool cycle(bool send_valid, DataType send_data, bool recv_ready, DataType& recv_data) {
        module_->s_axis_tvalid = send_valid;
        module_->s_axis_tdata = send_data;
        module_->m_axis_tready = recv_ready;
        tick();
        bool output_occurred = false;
        if (module_->m_axis_tvalid && module_->m_axis_tready) {
            recv_data = module_->m_axis_tdata;
            output_occurred = true;
        }
        return output_occurred;
    }

    // Simplified: send a word (if possible) and also receive any output that happens
    // during the same cycle. Returns true if the send was accepted.
    bool send(DataType data, DataType* recv_out = nullptr) {
        DataType dummy;
        bool out = cycle(true, data, true, dummy);
        if (recv_out) *recv_out = dummy;
        return module_->s_axis_tready; // after tick we can check if it was accepted? Actually we need before?
        // Better: we return true if the send was accepted. We can check by seeing if the module's
        // internal state advanced. For simplicity, we rely on the fact that tready is sampled.
        // But after tick, tready may have changed. We'll use a simpler method: run two cycles.
        // However, to keep tests readable, I'll rewrite tests using cycle() directly.
    }

    // Send a sequence and collect outputs.
    std::vector<DataType> run_sequence(const std::vector<DataType>& inputs, bool recv_always_ready = true) {
        reset();
        std::vector<DataType> outputs;
        auto it = inputs.begin();
        bool sending_done = false;
        while (!sending_done || outputs.size() < inputs.size()) {
            bool send_valid = (!sending_done && it != inputs.end());
            DataType send_data = send_valid ? *it : 0;
            DataType recv_data;
            bool recv_occurred = cycle(send_valid, send_data, recv_always_ready, recv_data);
            if (recv_occurred) {
                outputs.push_back(recv_data);
            }
            if (send_valid && module_->s_axis_tready) {
                ++it;
                if (it == inputs.end()) sending_done = true;
            }
        }
        return outputs;
    }

protected:
    std::unique_ptr<Vmain> module_;
};

TEST_F(TestSkidBufferCacheModule, ResetState) {
    reset();
    EXPECT_EQ(module_->s_axis_tready, 1);
    EXPECT_EQ(module_->m_axis_tvalid, 0);
}

TEST_F(TestSkidBufferCacheModule, SimpleTransfer) {
    reset();
    DataType recv_data;
    bool output = cycle(true, 0xAB, true, recv_data);
    EXPECT_TRUE(output);
    EXPECT_EQ(recv_data, 0xAB);
}

TEST_F(TestSkidBufferCacheModule, TwoWordTransfer) {
    std::vector<DataType> inputs = {0x12, 0x34};
    auto outputs = run_sequence(inputs);
    ASSERT_EQ(outputs.size(), 2);
    EXPECT_EQ(outputs[0], 0x12);
    EXPECT_EQ(outputs[1], 0x34);
}

TEST_F(TestSkidBufferCacheModule, BackpressureOnOutput) {
    reset();
    // Send first two words, but hold output ready low
    module_->m_axis_tready = 0;
    DataType recv_dummy;
    bool sent1 = cycle(true, 0x55, false, recv_dummy);
    EXPECT_TRUE(sent1);  // tready should be high (cnt<2)
    bool sent2 = cycle(true, 0xAA, false, recv_dummy);
    EXPECT_TRUE(sent2);  // second accepted, buffer full

    // Third send should be rejected
    bool sent3 = cycle(true, 0x77, false, recv_dummy);
    EXPECT_FALSE(sent3);

    // Release backpressure: now output should appear
    module_->m_axis_tready = 1;
    DataType out;
    bool recv1 = cycle(false, 0, true, out);
    EXPECT_TRUE(recv1);
    EXPECT_EQ(out, 0x55);
    bool recv2 = cycle(false, 0, true, out);
    EXPECT_TRUE(recv2);
    EXPECT_EQ(out, 0xAA);

    // Now buffer empty, third word can be sent
    bool sent4 = cycle(true, 0x77, true, out);
    EXPECT_TRUE(sent4);
    // And it should appear on next cycle
    bool recv3 = cycle(false, 0, true, out);
    EXPECT_TRUE(recv3);
    EXPECT_EQ(out, 0x77);
}

TEST_F(TestSkidBufferCacheModule, BackpressureOnInput) {
    reset();
    // Send two words normally
    DataType out;
    cycle(true, 0x11, true, out);
    cycle(true, 0x22, true, out);
    // Now stop sending, let outputs drain
    bool recv1 = cycle(false, 0, true, out);
    EXPECT_TRUE(recv1);
    EXPECT_EQ(out, 0x11);
    bool recv2 = cycle(false, 0, true, out);
    EXPECT_TRUE(recv2);
    EXPECT_EQ(out, 0x22);
    // No more output
    bool recv3 = cycle(false, 0, true, out);
    EXPECT_FALSE(recv3);
}

TEST_F(TestSkidBufferCacheModule, FullBufferDrain) {
    std::vector<DataType> inputs = {0xDE, 0xAD};
    auto outputs = run_sequence(inputs);
    ASSERT_EQ(outputs.size(), 2);
    EXPECT_EQ(outputs[0], 0xDE);
    EXPECT_EQ(outputs[1], 0xAD);
}

TEST_F(TestSkidBufferCacheModule, RandomSequenceTest) {
    const size_t SEQUENCE_LEN = 200;
    std::vector<DataType> inputs;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<DataType> dist(0, 255);
    for (size_t i = 0; i < SEQUENCE_LEN; ++i) {
        inputs.push_back(dist(gen));
    }
    auto outputs = run_sequence(inputs);
    ASSERT_EQ(outputs.size(), inputs.size());
    EXPECT_EQ(outputs, inputs);
}

TEST_F(TestSkidBufferCacheModule, CreateMainVcdFile) {
    std::vector<DataType> data;
    data.reserve(256);
    for (int i = 0; i < 256; ++i) {
        data.push_back(static_cast<DataType>(i));
    }
    generate_vcd_example(data);
}
