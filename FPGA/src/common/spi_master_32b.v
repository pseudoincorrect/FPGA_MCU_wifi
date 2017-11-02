////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// Project Name: SPI (Verilog)                                            ////
////                                                                        ////
//// Module Name: spi_master                                                ////
////                                                                        ////
////                                                                        ////
////  This file is part of the Ethernet IP core project                     ////
////  http://opencores.com/project,spi_verilog_master_slave                 ////
////                                                                        ////
////  Author(s):                                                            ////
////      Santhosh G (santhg @ opencores.org)                                 ////
////                                                                        ////
////  Refer to Readme.txt for more information                              ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// Copyright (C) 2014, 2015 Authors                                       ////
////                                                                        ////
//// This source file may be used and distributed without                   ////
//// restriction provided that this copyright statement is not              ////
//// removed from the file and that any derivative work contains            ////
//// the original copyright notice and the associated disclaimer.           ////
////                                                                        ////
//// This source file is free software; you can redistribute it             ////
//// and/or modify it under the terms of the GNU Lesser General             ////
//// Public License as published by the Free Software Foundation;           ////
//// either version 2.1 of the License, or (at your option) any             ////
//// later version.                                                         ////
////                                                                        ////
//// This source is distributed in the hope that it will be                 ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied             ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR                ////
//// PURPOSE.  See the GNU Lesser General Public License for more           ////
//// details.                                                               ////
////                                                                        ////
//// You should have received a copy of the GNU Lesser General              ////
//// Public License along with this source; if not, download it             ////
//// from http://www.opencores.org/lgpl.shtml                               ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SPI MODE 3
		CHANGE DATA  @  NEGEDGE
		read data  @ posedge

 RSTB-active low asyn reset, CLK-clock, T_RB = 0-rx  1-TX, 
 mlb = 0-LSB 1st 1-msb 1st
 START = 1- starts data transmission cdiv 0 = clk/4 1 = /8   2 = /131  3 = /32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

`include "timescale.v"


module spi_master(rstb, clk, mlb, start, tdat, cdiv, 
				  din,  ss, sck, dout, done_r, rdata);

parameter state_idle   = 4'd0;		
parameter state_send   = 4'd1; 
parameter state_finish = 4'd2;


input 		 rstb, clk, mlb, start;
input [31:0] tdat;  //transmit data
input [1:0]  cdiv;  //clock divider
input 		 din;
output reg 	 ss; 
output reg 	 sck; 
output reg 	 dout; 
output reg 	 done_r;
output reg 	 [31:0] rdata; //received data
wire [4:0] 	mid;

reg [3:0] 	current_state,next_state;
reg [31:0] 	treg,rreg;
reg [31:0] 	rdata_next;
reg [31:0] 	nbit;
reg [4:0] 	cnt;
reg 		shift, clr;
reg			done;

assign mid = 1;

//state transistion
always @ (negedge clk or negedge rstb) begin
	if (rstb == 0) 
		done_r  <= 1'b0;
	else 
		if (current_state == state_finish)
			done_r <= 1'b1;
		else 
			done_r <= 1'b0;
end

//state transistion
always @ (negedge clk or negedge rstb) begin
	if (rstb == 0) begin
		current_state <= state_finish;
		rdata         <= 0;
	end
	else begin
		current_state <= next_state;
		rdata         <= rdata_next;
	end
end


//FSM i/o
always  @ (start or current_state or nbit or cdiv or rreg or rdata) begin

	clr   = 0;  
	shift = 0;
	ss    = 1; 
	// done  = 0;
	rdata_next = rdata;
	next_state = current_state;

	/*	case (cdiv) // clk divider for spi sck 
		2'b00: mid = 2;
		2'b01: mid = 4;
		2'b10: mid = 8;
		2'b11: mid = 131;
	endcase*/

	case(current_state)

		state_idle: begin // 2'b00 = 0
			#1 // to avoid infinite simulation loop

			if (start == 1) begin 
				shift      = 1;
				next_state = state_send;	 
	        end
	    end    

		state_send: begin // 2'b10 = 2
			ss = 0;
			if (nbit != 32) begin
				shift = 1; 
			end
			else begin
				rdata_next = rreg;
				// done       = 1'b1;
				// next_state = state_wait_1;
				next_state = state_finish;
			end
		end		

		state_finish: begin // 2'b11 = 3
			shift = 0;
			ss    = 1;
			clr   = 1;
			// done  = 1'b1;
			next_state = state_idle;
		end

		default: next_state = state_finish;

	endcase
end


//setup falling edge (shift dout) sample rising edge (read din)
always @ (negedge clk or posedge clr) begin

	if (clr == 1) begin
		cnt = 5'd0; 
		sck = 0; 
	end 
	else begin
		if (shift == 1) begin
			cnt = cnt + 5'd1; 
			if (cnt == mid) begin
				sck = ~sck;
				cnt = 5'd0;
			end
		end 
	end 
end 


//sample @ rising edge (read din)
always @ (negedge sck or posedge clr ) begin // or negedge rstb

	if (clr == 1)  begin
		nbit = 7'd0;  
		rreg = 32'hFFFF_FFFF;  
	end 
	else begin 
		if (mlb == 0) begin //LSB first, din @ msb -> right shift
			rreg = { din, rreg[31:1] };  
		end 
		else begin //MSB first, din @ lsb -> left shift
			rreg = { rreg[30:0], din };  
		end
		nbit = nbit + 7'd1;
	end
end 


// shift dout @ falling edge (write dout)
always @ (posedge sck or posedge clr) begin

	if (clr == 1) begin
	  	treg = 32'h0;  
	  	dout = 0;  
	end  
	else begin
		if (nbit == 0) begin //load data into TREG
			treg = tdat; 
			dout = mlb ? treg[31] : treg[0];
		end //nbit_if
		else begin
			if (mlb == 0) begin //LSB first, shift right
				treg = { 1'b1, treg[31:1] }; 
				dout = treg[0]; 
			end 
			else begin //MSB first shift LEFT
				treg = { treg[30:0], 1'b1 }; 
				dout = treg[31]; 
			end
		end
	end 
end

endmodule


