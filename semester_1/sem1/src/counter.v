module counter (
    input clk,
    input in,
    output reg A,
    output reg B,
    output reg C
);
    reg [1:0] state;

    always @(posedge clk) begin
        if (in) begin
            A <= (state == 2'b00);
            B <= (state == 2'b01);
            C <= (state == 2'b10);
            if (state == 2'b10) begin
                state <= 2'b00;
            end else begin
                state <= state + 1;
            end
        end
    end
endmodule
