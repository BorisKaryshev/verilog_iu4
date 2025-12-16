module counter_by_module_three (
    input clk,
    input add_one,
    input subtract_one,
    input reset,
    output reg [1:0] out
);

always @ (posedge clk) begin
    if (!reset) begin // Active-low reset
        out <= 2'b00;
    end else begin
        if(add_one) begin
            if(out == 2'b10) out <= 2'b00;
            else out <= out + 2'b01;
        end else if (subtract_one) begin
            if(out == 2'b00) out <= 2'b10;
            else out <= out - 2'b01;
        end
    end
end

endmodule
