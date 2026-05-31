module main #(
    parameter DATA_WIDTH = 8,
    parameter ADDR_WIDTH = 2
) (
    input wire clk_in,
    input wire rst_in_n,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire push,
    output wire has_space,
    input wire clk_out,
    input wire rst_out_n,
    output wire [DATA_WIDTH-1:0] data_out,
    input wire pop,
    output wire has_value
);

wire [DATA_WIDTH-1:0] in_data_stage1, in_data_stage2, in_data_stage3;
wire in_valid_stage1, in_valid_stage2, in_valid_stage3;
wire in_ready_stage1, in_ready_stage2, in_ready_stage3;
wire [DATA_WIDTH-1:0] out_data_stage1;
wire [DATA_WIDTH-1:0] out_data_stage2, out_data_stage3;
wire out_valid_stage1, out_valid_stage2, out_valid_stage3;
wire out_ready_stage1, out_ready_stage2, out_ready_stage3;

axi_skid_buffer #(.DATA_WIDTH(DATA_WIDTH)) in_skid1 (
    .clk(clk_in), .rst(~rst_in_n),
    .s_axis_tdata(data_in), .s_axis_tvalid(push), .s_axis_tready(has_space),
    .m_axis_tdata(in_data_stage1), .m_axis_tvalid(in_valid_stage1), .m_axis_tready(in_ready_stage1)
);

axi_skid_buffer #(.DATA_WIDTH(DATA_WIDTH)) in_skid2 (
    .clk(clk_in), .rst(~rst_in_n),
    .s_axis_tdata(in_data_stage1), .s_axis_tvalid(in_valid_stage1), .s_axis_tready(in_ready_stage1),
    .m_axis_tdata(in_data_stage2), .m_axis_tvalid(in_valid_stage2), .m_axis_tready(in_ready_stage2)
);

axi_skid_buffer #(.DATA_WIDTH(DATA_WIDTH)) in_skid3 (
    .clk(clk_in), .rst(~rst_in_n),
    .s_axis_tdata(in_data_stage2), .s_axis_tvalid(in_valid_stage2), .s_axis_tready(in_ready_stage2),
    .m_axis_tdata(in_data_stage3), .m_axis_tvalid(in_valid_stage3), .m_axis_tready(in_ready_stage3)
);

axi_skid_buffer #(.DATA_WIDTH(DATA_WIDTH)) out_skid1 (
    .clk(clk_out), .rst(~rst_out_n),
    .s_axis_tdata(out_data_stage1), .s_axis_tvalid(out_valid_stage1), .s_axis_tready(out_ready_stage1),
    .m_axis_tdata(out_data_stage2), .m_axis_tvalid(out_valid_stage2), .m_axis_tready(out_ready_stage2)
);

axi_skid_buffer #(.DATA_WIDTH(DATA_WIDTH)) out_skid2 (
    .clk(clk_out), .rst(~rst_out_n),
    .s_axis_tdata(out_data_stage2), .s_axis_tvalid(out_valid_stage2), .s_axis_tready(out_ready_stage2),
    .m_axis_tdata(out_data_stage3), .m_axis_tvalid(out_valid_stage3), .m_axis_tready(out_ready_stage3)
);

axi_skid_buffer #(.DATA_WIDTH(DATA_WIDTH)) out_skid3 (
    .clk(clk_out), .rst(~rst_out_n),
    .s_axis_tdata(out_data_stage3), .s_axis_tvalid(out_valid_stage3), .s_axis_tready(out_ready_stage3),
    .m_axis_tdata(data_out), .m_axis_tvalid(has_value), .m_axis_tready(pop)
);

reg [DATA_WIDTH-1:0] mem [0:(1<<ADDR_WIDTH)-1];
reg [ADDR_WIDTH:0] wptr, rptr;

wire [ADDR_WIDTH:0] wptr_gray = (wptr >> 1) ^ wptr;
wire [ADDR_WIDTH:0] rptr_gray = (rptr >> 1) ^ rptr;

reg [ADDR_WIDTH:0] rptr_gray_sync1, rptr_gray_sync2;
reg [ADDR_WIDTH:0] wptr_gray_sync1, wptr_gray_sync2;

always @(posedge clk_in) begin
    rptr_gray_sync1 <= rptr_gray;
    rptr_gray_sync2 <= rptr_gray_sync1;
end

always @(posedge clk_out) begin
    wptr_gray_sync1 <= wptr_gray;
    wptr_gray_sync2 <= wptr_gray_sync1;
end

wire wfull = (wptr_gray == {~rptr_gray_sync2[ADDR_WIDTH:ADDR_WIDTH-1], rptr_gray_sync2[ADDR_WIDTH-2:0]});
wire rempty = (rptr_gray == wptr_gray_sync2);

wire [ADDR_WIDTH-1:0] waddr = wptr[ADDR_WIDTH-1:0];
assign in_ready_stage3 = !wfull;

always @(posedge clk_in or negedge rst_in_n) begin
    if (!rst_in_n)
        wptr <= 0;
    else if (in_valid_stage3 && !wfull) begin
        mem[waddr] <= in_data_stage3;
        wptr <= wptr + 1;
    end
end

wire [ADDR_WIDTH-1:0] raddr = rptr[ADDR_WIDTH-1:0];
assign out_data_stage1 = mem[raddr];
assign out_valid_stage1 = !rempty;

always @(posedge clk_out or negedge rst_out_n) begin
    if (!rst_out_n)
        rptr <= 0;
    else if (out_ready_stage1 && !rempty)
        rptr <= rptr + 1;
end

endmodule
