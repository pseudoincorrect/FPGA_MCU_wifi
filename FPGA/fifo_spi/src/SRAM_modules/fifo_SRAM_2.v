// Fifo implemented on a single port SRAM
// Due to the single port, the write operation
// has been implementd witht the priority on the read operation
// the pre write signal indicate the reader has to wait for one
// clock cycle before reading the fifo. 

module fifo_SRAM(
	// common
	input wire clk,
	input wire rst_n,
	// Sram pins
	output wire [19:0] Addr,
	inout [15:0] IO,
	output reg CE_n, // Chip enable_n
	output reg OE_n, // Output enable_n
	output reg WE_n, // Write  enable_n
	output reg LB_n, // Lower Byte Control_n
	output reg UB_n, // Upper   Byte Control_n
	// user interface
	input  wire [31:0] dataIn,
	output wire  [31:0] dataOut,
	input  wire user_we,
	input  wire user_re,
	output reg data_r_rdy,
	output reg busy,
	output reg full,
	output reg  [21:0] available,
	output wire [7:0] debug
);

	parameter S_RESET		= 4'd0;
	parameter S_IDLE		= 4'd1;
	parameter S_CATCH_WE_1	= 4'd2;
	parameter S_CATCH_WE_2	= 4'd3;
	parameter S_CATCH_RE_1	= 4'd4;
	parameter S_CATCH_RE_2	= 4'd5;
	parameter S_WRITE_UP_B	= 4'd6;
	parameter S_MID_WRITE	= 4'd7;
	parameter S_WRITE_LOW_B	= 4'd8;
	parameter S_READ_UP_B	= 4'd9;
	parameter S_READ_LOW_B	= 4'd10;
	parameter S_READ_DONE	= 4'd11;
	parameter S_READ_DONE_2	= 4'd12;

	parameter MUX_FPGA_TO_SRAM = 1'b1; // tristate parameter sram data
	parameter MUX_SRAM_TO_FPGA = 1'b0; // tristate parameter sram data

	// max size 1024*1024 halfwords (16 bits) with a margin of 4'hF (F_FFFF - 0_000F)
	// parameter BUFFER_SIZE = 21'h 00_1000; // = 2^20 (or 1 << 20) = 2 MB
	parameter BUFFER_SIZE = 21'b 1_0000_0000_0000_0000_0000; // = 2^20 (or 1 << 20) = 2 MB

	reg [3:0]  	current_state_f;
	reg [3:0]  	next_state;
	// reg [3:0] 	readAddr;
	// reg [3:0] 	writeAddr;
	reg [19:0] 	readAddr;
	reg [19:0] 	writeAddr;
	reg  		block;
	reg  		DAT_MUX;
	reg [31:0] 	data_IO_out_reg;
	reg [31:0] 	data_in_reg;
	reg [31:0] 	data_in_reg_prev;
	wire		data_in_reg_not_coherent;
	reg [15:0]  data_IO_in;
	reg 		read_write_reg;
	reg 	  	write_fifo;
	reg		  	read_fifo_start;
	reg		  	read_fifo;
	wire 		rollback_we;
	wire 		rollback_re;
	// reg 		empty;

	assign rollback_we = (writeAddr == 20'hF_FFFF) ? 1'b1 : 1'b0;
	assign rollback_re = (readAddr == 20'hF_FFFF) ? 1'b1 : 1'b0;

	// assign debug = {3'b0, data_in_reg_not_coherent, current_state_f};
	assign debug = {2'b0, rollback_re, rollback_we, current_state_f};

	assign Addr = (DAT_MUX == MUX_FPGA_TO_SRAM) ? {writeAddr} : {readAddr};

	// IO tri-state		write to RAM ?			  write to RAM	   High imp for reading from RAM
	assign IO   = (DAT_MUX == MUX_FPGA_TO_SRAM) ? data_IO_in : 16'bzzzz_zzzz_zzzz_zzzz;

	assign data_in_reg_not_coherent  = (data_in_reg != (data_in_reg_prev + 31'b1)) ? 1'b1 : 1'b0;

	// full reg
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			full <= 0;
	    end
		else begin
			full <= (available > ((BUFFER_SIZE >> 1) - 21'd512) )  ? 1'b1 : 1'b0; 
		end
	end


	// // empty reg
	// always @(posedge clk or negedge rst_n) begin
	// 	if (!rst_n) begin
	// 		empty <= 0;
	//     end
	// 	else begin
	// 		empty <= (writeAddr == readAddr )  ? 1'b1 : 1'b0; 
	// 	end
	// end


	// available data in the Fifo
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			available <= 0;
	    end
		else begin
			if (writeAddr > readAddr)
				available <= (writeAddr - readAddr) >> 1;
			else if (writeAddr < readAddr)
				available <= (BUFFER_SIZE + writeAddr - readAddr) >> 1;
			else
				available <= 0;
		end
	end


	// Data In Reg (to write to RAM)
	always @(*) begin
		case (current_state_f)
			S_WRITE_UP_B:  data_IO_in = data_in_reg[31:16];
			// S_MID_WRITE:   data_IO_in = data_in_reg[15:0];
			S_WRITE_LOW_B: data_IO_in = data_in_reg[15:0];
			default: 	   data_IO_in = 16'h0;
		endcase
	end


	// Data out Reg (to Read from RAM)
	assign dataOut = data_IO_out_reg;

	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			 data_IO_out_reg <= 0;
	    end
		else begin
			case (current_state_f)
				S_READ_UP_B:  data_IO_out_reg <= {IO, 16'b0};
				S_READ_LOW_B: data_IO_out_reg <= {data_IO_out_reg[31:16], IO};
				default: 	  data_IO_out_reg <= data_IO_out_reg;
			endcase
		end
	end


	// Write Address
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			writeAddr <= 0;
	    end
		else begin
			case (current_state_f)
				S_WRITE_UP_B:  writeAddr <= writeAddr + 20'b1;
				S_WRITE_LOW_B: writeAddr <= writeAddr + 20'b1;
				default: 	   writeAddr <= writeAddr;
			endcase
		end
	end


	// Read Address 
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			readAddr <= 0;
	    end
		else begin
			case (current_state_f)
				S_READ_UP_B:  readAddr <= readAddr + 20'b1;
				S_READ_LOW_B: readAddr <= readAddr + 20'b1;
				default: 	  readAddr <= readAddr;
			endcase
		end
	end

	// Data in Reg
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			 data_in_reg <= 0;
			 data_in_reg_prev <= 0;
	    end
		else begin
			if (user_we) begin
				data_in_reg 	 <= dataIn;
				data_in_reg_prev <= data_in_reg;
			end
			else begin
				data_in_reg 	 <= data_in_reg;
				data_in_reg_prev <= data_in_reg_prev;
			end
		end
	end

	
	// read_write register
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			 read_write_reg <= 0;
	    end
		else begin
			case (current_state_f)

				S_IDLE: begin
					if (user_re && user_we) read_write_reg <= 1'b1;
					else  					read_write_reg <= 1'b0;
				end

				S_CATCH_WE_1: begin
					if (user_we) read_write_reg <= 1'b1;
					else  		 read_write_reg <= 1'b0;
				end

				S_CATCH_WE_2: begin
					if (user_we) read_write_reg <= 1'b1;
					else  		 read_write_reg <= 1'b0;
				end

				S_CATCH_RE_1: begin
					if (user_re) read_write_reg <= 1'b1;
					else 		 read_write_reg <= 1'b0;
				end

				S_CATCH_RE_2: begin
					if (user_re) read_write_reg <= 1'b1;
					else 		 read_write_reg <= read_write_reg;
				end

				default: begin
					read_write_reg = read_write_reg;
				end

			endcase
		end
	end

	// assign busy = (current_state_f == S_IDLE) ? 1'b0 : 1'b1;

	// busy register
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			 busy <= 1'b0;
	    end
		else begin
			if (current_state_f == S_IDLE)
				busy <= 1'b0;
			else
				busy <= 1'b1;
		end
	end


	// Data_ready reg
	always @(posedge clk or negedge rst_n) begin
	if (!rst_n) begin
			 data_r_rdy = 1'b0;
	    end
		else begin
			case (current_state_f)
				S_READ_DONE:   data_r_rdy = 1'b1;
				S_READ_DONE_2: data_r_rdy = 1'b1;
				default: 	   data_r_rdy = 1'b0;
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

	// FSM State register
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			current_state_f <= S_RESET;
	    end
		else begin
			current_state_f <= next_state;
		end
	end


	// FSM Combinationnal in/out
	always @ (*) begin 
		next_state = current_state_f;
		DAT_MUX    = MUX_SRAM_TO_FPGA;

		WE_n = 1; // Write  enable_n
		OE_n = 0; // Output enable_n
		CE_n = 0; // Chip   enable_n
		UB_n = 0; // Upper Byte Control
		LB_n = 0; // Lower Byte Control


		case(current_state_f)

			////////////////////////////////////////////////////////
			S_RESET: begin  // 0
				next_state = S_IDLE;
			end

			////////////////////////////////////////////////////////
			S_IDLE: begin  // 1 

				case ({user_re, user_we})
					2'b00:   next_state = S_IDLE; 
					2'b01:   next_state = S_CATCH_RE_1; 	// read
					2'b10:   next_state = S_CATCH_WE_1;   // write
					2'b11:   next_state = S_WRITE_UP_B; // write read
					default: next_state = S_IDLE;
				endcase
			end

			////////////////////////////////////////////////////////
			S_CATCH_WE_1: begin  // 2
				UB_n = 1; // Upper Byte Control
				LB_n = 1; // Lower Byte Control
				if (user_we) next_state = S_WRITE_UP_B;
				else 		 next_state = S_CATCH_WE_2;
			end 

			////////////////////////////////////////////////////////
			S_CATCH_WE_2: begin  // 3
				UB_n = 1; // Upper Byte Control
				LB_n = 1; // Lower Byte Control
				if (user_we) next_state = S_WRITE_UP_B;
				else 		 next_state = S_READ_UP_B;
			end 

			////////////////////////////////////////////////////////
			S_CATCH_RE_1: begin  // 4
				UB_n = 1; // Upper Byte Control
				LB_n = 1; // Lower Byte Control
				next_state = S_CATCH_RE_2;
			end 

						////////////////////////////////////////////////////////
			S_CATCH_RE_2: begin  // 5
				UB_n = 1; // Upper Byte Control
				LB_n = 1; // Lower Byte Control
				next_state = S_WRITE_UP_B;
			end
			
			////////////////////////////////////////////////////////
			S_WRITE_UP_B: begin  // 7
				next_state = S_MID_WRITE;
				DAT_MUX = MUX_FPGA_TO_SRAM;
				WE_n = 0; // Write  enable_n
				OE_n = 1; // Output enable_n
			end 

			////////////////////////////////////////////////////////
			S_MID_WRITE: begin  // 6
				UB_n = 1; // Upper Byte Control
				LB_n = 1; // Lower Byte Control
				next_state = S_WRITE_LOW_B;
			end 

			////////////////////////////////////////////////////////
			S_WRITE_LOW_B: begin  // 8
				if (read_write_reg) begin
					next_state = S_READ_UP_B;
				end else begin
					next_state = S_IDLE;
				end
				DAT_MUX = MUX_FPGA_TO_SRAM;
				WE_n = 0; // Write  enable_n
				OE_n = 1; // Output enable_n
			end 
			
			////////////////////////////////////////////////////////
			S_READ_UP_B: begin  // 9
				next_state = S_READ_LOW_B;
			end 
			
			////////////////////////////////////////////////////////
			S_READ_LOW_B: begin  // 10
				// next_state = S_IDLE;
				next_state = S_READ_DONE;
			end 		

			////////////////////////////////////////////////////////
			S_READ_DONE: begin  // 11
				next_state = S_READ_DONE_2;
			end 	
						////////////////////////////////////////////////////////
			S_READ_DONE_2: begin  // 11
				next_state = S_IDLE;
			end 


			////////////////////////////////////////////////////////
			default : begin
				next_state = S_IDLE;
			end
		endcase
	end
		

endmodule
