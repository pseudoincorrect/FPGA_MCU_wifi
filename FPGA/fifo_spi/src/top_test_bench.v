/*888888888  .d88888b.  8888888b.  
    888     d88P" "Y88b 888   Y88b 
    888     888     888 888    888 
    888     888     888 888   d88P 
    888     888     888 8888888P"  
    888     888     888 888        
    888     Y88b. .d88P 888        
    888      "Y88888P"  8*/

`include "timescale.v"

module top_test_bench;


/*8       888 8888888 8888888b.  8888888888  .d8888b.   
888   o   888   888   888   Y88b 888        d88P  Y88b  
888  d8b  888   888   888    888 888        Y88b.       
888 d888b 888   888   888   d88P 8888888     "Y888b.    
888d88888b888   888   8888888P"  888            "Y88b.  
88888P Y88888   888   888 T88b   888              "888  
8888P   Y8888   888   888  T88b  888        Y88b  d88P  
888P     Y888 8888888 888   T88b 8888888888  "Y8888*/  

integer i_for;

wire 		dout;
reg 		clk;
reg 		nrst;
reg 		we;
reg [31:0]  din;
reg [7:0]	cnt_data;

/*8b     d888  .d88888b.  8888888b.  888     888 888      8888888888 
8888b   d8888 d88P" "Y88b 888  "Y88b 888     888 888      888        
88888b.d88888 888     888 888    888 888     888 888      888        
888Y88888P888 888     888 888    888 888     888 888      8888888    
888 Y888P 888 888     888 888    888 888     888 888      888        
888  Y8P  888 888     888 888    888 888     888 888      888        
888   "   888 Y88b. .d88P 888  .d88P Y88b. .d88P 888      888        
888       888  "Y88888P"  8888888P"   "Y88888P"  88888888 88888888*/

fifo_spi fifo_spi_t (
	.clk 	(clk),	
	.nrst 	(nrst),   
	.we  	(we),
	.din	(din),
	.dout	(dout)
  	);


// Clock generation
always #5 	  clk    = ~clk;

// start and simulation begin

initial begin

	$timeformat (-9, 1, " ns", 12);
	$display("------ Start TestBench ------");
	clk      = 0;
	nrst     = 1;
	din      = 0;
	we       = 0;
	cnt_data = 0;
	din      = 0;

	repeat(10)	@(posedge clk);

   	nrst = 0;
   	
   	repeat(10)	@(posedge clk);
   	
   	nrst = 1;
   	
   	repeat(10)	@(posedge clk);

   	repeat (10) begin
	   	task_send_cnt(254);
	   	repeat(1000) @(posedge clk);
	end
end

task task_send_cnt;
input cnt;
integer cnt;

begin
	@ (posedge clk or negedge nrst) begin

		if (!nrst) begin 
			cnt_data = 0;
			din      = 0;
		end
		else begin
			for (i_for=0; i_for<cnt; i_for=i_for+1) begin
				// $display("i = %d", i_for);
				#1;
				din = {4{cnt_data}}; //$random;
				we = 1;
				cnt_data = cnt_data + 8'b1;

				@ (posedge clk);
				#1
				we = 0;

				repeat (250) @(posedge clk);
			end
		end
	end
end 
endtask

endmodule


















