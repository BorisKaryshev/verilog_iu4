module multiplier (
    input clk,
    input [7:0] a,
    input [7:0] b,
    input enable_input,
    input rst_n,
    output reg [7:0] out
);

reg [7:0] delay_1;
// reg [7:0] delay_2;

always @ (posedge clk or negedge rst_n) begin
    if (!rst_n) begin // Active-low reset
        delay_1 <= 0;
        // delay_2 <= 0;
        out <= 0; // Reset to 0
    end else begin
        if (enable_input) begin
            delay_1 <= a * b;
        end
        // delay_2 <= delay_1;
        out <= delay_1;
    end
end

endmodule
