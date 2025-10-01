module additor (
    input clk,
    input [8:0] a,
    input [8:0] b,
    input reset,
    output reg [8:0] out
);

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        out <= 0; // Reset to 0
    end else begin
        out <= a + b;
    end
end

endmodule
