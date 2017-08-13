/*88888888 8888888 8888888888  .d88888b.          .d8888b.  8888888b. 8888888 
888          888   888        d88P" "Y88b        d88P  Y88b 888   Y88b  888   
888          888   888        888     888        Y88b.      888    888  888   
8888888      888   8888888    888     888         "Y888b.   888   d88P  888   
888          888   888        888     888            "Y88b. 8888888P"   888   
888          888   888        888     888              "888 888         888   
888          888   888        Y88b. .d88P        Y88b  d88P 888         888   
888        8888888 888         "Y88888P" 88888888 "Y8888P"  888       88888*/


`include "timescale.v"

module fifo_spi (clk, nrst, we, din, dout, sck, ss);

input clk;
input nrst;
input we;
input [31:0] din;

output dout;
output sck;
output reg ss;


// States
parameter state_rst      = 4'd0;		
parameter state_wait     = 4'd1;
parameter state_start	 = 4'd2;
parameter state_get_data = 4'd3;
parameter state_send     = 4'd4;	
parameter state_cnt      = 4'd5;	
parameter state_get_last = 4'd6;	
parameter state_send_lf  = 4'd7;	

// Frame cnt
parameter total_frame 	  = 255;
parameter last_frame_data = 32'h0000_1234;


/*8       888 8888888 8888888b.  8888888888  .d8888b.   
888   o   888   888   888   Y88b 888        d88P  Y88b  
888  d8b  888   888   888    888 888        Y88b.       
888 d888b 888   888   888   d88P 8888888     "Y888b.    
888d88888b888   888   8888888P"  888            "Y88b.  
88888P Y88888   888   888 T88b   888              "888  
8888P   Y8888   888   888  T88b  888        Y88b  d88P  
888P     Y888 8888888 888   T88b 8888888888  "Y8888*/  

// General
wire clk;
// SPI
wire 		spi_mlb;
wire 		spi_cdiv;
wire 		spi_din;
wire 		spi_ss_not_connected;
wire 		spi_sck;
wire 		spi_dout;
wire		spi_done;
wire [31:0]	spi_transmit_dat;
wire [31:0]	spi_transmit_dat_LSB;
wire [31:0] rdata;
reg 		spi_start;
reg			spi_start_reg;
// Fifo    
wire 		fifo_clr;
wire 		fifo_we;
wire 		fifo_full;
wire 		fifo_empty;
wire 		fifo_full_r;
wire 		fifo_empty_r;
wire 		fifo_full_n;
wire 		fifo_empty_n;
wire 		fifo_full_n_r;
wire 		fifo_empty_n_r;
wire 		fifo_ready;
wire [1:0]  fifo_level;
wire [31:0] fifo_din;
wire [31:0] fifo_dout;  
reg 		fifo_re;
// Data line
reg  [31:0] data_out_reg;
// Frame cnt
wire [8:0] frame_cnt_plus_one;
reg  [8:0] frame_cnt_next;
reg	 [8:0] frame_cnt_reg;
// State machine
reg  [3:0] current_state;
reg	 [3:0] next_state;     


/*8b     d888  .d88888b.  8888888b.  888     888 888      8888888888 
8888b   d8888 d88P" "Y88b 888  "Y88b 888     888 888      888        
88888b.d88888 888     888 888    888 888     888 888      888        
888Y88888P888 888     888 888    888 888     888 888      8888888    
888 Y888P 888 888     888 888    888 888     888 888      888        
888  Y8P  888 888     888 888    888 888     888 888      888        
888   "   888 Y88b. .d88P 888  .d88P Y88b. .d88P 888      888        
888       888  "Y88888P"  8888888P"   "Y88888P"  88888888 88888888*/

spi_master spi  (
		.rstb	(nrst), 			 	.clk	(clk),  	  
		.mlb	(spi_mlb),   			.start	(spi_start), 
		.tdat	(spi_transmit_dat), 	.cdiv	(spi_cdiv), 
		.din	(spi_din),   			.ss		(spi_ss_not_connected), 
		.sck	(spi_sck),   		 	.dout	(spi_dout), 
		.done	(spi_done), 			.rdata	(rdata) 
		);

generic_fifo_sc_a fifo  (
		.clk 	   (clk), 				.rst		(nrst),    	 
		.clr 	   (fifo_clr),      	.din		(fifo_din),    
		.we 	   (fifo_we), 			.dout		(fifo_dout),   
		.re 	   (fifo_re),       	.full		(fifo_full),   
		.empty 	   (fifo_empty), 		.full_r		(fifo_full_r), 
		.empty_r   (fifo_empty_r),		.full_n		(fifo_full_n),
		.empty_n   (fifo_empty_n),		.full_n_r	(fifo_full_n_r), 	
		.empty_n_r (fifo_empty_n_r),  	.level		(fifo_level)
		);

	defparam fifo.dw = 32;
	defparam fifo.aw = 10;
	defparam fifo.n  = 1000;


	   /*888  .d8888b.   .d8888b. 8888888  .d8888b.  888b    888 
      d88888 d88P  Y88b d88P  Y88b  888   d88P  Y88b 8888b   888 
     d88P888 Y88b.      Y88b.       888   888    888 88888b  888 
    d88P 888  "Y888b.    "Y888b.    888   888        888Y88b 888 
   d88P  888     "Y88b.     "Y88b.  888   888  88888 888 Y88b888 
  d88P   888       "888       "888  888   888    888 888  Y88888 
 d8888888888 Y88b  d88P Y88b  d88P  888   Y88b  d88P 888   Y8888 
d88P     888  "Y8888P"   "Y8888P" 8888888  "Y8888P88 888    Y8*/

// Frame cnt
assign frame_cnt_plus_one = frame_cnt_reg + 1'b1;
// SPI 
assign spi_mlb          = 1'b1;
assign spi_cdiv         = 1'b0;
assign spi_transmit_dat = data_out_reg;
// assign spi_transmit_dat_LSB = data_out_reg;
// assign spi_transmit_dat = {{spi_transmit_dat_LSB[7:0]},   {spi_transmit_dat_LSB[15:8]}, 
// 						   {spi_transmit_dat_LSB[23:16]}, {spi_transmit_dat_LSB[31:24]} };
// FIFO
assign fifo_din   = din;
assign fifo_we    = we;
assign fifo_ready = (fifo_level > 2'b1) ? 1'b1 : 1'b0;
assign fifo_clr   = 0;
// output
assign dout = spi_dout;
assign sck  = spi_sck;

/*88888888  .d8888b.  888b     d888 
888        d88P  Y88b 8888b   d8888 
888        Y88b.      88888b.d88888 
8888888     "Y888b.   888Y88888P888 
888            "Y88b. 888 Y888P 888 
888              "888 888  Y8P  888 
888        Y88b  d88P 888   "   888 
888         "Y8888P"  888       8*/

// State register and related register
always @ (negedge clk or negedge nrst) begin

	if (!nrst) begin
		current_state <= state_rst;
		frame_cnt_reg <= 9'd0;
		data_out_reg  <= 32'd0;
    end
	else begin
		current_state <= next_state;
		frame_cnt_reg <= frame_cnt_next;
		
		if 		(current_state == state_get_data)
			data_out_reg <= fifo_dout;
		else if (current_state == state_get_last)
			data_out_reg <= last_frame_data;
		else 
			data_out_reg <= data_out_reg;
	end
end
/*
always @ (negedge clk or negedge nrst) begin
	if (!nrst) begin
		spi_start_reg = 1'b0;
    end
	else begin
		if (spi_start)
			spi_start_reg = 1'b1;
		else 	
			spi_start_reg = 1'b0;
	end
end*/

// FSM combinationnal in/out
always @ (current_state or spi_done or fifo_ready or 
		  frame_cnt_reg or frame_cnt_plus_one ) begin

	ss 			   = 1'b0;
	fifo_re        = 1'b0;
	spi_start      = 1'b0;
	frame_cnt_next = 9'b0;
	next_state     = current_state;

	case (current_state)
		
		state_rst : begin // 0
			ss 		   = 1'b1;
			next_state = state_wait;
		end 

		state_wait : begin // 1
			if (fifo_ready)
				next_state = state_start;
			else 
				ss = 1'b1;
		end 

		state_start : begin // 2	
			fifo_re    = 1'b1;
			next_state = state_get_data;
		end  

		state_get_data: begin // 3	
			frame_cnt_next = frame_cnt_plus_one;
			next_state     = state_send;
		end

		state_send : begin // 4
			if (!spi_done) begin
				spi_start      = 1'b1; // TODO: when frame_cnt == 0
				frame_cnt_next = frame_cnt_reg;
			end 
			else begin
				frame_cnt_next = frame_cnt_reg;
				next_state     = state_cnt;
			end
		end

		state_cnt : begin // 5
			if (frame_cnt_reg < total_frame) begin
				frame_cnt_next = frame_cnt_reg;
				fifo_re        = 1'b1;
				next_state     = state_get_data;
			end
			else begin
				next_state = state_get_last;
			end
		end 

		state_get_last: begin // 6
			next_state = state_send_lf;
		end

		state_send_lf : begin // 7
			if (!spi_done) begin
				spi_start  = 1'b1;
				next_state = state_send_lf;
			end 
			else begin
				next_state = state_wait;
			end
		end 

		default : next_state = state_rst;

	endcase
end 

endmodule