`include "constants.v"

module solver(
    input clk,
    input rst,
    input [7:0] y_minus_one,
    input [7:0] x_minus_one,
    input [7:0] x,
    output reg [7:0] out
);

reg [7:0] B;

reg [7:0] A_B;
reg [7:0] A_square;

wire [7:0] mul1_out;
wire [7:0] mul2_out;
wire [7:0] mul3_out;

wire mul1_ready;
wire mul2_ready;
wire mul3_ready;

multiplier mul1 (
    .clk(clk),
    .a(A_square),
    .b(y_minus_one),
    .reset(rst),
    .out(mul1_out),
    .is_ready(mul1_ready)
);

multiplier mul2 (
    .clk(clk),
    .a(A_B),
    .b(x_minus_one),
    .reset(rst),
    .out(mul2_out),
    .is_ready(mul2_ready)
);

multiplier mul3 (
    .clk(clk),
    .a(B),
    .b(x),
    .reset(rst),
    .out(mul3_out),
    .is_ready(mul3_ready)
);

always @ (posedge clk) begin
    if (!rst) begin
        out <= 0;
        B <= `B;
        A_B <= `A * `B;
        A_square <= `A * `A;
    end else begin
        out <= mul1_out + mul2_out + mul3_out;
    end
end

endmodule
