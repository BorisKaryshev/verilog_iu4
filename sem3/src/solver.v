`include "constants.v"

module solver (
    input clk,
    input rst,
    input [7:0] x,
    input [7:0] x_prev,
    input [7:0] y_prev,
    output reg [7:0] out,
    output wire has_result
);

reg [7:0] AA = `A_CONSTANT * `A_CONSTANT;
reg [7:0] AB = `A_CONSTANT * `B_CONSTANT;
reg [7:0] B = `B_CONSTANT;

wire [7:0] mul1_out;
wire [7:0] mul2_out;
wire [7:0] mul3_out;
assign out = mul1_out + mul2_out + mul3_out;

wire mul1_has_result;
wire mul2_has_result;
wire mul3_has_result;
assign has_result = mul1_has_result && mul2_has_result && mul3_has_result;

multiplier mul1 (
    .clk(clk),
    .a(AA),
    .b(y_prev),
    .reset(rst),
    .out(mul1_out),
    .has_result(mul1_has_result)
);

multiplier mul2 (
    .clk(clk),
    .a(AB),
    .b(x_prev),
    .reset(rst),
    .out(mul2_out),
    .has_result(mul2_has_result)
);

multiplier mul3 (
    .clk(clk),
    .a(B),
    .b(x),
    .reset(rst),
    .out(mul3_out),
    .has_result(mul3_has_result)
);

endmodule
