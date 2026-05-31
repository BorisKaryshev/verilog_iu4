module axi_skid_buffer #(
    parameter DATA_WIDTH = 8
) (
    input wire clk,
    input wire rst,
    input wire [DATA_WIDTH-1:0] s_axis_tdata,
    input wire s_axis_tvalid,
    output wire s_axis_tready,
    output wire [DATA_WIDTH-1:0] m_axis_tdata,
    output wire m_axis_tvalid,
    input wire m_axis_tready
);

reg [DATA_WIDTH-1:0] entry0;
reg [DATA_WIDTH-1:0] entry1;
reg [1:0] cnt;

assign s_axis_tready = (cnt < 2);
assign m_axis_tvalid = (cnt > 0);
assign m_axis_tdata = entry0;

always @(posedge clk) begin
    if (rst) begin
        entry0 <= 0;
        entry1 <= 0;
        cnt <= 0;
    end else begin
        case (cnt)
            0: begin
                if (s_axis_tvalid && s_axis_tready) begin
                    entry0 <= s_axis_tdata;
                    cnt <= 1;
                end
            end
            1: begin
                if (m_axis_tvalid && m_axis_tready) begin
                    if (s_axis_tvalid && s_axis_tready) begin
                        entry0 <= s_axis_tdata;
                    end else begin
                        entry0 <= entry1;
                        cnt <= 0;
                    end
                end else if (s_axis_tvalid && s_axis_tready) begin
                    entry1 <= s_axis_tdata;
                    cnt <= 2;
                end
            end
            2: begin
                if (m_axis_tvalid && m_axis_tready) begin
                    entry0 <= entry1;
                    cnt <= 1;
                end
            end
        endcase
    end
end

endmodule
