module AXI_slave(
    input clk,
    input reset_n,
    input [7:0] data,
    input valid,
    input last,
    output reg ready,
    //выход для данных
    output reg [7:0] data_out,
    //сигнал остановки чтения
    input stop
);

//флаг наличия рукопожатия
wire flag_handshake;

//завязываем рукопожатие на значение сигналов
assign flag_handshake = ready & valid;

//завязываем valid на запись и счётчик
always @(posedge clk or negedge reset_n) begin
    ready <= ~stop & reset_n;
end

always @(posedge clk or negedge reset_n) begin
    if(~reset_n)
        data_out <= 8'b0;
    else
        if (flag_handshake)
            data_out <= data;
        else
            data_out <= 8'b0;
end

endmodule // AXI_slave
