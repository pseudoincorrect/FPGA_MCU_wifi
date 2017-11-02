/*8888b.  8888888b.         d8888 888b     d888          
d88P  Y88b 888   Y88b       d88888 8888b   d8888         
Y88b.      888    888      d88P888 88888b.d88888         
 "Y888b.   888   d88P     d88P 888 888Y88888P888         
    "Y88b. 8888888P"     d88P  888 888 Y888P 888         
      "888 888 T88b     d88P   888 888  Y8P  888         
Y88b  d88P 888  T88b   d8888888888 888   "   888         
 "Y8888P"  888   T88b d88P     888 888       888

 8888888888 8888888 8888888888  .d88888b.  
 888          888   888        d88P" "Y88b 
 888          888   888        888     888 
 8888888      888   8888888    888     888 
 888          888   888        888     888 
 888          888   888        888     888 
 888          888   888        Y88b. .d88P 
 888        8888888 888         "Y88888*/

/////////////////////////////////////////////////////////////////////
// Fifo implemented on a single port SRAM. Due to the single port, 
// the write operation and read operation have to be scheduled
// The catch write and catch read state are taking care of that point
// to sequentially write and read in case of simultaneous request 
// from the writting and reading devices. The busy signal indicate
// that the fifo won't accept any write or read request. the writter 
// and reader device have to consider the busy signal.
/////////////////////////////////////////////////////////////////////

