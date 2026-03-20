//////////////////////////////////////////////////////////////////////////////////////////
//											//
//	Verilog Queue for JPEG Encoder : Modified by Isuru Nawinne (2012/09/20)		//
//											//
//////////////////////////////////////////////////////////////////////////////////////////

// `timescale 1ns / 1ps

//`define DISPLAY_IO

`define PUSHREQ		TIE_FIFO_OUT2_PushReq 
`define PUSHDATA	TIE_FIFO_OUT2
`define POPREQ		TIE_FIFO_IN3_PopReq
`define POPDATA		TIE_FIFO_IN3
`define FULL		TIE_FIFO_OUT2_Full
`define EMPTY		TIE_FIFO_IN3_Empty
`define BITWIDTH	1
`define DEPTH		1
`define POINTERSIZE	1 // Size of wp and rp : Update the values in lines 38,39

module queueE2Fb(CLK, `PUSHREQ, `PUSHDATA, `FULL, `POPREQ, `POPDATA, `EMPTY);

  input                         CLK;

  input                         `PUSHREQ;
  input         		`PUSHDATA;
  output                        `FULL;

  input                         `POPREQ;   
  output        		`POPDATA;
  output                        `EMPTY;

  reg                           `FULL  = 1'b0;

  reg           [`POINTERSIZE-1:0]           wp              = 1'b0;
  reg           [`POINTERSIZE-1:0]           rp              = 1'b0;

  reg           		store[`DEPTH-1:0]; 


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
      $display("%t E2Fb_push = 0x%h", $time, `PUSHDATA);
      $display("%t E2Fb_pop  = 0x%h", $time, `POPDATA);
`endif
      store[wp]         <= #1 `PUSHDATA;
      wp                <= #1 ((wp + 1) % `DEPTH);
      rp                <= #1 ((rp + 1) % `DEPTH);
      `FULL    		<= #1 `FULL;
    end
    else if (`PUSHREQ && !`FULL) begin
`ifdef DISPLAY_IO
      $display("%t E2Fb_push = 0x%h", $time, `PUSHDATA);
`endif
      store[wp]         <= #1 `PUSHDATA;
      wp                <= #1 ((wp + 1) % `DEPTH);
      rp                <= #1 rp;
      `FULL    		<= #1 (((wp + 1) % `DEPTH == rp) ? 1'b1 : 1'b0);
    end
    else if (`POPREQ && !`EMPTY) begin
`ifdef DISPLAY_IO
      $display("%t E2Fb_pop  = 0x%h", $time, `POPDATA);
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

