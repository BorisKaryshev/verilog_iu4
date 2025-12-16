module ctr (
    input up_down,
    input clk,
    input rstn,
    output reg [2:0] out
);

always @ (posedge clk) begin
    if (!rstn) begin // Active-low reset
        out <= 3'b000; // Reset to 0
    end else begin
        if (up_down) begin // Count up
            out <= out + 1;
        end else begin // Count down
            out <= out - 1;
        end
    end
end

endmodule
