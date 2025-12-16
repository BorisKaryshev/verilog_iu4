`include "constants.v"

module solver (
    input clk,
    input rst_n,
    input enable_input,
    input [7:0] x,
    input [7:0] x_prev,
    input [7:0] y_prev,
    output [7:0] out,
    output has_result
);

reg [7:0] AA = `A_CONSTANT * `A_CONSTANT;
reg [7:0] AB = `A_CONSTANT * `B_CONSTANT;
reg [7:0] B = `B_CONSTANT;

reg [1:0] inter_cnt;
wire mul_enable_input = enable_input;

wire [7:0] mul1_result;
multiplier mul1 (
    .clk(clk),
    .a(AA),
    .b(y_prev),
    .enable_input(mul_enable_input),
    .rst_n(rst_n),
    .out(mul1_result)
);

wire [7:0] mul2_result;
multiplier mul2 (
    .clk(clk),
    .a(AB),
    .b(x_prev),
    .enable_input(mul_enable_input),
    .rst_n(rst_n),
    .out(mul2_result)
);

wire [7:0] mul3_result;
multiplier mul3 (
    .clk(clk),
    .a(B),
    .b(x),
    .enable_input(mul_enable_input),
    .rst_n(rst_n),
    .out(mul3_result)
);

assign has_result = (inter_cnt == 2'b10);
assign out = mul1_result + mul2_result + mul3_result;

always @ (posedge clk or negedge rst_n) begin
    if (!rst_n) begin // Active-low reset
        inter_cnt <= 0;
    end else begin
        if (enable_input && (inter_cnt == 2'b00)) begin
            inter_cnt <= inter_cnt + 1;
        end else if (enable_input && has_result) begin
            inter_cnt <= 2'b01;
        end else if(has_result) begin
            inter_cnt <= 0;
        end else if(inter_cnt != 2'b00) begin
            inter_cnt <= inter_cnt + 1;
        end
    end
end

endmodule