module fifo_SRAM(
	// common
	input wire clk,
	input wire rst_n,
	// Sram pins
	output wire [19:0] Addr,
	inout [15:0] IO,
	output reg  WE_n, // Write  enable_n
	output reg  OE_n, // Output enable_n
	output wire CE_n, // Chip enable_n
	output wire LB_n, // Lower Byte Control_n
	output wire UB_n, // Upper   Byte Control_n
	// user interface
	input  wire [31:0] dataIn,
	output wire  [31:0] dataOut,
	input  wire user_we,
	input  wire user_re,
	output reg  data_r_rdy,
	output reg  busy,
	output reg  full,
	output reg  [21:0] available,
	output wire [7:0]  debug
);

	parameter S_RESET           = 4'd0;
	parameter S_IDLE            = 4'd1;
	parameter S_CATCH_WE_1      = 4'd2;
	parameter S_CATCH_WE_2      = 4'd3;
	parameter S_CATCH_RE_1      = 4'd4;
	parameter S_CATCH_RE_2      = 4'd5;
	parameter S_WRITE_UP_READY  = 4'd6;
	parameter S_WRITE_UP        = 4'd7;
	parameter S_WRITE_LOW_READY = 4'd8;
	parameter S_WRITE_LOW       = 4'd9;
	parameter S_READ_READY      = 4'd10;
	parameter S_READ_UP         = 4'd11;
	parameter S_READ_LOW        = 4'd12;
	parameter S_READ_DONE       = 4'd13;

	parameter MUX_FPGA_TO_SRAM = 1'b1; // tristate parameter sram data
	parameter MUX_SRAM_TO_FPGA = 1'b0; // tristate parameter sram data

	// max size 1024*1024 halfwords (16 bits) with a margin of 4'hF (F_FFFF - 0_000F)
	parameter BUFFER_SIZE = 21'b 1_0000_0000_0000_0000_0000; // = 2^20 (or 1 << 20) = 2 MB


	/*88888b.  8888888888  .d8888b.  888      
	888  "Y88b 888        d88P  Y88b 888      
	888    888 888        888    888 888      
	888    888 8888888    888        888      
	888    888 888        888        888      
	888    888 888        888    888 888      
	888  .d88P 888        Y88b  d88P 888      
	8888888P"  8888888888  "Y8888P"  888888*/ 

	reg [3:0]  	next_state;
	reg [19:0] 	read_addr;
	reg [19:0] 	write_addr;
	reg  		block;
	reg  		DAT_MUX;
	reg [31:0] 	data_IO_out_reg;
	reg [31:0] 	data_in_reg;	
	reg [15:0]  IO_to_SRAM;
	wire [15:0] IO_from_SRAM;
	wire [15:0] out_enable;
	reg 		read_write_reg;
	reg 	  	write_fifo;
	reg		  	read_fifo_start;
	reg		  	read_fifo;
	wire 		rollback_we;
	wire 		rollback_re;


		   /*888  .d8888b.   .d8888b. 8888888  .d8888b.  888b    888 
	      d88888 d88P  Y88b d88P  Y88b  888   d88P  Y88b 8888b   888 
	     d88P888 Y88b.      Y88b.       888   888    888 88888b  888 
	    d88P 888  "Y888b.    "Y888b.    888   888        888Y88b 888 
	   d88P  888     "Y88b.     "Y88b.  888   888  88888 888 Y88b888 
	  d88P   888       "888       "888  888   888    888 888  Y88888 
	 d8888888888 Y88b  d88P Y88b  d88P  888   Y88b  d88P 888   Y8888 
	d88P     888  "Y8888P"   "Y8888P" 8888888  "Y8888P88 888    Y8*/


	assign rollback_we = (write_addr == 20'hF_FFFF) ? 1'b1 : 1'b0;
	assign rollback_re = (read_addr  == 20'hF_FFFF) ? 1'b1 : 1'b0;

	assign debug = {2'b0, rollback_re, rollback_we, next_state};

	assign Addr = (DAT_MUX == MUX_FPGA_TO_SRAM) ? {write_addr} : {read_addr};

	// IO tri-state		write to RAM ?			  write to RAM	   High imp for reading from RAM
	assign IO   = (DAT_MUX == MUX_FPGA_TO_SRAM) ? IO_to_SRAM : 16'bzzzz_zzzz_zzzz_zzzz;
	assign IO_from_SRAM =  IO;

	assign CE_n = 0; // Chip   enable_n
	assign UB_n = 0; // Upper Byte Control
	assign LB_n = 0; // Lower Byte Control

	// Data out Reg (to Read from RAM)
	assign dataOut = data_IO_out_reg;


	/*88888b.  8888888888  .d8888b.  
	888   Y88b 888        d88P  Y88b 
	888    888 888        888    888 
	888   d88P 8888888    888        
	8888888P"  888        888  88888 
	888 T88b   888        888    888 
	888  T88b  888        Y88b  d88P 
	888   T88b 8888888888  "Y8888P*/

	// full reg
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			full <= 0;
	    end
		else begin
			full <= (available > ((BUFFER_SIZE >> 1) - 21'd512) )  ? 1'b1 : 1'b0; 
		end
	end


	// available data in the Fifo
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			available <= 0;
	    end
		else begin
			if (write_addr > read_addr)
				available <= (write_addr - read_addr) >> 1;
			else if (write_addr < read_addr)
				available <= (BUFFER_SIZE + write_addr - read_addr) >> 1;
			else
				available <= 0;
		end
	end


	// Data in Reg
	always @(posedge clk or negedge rst_n) begin
		if (!rst_n) data_in_reg <= 0;	    
		else 
			if (user_we) data_in_reg <= dataIn;
			else 		 data_in_reg <= data_in_reg;
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
			next_state <= S_RESET;
			DAT_MUX         <= MUX_SRAM_TO_FPGA;
			WE_n            <= 1; // Write disabled
			busy            <= 0;
			data_r_rdy      <= 0;
			write_addr      <= 0;
			read_addr       <= 0;
			read_write_reg  <= 0;
			data_IO_out_reg <= 0;
	    end
		else begin

			case(next_state)

				////////////////////////////////////////////////////////
				//////////////////// RESET /////////////////////////////
				////////////////////////////////////////////////////////
				S_RESET: begin  // state no : 0
					next_state      <= S_IDLE;
					DAT_MUX         <= MUX_SRAM_TO_FPGA;
					WE_n            <= 1; // Write disabled
					busy            <= 0;
					data_r_rdy      <= 0;
					write_addr      <= 0;
					read_addr       <= 0;
					read_write_reg  <= 0;
					data_IO_out_reg <= 0;
				end

				////////////////////////////////////////////////////////
				///////////////////// IDLE /////////////////////////////
				////////////////////////////////////////////////////////
				S_IDLE: begin  // state no : 1 
					data_r_rdy      <= 0;

					case ({user_re, user_we})
						2'b00: begin // nothing
							next_state     <= S_IDLE; 
							busy           <= 0;
							read_write_reg <= 0;
						end
						2'b01: begin // write
							next_state     <= S_CATCH_RE_1; 	
							busy           <= 1;
							read_write_reg <= 0;
						end
						2'b10: begin // read
							next_state     <= S_CATCH_WE_1;   
							busy           <= 1;
							read_write_reg <= 0;
						end
						2'b11: begin // write read
							next_state     <= S_WRITE_UP_READY; 
							DAT_MUX = MUX_FPGA_TO_SRAM;
							busy           <= 1;
							read_write_reg <= 1;
						end
						default: begin 
							next_state     <= S_IDLE;
							busy           <= 1;
							read_write_reg <= 0;
						end
					endcase
				end

				////////////////////////////////////////////////////////
				//////////////////// CATCH /////////////////////////////
				////////////////////////////////////////////////////////
				S_CATCH_WE_1: begin  // state no : 2
					if (user_we) begin 
						next_state     <= S_WRITE_UP_READY;
						DAT_MUX = MUX_FPGA_TO_SRAM;
						read_write_reg <= 1;
					end
					else next_state <= S_CATCH_WE_2;
				end 
				////////////////////////////////////////////////////////
				S_CATCH_WE_2: begin  // state no : 3
					if (user_we) begin 
						next_state     <= S_WRITE_UP_READY;
						DAT_MUX = MUX_FPGA_TO_SRAM;
						read_write_reg <= 1;
					end
					else next_state <= S_READ_READY;
				end 
				////////////////////////////////////////////////////////
				S_CATCH_RE_1: begin  // state no : 4
					if (user_re) begin	
						next_state     <= S_WRITE_UP_READY;
						DAT_MUX = MUX_FPGA_TO_SRAM;
						read_write_reg <= 1;
					end
					else next_state <= S_CATCH_RE_2;
				end 
				////////////////////////////////////////////////////////
				S_CATCH_RE_2: begin  // state no : 5
					next_state <= S_WRITE_UP_READY;
					DAT_MUX = MUX_FPGA_TO_SRAM;
					if (user_re)  read_write_reg <= 1;
				end
				
				////////////////////////////////////////////////////////
				/////////////////// WRITE //////////////////////////////
				////////////////////////////////////////////////////////
				S_WRITE_UP_READY: begin  // state no : 6
					next_state <= S_WRITE_UP;
					DAT_MUX    <= MUX_FPGA_TO_SRAM;
					WE_n       <= 0;
					OE_n       <= 1;
					IO_to_SRAM <= data_in_reg[31:16];
				end 
				////////////////////////////////////////////////////////
				S_WRITE_UP: begin  // state no : 7
					next_state <= S_WRITE_LOW_READY;
					DAT_MUX    <= MUX_FPGA_TO_SRAM;
					WE_n       <= 1; 
					OE_n       <= 0; 
					write_addr <= write_addr + 20'd1;
				end 
				////////////////////////////////////////////////////////
				S_WRITE_LOW_READY: begin  // state no : 8
					next_state <= S_WRITE_LOW;
					DAT_MUX    <= MUX_FPGA_TO_SRAM;
					WE_n       <= 0;
					OE_n       <= 1;
					IO_to_SRAM <= data_in_reg[15:0];
				end 
				////////////////////////////////////////////////////////
				S_WRITE_LOW: begin  // state no : 9
					if (read_write_reg) next_state <= S_READ_READY;
					else 			   	next_state <= S_IDLE;
					DAT_MUX    <= MUX_SRAM_TO_FPGA;
					WE_n       <= 1; 
					OE_n       <= 0; 
					write_addr <= write_addr + 20'd1;
				end 

				////////////////////////////////////////////////////////
				//////////////////// READ //////////////////////////////
				////////////////////////////////////////////////////////
				S_READ_READY: begin  // state no : 10
					next_state <= S_READ_UP;
					WE_n       <= 1; 
					OE_n       <= 0; 
					DAT_MUX    <= MUX_SRAM_TO_FPGA;
				end 
				////////////////////////////////////////////////////////
				S_READ_UP: begin  // state no : 11
					next_state      <= S_READ_LOW;
					data_IO_out_reg <= {IO_from_SRAM, 16'b0};
					read_addr       <= read_addr + 20'd1;
				end 
				////////////////////////////////////////////////////////
				S_READ_LOW: begin  // state no : 12
					data_IO_out_reg <= {data_IO_out_reg[31:16], IO_from_SRAM};
					next_state      <= S_READ_DONE;
					read_addr       <= read_addr + 20'd1;
				end 
				////////////////////////////////////////////////////////
				S_READ_DONE: begin  // state no : 13
					next_state <= S_IDLE;
					data_r_rdy <= 1;
				end 

				////////////////////////////////////////////////////////
				////////////////// DEFAULT /////////////////////////////
				////////////////////////////////////////////////////////
				default : begin
					next_state <= S_IDLE;
				end
			endcase
		end
	end	

endmodule
