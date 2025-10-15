module multiplier (
    input clk,
    input [7:0] a,
    input [7:0] b,
    input reset,
    output reg [7:0] out,
    output wire is_ready
);

reg is_running;
assign is_ready = ~is_running;

reg [7:0] m1;

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        m1 <= 0;
        out <= 0; // Reset to 0
        is_running = 1;
    end else begin
        if (~is_running) begin
            m1 <= a * b;
            is_running <= 1'b1;
        end else begin
            out <= m1;
            is_running <= 1'b0;
        end
    end
end

endmodule
