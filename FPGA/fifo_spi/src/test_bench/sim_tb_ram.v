module sim_tb_ram (
	// main clock
	input CLOCk_50,
	// Switches & keys
	input  [17:0] SW,
	input  [3:0]  KEY,
	output [17:0] LEDR,
	// Hex displays (seven segment displays)
	output [6:0]  HEX0, HEX1, HEX2, HEX3, HEX4, HEX5, HEX6, HEX7,
	// SRAM
	inout  [15:0] SRAM_DQ,
	output [19:0] SRAM_ADDR,
	output SRAM_CE_N, SRAM_OE_N, SRAM_WE_N, SRAM_UB_N, SRAM_LB_N
	);

	parameter S_RESET 		= 4'd0;
	parameter S_IDLE 		= 4'd1;
	parameter S_WE_PUSHED 	= 4'd2;
	parameter S_WE_RELEASED = 4'd3;
	parameter S_RE_PUSHED 	= 4'd4;
	parameter S_RE_RELEASED = 4'd5;

	reg clk, rst_n;
	reg user_re, user_we;
	reg  user_re_reg, user_we_reg;
	wire data_r_rdy, busy, overflow;
	wire [4:0] ssd_0_data;
	wire [4:0] ssd_1_data;
	wire [4:0] ssd_2_data;
	wire [4:0] ssd_3_data;
	wire [4:0] ssd_4_data;
	wire [4:0] ssd_5_data;
	wire [4:0] ssd_6_data;
	wire [4:0] ssd_7_data;
	reg [31:0] fifo_data_in;
	wire [31:0] fifo_data_out;
	reg  [31:0] fifo_data_out_r;

	wire [3:0] state_sram;
	reg [3:0] current_push_state, next_push_state;

	// SIMULATION
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	always begin
		clk = 1; #5; clk = 0; #5;
	end

	initial begin
		user_re = 0;
		user_we = 0;
		fifo_data_in = 0;
		rst_n   = 0;
		#21;

		rst_n   = 1;
		#20;

		fifo_data_in = 5;
		user_we = 1;
		#10;
		user_we = 0;
		#40;

		fifo_data_in = 6;
		user_we = 1;
		#10;
		user_we = 0;
		#40;

		user_re = 1;
		#10;
		user_re = 0;
		#40;

		fifo_data_in = 7;
		user_we = 1;
		#10;
		user_we = 0;
		#40;

		fifo_data_in = 8;
		user_we = 1;
		#10;
		user_we = 0;
		#40;

		user_re = 1;
		#10;
		user_re = 0;
		#40;

	end
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////


	// fifo Data out
	assign {ssd_1_data[3:0], ssd_0_data[3:0]} = fifo_data_out[7:0];
	assign {ssd_1_data[4], ssd_0_data[4]} = 2'b0;
	// fifo Data in
	// assign fifo_data_in = {24'b0, SW[7:0]};
	assign {ssd_7_data[3:0], ssd_6_data[3:0]} = fifo_data_in[7:0];
	assign {ssd_7_data[4], ssd_6_data[4]} = 2'b0;
	// common
	// assign clk        = CLOCk_50;
	// assign user_re    = !KEY[2];
	// assign user_we    = !KEY[1];
	// assign rst_n      = KEY[0];
	assign LEDR[17:3] = 15'b0;
	assign LEDR[0]    = !rst_n;
	assign LEDR[1]	  = overflow;
	assign LEDR[2]	  = busy;
	// ssd inactive lines
	assign ssd_2_data = {1'b0, state_sram};
	assign ssd_3_data = 5'b11111;
	assign ssd_4_data = 5'b11111;
	assign ssd_5_data = 5'b11111;

	SSD ssd_i_0 (ssd_0_data, HEX0);
	SSD ssd_i_1 (ssd_1_data, HEX1);
	SSD ssd_i_2 (ssd_2_data, HEX2);
	SSD ssd_i_3 (ssd_3_data, HEX3);
	SSD ssd_i_4 (ssd_4_data, HEX4);
	SSD ssd_i_5 (ssd_5_data, HEX5);
	SSD ssd_i_6 (ssd_6_data, HEX6);
	SSD ssd_i_7 (ssd_7_data, HEX7);

	fifo_SRAM fifo_SRAM_i_0 (
		.clk		(clk),
		.rst_n		(rst_n),
		// SRAM pins
		.Addr		(SRAM_ADDR),
		.IO			(SRAM_DQ),
		.CE_n		(SRAM_CE_N), // Chip 
		.OE_n		(SRAM_OE_N), // Outpu
		.WE_n		(SRAM_WE_N), // Write
		.LB_n		(SRAM_LB_N), // Lower
		.UB_n		(SRAM_UB_N), // Upper
		// user interface
		.dataIn		(fifo_data_in),
		.dataOut	(fifo_data_out),
		.user_we	(user_we_reg),
		.user_re	(user_re_reg),
		.data_r_rdy	(data_r_rdy),
		.busy		(busy),
		.overflow	(overflow),
		.state   	(state_sram)
	);

	// Data out register update
	always @ (posedge clk) begin
		if (!rst_n) begin
			fifo_data_out_r = 32'b0;
	    end
		else begin
			if (data_r_rdy)
				fifo_data_out_r = fifo_data_out;
			else 
				fifo_data_out_r = fifo_data_out_r;
		end
	end

	////////////////////////////////////////////////////////////////
	// 							FSM								  //
	////////////////////////////////////////////////////////////////
	// State register
	always @ (posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			current_push_state = S_RESET;
	    end
		else begin
			current_push_state = next_push_state;
		end
	end

	// WE reg
	always @ (posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			user_we_reg = 1'b0;
	    end
		else begin
			if (current_push_state == S_WE_PUSHED)
				user_we_reg = 1'b1;
			else
				user_we_reg = 1'b0;
		end
	end

	// RE reg
	always @ (posedge clk or negedge rst_n) begin
		if (!rst_n) begin
			user_re_reg = 1'b0;
	    end
		else begin
		if (current_push_state == S_RE_PUSHED)
			user_re_reg = 1'b1;
		else
			user_re_reg = 1'b0;
		end
	end

	// FSM Combinationnal in/out
	always @ (*) begin

		next_push_state = current_push_state;

		case(current_push_state)

			////////////////////////////////////////////////////////
			S_RESET: begin
				next_push_state = S_IDLE;
			end

			////////////////////////////////////////////////////////
			S_IDLE: begin
				if (user_we)
					next_push_state = S_WE_PUSHED;
				if (user_re)
					next_push_state = S_RE_PUSHED;
			end

			////////////////////////////////////////////////////////
			S_WE_PUSHED: begin
				next_push_state = S_WE_RELEASED;
			end

			////////////////////////////////////////////////////////
			S_WE_RELEASED: begin
				if (!user_we)
					next_push_state = S_IDLE;
			end

			////////////////////////////////////////////////////////
			S_RE_PUSHED: begin
				next_push_state = S_RE_RELEASED;
			end

			////////////////////////////////////////////////////////
			S_RE_RELEASED: begin
				if (!user_re)
					next_push_state = S_IDLE;
			end

			////////////////////////////////////////////////////////
			default: begin
				next_push_state = S_IDLE;
			end

		endcase
	end
endmodule