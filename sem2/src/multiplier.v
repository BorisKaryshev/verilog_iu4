module multiplier (
    input clk,
    input [8:0] a,
    input [8:0] b,
    input reset,
    output reg [8:0] out
);

reg [8:0] m1;
reg [8:0] m2;

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        m1 <= 0;
        m2 <= 0;
        out <= 0; // Reset to 0
    end else begin
        m1 <= a * b;
        m2 <= m1;
        out <= m2;
    end
end

endmodule
