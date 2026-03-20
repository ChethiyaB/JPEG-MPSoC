//////////////////////////////////////////////////////////////////////////////////////////
//											//
//	Verilog Queue for JPEG Encoder : Modified by Isuru Nawinne (2012/09/20)		//
//											//
//////////////////////////////////////////////////////////////////////////////////////////

// `timescale 1ns / 1ps

//`define DISPLAY_IO

`define PUSHREQ		TIE_FIFO_OUT6_PushReq 
`define PUSHDATA	TIE_FIFO_OUT6
`define POPREQ		TIE_FIFO_IN1_PopReq
`define POPDATA		TIE_FIFO_IN1
`define FULL		TIE_FIFO_OUT6_Full
`define EMPTY		TIE_FIFO_IN1_Empty
`define BITWIDTH	8
`define DEPTH		100
`define POINTERSIZE	7 // Size of wp and rp : Update the values in lines 38,39

module queueA2F(CLK, `PUSHREQ, `PUSHDATA, `FULL, `POPREQ, `POPDATA, `EMPTY);

  input                         CLK;

  input                         `PUSHREQ;
  input         [`BITWIDTH-1:0]  `PUSHDATA;
  output                        `FULL;

  input                         `POPREQ;   
  output        [`BITWIDTH-1:0]  `POPDATA;
  output                        `EMPTY;

  reg                           `FULL  = 1'b0;

  reg           [`POINTERSIZE-1:0]           wp              = 7'b0;
  reg           [`POINTERSIZE-1:0]           rp              = 7'b0;

  reg           [`BITWIDTH-1:0]          store[`DEPTH-1:0];


  initial begin
    if($test$plusargs("dumpvars")) begin
      $dumpvars();
    end
  end

  initial begin
    store[0]    <= 0;
  end

  assign `POPDATA       = store[rp];
  assign `EMPTY = (wp == rp) && !`FULL;



  always @(posedge CLK) begin
    if ((`PUSHREQ && !`FULL) && (`POPREQ && !`EMPTY)) begin
`ifdef DISPLAY_IO
      $display("%t A2F_push = 0x%h", $time, `PUSHDATA);
      $display("%t A2F_pop  = 0x%h", $time, `POPDATA);
`endif
      store[wp]         <= #1 `PUSHDATA;
      wp                <= #1 ((wp + 1) % `DEPTH);
      rp                <= #1 ((rp + 1) % `DEPTH);
      `FULL    		<= #1 `FULL;
    end
    else if (`PUSHREQ && !`FULL) begin
`ifdef DISPLAY_IO
      $display("%t A2F_push = 0x%h", $time, `PUSHDATA);
`endif
      store[wp]         <= #1 `PUSHDATA;
      wp                <= #1 ((wp + 1) % `DEPTH);
      rp                <= #1 rp;
      `FULL    		<= #1 (((wp + 1) % `DEPTH == rp) ? 1'b1 : 1'b0);
    end
    else if (`POPREQ && !`EMPTY) begin
`ifdef DISPLAY_IO
      $display("%t A2F_pop  = 0x%h", $time, `POPDATA);
`endif
      wp                <= #1 wp;
      rp                <= #1 ((rp + 1) % `DEPTH);
      `FULL  	  	<= #1 1'b0;
    end
    else begin
      wp                <= #1 wp;
      rp                <= #1 rp;
      `FULL    		<= #1 `FULL;
    end
  end


endmodule

