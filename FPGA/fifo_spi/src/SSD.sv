module SSD ( input 	 [4:0] number,
			 output reg [6:0] segOut 
);	

	always @ (*)
		case (number)

			5'h0 : segOut = 7'b100_0000; 	// 0
			5'h1 : segOut = 7'b111_1001; 	// 1	
			5'h2 : segOut = 7'b010_0100; 	// 2
			5'h3 : segOut = 7'b011_0000; 	// 3
			5'h4 : segOut = 7'b001_1001; 	// 4
			5'h5 : segOut = 7'b001_0010; 	// 5
			5'h6 : segOut = 7'b000_0010; 	// 6
			5'h7 : segOut = 7'b111_1000; 	// 7
			5'h8 : segOut = 7'b000_0000; 	// 8
			5'h9 : segOut = 7'b001_1000; 	// 9
			5'ha : segOut = 7'b000_1000; 	// A
			5'hb : segOut = 7'b000_0011; 	// B
			5'hc : segOut = 7'b100_0110; 	// C
			5'hd : segOut = 7'b010_0001; 	// D
			5'he : segOut = 7'b000_0110; 	// E
			5'hf : segOut = 7'b000_1110; 	// F

			default : segOut = 7'b111_1111; // "OFF"

		endcase

endmodule


			

/* segOut = 7'b "abcdefg"
 a a a
f     b
f     b
f     b
 g g g 
e     c
e     c
e     c
 d d d
*/

























