module main (
    input clk,
    input signal,
    input begining,
    input finish,
    input reset,
    output reg is_dividable_by_three,
    output reg has_result
);


reg is_transition_going;
wire [1:0] inter_value;
reg is_current_bit_odd;
reg has_result_delay;

reg [63:0] inner_value;

wire add_one;
wire subtract_one;
assign add_one = (is_current_bit_odd && signal && (is_transition_going && ~finish && ~begining));
assign subtract_one = (~is_current_bit_odd && signal && (is_transition_going && ~finish && ~begining));

counter_by_module_three counter (
    .clk(clk),
    .add_one(add_one),
    .subtract_one(subtract_one),
    .reset(reset),
    .out(inter_value)
);

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        is_transition_going <= 1'b0;
        is_dividable_by_three <= 1'b0;
        has_result <= 1'b0;
        has_result_delay <= 1'b0;
        is_current_bit_odd <= 1'b0;
        inner_value <= 0;
    end else begin

        if(has_result_delay) begin
            is_dividable_by_three <= (inter_value == 2'b00) ? 1 : 0;
        end

        has_result <= has_result_delay;
        if(begining) begin
            is_transition_going <= 1'b1;
        end else if (finish) begin
            is_transition_going <= 1'b0;
            has_result_delay <= 1'b1;
        end

        if(is_transition_going && ~finish) begin
            inner_value <= (inner_value << 1) | ((signal) ? 64'h1 : 64'h0);
            is_current_bit_odd <= ~is_current_bit_odd;
        end
    end

end

endmodule
