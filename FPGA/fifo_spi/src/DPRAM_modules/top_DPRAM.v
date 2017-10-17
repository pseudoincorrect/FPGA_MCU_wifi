/*888888888  .d88888b.  8888888b.  
    888     d88P" "Y88b 888   Y88b 
    888     888     888 888    888 
    888     888     888 888   d88P 
    888     888     888 8888888P"  
    888     888     888 888        
    888     Y88b. .d88P 888        
    888      "Y88888P"  8*/

/////////////////////////////////////////////////////////////////////
//  In this TOP module, we create data, replacing data from from 
//  sensor, or other. These data are under the form of a counter
//  to further on check more easily the integrity of the received
//  data. We simply send counting data to the transmitting module
//  (fifo_spi) at regular intervall.
//
//	The trick here is that we have a spi block data signal that 
//  will in our case stop us from sending more data to the 
//  fifo_spi module. this signal is necesseray with dealing with
//  the CC3200 TI wireless module, that sometime cannot accept  to 
//  receive data. It force us, either to stop procucing data (that's 
//  what we are doing here) or to store the produced data to a memory 
//  before sending them later when the CC3200 will get "unstuck" and 
//  will again accept receiving data. the later option is to implement
//  since it will bring much more flexibility to the system.
/////////////////////////////////////////////////////////////////////


module top_DPRAM (
	input 		  CLOCK_50,	//	50 MHz clock
	input  [3:0]  KEY,      //	Pushbutton[3:0]
	input  [17:0] SW,		//	Toggle Switch[17:0]
	output [6:0]  HEX0,HEX1,HEX2,HEX3,HEX4,HEX5,HEX6,HEX7,  // Seven Segment Digits
	output [8:0]  LEDG,  //	LED Green
	output [17:0] LEDR,  //	LED Red
	inout  [35:0]  GPIO	//	GPIO Connections
  	);

parameter 	state_rst         = 4'd0, 		 
			state_wait        = 4'd1, 
			state_delay       = 4'd2,
			state_check_block = 4'd3,
			state_start_send  = 4'd4,
			state_end_send 	  = 4'd5;


/*8       888 8888888 8888888b.  8888888888  .d8888b.   
888   o   888   888   888   Y88b 888        d88P  Y88b  
888  d8b  888   888   888    888 888        Y88b.       
888 d888b 888   888   888   d88P 8888888     "Y888b.    
888d88888b888   888   8888888P"  888            "Y88b.  
88888P Y88888   888   888 T88b   888              "888  
8888P   Y8888   888   888  T88b  888        Y88b  d88P  
888P     Y888 8888888 888   T88b 8888888888  "Y8888*/  

// general
wire clk;
wire rst_n;
wire locked;
wire [7:0] sum_SW;
wire [31:0] delay;
wire SW_17_debounced;
// Pll
wire pll_in_c0;
wire pll_out_c0;
// Fifo spi
wire dout;
wire sck;
wire ss;
wire send_data;
wire spi_block;
wire [31:0] din;
reg  we;
// Seven segment display
wire [4:0]  number_0, number_1, number_2, number_3,
			number_4, number_5, number_6, number_7;
// State Machine
wire [31:0] delay_cnt_plus_one;
wire [31:0] data_cnt_plus_one;
reg  [31:0] delay_cnt;
reg  [31:0] data_cnt;
reg  [3:0] current_state, next_state;
reg tick;


/*8b     d888  .d88888b.  8888888b.  888     888 888      8888888888 
8888b   d8888 d88P" "Y88b 888  "Y88b 888     888 888      888        
88888b.d88888 888     888 888    888 888     888 888      888        
888Y88888P888 888     888 888    888 888     888 888      8888888    
888 Y888P 888 888     888 888    888 888     888 888      888        
888  Y8P  888 888     888 888    888 888     888 888      888        
888   "   888 Y88b. .d88P 888  .d88P Y88b. .d88P 888      888        
888       888  "Y88888P"  8888888P"   "Y88888P"  88888888 88888888*/

// deboucer for the SW17 switch
DeBounce DeBounce_i_0 (clk, rst_n, SW[17], SW_17_debounced);

// pll for 40 MHz clock (to get the 20 MHz SPI)
pll pll_0 (
	.areset (!rst_n),
	.inclk0 (pll_in_c0),
	.c0 	(pll_out_c0),
	.locked (locked)
	);

// Main module
fifo_spi_dpram fifo_spi_i (
		.clk	(clk),
		.nrst	(rst_n),
		.we		(we),
		.din	(din),
		.dout 	(dout),
		.sck  	(sck),
		.ss   	(ss)
		);

//Seven segment display
SSD seven_seg_0 (number_0, HEX0);
SSD seven_seg_1 (number_1, HEX1);
SSD seven_seg_2 (number_2, HEX2);
SSD seven_seg_3 (number_3, HEX3);
SSD seven_seg_4 (number_4, HEX4);
SSD seven_seg_5 (number_5, HEX5);
SSD seven_seg_6 (number_6, HEX6);
SSD seven_seg_7 (number_7, HEX7);


	   /*888  .d8888b.   .d8888b. 8888888  .d8888b.  888b    888 
      d88888 d88P  Y88b d88P  Y88b  888   d88P  Y88b 8888b   888 
     d88P888 Y88b.      Y88b.       888   888    888 88888b  888 
    d88P 888  "Y888b.    "Y888b.    888   888        888Y88b 888 
   d88P  888     "Y88b.     "Y88b.  888   888  88888 888 Y88b888 
  d88P   888       "888       "888  888   888    888 888  Y88888 
 d8888888888 Y88b  d88P Y88b  d88P  888   Y88b  d88P 888   Y8888 
d88P     888  "Y8888P"   "Y8888P" 8888888  "Y8888P88 888    Y8*/

