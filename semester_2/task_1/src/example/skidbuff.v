module skid_buff(
    input clk,
    input reset_n,

    input [7:0] s_data,
    input s_valid,
    input s_last,
    output reg s_ready,

    output reg [7:0] m_data,
    output reg m_valid,
    output reg m_last,
    input m_ready
);

//объявляем возможные состояния
//(0 - обычная работа, 1 - работа в режиме буферизации)
reg STATE = 1'b0;

//объявляем память для хранения данных
reg [7:0] mem_data = 8'b0;
reg mem_last = 1'b0;

//это отображения значения лог выражений, для удобства
wire STATE_0_TO_1;
wire STATE_1_TO_0;

assign STATE_0_TO_1 = ~STATE & s_ready & ~m_ready;
assign STATE_1_TO_0 = STATE & m_ready;

// Блок 0: Инициализация всех регистров при сбросе
always @(posedge clk or negedge reset_n) begin
    if (~reset_n) begin
        STATE <= 1'b0;
        mem_data <= 8'b0;
        mem_last <= 1'b0;
    end
end

// Блок 1: Логика передачи данных (m_data, m_last)
always @(posedge clk or negedge reset_n) begin
    if (~reset_n) begin
        m_data <= 8'b0;
        m_last <= 1'b0;
    end else
        if (STATE_1_TO_0) begin
            m_data <= mem_data;
            m_last <= mem_last;
        end else if (~STATE & ~STATE_0_TO_1) begin
            m_data <= s_data;
            m_last <= s_last;
        end
end

// Блок 2: Логика m_valid (только сигнал валидности)
always @(posedge clk or negedge reset_n) begin
    if (~reset_n)
        m_valid <= 1'b0;
    else
        if (STATE_1_TO_0)
            m_valid <= 1'b1;
        else if (~STATE)
            m_valid <= s_valid;
end

// Блок 3: Логика s_ready (только сигнал готовности входа)
always @(posedge clk or negedge reset_n) begin
    if (~reset_n)
        s_ready <= 1'b0;
    else
        if (STATE_0_TO_1)
            s_ready <= 1'b0;
        else if (STATE_1_TO_0)
            s_ready <= 1'b1;
        else if (~STATE)
            s_ready <= m_ready;
end

// Блок 4: Логика записи данных в буфер
always @(posedge clk or negedge reset_n) begin
    if (~reset_n) begin
        mem_data <= 8'b0;
        mem_last <= 1'b0;
    end else
        if (STATE_0_TO_1) begin
            mem_data <= s_data;
            mem_last <= s_last;
        end
end

// Блок 5: Логика управления состояниями
always @(posedge clk or negedge reset_n) begin
    if (~reset_n)
        STATE <= 1'b0;
    else
        if (STATE_0_TO_1)
            STATE <= 1'b1;
        else if (STATE_1_TO_0)
            STATE <= 1'b0;
end

endmodule // skid_buff
