
/*88888888 8888888 8888888888  .d88888b.          .d8888b.  8888888b. 8888888 
888          888   888        d88P" "Y88b        d88P  Y88b 888   Y88b  888   
888          888   888        888     888        Y88b.      888    888  888   
8888888      888   8888888    888     888         "Y888b.   888   d88P  888   
888          888   888        888     888            "Y88b. 8888888P"   888   
888          888   888        888     888              "888 888         888   
888          888   888        Y88b. .d88P        Y88b  d88P 888         888   
888        8888888 888         "Y88888P" 88888888 "Y8888P"  888       88888*/


/////////////////////////////////////////////////////////////////////
// This module realise the connexion and control between a fifo 
// and a SPI module. 
// workflow: It accepts 32 bits data in, and store them in a fifo.
// at the moment the fifo reach 255 data in, the spi module start
// to output these 255 data plus 1 word (end frame data).
//
// FUTURE WORK: Implement a SPI block signal that will block 
// 			  	the SPI. Meanwhile, we keep putting the "data in" 
//				in the fifo until the SPI unblock and transmitt data. 
// 			  	Implement a fifo overflow signal
/////////////////////////////////////////////////////////////////////


`include "timescale.v"

module fifo_spi_sram (clk, nrst, block, we, din, dout, fifo_busy, fifo_full, sck, ss, 
		fifo_SRAM_ADDR, fifo_SRAM_DQ, fifo_SRAM_CE_N, fifo_SRAM_OE_N, 
		fifo_SRAM_WE_N, fifo_SRAM_LB_N, fifo_SRAM_UB_N, debug, available);

	// common
	input clk;
	input nrst;
	// Microcontroller block signal
	input block;
	// writting module (data and command)
	input we;
	input [31:0] din;
	output fifo_busy;
	output fifo_full;
	// SPI 
	output dout;
	output sck;
	output reg ss;
	// SRAM
	output wire [19:0] fifo_SRAM_ADDR;
	inout [15:0] fifo_SRAM_DQ;
	output wire fifo_SRAM_CE_N;
	output wire fifo_SRAM_OE_N;
	output wire fifo_SRAM_WE_N;
	output wire fifo_SRAM_LB_N;
	output wire fifo_SRAM_UB_N;
	// debug
	output wire [31:0] debug;
	output wire [21:0] available;
	// States
	parameter 	state_rst    		   = 4'd0,		
				state_wait             = 4'd1,
				state_start            = 4'd2,
				state_wait_data        = 4'd3,
				state_get_data         = 4'd4,
				state_start_spi        = 4'd5,
				state_send             = 4'd6,	
				state_cnt              = 4'd7,	
				state_get_last         = 4'd8,	
				state_start_spi_last   = 4'd9,
				state_send_last        = 4'd10,
				state_wait_inter_frame = 4'd11;

	// Frame cnt
	// parameter total_frame 	  = 5;
	parameter total_frame 	  = 255;
	parameter last_frame_data = 32'h0000_1234;
	parameter MINIMUM_DELAY   = 32'd31111;


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
	wire data_available; 
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
	wire 		fifo_we;
	reg 	 	fifo_re;
	// reg 	 	fifo_re_r;
	wire 		fifo_full;
	wire 		fifo_empty;
	wire 		fifo_data_r_rdy;
	wire [20:0] fifo_available;
	wire [31:0] fifo_din;
	wire [31:0] fifo_dout;  
	wire [7:0] 	fifo_debug;
	// Data line
	reg  [31:0] data_out_reg;
	reg  [31:0] data_out_reg_prev;
	wire 		data_out_not_coherent;
	// Frame cnt
	wire [8:0] frame_cnt_plus_one;
	reg	 [8:0] frame_cnt_reg;
	reg  [32:0] cnt_delay_frame;
	wire [32:0] cnt_delay_frame_plus_one;
	// State machine
	reg  [3:0] current_state_fs;
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
		.done_r	(spi_done), 			.rdata	(rdata) 
		);


	fifo_SRAM fifo_SRAM_i_0 (
		.clk		(clk),
		.rst_n		(nrst),
		// SRAM pins
		.Addr		(fifo_SRAM_ADDR),
		.IO			(fifo_SRAM_DQ),
		.CE_n		(fifo_SRAM_CE_N), // Chip 
		.OE_n		(fifo_SRAM_OE_N), // Outpu
		.WE_n		(fifo_SRAM_WE_N), // Write
		.LB_n		(fifo_SRAM_LB_N), // Lower
		.UB_n		(fifo_SRAM_UB_N), // Upper
		// user interface
		.dataIn		(fifo_din),
		.dataOut	(fifo_dout),
		.user_we	(fifo_we),
		.user_re	(fifo_re),
		// .user_re	(fifo_re_r),
		.data_r_rdy	(fifo_data_r_rdy),
		.busy		(fifo_busy),
		.full		(fifo_full),
		.available  (fifo_available),
		.debug   	(fifo_debug)
	);


		   /*888  .d8888b.   .d8888b. 8888888  .d8888b.  888b    888 
	      d88888 d88P  Y88b d88P  Y88b  888   d88P  Y88b 8888b   888 
	     d88P888 Y88b.      Y88b.       888   888    888 88888b  888 
	    d88P 888  "Y888b.    "Y888b.    888   888        888Y88b 888 
	   d88P  888     "Y88b.     "Y88b.  888   888  88888 888 Y88b888 
	  d88P   888       "888       "888  888   888    888 888  Y88888 
	 d8888888888 Y88b  d88P Y88b  d88P  888   Y88b  d88P 888   Y8888 
	d88P     888  "Y8888P"   "Y8888P" 8888888  "Y8888P88 888    Y8*/

	// Frame cnt
	assign frame_cnt_plus_one = frame_cnt_reg + 9'b1;
	assign cnt_delay_frame_plus_one = cnt_delay_frame + 32'b1;
	// SPI 
	assign spi_mlb          = 1'b1;
	assign spi_cdiv         = 1'b0;
	assign spi_transmit_dat = data_out_reg;
	// assign spi_transmit_dat_LSB = data_out_reg;
	// assign spi_transmit_dat = {{spi_transmit_dat_LSB[7:0]},   {spi_transmit_dat_LSB[15:8]}, 
	// 						   	  {spi_transmit_dat_LSB[23:16]}, {spi_transmit_dat_LSB[31:24]} };
	// FIFO
	assign available = fifo_available;
	assign fifo_din = din;  // data for the fifo
	assign fifo_we  = we;   // write enable for the fifo
	// Control: is there at least 255 words in the fifo ?
	assign data_available = (fifo_available >= (total_frame + 1) ) ? 1'b1 : 1'b0; 
	// output
	assign dout = spi_dout;
	assign sck  = spi_sck;
	// debug
	// assign debug = {4'd0, current_state_fs};
	assign debug = {21'b0, fifo_debug[5], fifo_debug[4], data_out_not_coherent, current_state_fs, fifo_debug[3:0]};

	assign data_out_not_coherent = ( (data_out_reg 		!= (data_out_reg_prev + 32'b1)) &&
							  		 (data_out_reg 	 	!= (last_frame_data)) 		    &&
							  		 (data_out_reg_prev != (last_frame_data)) )
							   		  ? 1'b1 : 1'b0;

	/*88888b.  8888888888  .d8888b.  
	888   Y88b 888        d88P  Y88b 
	888    888 888        888    888 
	888   d88P 8888888    888        
	8888888P"  888        888  88888 
	888 T88b   888        888    888 
	888  T88b  888        Y88b  d88P 
	888   T88b 8888888888  "Y8888P*/

	// data out reg
	always @(posedge clk or negedge nrst) begin
		if (!nrst) begin
			data_out_reg      <= 32'd0;
			data_out_reg_prev <= 32'd0; // debug purpose
	    end
		else begin
			case (current_state_fs)

				state_get_data: begin
					data_out_reg      <= fifo_dout;
					data_out_reg_prev <= data_out_reg; // debug purpose
				end

					state_get_last: begin
					data_out_reg      <= last_frame_data;
					data_out_reg_prev <= data_out_reg_prev; // debug purpose
				end

					default: begin
					data_out_reg      <= data_out_reg;
					data_out_reg_prev <= data_out_reg_prev; // debug purpose
				end
			endcase
		end
	end


	// fifo_re reg
	always @ (negedge clk or negedge nrst) begin
		if (!nrst)
			fifo_re <= 1'd0;
	    else
	    	case (current_state_fs)

				state_start: begin
					if (!fifo_busy)
						fifo_re <= 1'd1;
					else
						fifo_re <= 1'd0;
				end

				state_cnt:  begin
					if (!fifo_busy) begin 
						if (frame_cnt_reg < total_frame)
							fifo_re <= 1'd1;
						else 
							fifo_re <= 1'd0;
					end
				end

				default: fifo_re <= 1'd0;
			endcase
	end


	// Frame counter reg
	always @ (negedge clk or negedge nrst) begin
		if (!nrst)
			frame_cnt_reg <= 9'd0;
		else begin
	    	case (current_state_fs)
				state_get_data:  frame_cnt_reg  <= frame_cnt_plus_one;
				state_start_spi: frame_cnt_reg  <= frame_cnt_reg;
				state_send: 	 frame_cnt_reg  <= frame_cnt_reg;
				state_cnt: 		 frame_cnt_reg  <= frame_cnt_reg;
				state_wait_data: frame_cnt_reg  <= frame_cnt_reg;
				default: 		 frame_cnt_reg  <= 9'd0;
			endcase
		end
	end


	//  counter inter frame reg
	always @ (negedge clk or negedge nrst) begin
		if (!nrst)
			cnt_delay_frame <= 32'd0;
	    else begin
	    	case (current_state_fs)
				state_wait: 	cnt_delay_frame <= 0;
				default: 		cnt_delay_frame <= cnt_delay_frame_plus_one;
			endcase
		end
	end


	//  slave select reg
	always @ (negedge clk or negedge nrst) begin
		if (!nrst)
			ss <= 1'b0;
	    else begin
	    	case (current_state_fs)
				state_rst: 				ss <= 1'b1;
				state_wait: 			ss <= 1'b1;
				state_wait_inter_frame: ss <= 1'b1;
				default: 				ss <= 1'b0;
			endcase
		end
	end


	//  spi start reg
	always @ (negedge clk or negedge nrst) begin
		if (!nrst)
			spi_start <= 1'd0;
	    else begin
	    	case (current_state_fs)
				state_start_spi:  	  spi_start <= 1'b1;
				state_start_spi_last: spi_start <= 1'b1;
				default: 			  spi_start <= 1'b0;
			endcase
		end
	end


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
		if (!nrst) 
			current_state_fs <= state_rst;
		else 
			current_state_fs <= next_state;
	end


	// FSM combinationnal in/out
	always @ (*) begin

		next_state = current_state_fs;

		case (current_state_fs)
			
			state_rst : begin 				// state: 0
				next_state = state_wait;
			end 

			// Wait we got 255 word data is in the fifo
			state_wait : begin 				// state: 1
				if (data_available && (!fifo_busy) && (!block)) 
					next_state = state_start; 
			end 

			// Start the sending of the 255 word data packet
			state_start : begin				// state: 2
				if (!fifo_busy) 
					next_state = state_wait_data;
			end  

			// Get one word data				
			state_wait_data: begin			// state: 3
				// if a glitch happen in data_r_rdy, ==> infinite loop, must optimise this
				// TODO implement a watchdog counter
				if (fifo_data_r_rdy) 
					next_state     = state_get_data;
			end

			// Get one word data				
			state_get_data: begin			// state: 4
					next_state     = state_start_spi;
			end

			state_start_spi: begin    		// state: 5
				next_state = state_send;
			end

			// Send it through the SPI
			state_send : begin 				// state: 6				
				if (spi_done)
					next_state = state_cnt;
			end

			// Check: Did we send all the SPI data for one frame ?
			state_cnt : begin  				// state: 7
				if (!fifo_busy) begin 
					if (frame_cnt_reg < total_frame)
						next_state = state_wait_data;
					else 
						next_state = state_get_last;
				end
			end

			// We send the End data (control) word
			state_get_last: begin 			// state: 8
				next_state = state_start_spi_last;
			end
			
			state_start_spi_last: begin 	// state: 9
				next_state = state_send_last;
			end

			// Check: did the last data has been sent ?
			state_send_last : begin 		// state: 10
				if (spi_done) 
					next_state = state_wait_inter_frame;
					// next_state = state_wait_inter_frame;
			end 

			// Check: did the last data has been sent ?
			state_wait_inter_frame : begin 	// state: 11
				if (cnt_delay_frame > MINIMUM_DELAY) 
					next_state = state_wait;
			end

			default : next_state = state_rst;

		endcase
	end 

endmodule