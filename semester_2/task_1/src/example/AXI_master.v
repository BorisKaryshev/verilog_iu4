module AXI_master(
    input clk,
    input reset_n,
    output reg [7:0] data,
    output reg valid,
    output reg last,
    input ready,
    //вход для данных
    input [63:0] data_in,
    //сигнал для записи данных в буфер
    input we
);

//буфер для хранения не более 8 байт данных
reg [63:0] data_buff = 64'b0;
reg [2:0] buff_count = 3'b0;

//флаг наличия рукопожатия
wire flag_handshake;

//завязываем рукопожатие на значение сигналов
assign flag_handshake = ready & valid;

//логика для valid
always @(posedge clk or negedge reset_n) begin
    if (~reset_n) valid <= 1'b0;
    else if ((buff_count > 0) & ~we) begin
        if (valid == 1'b0) begin
            data <= data_buff[7:0];
            data_buff <= {8'b0, data_buff[63:8]};
        end
        valid <= 1'b1;
    end else valid <= 1'b0;
end

//always для работы буфера
always @(posedge clk or negedge reset_n) begin
    if(~reset_n) begin
        data_buff <= 64'b0;
        buff_count <= 4'b0;
    end else begin
        if (we) begin
            data_buff <= data_in;
            buff_count <= 3'b111;
        end
        if (flag_handshake & (buff_count > 0) & ~we) begin
            if(buff_count > 1) data_buff <= {8'b0, data_buff[63:8]};
            buff_count <= buff_count - 1;
        end
    end
end

//второй always для работы протокола
always @(posedge clk or negedge reset_n) begin
    if(~reset_n) begin
        data <= 8'b0;
        last <= 1'b0;
    end else
        if(flag_handshake) begin
            if(buff_count == 1) last <= 1'b1;
            else last <= 1'b0;
            data <= data_buff[7:0];
        end else last <= 1'b0;
end

endmodule
