`include "solver.v"
`include "naive_solver.v"


module main (
    input clk,
    input rst,
    input [7:0] x,
    output reg [7:0] out
);

reg is_current_input_odd;

reg read_odd;
reg read_even;

reg [7:0] even_x;
reg [7:0] odd_x;

reg [7:0] even_y; // Четный
reg [7:0] odd_y;  // Нечетный

solver even_solver(
    .clk(clk),
    .rst(rst),
    .y_minus_one(even_y),
    .x_minus_one(odd_x),
    .x(x),
    .read_input(read_even),
    .out(even_y)
);

solver odd_solver(
    .clk(clk),
    .rst(rst),
    .y_minus_one(odd_y),
    .x_minus_one(even_x),
    .x(x),
    .read_input(read_odd),
    .out(odd_y)
);

reg [7:0] first_y;
naive_solver first_solver(
    .clk(clk),
    .rst(rst),
    .y_minus_one(odd_y),
    .x(x),
    .out(first_y)
);

reg [2:0] is_begining_of_sequence_ready; // Then this var equals mul delay + 1 it is ready. So then == 2'b11

always @ (posedge clk) begin
    if (!rst) begin // Active-low reset
        is_current_input_odd <= 0;
        even_x <= 0;
        odd_x <= 0;
        is_begining_of_sequence_ready <= 0;
    end else begin
        is_current_input_odd <= ~is_current_input_odd;
        if(is_begining_of_sequence_ready != 3'b100) begin
            read_even <= 0;
            read_odd <= 0;
            is_begining_of_sequence_ready <= is_begining_of_sequence_ready + 3'b001;
        end else if (odd_y == 0) begin
            read_even <= ~is_current_input_odd;
            read_odd <= is_current_input_odd;
            odd_y <= first_y;
        end

        if(is_current_input_odd) begin
            odd_x <= x;
            out <= odd_y;
        end else begin
            even_x <= x;
            out <= even_y;
        end
    end

end

endmodule
