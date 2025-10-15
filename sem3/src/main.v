`include "constants.v"

module main (
    input clk,
    input rst_n,
    input [7:0] x,
    output [7:0] out,
    output has_result
);


reg [7:0] odd_y; // Нечетное
reg [7:0] even_y; // Четное
reg [7:0] prev_x;
reg is_odd;

reg odd_delay;
wire odd_enable_input = (is_odd && odd_delay);
/* verilator lint_off UNUSEDSIGNAL */
wire odd_has_result;
/* verilator lint_on UNUSEDSIGNAL */
solver odd_solv (
    .clk(clk),
    .rst_n(rst_n),
    .enable_input(odd_enable_input),
    .x(x),
    .x_prev(prev_x),
    .y_prev(odd_y),
    .out(odd_y),
    .has_result(odd_has_result)
);

wire first_has_result;
wire first_enable = (!is_odd) && (!first_has_result);
wire first_clk = clk && !first_has_result;
reg [7:0] first_y;
naive_solver first_solver(
    .clk(first_clk),
    .rst_n(rst_n),
    .enable_input(first_enable),
    .x(x),
    .y_prev(even_y),
    .out(first_y),
    .has_result(first_has_result)
);

wire even_enable_input = (!is_odd && first_has_result);
/* verilator lint_off UNUSEDSIGNAL */
wire even_has_result;
/* verilator lint_on UNUSEDSIGNAL */
reg read_first_result;
wire [7:0] even_input = (read_first_result) ? (first_y) : (even_y);
solver even_solv (
    .clk(clk),
    .rst_n(rst_n),
    .enable_input(even_enable_input),
    .x(x),
    .x_prev(prev_x),
    .y_prev(even_input),
    .out(even_y),
    .has_result(even_has_result)
);

assign has_result = (first_has_result || odd_has_result || even_has_result);
assign out = (is_odd) ? (odd_y) : (
    (read_first_result) ? (first_y) : (even_y)
);

always @ (posedge clk or negedge rst_n) begin
    if (!rst_n) begin // Active-low reset
        is_odd <= 0;
        prev_x <= 0;
        odd_delay <= 0;
        read_first_result <= 1;
        // odd_y <= `Y_0_CONSTANT;
        // even_y <= `Y_0_CONSTANT;
    end else begin
        if(even_enable_input) begin
            read_first_result <= 0;
        end
        odd_delay <= 1;
        is_odd <= ~is_odd;
        prev_x <= x;
    end
end

endmodule
