module encoder #(
    parameter WIDTH = 8
) (
    input [WIDTH - 1:0] onehot_in,
    output [$clog2(WIDTH) - 1:0] binary_out
);

    genvar i;
    generate
        for (i = 0; i < $clog2(WIDTH); i = i + 1) begin : bit_encoder
            wire [WIDTH - 1:0] bit_mask;

            // Create mask where each bit position is 1 if the i-th bit
            // in its binary representation is 1
            genvar j;
            for (j = 0; j < WIDTH; j = j + 1) begin : mask_gen
                assign bit_mask[j] = j[i];
            end

            assign binary_out[i] = | (onehot_in & bit_mask);
        end
    endgenerate

    /*
    This method works only for WIDTH = power of 2.
    genvar i;
    generate
        for (i = 1; i <= $clog2(WIDTH); i = i + 1) begin : bit_encoder
            wire [WIDTH - 1:0] bit_mask = { (WIDTH/(2**i)) { {(2**(i-1)){1'b1}}, {(2**(i-1)){1'b0}} } };
            assign binary_out[i - 1] =| (onehot_in & bit_mask);
        end
    endgenerate


    For both this generate loop and loop above idea is the same.
    */
endmodule
/*

Works based on OR

bit mask
0 10101010
1 11001100
2 11110000

*/

/*
j:
    - 000
    - 001
    - 010
    - 011
    - 100
    - 101
    - 110
    - 111

Taking bit with j[i] we will have mask from above
*/
