module main (
    input sclk,
    input rst,
    input mosi,
    /* verilator lint_off UNUSEDSIGNAL */
    input miso,
    /* verilator lint_on UNUSEDSIGNAL */
    input ss,
    output reg is_ready,
    output reg [7:0] command,
    output reg [15:0] address,
    output reg [31:0] data
);

reg [7:0] bit_counter;

always @ (posedge sclk) begin
    if (!rst) begin // Active-low reset
        is_ready <= 0;
        command <= 0;
        address <= 0;
        data <= 0;
        bit_counter <= 0;
    end else if(!ss) begin
        bit_counter <= bit_counter + 1;
        if(bit_counter < 8) begin
            /* verilator lint_off WIDTHEXPAND */
            command <= (command << 1) | mosi;
            /* verilator lint_on WIDTHEXPAND */
        end else if (bit_counter < (3 * 8)) begin
            /* verilator lint_off WIDTHEXPAND */
            address <= (address << 1) | mosi;
            /* verilator lint_on WIDTHEXPAND */
        end else if (bit_counter < (7 * 8)) begin
            /* verilator lint_off WIDTHEXPAND */
            data <= (data << 1) | mosi;
            /* verilator lint_on WIDTHEXPAND */
        end
        if (bit_counter == (7  * 8 - 1)) begin
            is_ready <= 1;
        end
    end
end

endmodule
