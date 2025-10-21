module main #(
    parameter CACHE_SIZE = 8,
    parameter VAR_WIDTH = 8
) (
    /* verilator lint_off UNUSEDSIGNAL */
    input clk,
    input rst,
    input [VAR_WIDTH - 1:0] x,
    output reg [VAR_WIDTH - 1:0] cache [0:CACHE_SIZE - 1]
);

    wire [CACHE_SIZE - 1:0] encoder_in;
    wire [$clog2(CACHE_SIZE) - 1:0] encoder_out;
    encoder #(
        .WIDTH(VAR_WIDTH)
    ) local_encoder (
        .onehot_in(encoder_in),
        .binary_out(encoder_out)
    );


    genvar i;
    generate
        for (i = 0; i < CACHE_SIZE; i = i + 1) begin : encoder_input_creation
            assign encoder_in[i] = (cache[i] == x);
        end
        for (i = 1; i < CACHE_SIZE - 1; i = i + 1) begin : cache_shift
            always @(posedge clk) begin
                if(!rst) begin
                    cache[i] <= 0;
                end else if((encoder_in == 0) || i <= encoder_out) begin
                    cache[i] <= cache[i - 1];
                end
            end
        end
    endgenerate
    always @(posedge clk) begin
        if(!rst) begin
            cache[CACHE_SIZE - 1] <= 0;
/* verilator lint_off WIDTHEXPAND */
        end else if((encoder_in == 0) || encoder_out == (CACHE_SIZE - 1)) begin
/* verilator lint_on WIDTHEXPAND */
            cache[CACHE_SIZE - 1] <= cache[CACHE_SIZE - 2];
        end
    end

    always @(posedge clk) begin
        if(!rst) begin
            cache[0] <= 0;
        end else begin
            cache[0] <= x;
        end
    end

endmodule
