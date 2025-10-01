`include "multiplier.v"
`include "additor.v"

module main (
    input clk,
    input [8:0] a,
    input [8:0] b,
    input [8:0] c,
    input reset,
    output reg [8:0] out
);

wire [8:0] mul_res;
reg [8:0] add_input;
reg [8:0] m1;
reg [8:0] m2;

multiplier mul(
    .clk(clk),
    .a(a),
    .b(b),
    .reset(reset),
    .out(mul_res)
);
additor add(
    .clk(clk),
    .a(mul_res),
    .b(add_input),
    .reset(reset),
    .out(out)
);

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        m1 <= 0;
        m2 <= 0;
        out <= 0; // Reset to 0
    end else begin
        m1 <= c;
        m2 <= m1;
        add_input <= m2;
    end
end

endmodule
