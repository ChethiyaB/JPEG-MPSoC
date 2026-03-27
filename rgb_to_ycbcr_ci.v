module rgb_to_ycbcr_ci (
    input  wire [31:0] dataa,  // Packed RGB from C
    output wire [31:0] result  // Packed YCbCr to C
);

    // 1. Unpack the inputs
    wire signed [31:0] r = {24'd0, dataa[23:16]};
    wire signed [31:0] g = {24'd0, dataa[15:8]};
    wire signed [31:0] b = {24'd0, dataa[7:0]};

    // 2. Perform the math (Quartus will synthesize these into DSP blocks)
    wire signed [31:0] y  = (( 19595 * r + 38469 * g +  7471 * b) >> 16);
    wire signed [31:0] cb = ((-11059 * r - 21709 * g + 32768 * b) >> 16) + 128;
    wire signed [31:0] cr = (( 32768 * r - 27439 * g -  5329 * b) >> 16) + 128;

    // 3. Pack the results
    assign result = {8'd0, y[7:0], cb[7:0], cr[7:0]};

endmodule