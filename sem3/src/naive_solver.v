`include "constants.v"

module naive_solver (
    input clk,
    input rst,
    input [7:0] y,
    input [7:0] x,
    output reg [7:0] out,
    output wire has_result
);

reg [7:0] A = `A_CONSTANT;
reg [7:0] B = `B_CONSTANT;

wire [7:0] mul1_out;
wire [7:0] mul2_out;
assign out = mul1_out + mul2_out;

wire mul1_has_result;
wire mul2_has_result;
assign has_result = mul1_has_result && mul2_has_result;

multiplier mul1 (
    .clk(clk),
    .a(A),
    .b(y),
    .reset(rst),
    .out(mul1_out),
    .has_result(mul1_has_result)
);

multiplier mul2 (
    .clk(clk),
    .a(B),
    .b(x),
    .reset(rst),
    .out(mul2_out),
    .has_result(mul2_has_result)
);

endmodule