// general
assign sum_SW = SW[0] + SW[1] + SW[2] + SW[3] + SW[4] + SW[5] + 
				SW[6] + SW[7] + SW[8] + SW[9] + SW[10] + SW[11];
assign delay = 32'd128 + (1 << sum_SW);
// sum_SW == 12 ==> 0.3 Mbits/sec
// sum_SW == 11 ==> 0.7 Mbits/sec
// sum_SW == 10 ==> 1.3 Mbits/sec
// sum_SW == 9 ==>  2.5 Mbits/sec
// sum_SW == 8 ==>  4.1 Mbits/sec
// sum_SW == 7 ==>  6.1 Mbits/sec
// sum_SW == 6 ==>  8.2 Mbits/sec

// pll
assign pll_in_c0 = CLOCK_50;
assign clk = pll_out_c0;
// Fifo spi
assign din       = data_cnt;
assign rst_n     = KEY[0];
assign send_data = KEY[1];
assign GPIO[0]   = sck;
assign GPIO[1]   = dout;
assign GPIO[2]   = ss;
// debug
assign GPIO[6]   = sck;
assign GPIO[7]   = dout;
assign GPIO[8]   = ss;
assign GPIO[9]   = tick;
// debug tick to measure the frequency at which the 
// 256*32 bits data are sent to the fifo_spi module
assign GPIO[3]    = tick; 
assign GPIO[35:10] = {25{1'bz}}; // High impedance for the inputs

// debug (fake spi block)
// assign spi_block = SW_17_debounced;
// assign GPIO[35]  = SW_17_debounced;
// assign GPIO[34]  = SW[17];
assign spi_block  = GPIO[35];

// State Machine
assign delay_cnt_plus_one = delay_cnt + 32'b1;
assign data_cnt_plus_one  = data_cnt  + 32'b1;
// Seven segment display
assign number_0 = 5'h1_0;
assign number_1 = 5'h1_0;
assign number_2 = 5'h1_0;
assign number_3 = 5'h1_0;
assign number_4 = 5'h1_0;
assign number_5 = 5'h1_0;
assign number_6 = 5'h1_0;
assign number_7 = {1'b0, current_state};
// default values
assign LEDR       = 0;
assign LEDG       = 0;


/*88888888  .d8888b.  888b     d888 
888        d88P  Y88b 8888b   d8888 
888        Y88b.      88888b.d88888 
8888888     "Y888b.   888Y88888P888 
888            "Y88b. 888 Y888P 888 
888              "888 888  Y8P  888 
888        Y88b  d88P 888   "   888 
888         "Y8888P"  888       8*/

// Registers flops :
//		State register 
// 		Delay counter register (to create sending time interval)
// 		Data counter register (to create the fake data sent to fifo_spi)
always @ (negedge clk or negedge rst_n) begin

	if (!rst_n) begin
		current_state <= state_rst;
		delay_cnt     <= 0;
		data_cnt	  <= 0;
    end

	else begin

		current_state <= next_state;

		case (current_state)

			state_wait : begin
				delay_cnt <= 0;
				data_cnt  <= data_cnt;
			end			

			state_delay : begin
				delay_cnt <= delay_cnt_plus_one;
				data_cnt  <= data_cnt;
			end

			state_start_send : begin
				delay_cnt <= delay_cnt;
				data_cnt  <= data_cnt_plus_one;
			end

			default : begin
				delay_cnt <= delay_cnt;
				data_cnt  <= data_cnt;
			end
		endcase
	end
end


// FSM combinationnal in/out
// Used to send data regularly to the fifo_spi module
always @ (*) begin
	
	next_state = current_state;
	we         = 1'b0;
	tick	   = 1'b0;

	case (current_state)
		
		state_rst : begin
			if (!send_data) begin
				next_state = state_wait;
			end
		end 

		// Step used to reinit the counter register
		state_wait : begin
			tick       = 1'b1;			
			next_state = state_delay;
		end 

		// Check: is this the time to send a 32 bit data to fifo_spi ?
		state_delay : begin
			if (delay_cnt >= delay) begin
				next_state = state_check_block;
			end
		end

		// Check: is the SPI blocked (CC3200 TCP conn slow) ?
		state_check_block : begin
			if (!spi_block) 
				next_state = state_start_send;
			else
				next_state = state_check_block;
			
		end

		// send data to the fifo_spi module
		state_start_send: begin
			we         = 1'b1;
			tick       = 1'b1;
			next_state = state_end_send;
		end

		// extra step useful for debugging
		state_end_send : begin
			tick       = 1'b1;
			next_state = state_wait;
		end 

		default : next_state = state_rst;

	endcase
end

endmodule





