module main #(
    parameter WORD_WIDTH = 8
) (
    input clk,
    input rst,

    input [WORD_WIDTH - 1:0] in,
    input ready_m, // low for true
    input in_enable, // low fo true

    output [WORD_WIDTH - 1:0] out,
    output ready_s, // low for true
    output read_ready // low for true
);
    reg [WORD_WIDTH - 1:0] first_buffer;
    reg [WORD_WIDTH - 1:0] second_buffer;
    assign out = first_buffer;

    reg ready_buffer;
    assign ready_s = ready_buffer;

    reg [1:0] n_of_elements;

    assign read_ready = !(n_of_elements != 2'b00);

    always @(posedge clk) begin
        if(!rst) begin
            first_buffer <= 0;
            n_of_elements <= 0;
            second_buffer <= 0;
            ready_buffer <= 0;
        end else begin
            if(!ready_m) begin
                if(n_of_elements == 2'b10) begin
                    n_of_elements <= 2'b01;
                    first_buffer <= second_buffer;
                end else if (n_of_elements == 2'b01) begin
                    first_buffer <= in;
                end else if(n_of_elements == 2'b00) begin
                    first_buffer <= in;
                    n_of_elements <= 2'b01;
                end
            end
            else begin
                if(n_of_elements == 2'b00) begin
                    first_buffer <= second_buffer;
                    second_buffer <= in;
                    n_of_elements <= n_of_elements + 2'b01;
                end
                else if(n_of_elements == 2'b01) begin
                    first_buffer <= second_buffer;
                    second_buffer <= in;
                    n_of_elements <= n_of_elements + 2'b01;
                    ready_buffer <= 1'b1;
                end
            end
        end
    end

endmodule
