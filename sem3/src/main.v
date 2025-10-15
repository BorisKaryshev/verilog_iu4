`include "constants.v"

module main (
    input clk,
    input rst,
    input [7:0] x,
    output [7:0] out,
    output has_result
);

// odd - нечетный
// even - четный

reg [7:0] x_prev;
reg [7:0] y_zero = `Y_0_CONSTANT;

wire [7:0] odd_y;

reg has_first_result;
wire [7:0] even_y;

wire first_solver_has_result;
wire [7:0] first_solver_result;
wire first_solver_clk = clk && ~first_solver_has_result;
naive_solver first_solver(
    .clk(first_solver_clk),
    .y(y_zero),
    .x(x),
    .rst(rst),
    .out(first_solver_result),
    .has_result(first_solver_has_result)
);

wire odd_solver_has_result;
wire [7:0] odd_solver_result;
reg odd_can_run;
wire odd_solver_clk = clk && odd_can_run;;
solver odd_solver(
    .clk(odd_solver_clk),
    .rst(rst),
    .y_prev(odd_y),
    .x(x),
    .x_prev(x_prev),
    .out(odd_solver_result),
    .has_result(odd_solver_has_result)
);

wire even_solver_has_result;
wire [7:0] even_solver_result;
wire even_solver_clk = clk && first_solver_has_result;;
solver even_solver(
    .clk(even_solver_clk),
    .rst(rst),
    .y_prev(even_y),
    .x(x),
    .x_prev(x_prev),
    .out(even_solver_result),
    .has_result(even_solver_has_result)
);

assign has_result = first_solver_has_result || odd_solver_has_result || even_solver_has_result;
assign even_y = (~even_solver_has_result) ? y_zero : even_solver_result;
assign odd_y = (~odd_solver_has_result) ? odd_solver_result : first_solver_result;

reg is_current_odd;
assign out = (is_current_odd) ? odd_y : even_y;

always @ (posedge clk) begin
    if (!rst) begin // Active-low reset
        x_prev <= 0;
        odd_can_run <= 0;
        is_current_odd <= 1;
    end else begin
        is_current_odd <= ~is_current_odd;
        x_prev <= x;
        odd_can_run <= 1;
        has_first_result <= first_solver_has_result || has_first_result;
    end
end

endmodule
