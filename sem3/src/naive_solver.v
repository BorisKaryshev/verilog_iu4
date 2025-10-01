`include "constants.v"

module naive_solver(
    input clk,
    input rst,
    input [7:0] y_minus_one,
    input [7:0] x,
    output reg [7:0] out
);

reg [7:0] A;
reg [7:0] B;

wire [7:0] mul1_out;
wire [7:0] mul2_out;

multiplier mul1 (
    .clk(clk),
    .a(A),
    .b(y_minus_one),
    .reset(rst),
    .out(mul1_out)
);

multiplier mul2 (
    .clk(clk),
    .a(B),
    .b(x),
    .reset(rst),
    .out(mul2_out)
);

always @ (posedge clk) begin
    if (!rst) begin
        out <= 0;
        A <= `A;
        B <= `B;
    end else begin
        out <= mul1_out + mul2_out;
    end
end

endmodule
