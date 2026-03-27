module dct_accelerator (
    input  wire        clk,
    input  wire        reset,
    input  wire        clk_en,
    input  wire        start,     
    input  wire [1:0]  n,         
    input  wire [31:0] dataa,     
    input  wire [31:0] datab,     
    output reg         done,      
    output reg  [31:0] result     
);

    reg signed [15:0] block_ram [0:63];
    reg signed [15:0] trans_ram [0:63];
    
    reg [2:0] state;
    localparam IDLE = 0, CALC_ROW = 1, CALC_COL = 2, FINISH = 3;

    reg [2:0] loop_y;
    reg [2:0] loop_u;

    // Corrected Fixed-Point Cosine Table
    // Scaled by 8192 (2^13). Safely fits inside 16-bit signed limit (32767).
    reg signed [15:0] C0, C1, C2, C3, C4, C5, C6, C7;

    always @(*) begin
        case (loop_u)
            3'd0: begin C0=16'd2896; C1=16'd2896; C2=16'd2896; C3=16'd2896; C4=16'd2896; C5=16'd2896; C6=16'd2896; C7=16'd2896; end
            3'd1: begin C0=16'd4015; C1=16'd3406; C2=16'd2276; C3=16'd799;  C4=-16'd799; C5=-16'd2276;C6=-16'd3406;C7=-16'd4015;end
            3'd2: begin C0=16'd3784; C1=16'd1568; C2=-16'd1568;C3=-16'd3784;C4=-16'd3784;C5=-16'd1568;C6=16'd1568; C7=16'd3784; end
            3'd3: begin C0=16'd3406; C1=-16'd799; C2=-16'd4015;C3=-16'd2276;C4=16'd2276; C5=16'd4015; C6=16'd799;  C7=-16'd3406;end
            3'd4: begin C0=16'd2896; C1=-16'd2896;C2=-16'd2896;C3=16'd2896; C4=16'd2896; C5=-16'd2896;C6=-16'd2896;C7=16'd2896; end
            3'd5: begin C0=16'd2276; C1=-16'd4015;C2=16'd799;  C3=16'd3406; C4=-16'd3406;C5=-16'd799; C6=16'd4015; C7=-16'd2276;end
            3'd6: begin C0=16'd1568; C1=-16'd3784;C2=16'd3784; C3=-16'd1568;C4=-16'd1568;C5=16'd3784; C6=-16'd3784;C7=16'd1568; end
            3'd7: begin C0=16'd799;  C1=-16'd2276;C2=16'd3406; C3=-16'd4015;C4=16'd4015; C5=-16'd3406;C6=16'd2276; C7=-16'd799;  end
        endcase
    end

    wire [5:0] idx_0 = (loop_y * 8) + 0;
    wire [5:0] idx_1 = (loop_y * 8) + 1;
    wire [5:0] idx_2 = (loop_y * 8) + 2;
    wire [5:0] idx_3 = (loop_y * 8) + 3;
    wire [5:0] idx_4 = (loop_y * 8) + 4;
    wire [5:0] idx_5 = (loop_y * 8) + 5;
    wire [5:0] idx_6 = (loop_y * 8) + 6;
    wire [5:0] idx_7 = (loop_y * 8) + 7;
    
    wire [5:0] w_idx = (loop_u * 8) + loop_y;

    wire signed [31:0] mac_row = (C0 * block_ram[idx_0]) + (C1 * block_ram[idx_1]) + 
                                 (C2 * block_ram[idx_2]) + (C3 * block_ram[idx_3]) + 
                                 (C4 * block_ram[idx_4]) + (C5 * block_ram[idx_5]) + 
                                 (C6 * block_ram[idx_6]) + (C7 * block_ram[idx_7]);

    wire signed [31:0] mac_col = (C0 * trans_ram[idx_0]) + (C1 * trans_ram[idx_1]) + 
                                 (C2 * trans_ram[idx_2]) + (C3 * trans_ram[idx_3]) + 
                                 (C4 * trans_ram[idx_4]) + (C5 * trans_ram[idx_5]) + 
                                 (C6 * trans_ram[idx_6]) + (C7 * trans_ram[idx_7]);

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            done <= 0;
            result <= 0;
            state <= IDLE;
            loop_y <= 0;
            loop_u <= 0;
        end else if (clk_en) begin
            done <= 0; 

            case (state)
                IDLE: begin
                    if (start) begin
                        if (n == 2'd0) begin
                            block_ram[dataa[5:0]] <= datab[15:0];
                            done <= 1; 
                        end 
                        else if (n == 2'd2) begin
                            result <= {{16{block_ram[dataa[5:0]][15]}}, block_ram[dataa[5:0]]};
                            done <= 1;
                        end 
                        else if (n == 2'd1) begin
                            loop_y <= 0;
                            loop_u <= 0;
                            state <= CALC_ROW;
                        end
                    end
                end

                CALC_ROW: begin
                    // Arithmetic Right Shift (>>>) by 13 to remove the scale correctly for negative numbers
                    trans_ram[w_idx] <= (mac_row >>> 13);

                    if (loop_u == 3'd7) begin
                        loop_u <= 0;
                        if (loop_y == 3'd7) begin
                            loop_y <= 0;
                            state <= CALC_COL; 
                        end else begin
                            loop_y <= loop_y + 1'b1;
                        end
                    end else begin
                        loop_u <= loop_u + 1'b1;
                    end
                end

                CALC_COL: begin
                    // Arithmetic Right Shift (>>>) by 13
                    block_ram[w_idx] <= (mac_col >>> 13);

                    if (loop_u == 3'd7) begin
                        loop_u <= 0;
                        if (loop_y == 3'd7) begin
                            state <= FINISH;
                        end else begin
                            loop_y <= loop_y + 1'b1;
                        end
                    end else begin
                        loop_u <= loop_u + 1'b1;
                    end
                end

                FINISH: begin
                    done <= 1;
                    state <= IDLE;
                end
            endcase
        end
    end
endmodule