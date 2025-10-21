module encoder #(
    parameter WIDTH = 8
) (
    input [WIDTH - 1:0] onehot_in,
    output [$clog2(WIDTH) - 1:0] binary_out
);

    genvar i;
    generate
        for (i = 1; i <= $clog2(WIDTH); i = i + 1) begin : bit_encoder
            wire [WIDTH - 1:0] bit_mask = { (WIDTH/(2**i)) { {(2**(i-1)){1'b1}}, {(2**(i-1)){1'b0}} } };
            assign binary_out[i - 1] =| (onehot_in & bit_mask);
        end
    endgenerate

endmodule

/*

Works based on OR

bit mask
0 10101010
1 11001100
2 11110000

*/
