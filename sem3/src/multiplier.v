module multiplier (
    input clk,
    input [7:0] a,
    input [7:0] b,
    input reset,
    output reg [7:0] out,
    output reg has_result
);

reg [7:0] delay_1;
reg [7:0] delay_2;

reg has_result_delay_1;
reg has_result_delay_2;

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        delay_1 <= 0;
        delay_2 <= 0;
        out <= 0; // Reset to 0

        has_result_delay_1 <= 0;
        has_result_delay_2 <= 0;
        has_result <= 0;
    end else begin
        if (has_result) begin
            delay_1 <= a * b;

            has_result_delay_1 <= 1;
            has_result_delay_2 <= 0;
            has_result <= 0;
        end else begin
            has_result_delay_1 <= 1;
            has_result_delay_2 <= has_result_delay_1;
            has_result <= has_result_delay_2;
        end
        delay_2 <= delay_1;
        out <= delay_2;

    end
end

endmodule
