`ifndef outfile
	`define outfile "/dev/stdout"
`endif
module testbench;

integer i;
integer file;

reg [1023:0] filename;

reg [31:0] xorshift128_x = 123456789;
reg [31:0] xorshift128_y = 362436069;
reg [31:0] xorshift128_z = 521288629;
reg [31:0] xorshift128_w = 1652613690; // <-- seed value
reg [31:0] xorshift128_t;

task xorshift128;
begin
	xorshift128_t = xorshift128_x ^ (xorshift128_x << 11);
	xorshift128_x = xorshift128_y;
	xorshift128_y = xorshift128_z;
	xorshift128_z = xorshift128_w;
	xorshift128_w = xorshift128_w ^ (xorshift128_w >> 19) ^ xorshift128_t ^ (xorshift128_t >> 8);
end
endtask

reg [3:0] sig_UnaryOps_a;
wire [3:0] sig_UnaryOps_logic_not_;
wire [0:0] sig_UnaryOps_logic_not_bit;
wire [3:0] sig_UnaryOps_logic_not_signed;
wire [3:0] sig_UnaryOps_neg_;
wire [3:0] sig_UnaryOps_neg_signed;
wire [5:0] sig_UnaryOps_neg_signed_w;
wire [5:0] sig_UnaryOps_neg_w;
wire [3:0] sig_UnaryOps_not_;
wire [3:0] sig_UnaryOps_not_signed;
wire [5:0] sig_UnaryOps_not_signed_w;
wire [5:0] sig_UnaryOps_not_w;
wire [3:0] sig_UnaryOps_pos_;
wire [3:0] sig_UnaryOps_pos_signed;
wire [5:0] sig_UnaryOps_pos_signed_w;
wire [5:0] sig_UnaryOps_pos_w;
wire [3:0] sig_UnaryOps_reduce_and_;
wire [0:0] sig_UnaryOps_reduce_and_bit;
wire [3:0] sig_UnaryOps_reduce_and_signed;
wire [3:0] sig_UnaryOps_reduce_bool_;
wire [0:0] sig_UnaryOps_reduce_bool_bit;
wire [3:0] sig_UnaryOps_reduce_bool_signed;
wire [3:0] sig_UnaryOps_reduce_or_;
wire [0:0] sig_UnaryOps_reduce_or_bit;
wire [3:0] sig_UnaryOps_reduce_or_signed;
wire [3:0] sig_UnaryOps_reduce_xnor_;
wire [0:0] sig_UnaryOps_reduce_xnor_bit;
wire [0:0] sig_UnaryOps_reduce_xnor_bit_bit;
wire [3:0] sig_UnaryOps_reduce_xnor_signed;
wire [3:0] sig_UnaryOps_reduce_xor_;
wire [0:0] sig_UnaryOps_reduce_xor_bit;
wire [0:0] sig_UnaryOps_reduce_xor_bit_bit;
wire [3:0] sig_UnaryOps_reduce_xor_signed;
UnaryOps uut_UnaryOps(
	.a(sig_UnaryOps_a),
	.logic_not_(sig_UnaryOps_logic_not_),
	.logic_not_bit(sig_UnaryOps_logic_not_bit),
	.logic_not_signed(sig_UnaryOps_logic_not_signed),
	.neg_(sig_UnaryOps_neg_),
	.neg_signed(sig_UnaryOps_neg_signed),
	.neg_signed_w(sig_UnaryOps_neg_signed_w),
	.neg_w(sig_UnaryOps_neg_w),
	.not_(sig_UnaryOps_not_),
	.not_signed(sig_UnaryOps_not_signed),
	.not_signed_w(sig_UnaryOps_not_signed_w),
	.not_w(sig_UnaryOps_not_w),
	.pos_(sig_UnaryOps_pos_),
	.pos_signed(sig_UnaryOps_pos_signed),
	.pos_signed_w(sig_UnaryOps_pos_signed_w),
	.pos_w(sig_UnaryOps_pos_w),
	.reduce_and_(sig_UnaryOps_reduce_and_),
	.reduce_and_bit(sig_UnaryOps_reduce_and_bit),
	.reduce_and_signed(sig_UnaryOps_reduce_and_signed),
	.reduce_bool_(sig_UnaryOps_reduce_bool_),
	.reduce_bool_bit(sig_UnaryOps_reduce_bool_bit),
	.reduce_bool_signed(sig_UnaryOps_reduce_bool_signed),
	.reduce_or_(sig_UnaryOps_reduce_or_),
	.reduce_or_bit(sig_UnaryOps_reduce_or_bit),
	.reduce_or_signed(sig_UnaryOps_reduce_or_signed),
	.reduce_xnor_(sig_UnaryOps_reduce_xnor_),
	.reduce_xnor_bit(sig_UnaryOps_reduce_xnor_bit),
	.reduce_xnor_bit_bit(sig_UnaryOps_reduce_xnor_bit_bit),
	.reduce_xnor_signed(sig_UnaryOps_reduce_xnor_signed),
	.reduce_xor_(sig_UnaryOps_reduce_xor_),
	.reduce_xor_bit(sig_UnaryOps_reduce_xor_bit),
	.reduce_xor_bit_bit(sig_UnaryOps_reduce_xor_bit_bit),
	.reduce_xor_signed(sig_UnaryOps_reduce_xor_signed)
);

task UnaryOps_reset;
begin
	sig_UnaryOps_a <= #2 0;
	#100;
	sig_UnaryOps_a <= #2 ~0;
	#100;
	#0;
end
endtask

task UnaryOps_update_data;
begin
	xorshift128;
	sig_UnaryOps_a <= #2 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	#100;
end
endtask

task UnaryOps_update_clock;
begin
end
endtask

task UnaryOps_print_status;
begin
	$fdisplay(file, "#OUT# %b %b %b %t %d", { sig_UnaryOps_a }, { 1'bx }, { sig_UnaryOps_logic_not_, sig_UnaryOps_logic_not_bit, sig_UnaryOps_logic_not_signed, sig_UnaryOps_neg_, sig_UnaryOps_neg_signed, sig_UnaryOps_neg_signed_w, sig_UnaryOps_neg_w, sig_UnaryOps_not_, sig_UnaryOps_not_signed, sig_UnaryOps_not_signed_w, sig_UnaryOps_not_w, sig_UnaryOps_pos_, sig_UnaryOps_pos_signed, sig_UnaryOps_pos_signed_w, sig_UnaryOps_pos_w, sig_UnaryOps_reduce_and_, sig_UnaryOps_reduce_and_bit, sig_UnaryOps_reduce_and_signed, sig_UnaryOps_reduce_bool_, sig_UnaryOps_reduce_bool_bit, sig_UnaryOps_reduce_bool_signed, sig_UnaryOps_reduce_or_, sig_UnaryOps_reduce_or_bit, sig_UnaryOps_reduce_or_signed, sig_UnaryOps_reduce_xnor_, sig_UnaryOps_reduce_xnor_bit, sig_UnaryOps_reduce_xnor_bit_bit, sig_UnaryOps_reduce_xnor_signed, sig_UnaryOps_reduce_xor_, sig_UnaryOps_reduce_xor_bit, sig_UnaryOps_reduce_xor_bit_bit, sig_UnaryOps_reduce_xor_signed }, $time, i);
end
endtask

task UnaryOps_print_header;
begin
	$fdisplay(file, "#OUT#");
	$fdisplay(file, "#OUT#   A   sig_UnaryOps_a");
	$fdisplay(file, "#OUT#   B   sig_UnaryOps_logic_not_");
	$fdisplay(file, "#OUT#   C   sig_UnaryOps_logic_not_bit");
	$fdisplay(file, "#OUT#   D   sig_UnaryOps_logic_not_signed");
	$fdisplay(file, "#OUT#   E   sig_UnaryOps_neg_");
	$fdisplay(file, "#OUT#   F   sig_UnaryOps_neg_signed");
	$fdisplay(file, "#OUT#   G   sig_UnaryOps_neg_signed_w");
	$fdisplay(file, "#OUT#   H   sig_UnaryOps_neg_w");
	$fdisplay(file, "#OUT#   I   sig_UnaryOps_not_");
	$fdisplay(file, "#OUT#   J   sig_UnaryOps_not_signed");
	$fdisplay(file, "#OUT#   K   sig_UnaryOps_not_signed_w");
	$fdisplay(file, "#OUT#   L   sig_UnaryOps_not_w");
	$fdisplay(file, "#OUT#   M   sig_UnaryOps_pos_");
	$fdisplay(file, "#OUT#   N   sig_UnaryOps_pos_signed");
	$fdisplay(file, "#OUT#   O   sig_UnaryOps_pos_signed_w");
	$fdisplay(file, "#OUT#   P   sig_UnaryOps_pos_w");
	$fdisplay(file, "#OUT#   Q   sig_UnaryOps_reduce_and_");
	$fdisplay(file, "#OUT#   R   sig_UnaryOps_reduce_and_bit");
	$fdisplay(file, "#OUT#   S   sig_UnaryOps_reduce_and_signed");
	$fdisplay(file, "#OUT#   T   sig_UnaryOps_reduce_bool_");
	$fdisplay(file, "#OUT#   U   sig_UnaryOps_reduce_bool_bit");
	$fdisplay(file, "#OUT#   V   sig_UnaryOps_reduce_bool_signed");
	$fdisplay(file, "#OUT#   W   sig_UnaryOps_reduce_or_");
	$fdisplay(file, "#OUT#   X   sig_UnaryOps_reduce_or_bit");
	$fdisplay(file, "#OUT#   Y   sig_UnaryOps_reduce_or_signed");
	$fdisplay(file, "#OUT#   Z   sig_UnaryOps_reduce_xnor_");
	$fdisplay(file, "#OUT#   A   sig_UnaryOps_reduce_xnor_bit");
	$fdisplay(file, "#OUT#   B   sig_UnaryOps_reduce_xnor_bit_bit");
	$fdisplay(file, "#OUT#   C   sig_UnaryOps_reduce_xnor_signed");
	$fdisplay(file, "#OUT#   D   sig_UnaryOps_reduce_xor_");
	$fdisplay(file, "#OUT#   E   sig_UnaryOps_reduce_xor_bit");
	$fdisplay(file, "#OUT#   F   sig_UnaryOps_reduce_xor_bit_bit");
	$fdisplay(file, "#OUT#   G   sig_UnaryOps_reduce_xor_signed");
	$fdisplay(file, "#OUT#");
	$fdisplay(file, {"#OUT# ", "/--A", " ", "#", " ", "/--B", "C", "/--D", "/--E", "/--F", "/----G", "/----H", "/--I", "/--J", "/----K", "/----L", "/--M", "/--N", "/----O", "/----P", "/--Q", "R", "/--S", "/--T", "U", "/--V", "/--W", "X", "/--Y", "/--Z", "A", "B", "/--C", "/--D", "E", "F", "/--G"});
end
endtask

task UnaryOps_test;
begin
	$fdisplay(file, "#OUT#\n#OUT# ==== UnaryOps ====");
	UnaryOps_reset;
	for (i=0; i<1000; i=i+1) begin
		if (i % 20 == 0) UnaryOps_print_header;
		#100; UnaryOps_update_data;
		#100; UnaryOps_update_clock;
		#100; UnaryOps_print_status;
	end
end
endtask

initial begin
	if ($value$plusargs("VCD=%s", filename)) begin
		$dumpfile(filename);
		$dumpvars(0, testbench);
	end
	if ($value$plusargs("OUT=%s", filename)) begin
		file = $fopen(filename);
	end else begin
		file = $fopen(`outfile);
	end
	UnaryOps_test;
	$fclose(file);
	$finish;
end

endmodule
