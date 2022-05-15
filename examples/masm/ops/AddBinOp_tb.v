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
reg [31:0] xorshift128_w = 1652621220; // <-- seed value
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

reg [0:0] sig_AddBinOp_a1;
reg [9:0] sig_AddBinOp_a10;
reg [13:0] sig_AddBinOp_a14;
reg [7:0] sig_AddBinOp_a8;
reg [0:0] sig_AddBinOp_b1;
reg [9:0] sig_AddBinOp_b10;
reg [13:0] sig_AddBinOp_b14;
reg [7:0] sig_AddBinOp_b8;
wire [9:0] sig_AddBinOp_r10_10_10_ss;
wire [9:0] sig_AddBinOp_r10_10_10_su;
wire [9:0] sig_AddBinOp_r10_10_10_us;
wire [9:0] sig_AddBinOp_r10_10_10_uu;
wire [9:0] sig_AddBinOp_r10_10_14_ss;
wire [9:0] sig_AddBinOp_r10_10_14_su;
wire [9:0] sig_AddBinOp_r10_10_14_us;
wire [9:0] sig_AddBinOp_r10_10_14_uu;
wire [9:0] sig_AddBinOp_r10_10_1_ss;
wire [9:0] sig_AddBinOp_r10_10_1_su;
wire [9:0] sig_AddBinOp_r10_10_1_us;
wire [9:0] sig_AddBinOp_r10_10_1_uu;
wire [9:0] sig_AddBinOp_r10_10_8_ss;
wire [9:0] sig_AddBinOp_r10_10_8_su;
wire [9:0] sig_AddBinOp_r10_10_8_us;
wire [9:0] sig_AddBinOp_r10_10_8_uu;
wire [9:0] sig_AddBinOp_r10_14_10_ss;
wire [9:0] sig_AddBinOp_r10_14_10_su;
wire [9:0] sig_AddBinOp_r10_14_10_us;
wire [9:0] sig_AddBinOp_r10_14_10_uu;
wire [9:0] sig_AddBinOp_r10_14_14_ss;
wire [9:0] sig_AddBinOp_r10_14_14_su;
wire [9:0] sig_AddBinOp_r10_14_14_us;
wire [9:0] sig_AddBinOp_r10_14_14_uu;
wire [9:0] sig_AddBinOp_r10_14_1_ss;
wire [9:0] sig_AddBinOp_r10_14_1_su;
wire [9:0] sig_AddBinOp_r10_14_1_us;
wire [9:0] sig_AddBinOp_r10_14_1_uu;
wire [9:0] sig_AddBinOp_r10_14_8_ss;
wire [9:0] sig_AddBinOp_r10_14_8_su;
wire [9:0] sig_AddBinOp_r10_14_8_us;
wire [9:0] sig_AddBinOp_r10_14_8_uu;
wire [9:0] sig_AddBinOp_r10_1_10_ss;
wire [9:0] sig_AddBinOp_r10_1_10_su;
wire [9:0] sig_AddBinOp_r10_1_10_us;
wire [9:0] sig_AddBinOp_r10_1_10_uu;
wire [9:0] sig_AddBinOp_r10_1_14_ss;
wire [9:0] sig_AddBinOp_r10_1_14_su;
wire [9:0] sig_AddBinOp_r10_1_14_us;
wire [9:0] sig_AddBinOp_r10_1_14_uu;
wire [9:0] sig_AddBinOp_r10_1_1_ss;
wire [9:0] sig_AddBinOp_r10_1_1_su;
wire [9:0] sig_AddBinOp_r10_1_1_us;
wire [9:0] sig_AddBinOp_r10_1_1_uu;
wire [9:0] sig_AddBinOp_r10_1_8_ss;
wire [9:0] sig_AddBinOp_r10_1_8_su;
wire [9:0] sig_AddBinOp_r10_1_8_us;
wire [9:0] sig_AddBinOp_r10_1_8_uu;
wire [9:0] sig_AddBinOp_r10_8_10_ss;
wire [9:0] sig_AddBinOp_r10_8_10_su;
wire [9:0] sig_AddBinOp_r10_8_10_us;
wire [9:0] sig_AddBinOp_r10_8_10_uu;
wire [9:0] sig_AddBinOp_r10_8_14_ss;
wire [9:0] sig_AddBinOp_r10_8_14_su;
wire [9:0] sig_AddBinOp_r10_8_14_us;
wire [9:0] sig_AddBinOp_r10_8_14_uu;
wire [9:0] sig_AddBinOp_r10_8_1_ss;
wire [9:0] sig_AddBinOp_r10_8_1_su;
wire [9:0] sig_AddBinOp_r10_8_1_us;
wire [9:0] sig_AddBinOp_r10_8_1_uu;
wire [9:0] sig_AddBinOp_r10_8_8_ss;
wire [9:0] sig_AddBinOp_r10_8_8_su;
wire [9:0] sig_AddBinOp_r10_8_8_us;
wire [9:0] sig_AddBinOp_r10_8_8_uu;
wire [13:0] sig_AddBinOp_r14_10_10_ss;
wire [13:0] sig_AddBinOp_r14_10_10_su;
wire [13:0] sig_AddBinOp_r14_10_10_us;
wire [13:0] sig_AddBinOp_r14_10_10_uu;
wire [13:0] sig_AddBinOp_r14_10_14_ss;
wire [13:0] sig_AddBinOp_r14_10_14_su;
wire [13:0] sig_AddBinOp_r14_10_14_us;
wire [13:0] sig_AddBinOp_r14_10_14_uu;
wire [13:0] sig_AddBinOp_r14_10_1_ss;
wire [13:0] sig_AddBinOp_r14_10_1_su;
wire [13:0] sig_AddBinOp_r14_10_1_us;
wire [13:0] sig_AddBinOp_r14_10_1_uu;
wire [13:0] sig_AddBinOp_r14_10_8_ss;
wire [13:0] sig_AddBinOp_r14_10_8_su;
wire [13:0] sig_AddBinOp_r14_10_8_us;
wire [13:0] sig_AddBinOp_r14_10_8_uu;
wire [13:0] sig_AddBinOp_r14_14_10_ss;
wire [13:0] sig_AddBinOp_r14_14_10_su;
wire [13:0] sig_AddBinOp_r14_14_10_us;
wire [13:0] sig_AddBinOp_r14_14_10_uu;
wire [13:0] sig_AddBinOp_r14_14_14_ss;
wire [13:0] sig_AddBinOp_r14_14_14_su;
wire [13:0] sig_AddBinOp_r14_14_14_us;
wire [13:0] sig_AddBinOp_r14_14_14_uu;
wire [13:0] sig_AddBinOp_r14_14_1_ss;
wire [13:0] sig_AddBinOp_r14_14_1_su;
wire [13:0] sig_AddBinOp_r14_14_1_us;
wire [13:0] sig_AddBinOp_r14_14_1_uu;
wire [13:0] sig_AddBinOp_r14_14_8_ss;
wire [13:0] sig_AddBinOp_r14_14_8_su;
wire [13:0] sig_AddBinOp_r14_14_8_us;
wire [13:0] sig_AddBinOp_r14_14_8_uu;
wire [13:0] sig_AddBinOp_r14_1_10_ss;
wire [13:0] sig_AddBinOp_r14_1_10_su;
wire [13:0] sig_AddBinOp_r14_1_10_us;
wire [13:0] sig_AddBinOp_r14_1_10_uu;
wire [13:0] sig_AddBinOp_r14_1_14_ss;
wire [13:0] sig_AddBinOp_r14_1_14_su;
wire [13:0] sig_AddBinOp_r14_1_14_us;
wire [13:0] sig_AddBinOp_r14_1_14_uu;
wire [13:0] sig_AddBinOp_r14_1_1_ss;
wire [13:0] sig_AddBinOp_r14_1_1_su;
wire [13:0] sig_AddBinOp_r14_1_1_us;
wire [13:0] sig_AddBinOp_r14_1_1_uu;
wire [13:0] sig_AddBinOp_r14_1_8_ss;
wire [13:0] sig_AddBinOp_r14_1_8_su;
wire [13:0] sig_AddBinOp_r14_1_8_us;
wire [13:0] sig_AddBinOp_r14_1_8_uu;
wire [13:0] sig_AddBinOp_r14_8_10_ss;
wire [13:0] sig_AddBinOp_r14_8_10_su;
wire [13:0] sig_AddBinOp_r14_8_10_us;
wire [13:0] sig_AddBinOp_r14_8_10_uu;
wire [13:0] sig_AddBinOp_r14_8_14_ss;
wire [13:0] sig_AddBinOp_r14_8_14_su;
wire [13:0] sig_AddBinOp_r14_8_14_us;
wire [13:0] sig_AddBinOp_r14_8_14_uu;
wire [13:0] sig_AddBinOp_r14_8_1_ss;
wire [13:0] sig_AddBinOp_r14_8_1_su;
wire [13:0] sig_AddBinOp_r14_8_1_us;
wire [13:0] sig_AddBinOp_r14_8_1_uu;
wire [13:0] sig_AddBinOp_r14_8_8_ss;
wire [13:0] sig_AddBinOp_r14_8_8_su;
wire [13:0] sig_AddBinOp_r14_8_8_us;
wire [13:0] sig_AddBinOp_r14_8_8_uu;
wire [0:0] sig_AddBinOp_r1_10_10_ss;
wire [0:0] sig_AddBinOp_r1_10_10_su;
wire [0:0] sig_AddBinOp_r1_10_10_us;
wire [0:0] sig_AddBinOp_r1_10_10_uu;
wire [0:0] sig_AddBinOp_r1_10_14_ss;
wire [0:0] sig_AddBinOp_r1_10_14_su;
wire [0:0] sig_AddBinOp_r1_10_14_us;
wire [0:0] sig_AddBinOp_r1_10_14_uu;
wire [0:0] sig_AddBinOp_r1_10_1_ss;
wire [0:0] sig_AddBinOp_r1_10_1_su;
wire [0:0] sig_AddBinOp_r1_10_1_us;
wire [0:0] sig_AddBinOp_r1_10_1_uu;
wire [0:0] sig_AddBinOp_r1_10_8_ss;
wire [0:0] sig_AddBinOp_r1_10_8_su;
wire [0:0] sig_AddBinOp_r1_10_8_us;
wire [0:0] sig_AddBinOp_r1_10_8_uu;
wire [0:0] sig_AddBinOp_r1_14_10_ss;
wire [0:0] sig_AddBinOp_r1_14_10_su;
wire [0:0] sig_AddBinOp_r1_14_10_us;
wire [0:0] sig_AddBinOp_r1_14_10_uu;
wire [0:0] sig_AddBinOp_r1_14_14_ss;
wire [0:0] sig_AddBinOp_r1_14_14_su;
wire [0:0] sig_AddBinOp_r1_14_14_us;
wire [0:0] sig_AddBinOp_r1_14_14_uu;
wire [0:0] sig_AddBinOp_r1_14_1_ss;
wire [0:0] sig_AddBinOp_r1_14_1_su;
wire [0:0] sig_AddBinOp_r1_14_1_us;
wire [0:0] sig_AddBinOp_r1_14_1_uu;
wire [0:0] sig_AddBinOp_r1_14_8_ss;
wire [0:0] sig_AddBinOp_r1_14_8_su;
wire [0:0] sig_AddBinOp_r1_14_8_us;
wire [0:0] sig_AddBinOp_r1_14_8_uu;
wire [0:0] sig_AddBinOp_r1_1_10_ss;
wire [0:0] sig_AddBinOp_r1_1_10_su;
wire [0:0] sig_AddBinOp_r1_1_10_us;
wire [0:0] sig_AddBinOp_r1_1_10_uu;
wire [0:0] sig_AddBinOp_r1_1_14_ss;
wire [0:0] sig_AddBinOp_r1_1_14_su;
wire [0:0] sig_AddBinOp_r1_1_14_us;
wire [0:0] sig_AddBinOp_r1_1_14_uu;
wire [0:0] sig_AddBinOp_r1_1_1_ss;
wire [0:0] sig_AddBinOp_r1_1_1_su;
wire [0:0] sig_AddBinOp_r1_1_1_us;
wire [0:0] sig_AddBinOp_r1_1_1_uu;
wire [0:0] sig_AddBinOp_r1_1_8_ss;
wire [0:0] sig_AddBinOp_r1_1_8_su;
wire [0:0] sig_AddBinOp_r1_1_8_us;
wire [0:0] sig_AddBinOp_r1_1_8_uu;
wire [0:0] sig_AddBinOp_r1_8_10_ss;
wire [0:0] sig_AddBinOp_r1_8_10_su;
wire [0:0] sig_AddBinOp_r1_8_10_us;
wire [0:0] sig_AddBinOp_r1_8_10_uu;
wire [0:0] sig_AddBinOp_r1_8_14_ss;
wire [0:0] sig_AddBinOp_r1_8_14_su;
wire [0:0] sig_AddBinOp_r1_8_14_us;
wire [0:0] sig_AddBinOp_r1_8_14_uu;
wire [0:0] sig_AddBinOp_r1_8_1_ss;
wire [0:0] sig_AddBinOp_r1_8_1_su;
wire [0:0] sig_AddBinOp_r1_8_1_us;
wire [0:0] sig_AddBinOp_r1_8_1_uu;
wire [0:0] sig_AddBinOp_r1_8_8_ss;
wire [0:0] sig_AddBinOp_r1_8_8_su;
wire [0:0] sig_AddBinOp_r1_8_8_us;
wire [0:0] sig_AddBinOp_r1_8_8_uu;
wire [7:0] sig_AddBinOp_r8_10_10_ss;
wire [7:0] sig_AddBinOp_r8_10_10_su;
wire [7:0] sig_AddBinOp_r8_10_10_us;
wire [7:0] sig_AddBinOp_r8_10_10_uu;
wire [7:0] sig_AddBinOp_r8_10_14_ss;
wire [7:0] sig_AddBinOp_r8_10_14_su;
wire [7:0] sig_AddBinOp_r8_10_14_us;
wire [7:0] sig_AddBinOp_r8_10_14_uu;
wire [7:0] sig_AddBinOp_r8_10_1_ss;
wire [7:0] sig_AddBinOp_r8_10_1_su;
wire [7:0] sig_AddBinOp_r8_10_1_us;
wire [7:0] sig_AddBinOp_r8_10_1_uu;
wire [7:0] sig_AddBinOp_r8_10_8_ss;
wire [7:0] sig_AddBinOp_r8_10_8_su;
wire [7:0] sig_AddBinOp_r8_10_8_us;
wire [7:0] sig_AddBinOp_r8_10_8_uu;
wire [7:0] sig_AddBinOp_r8_14_10_ss;
wire [7:0] sig_AddBinOp_r8_14_10_su;
wire [7:0] sig_AddBinOp_r8_14_10_us;
wire [7:0] sig_AddBinOp_r8_14_10_uu;
wire [7:0] sig_AddBinOp_r8_14_14_ss;
wire [7:0] sig_AddBinOp_r8_14_14_su;
wire [7:0] sig_AddBinOp_r8_14_14_us;
wire [7:0] sig_AddBinOp_r8_14_14_uu;
wire [7:0] sig_AddBinOp_r8_14_1_ss;
wire [7:0] sig_AddBinOp_r8_14_1_su;
wire [7:0] sig_AddBinOp_r8_14_1_us;
wire [7:0] sig_AddBinOp_r8_14_1_uu;
wire [7:0] sig_AddBinOp_r8_14_8_ss;
wire [7:0] sig_AddBinOp_r8_14_8_su;
wire [7:0] sig_AddBinOp_r8_14_8_us;
wire [7:0] sig_AddBinOp_r8_14_8_uu;
wire [7:0] sig_AddBinOp_r8_1_10_ss;
wire [7:0] sig_AddBinOp_r8_1_10_su;
wire [7:0] sig_AddBinOp_r8_1_10_us;
wire [7:0] sig_AddBinOp_r8_1_10_uu;
wire [7:0] sig_AddBinOp_r8_1_14_ss;
wire [7:0] sig_AddBinOp_r8_1_14_su;
wire [7:0] sig_AddBinOp_r8_1_14_us;
wire [7:0] sig_AddBinOp_r8_1_14_uu;
wire [7:0] sig_AddBinOp_r8_1_1_ss;
wire [7:0] sig_AddBinOp_r8_1_1_su;
wire [7:0] sig_AddBinOp_r8_1_1_us;
wire [7:0] sig_AddBinOp_r8_1_1_uu;
wire [7:0] sig_AddBinOp_r8_1_8_ss;
wire [7:0] sig_AddBinOp_r8_1_8_su;
wire [7:0] sig_AddBinOp_r8_1_8_us;
wire [7:0] sig_AddBinOp_r8_1_8_uu;
wire [7:0] sig_AddBinOp_r8_8_10_ss;
wire [7:0] sig_AddBinOp_r8_8_10_su;
wire [7:0] sig_AddBinOp_r8_8_10_us;
wire [7:0] sig_AddBinOp_r8_8_10_uu;
wire [7:0] sig_AddBinOp_r8_8_14_ss;
wire [7:0] sig_AddBinOp_r8_8_14_su;
wire [7:0] sig_AddBinOp_r8_8_14_us;
wire [7:0] sig_AddBinOp_r8_8_14_uu;
wire [7:0] sig_AddBinOp_r8_8_1_ss;
wire [7:0] sig_AddBinOp_r8_8_1_su;
wire [7:0] sig_AddBinOp_r8_8_1_us;
wire [7:0] sig_AddBinOp_r8_8_1_uu;
wire [7:0] sig_AddBinOp_r8_8_8_ss;
wire [7:0] sig_AddBinOp_r8_8_8_su;
wire [7:0] sig_AddBinOp_r8_8_8_us;
wire [7:0] sig_AddBinOp_r8_8_8_uu;
AddBinOp uut_AddBinOp(
	.a1(sig_AddBinOp_a1),
	.a10(sig_AddBinOp_a10),
	.a14(sig_AddBinOp_a14),
	.a8(sig_AddBinOp_a8),
	.b1(sig_AddBinOp_b1),
	.b10(sig_AddBinOp_b10),
	.b14(sig_AddBinOp_b14),
	.b8(sig_AddBinOp_b8),
	.r10_10_10_ss(sig_AddBinOp_r10_10_10_ss),
	.r10_10_10_su(sig_AddBinOp_r10_10_10_su),
	.r10_10_10_us(sig_AddBinOp_r10_10_10_us),
	.r10_10_10_uu(sig_AddBinOp_r10_10_10_uu),
	.r10_10_14_ss(sig_AddBinOp_r10_10_14_ss),
	.r10_10_14_su(sig_AddBinOp_r10_10_14_su),
	.r10_10_14_us(sig_AddBinOp_r10_10_14_us),
	.r10_10_14_uu(sig_AddBinOp_r10_10_14_uu),
	.r10_10_1_ss(sig_AddBinOp_r10_10_1_ss),
	.r10_10_1_su(sig_AddBinOp_r10_10_1_su),
	.r10_10_1_us(sig_AddBinOp_r10_10_1_us),
	.r10_10_1_uu(sig_AddBinOp_r10_10_1_uu),
	.r10_10_8_ss(sig_AddBinOp_r10_10_8_ss),
	.r10_10_8_su(sig_AddBinOp_r10_10_8_su),
	.r10_10_8_us(sig_AddBinOp_r10_10_8_us),
	.r10_10_8_uu(sig_AddBinOp_r10_10_8_uu),
	.r10_14_10_ss(sig_AddBinOp_r10_14_10_ss),
	.r10_14_10_su(sig_AddBinOp_r10_14_10_su),
	.r10_14_10_us(sig_AddBinOp_r10_14_10_us),
	.r10_14_10_uu(sig_AddBinOp_r10_14_10_uu),
	.r10_14_14_ss(sig_AddBinOp_r10_14_14_ss),
	.r10_14_14_su(sig_AddBinOp_r10_14_14_su),
	.r10_14_14_us(sig_AddBinOp_r10_14_14_us),
	.r10_14_14_uu(sig_AddBinOp_r10_14_14_uu),
	.r10_14_1_ss(sig_AddBinOp_r10_14_1_ss),
	.r10_14_1_su(sig_AddBinOp_r10_14_1_su),
	.r10_14_1_us(sig_AddBinOp_r10_14_1_us),
	.r10_14_1_uu(sig_AddBinOp_r10_14_1_uu),
	.r10_14_8_ss(sig_AddBinOp_r10_14_8_ss),
	.r10_14_8_su(sig_AddBinOp_r10_14_8_su),
	.r10_14_8_us(sig_AddBinOp_r10_14_8_us),
	.r10_14_8_uu(sig_AddBinOp_r10_14_8_uu),
	.r10_1_10_ss(sig_AddBinOp_r10_1_10_ss),
	.r10_1_10_su(sig_AddBinOp_r10_1_10_su),
	.r10_1_10_us(sig_AddBinOp_r10_1_10_us),
	.r10_1_10_uu(sig_AddBinOp_r10_1_10_uu),
	.r10_1_14_ss(sig_AddBinOp_r10_1_14_ss),
	.r10_1_14_su(sig_AddBinOp_r10_1_14_su),
	.r10_1_14_us(sig_AddBinOp_r10_1_14_us),
	.r10_1_14_uu(sig_AddBinOp_r10_1_14_uu),
	.r10_1_1_ss(sig_AddBinOp_r10_1_1_ss),
	.r10_1_1_su(sig_AddBinOp_r10_1_1_su),
	.r10_1_1_us(sig_AddBinOp_r10_1_1_us),
	.r10_1_1_uu(sig_AddBinOp_r10_1_1_uu),
	.r10_1_8_ss(sig_AddBinOp_r10_1_8_ss),
	.r10_1_8_su(sig_AddBinOp_r10_1_8_su),
	.r10_1_8_us(sig_AddBinOp_r10_1_8_us),
	.r10_1_8_uu(sig_AddBinOp_r10_1_8_uu),
	.r10_8_10_ss(sig_AddBinOp_r10_8_10_ss),
	.r10_8_10_su(sig_AddBinOp_r10_8_10_su),
	.r10_8_10_us(sig_AddBinOp_r10_8_10_us),
	.r10_8_10_uu(sig_AddBinOp_r10_8_10_uu),
	.r10_8_14_ss(sig_AddBinOp_r10_8_14_ss),
	.r10_8_14_su(sig_AddBinOp_r10_8_14_su),
	.r10_8_14_us(sig_AddBinOp_r10_8_14_us),
	.r10_8_14_uu(sig_AddBinOp_r10_8_14_uu),
	.r10_8_1_ss(sig_AddBinOp_r10_8_1_ss),
	.r10_8_1_su(sig_AddBinOp_r10_8_1_su),
	.r10_8_1_us(sig_AddBinOp_r10_8_1_us),
	.r10_8_1_uu(sig_AddBinOp_r10_8_1_uu),
	.r10_8_8_ss(sig_AddBinOp_r10_8_8_ss),
	.r10_8_8_su(sig_AddBinOp_r10_8_8_su),
	.r10_8_8_us(sig_AddBinOp_r10_8_8_us),
	.r10_8_8_uu(sig_AddBinOp_r10_8_8_uu),
	.r14_10_10_ss(sig_AddBinOp_r14_10_10_ss),
	.r14_10_10_su(sig_AddBinOp_r14_10_10_su),
	.r14_10_10_us(sig_AddBinOp_r14_10_10_us),
	.r14_10_10_uu(sig_AddBinOp_r14_10_10_uu),
	.r14_10_14_ss(sig_AddBinOp_r14_10_14_ss),
	.r14_10_14_su(sig_AddBinOp_r14_10_14_su),
	.r14_10_14_us(sig_AddBinOp_r14_10_14_us),
	.r14_10_14_uu(sig_AddBinOp_r14_10_14_uu),
	.r14_10_1_ss(sig_AddBinOp_r14_10_1_ss),
	.r14_10_1_su(sig_AddBinOp_r14_10_1_su),
	.r14_10_1_us(sig_AddBinOp_r14_10_1_us),
	.r14_10_1_uu(sig_AddBinOp_r14_10_1_uu),
	.r14_10_8_ss(sig_AddBinOp_r14_10_8_ss),
	.r14_10_8_su(sig_AddBinOp_r14_10_8_su),
	.r14_10_8_us(sig_AddBinOp_r14_10_8_us),
	.r14_10_8_uu(sig_AddBinOp_r14_10_8_uu),
	.r14_14_10_ss(sig_AddBinOp_r14_14_10_ss),
	.r14_14_10_su(sig_AddBinOp_r14_14_10_su),
	.r14_14_10_us(sig_AddBinOp_r14_14_10_us),
	.r14_14_10_uu(sig_AddBinOp_r14_14_10_uu),
	.r14_14_14_ss(sig_AddBinOp_r14_14_14_ss),
	.r14_14_14_su(sig_AddBinOp_r14_14_14_su),
	.r14_14_14_us(sig_AddBinOp_r14_14_14_us),
	.r14_14_14_uu(sig_AddBinOp_r14_14_14_uu),
	.r14_14_1_ss(sig_AddBinOp_r14_14_1_ss),
	.r14_14_1_su(sig_AddBinOp_r14_14_1_su),
	.r14_14_1_us(sig_AddBinOp_r14_14_1_us),
	.r14_14_1_uu(sig_AddBinOp_r14_14_1_uu),
	.r14_14_8_ss(sig_AddBinOp_r14_14_8_ss),
	.r14_14_8_su(sig_AddBinOp_r14_14_8_su),
	.r14_14_8_us(sig_AddBinOp_r14_14_8_us),
	.r14_14_8_uu(sig_AddBinOp_r14_14_8_uu),
	.r14_1_10_ss(sig_AddBinOp_r14_1_10_ss),
	.r14_1_10_su(sig_AddBinOp_r14_1_10_su),
	.r14_1_10_us(sig_AddBinOp_r14_1_10_us),
	.r14_1_10_uu(sig_AddBinOp_r14_1_10_uu),
	.r14_1_14_ss(sig_AddBinOp_r14_1_14_ss),
	.r14_1_14_su(sig_AddBinOp_r14_1_14_su),
	.r14_1_14_us(sig_AddBinOp_r14_1_14_us),
	.r14_1_14_uu(sig_AddBinOp_r14_1_14_uu),
	.r14_1_1_ss(sig_AddBinOp_r14_1_1_ss),
	.r14_1_1_su(sig_AddBinOp_r14_1_1_su),
	.r14_1_1_us(sig_AddBinOp_r14_1_1_us),
	.r14_1_1_uu(sig_AddBinOp_r14_1_1_uu),
	.r14_1_8_ss(sig_AddBinOp_r14_1_8_ss),
	.r14_1_8_su(sig_AddBinOp_r14_1_8_su),
	.r14_1_8_us(sig_AddBinOp_r14_1_8_us),
	.r14_1_8_uu(sig_AddBinOp_r14_1_8_uu),
	.r14_8_10_ss(sig_AddBinOp_r14_8_10_ss),
	.r14_8_10_su(sig_AddBinOp_r14_8_10_su),
	.r14_8_10_us(sig_AddBinOp_r14_8_10_us),
	.r14_8_10_uu(sig_AddBinOp_r14_8_10_uu),
	.r14_8_14_ss(sig_AddBinOp_r14_8_14_ss),
	.r14_8_14_su(sig_AddBinOp_r14_8_14_su),
	.r14_8_14_us(sig_AddBinOp_r14_8_14_us),
	.r14_8_14_uu(sig_AddBinOp_r14_8_14_uu),
	.r14_8_1_ss(sig_AddBinOp_r14_8_1_ss),
	.r14_8_1_su(sig_AddBinOp_r14_8_1_su),
	.r14_8_1_us(sig_AddBinOp_r14_8_1_us),
	.r14_8_1_uu(sig_AddBinOp_r14_8_1_uu),
	.r14_8_8_ss(sig_AddBinOp_r14_8_8_ss),
	.r14_8_8_su(sig_AddBinOp_r14_8_8_su),
	.r14_8_8_us(sig_AddBinOp_r14_8_8_us),
	.r14_8_8_uu(sig_AddBinOp_r14_8_8_uu),
	.r1_10_10_ss(sig_AddBinOp_r1_10_10_ss),
	.r1_10_10_su(sig_AddBinOp_r1_10_10_su),
	.r1_10_10_us(sig_AddBinOp_r1_10_10_us),
	.r1_10_10_uu(sig_AddBinOp_r1_10_10_uu),
	.r1_10_14_ss(sig_AddBinOp_r1_10_14_ss),
	.r1_10_14_su(sig_AddBinOp_r1_10_14_su),
	.r1_10_14_us(sig_AddBinOp_r1_10_14_us),
	.r1_10_14_uu(sig_AddBinOp_r1_10_14_uu),
	.r1_10_1_ss(sig_AddBinOp_r1_10_1_ss),
	.r1_10_1_su(sig_AddBinOp_r1_10_1_su),
	.r1_10_1_us(sig_AddBinOp_r1_10_1_us),
	.r1_10_1_uu(sig_AddBinOp_r1_10_1_uu),
	.r1_10_8_ss(sig_AddBinOp_r1_10_8_ss),
	.r1_10_8_su(sig_AddBinOp_r1_10_8_su),
	.r1_10_8_us(sig_AddBinOp_r1_10_8_us),
	.r1_10_8_uu(sig_AddBinOp_r1_10_8_uu),
	.r1_14_10_ss(sig_AddBinOp_r1_14_10_ss),
	.r1_14_10_su(sig_AddBinOp_r1_14_10_su),
	.r1_14_10_us(sig_AddBinOp_r1_14_10_us),
	.r1_14_10_uu(sig_AddBinOp_r1_14_10_uu),
	.r1_14_14_ss(sig_AddBinOp_r1_14_14_ss),
	.r1_14_14_su(sig_AddBinOp_r1_14_14_su),
	.r1_14_14_us(sig_AddBinOp_r1_14_14_us),
	.r1_14_14_uu(sig_AddBinOp_r1_14_14_uu),
	.r1_14_1_ss(sig_AddBinOp_r1_14_1_ss),
	.r1_14_1_su(sig_AddBinOp_r1_14_1_su),
	.r1_14_1_us(sig_AddBinOp_r1_14_1_us),
	.r1_14_1_uu(sig_AddBinOp_r1_14_1_uu),
	.r1_14_8_ss(sig_AddBinOp_r1_14_8_ss),
	.r1_14_8_su(sig_AddBinOp_r1_14_8_su),
	.r1_14_8_us(sig_AddBinOp_r1_14_8_us),
	.r1_14_8_uu(sig_AddBinOp_r1_14_8_uu),
	.r1_1_10_ss(sig_AddBinOp_r1_1_10_ss),
	.r1_1_10_su(sig_AddBinOp_r1_1_10_su),
	.r1_1_10_us(sig_AddBinOp_r1_1_10_us),
	.r1_1_10_uu(sig_AddBinOp_r1_1_10_uu),
	.r1_1_14_ss(sig_AddBinOp_r1_1_14_ss),
	.r1_1_14_su(sig_AddBinOp_r1_1_14_su),
	.r1_1_14_us(sig_AddBinOp_r1_1_14_us),
	.r1_1_14_uu(sig_AddBinOp_r1_1_14_uu),
	.r1_1_1_ss(sig_AddBinOp_r1_1_1_ss),
	.r1_1_1_su(sig_AddBinOp_r1_1_1_su),
	.r1_1_1_us(sig_AddBinOp_r1_1_1_us),
	.r1_1_1_uu(sig_AddBinOp_r1_1_1_uu),
	.r1_1_8_ss(sig_AddBinOp_r1_1_8_ss),
	.r1_1_8_su(sig_AddBinOp_r1_1_8_su),
	.r1_1_8_us(sig_AddBinOp_r1_1_8_us),
	.r1_1_8_uu(sig_AddBinOp_r1_1_8_uu),
	.r1_8_10_ss(sig_AddBinOp_r1_8_10_ss),
	.r1_8_10_su(sig_AddBinOp_r1_8_10_su),
	.r1_8_10_us(sig_AddBinOp_r1_8_10_us),
	.r1_8_10_uu(sig_AddBinOp_r1_8_10_uu),
	.r1_8_14_ss(sig_AddBinOp_r1_8_14_ss),
	.r1_8_14_su(sig_AddBinOp_r1_8_14_su),
	.r1_8_14_us(sig_AddBinOp_r1_8_14_us),
	.r1_8_14_uu(sig_AddBinOp_r1_8_14_uu),
	.r1_8_1_ss(sig_AddBinOp_r1_8_1_ss),
	.r1_8_1_su(sig_AddBinOp_r1_8_1_su),
	.r1_8_1_us(sig_AddBinOp_r1_8_1_us),
	.r1_8_1_uu(sig_AddBinOp_r1_8_1_uu),
	.r1_8_8_ss(sig_AddBinOp_r1_8_8_ss),
	.r1_8_8_su(sig_AddBinOp_r1_8_8_su),
	.r1_8_8_us(sig_AddBinOp_r1_8_8_us),
	.r1_8_8_uu(sig_AddBinOp_r1_8_8_uu),
	.r8_10_10_ss(sig_AddBinOp_r8_10_10_ss),
	.r8_10_10_su(sig_AddBinOp_r8_10_10_su),
	.r8_10_10_us(sig_AddBinOp_r8_10_10_us),
	.r8_10_10_uu(sig_AddBinOp_r8_10_10_uu),
	.r8_10_14_ss(sig_AddBinOp_r8_10_14_ss),
	.r8_10_14_su(sig_AddBinOp_r8_10_14_su),
	.r8_10_14_us(sig_AddBinOp_r8_10_14_us),
	.r8_10_14_uu(sig_AddBinOp_r8_10_14_uu),
	.r8_10_1_ss(sig_AddBinOp_r8_10_1_ss),
	.r8_10_1_su(sig_AddBinOp_r8_10_1_su),
	.r8_10_1_us(sig_AddBinOp_r8_10_1_us),
	.r8_10_1_uu(sig_AddBinOp_r8_10_1_uu),
	.r8_10_8_ss(sig_AddBinOp_r8_10_8_ss),
	.r8_10_8_su(sig_AddBinOp_r8_10_8_su),
	.r8_10_8_us(sig_AddBinOp_r8_10_8_us),
	.r8_10_8_uu(sig_AddBinOp_r8_10_8_uu),
	.r8_14_10_ss(sig_AddBinOp_r8_14_10_ss),
	.r8_14_10_su(sig_AddBinOp_r8_14_10_su),
	.r8_14_10_us(sig_AddBinOp_r8_14_10_us),
	.r8_14_10_uu(sig_AddBinOp_r8_14_10_uu),
	.r8_14_14_ss(sig_AddBinOp_r8_14_14_ss),
	.r8_14_14_su(sig_AddBinOp_r8_14_14_su),
	.r8_14_14_us(sig_AddBinOp_r8_14_14_us),
	.r8_14_14_uu(sig_AddBinOp_r8_14_14_uu),
	.r8_14_1_ss(sig_AddBinOp_r8_14_1_ss),
	.r8_14_1_su(sig_AddBinOp_r8_14_1_su),
	.r8_14_1_us(sig_AddBinOp_r8_14_1_us),
	.r8_14_1_uu(sig_AddBinOp_r8_14_1_uu),
	.r8_14_8_ss(sig_AddBinOp_r8_14_8_ss),
	.r8_14_8_su(sig_AddBinOp_r8_14_8_su),
	.r8_14_8_us(sig_AddBinOp_r8_14_8_us),
	.r8_14_8_uu(sig_AddBinOp_r8_14_8_uu),
	.r8_1_10_ss(sig_AddBinOp_r8_1_10_ss),
	.r8_1_10_su(sig_AddBinOp_r8_1_10_su),
	.r8_1_10_us(sig_AddBinOp_r8_1_10_us),
	.r8_1_10_uu(sig_AddBinOp_r8_1_10_uu),
	.r8_1_14_ss(sig_AddBinOp_r8_1_14_ss),
	.r8_1_14_su(sig_AddBinOp_r8_1_14_su),
	.r8_1_14_us(sig_AddBinOp_r8_1_14_us),
	.r8_1_14_uu(sig_AddBinOp_r8_1_14_uu),
	.r8_1_1_ss(sig_AddBinOp_r8_1_1_ss),
	.r8_1_1_su(sig_AddBinOp_r8_1_1_su),
	.r8_1_1_us(sig_AddBinOp_r8_1_1_us),
	.r8_1_1_uu(sig_AddBinOp_r8_1_1_uu),
	.r8_1_8_ss(sig_AddBinOp_r8_1_8_ss),
	.r8_1_8_su(sig_AddBinOp_r8_1_8_su),
	.r8_1_8_us(sig_AddBinOp_r8_1_8_us),
	.r8_1_8_uu(sig_AddBinOp_r8_1_8_uu),
	.r8_8_10_ss(sig_AddBinOp_r8_8_10_ss),
	.r8_8_10_su(sig_AddBinOp_r8_8_10_su),
	.r8_8_10_us(sig_AddBinOp_r8_8_10_us),
	.r8_8_10_uu(sig_AddBinOp_r8_8_10_uu),
	.r8_8_14_ss(sig_AddBinOp_r8_8_14_ss),
	.r8_8_14_su(sig_AddBinOp_r8_8_14_su),
	.r8_8_14_us(sig_AddBinOp_r8_8_14_us),
	.r8_8_14_uu(sig_AddBinOp_r8_8_14_uu),
	.r8_8_1_ss(sig_AddBinOp_r8_8_1_ss),
	.r8_8_1_su(sig_AddBinOp_r8_8_1_su),
	.r8_8_1_us(sig_AddBinOp_r8_8_1_us),
	.r8_8_1_uu(sig_AddBinOp_r8_8_1_uu),
	.r8_8_8_ss(sig_AddBinOp_r8_8_8_ss),
	.r8_8_8_su(sig_AddBinOp_r8_8_8_su),
	.r8_8_8_us(sig_AddBinOp_r8_8_8_us),
	.r8_8_8_uu(sig_AddBinOp_r8_8_8_uu)
);

task AddBinOp_reset;
begin
	sig_AddBinOp_a1 <= #2 0;
	sig_AddBinOp_a10 <= #4 0;
	sig_AddBinOp_a14 <= #6 0;
	sig_AddBinOp_a8 <= #8 0;
	sig_AddBinOp_b1 <= #10 0;
	sig_AddBinOp_b10 <= #12 0;
	sig_AddBinOp_b14 <= #14 0;
	sig_AddBinOp_b8 <= #16 0;
	#100;
	sig_AddBinOp_a1 <= #2 ~0;
	sig_AddBinOp_a10 <= #4 ~0;
	sig_AddBinOp_a14 <= #6 ~0;
	sig_AddBinOp_a8 <= #8 ~0;
	sig_AddBinOp_b1 <= #10 ~0;
	sig_AddBinOp_b10 <= #12 ~0;
	sig_AddBinOp_b14 <= #14 ~0;
	sig_AddBinOp_b8 <= #16 ~0;
	#100;
	#0;
end
endtask

task AddBinOp_update_data;
begin
	xorshift128;
	sig_AddBinOp_a1 <= #2 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_a10 <= #4 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_a14 <= #6 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_a8 <= #8 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_b1 <= #10 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_b10 <= #12 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_b14 <= #14 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	xorshift128;
	sig_AddBinOp_b8 <= #16 { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };
	#100;
end
endtask

task AddBinOp_update_clock;
begin
end
endtask

task AddBinOp_print_status;
begin
	$fdisplay(file, "#OUT# %b %b %b %t %d", { sig_AddBinOp_a1, sig_AddBinOp_a10, sig_AddBinOp_a14, sig_AddBinOp_a8, sig_AddBinOp_b1, sig_AddBinOp_b10, sig_AddBinOp_b14, sig_AddBinOp_b8 }, { 1'bx }, { sig_AddBinOp_r10_10_10_ss, sig_AddBinOp_r10_10_10_su, sig_AddBinOp_r10_10_10_us, sig_AddBinOp_r10_10_10_uu, sig_AddBinOp_r10_10_14_ss, sig_AddBinOp_r10_10_14_su, sig_AddBinOp_r10_10_14_us, sig_AddBinOp_r10_10_14_uu, sig_AddBinOp_r10_10_1_ss, sig_AddBinOp_r10_10_1_su, sig_AddBinOp_r10_10_1_us, sig_AddBinOp_r10_10_1_uu, sig_AddBinOp_r10_10_8_ss, sig_AddBinOp_r10_10_8_su, sig_AddBinOp_r10_10_8_us, sig_AddBinOp_r10_10_8_uu, sig_AddBinOp_r10_14_10_ss, sig_AddBinOp_r10_14_10_su, sig_AddBinOp_r10_14_10_us, sig_AddBinOp_r10_14_10_uu, sig_AddBinOp_r10_14_14_ss, sig_AddBinOp_r10_14_14_su, sig_AddBinOp_r10_14_14_us, sig_AddBinOp_r10_14_14_uu, sig_AddBinOp_r10_14_1_ss, sig_AddBinOp_r10_14_1_su, sig_AddBinOp_r10_14_1_us, sig_AddBinOp_r10_14_1_uu, sig_AddBinOp_r10_14_8_ss, sig_AddBinOp_r10_14_8_su, sig_AddBinOp_r10_14_8_us, sig_AddBinOp_r10_14_8_uu, sig_AddBinOp_r10_1_10_ss, sig_AddBinOp_r10_1_10_su, sig_AddBinOp_r10_1_10_us, sig_AddBinOp_r10_1_10_uu, sig_AddBinOp_r10_1_14_ss, sig_AddBinOp_r10_1_14_su, sig_AddBinOp_r10_1_14_us, sig_AddBinOp_r10_1_14_uu, sig_AddBinOp_r10_1_1_ss, sig_AddBinOp_r10_1_1_su, sig_AddBinOp_r10_1_1_us, sig_AddBinOp_r10_1_1_uu, sig_AddBinOp_r10_1_8_ss, sig_AddBinOp_r10_1_8_su, sig_AddBinOp_r10_1_8_us, sig_AddBinOp_r10_1_8_uu, sig_AddBinOp_r10_8_10_ss, sig_AddBinOp_r10_8_10_su, sig_AddBinOp_r10_8_10_us, sig_AddBinOp_r10_8_10_uu, sig_AddBinOp_r10_8_14_ss, sig_AddBinOp_r10_8_14_su, sig_AddBinOp_r10_8_14_us, sig_AddBinOp_r10_8_14_uu, sig_AddBinOp_r10_8_1_ss, sig_AddBinOp_r10_8_1_su, sig_AddBinOp_r10_8_1_us, sig_AddBinOp_r10_8_1_uu, sig_AddBinOp_r10_8_8_ss, sig_AddBinOp_r10_8_8_su, sig_AddBinOp_r10_8_8_us, sig_AddBinOp_r10_8_8_uu, sig_AddBinOp_r14_10_10_ss, sig_AddBinOp_r14_10_10_su, sig_AddBinOp_r14_10_10_us, sig_AddBinOp_r14_10_10_uu, sig_AddBinOp_r14_10_14_ss, sig_AddBinOp_r14_10_14_su, sig_AddBinOp_r14_10_14_us, sig_AddBinOp_r14_10_14_uu, sig_AddBinOp_r14_10_1_ss, sig_AddBinOp_r14_10_1_su, sig_AddBinOp_r14_10_1_us, sig_AddBinOp_r14_10_1_uu, sig_AddBinOp_r14_10_8_ss, sig_AddBinOp_r14_10_8_su, sig_AddBinOp_r14_10_8_us, sig_AddBinOp_r14_10_8_uu, sig_AddBinOp_r14_14_10_ss, sig_AddBinOp_r14_14_10_su, sig_AddBinOp_r14_14_10_us, sig_AddBinOp_r14_14_10_uu, sig_AddBinOp_r14_14_14_ss, sig_AddBinOp_r14_14_14_su, sig_AddBinOp_r14_14_14_us, sig_AddBinOp_r14_14_14_uu, sig_AddBinOp_r14_14_1_ss, sig_AddBinOp_r14_14_1_su, sig_AddBinOp_r14_14_1_us, sig_AddBinOp_r14_14_1_uu, sig_AddBinOp_r14_14_8_ss, sig_AddBinOp_r14_14_8_su, sig_AddBinOp_r14_14_8_us, sig_AddBinOp_r14_14_8_uu, sig_AddBinOp_r14_1_10_ss, sig_AddBinOp_r14_1_10_su, sig_AddBinOp_r14_1_10_us, sig_AddBinOp_r14_1_10_uu, sig_AddBinOp_r14_1_14_ss, sig_AddBinOp_r14_1_14_su, sig_AddBinOp_r14_1_14_us, sig_AddBinOp_r14_1_14_uu, sig_AddBinOp_r14_1_1_ss, sig_AddBinOp_r14_1_1_su, sig_AddBinOp_r14_1_1_us, sig_AddBinOp_r14_1_1_uu, sig_AddBinOp_r14_1_8_ss, sig_AddBinOp_r14_1_8_su, sig_AddBinOp_r14_1_8_us, sig_AddBinOp_r14_1_8_uu, sig_AddBinOp_r14_8_10_ss, sig_AddBinOp_r14_8_10_su, sig_AddBinOp_r14_8_10_us, sig_AddBinOp_r14_8_10_uu, sig_AddBinOp_r14_8_14_ss, sig_AddBinOp_r14_8_14_su, sig_AddBinOp_r14_8_14_us, sig_AddBinOp_r14_8_14_uu, sig_AddBinOp_r14_8_1_ss, sig_AddBinOp_r14_8_1_su, sig_AddBinOp_r14_8_1_us, sig_AddBinOp_r14_8_1_uu, sig_AddBinOp_r14_8_8_ss, sig_AddBinOp_r14_8_8_su, sig_AddBinOp_r14_8_8_us, sig_AddBinOp_r14_8_8_uu, sig_AddBinOp_r1_10_10_ss, sig_AddBinOp_r1_10_10_su, sig_AddBinOp_r1_10_10_us, sig_AddBinOp_r1_10_10_uu, sig_AddBinOp_r1_10_14_ss, sig_AddBinOp_r1_10_14_su, sig_AddBinOp_r1_10_14_us, sig_AddBinOp_r1_10_14_uu, sig_AddBinOp_r1_10_1_ss, sig_AddBinOp_r1_10_1_su, sig_AddBinOp_r1_10_1_us, sig_AddBinOp_r1_10_1_uu, sig_AddBinOp_r1_10_8_ss, sig_AddBinOp_r1_10_8_su, sig_AddBinOp_r1_10_8_us, sig_AddBinOp_r1_10_8_uu, sig_AddBinOp_r1_14_10_ss, sig_AddBinOp_r1_14_10_su, sig_AddBinOp_r1_14_10_us, sig_AddBinOp_r1_14_10_uu, sig_AddBinOp_r1_14_14_ss, sig_AddBinOp_r1_14_14_su, sig_AddBinOp_r1_14_14_us, sig_AddBinOp_r1_14_14_uu, sig_AddBinOp_r1_14_1_ss, sig_AddBinOp_r1_14_1_su, sig_AddBinOp_r1_14_1_us, sig_AddBinOp_r1_14_1_uu, sig_AddBinOp_r1_14_8_ss, sig_AddBinOp_r1_14_8_su, sig_AddBinOp_r1_14_8_us, sig_AddBinOp_r1_14_8_uu, sig_AddBinOp_r1_1_10_ss, sig_AddBinOp_r1_1_10_su, sig_AddBinOp_r1_1_10_us, sig_AddBinOp_r1_1_10_uu, sig_AddBinOp_r1_1_14_ss, sig_AddBinOp_r1_1_14_su, sig_AddBinOp_r1_1_14_us, sig_AddBinOp_r1_1_14_uu, sig_AddBinOp_r1_1_1_ss, sig_AddBinOp_r1_1_1_su, sig_AddBinOp_r1_1_1_us, sig_AddBinOp_r1_1_1_uu, sig_AddBinOp_r1_1_8_ss, sig_AddBinOp_r1_1_8_su, sig_AddBinOp_r1_1_8_us, sig_AddBinOp_r1_1_8_uu, sig_AddBinOp_r1_8_10_ss, sig_AddBinOp_r1_8_10_su, sig_AddBinOp_r1_8_10_us, sig_AddBinOp_r1_8_10_uu, sig_AddBinOp_r1_8_14_ss, sig_AddBinOp_r1_8_14_su, sig_AddBinOp_r1_8_14_us, sig_AddBinOp_r1_8_14_uu, sig_AddBinOp_r1_8_1_ss, sig_AddBinOp_r1_8_1_su, sig_AddBinOp_r1_8_1_us, sig_AddBinOp_r1_8_1_uu, sig_AddBinOp_r1_8_8_ss, sig_AddBinOp_r1_8_8_su, sig_AddBinOp_r1_8_8_us, sig_AddBinOp_r1_8_8_uu, sig_AddBinOp_r8_10_10_ss, sig_AddBinOp_r8_10_10_su, sig_AddBinOp_r8_10_10_us, sig_AddBinOp_r8_10_10_uu, sig_AddBinOp_r8_10_14_ss, sig_AddBinOp_r8_10_14_su, sig_AddBinOp_r8_10_14_us, sig_AddBinOp_r8_10_14_uu, sig_AddBinOp_r8_10_1_ss, sig_AddBinOp_r8_10_1_su, sig_AddBinOp_r8_10_1_us, sig_AddBinOp_r8_10_1_uu, sig_AddBinOp_r8_10_8_ss, sig_AddBinOp_r8_10_8_su, sig_AddBinOp_r8_10_8_us, sig_AddBinOp_r8_10_8_uu, sig_AddBinOp_r8_14_10_ss, sig_AddBinOp_r8_14_10_su, sig_AddBinOp_r8_14_10_us, sig_AddBinOp_r8_14_10_uu, sig_AddBinOp_r8_14_14_ss, sig_AddBinOp_r8_14_14_su, sig_AddBinOp_r8_14_14_us, sig_AddBinOp_r8_14_14_uu, sig_AddBinOp_r8_14_1_ss, sig_AddBinOp_r8_14_1_su, sig_AddBinOp_r8_14_1_us, sig_AddBinOp_r8_14_1_uu, sig_AddBinOp_r8_14_8_ss, sig_AddBinOp_r8_14_8_su, sig_AddBinOp_r8_14_8_us, sig_AddBinOp_r8_14_8_uu, sig_AddBinOp_r8_1_10_ss, sig_AddBinOp_r8_1_10_su, sig_AddBinOp_r8_1_10_us, sig_AddBinOp_r8_1_10_uu, sig_AddBinOp_r8_1_14_ss, sig_AddBinOp_r8_1_14_su, sig_AddBinOp_r8_1_14_us, sig_AddBinOp_r8_1_14_uu, sig_AddBinOp_r8_1_1_ss, sig_AddBinOp_r8_1_1_su, sig_AddBinOp_r8_1_1_us, sig_AddBinOp_r8_1_1_uu, sig_AddBinOp_r8_1_8_ss, sig_AddBinOp_r8_1_8_su, sig_AddBinOp_r8_1_8_us, sig_AddBinOp_r8_1_8_uu, sig_AddBinOp_r8_8_10_ss, sig_AddBinOp_r8_8_10_su, sig_AddBinOp_r8_8_10_us, sig_AddBinOp_r8_8_10_uu, sig_AddBinOp_r8_8_14_ss, sig_AddBinOp_r8_8_14_su, sig_AddBinOp_r8_8_14_us, sig_AddBinOp_r8_8_14_uu, sig_AddBinOp_r8_8_1_ss, sig_AddBinOp_r8_8_1_su, sig_AddBinOp_r8_8_1_us, sig_AddBinOp_r8_8_1_uu, sig_AddBinOp_r8_8_8_ss, sig_AddBinOp_r8_8_8_su, sig_AddBinOp_r8_8_8_us, sig_AddBinOp_r8_8_8_uu }, $time, i);
end
endtask

task AddBinOp_print_header;
begin
	$fdisplay(file, "#OUT#");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_a1");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_a10");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_a14");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_a8");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_b1");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_b10");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_b14");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_b8");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r10_10_10_ss");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r10_10_10_su");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r10_10_10_us");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r10_10_10_uu");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r10_10_14_ss");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r10_10_14_su");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r10_10_14_us");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r10_10_14_uu");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r10_10_1_ss");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r10_10_1_su");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r10_10_1_us");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r10_10_1_uu");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r10_10_8_ss");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r10_10_8_su");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r10_10_8_us");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r10_10_8_uu");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r10_14_10_ss");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r10_14_10_su");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r10_14_10_us");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r10_14_10_uu");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r10_14_14_ss");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r10_14_14_su");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r10_14_14_us");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r10_14_14_uu");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r10_14_1_ss");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r10_14_1_su");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r10_14_1_us");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r10_14_1_uu");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r10_14_8_ss");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r10_14_8_su");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r10_14_8_us");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r10_14_8_uu");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r10_1_10_ss");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r10_1_10_su");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r10_1_10_us");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r10_1_10_uu");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r10_1_14_ss");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r10_1_14_su");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r10_1_14_us");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r10_1_14_uu");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r10_1_1_ss");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r10_1_1_su");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r10_1_1_us");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r10_1_1_uu");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r10_1_8_ss");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r10_1_8_su");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r10_1_8_us");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r10_1_8_uu");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r10_8_10_ss");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r10_8_10_su");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r10_8_10_us");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r10_8_10_uu");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r10_8_14_ss");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r10_8_14_su");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r10_8_14_us");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r10_8_14_uu");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r10_8_1_ss");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r10_8_1_su");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r10_8_1_us");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r10_8_1_uu");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r10_8_8_ss");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r10_8_8_su");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r10_8_8_us");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r10_8_8_uu");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r14_10_10_ss");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r14_10_10_su");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r14_10_10_us");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r14_10_10_uu");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r14_10_14_ss");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r14_10_14_su");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r14_10_14_us");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r14_10_14_uu");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r14_10_1_ss");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r14_10_1_su");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r14_10_1_us");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r14_10_1_uu");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r14_10_8_ss");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r14_10_8_su");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r14_10_8_us");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r14_10_8_uu");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r14_14_10_ss");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r14_14_10_su");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r14_14_10_us");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r14_14_10_uu");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r14_14_14_ss");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r14_14_14_su");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r14_14_14_us");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r14_14_14_uu");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r14_14_1_ss");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r14_14_1_su");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r14_14_1_us");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r14_14_1_uu");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r14_14_8_ss");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r14_14_8_su");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r14_14_8_us");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r14_14_8_uu");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r14_1_10_ss");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r14_1_10_su");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r14_1_10_us");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r14_1_10_uu");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r14_1_14_ss");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r14_1_14_su");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r14_1_14_us");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r14_1_14_uu");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r14_1_1_ss");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r14_1_1_su");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r14_1_1_us");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r14_1_1_uu");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r14_1_8_ss");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r14_1_8_su");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r14_1_8_us");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r14_1_8_uu");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r14_8_10_ss");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r14_8_10_su");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r14_8_10_us");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r14_8_10_uu");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r14_8_14_ss");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r14_8_14_su");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r14_8_14_us");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r14_8_14_uu");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r14_8_1_ss");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r14_8_1_su");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r14_8_1_us");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r14_8_1_uu");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r14_8_8_ss");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r14_8_8_su");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r14_8_8_us");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r14_8_8_uu");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r1_10_10_ss");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r1_10_10_su");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r1_10_10_us");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r1_10_10_uu");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r1_10_14_ss");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r1_10_14_su");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r1_10_14_us");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r1_10_14_uu");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r1_10_1_ss");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r1_10_1_su");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r1_10_1_us");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r1_10_1_uu");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r1_10_8_ss");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r1_10_8_su");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r1_10_8_us");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r1_10_8_uu");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r1_14_10_ss");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r1_14_10_su");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r1_14_10_us");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r1_14_10_uu");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r1_14_14_ss");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r1_14_14_su");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r1_14_14_us");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r1_14_14_uu");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r1_14_1_ss");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r1_14_1_su");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r1_14_1_us");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r1_14_1_uu");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r1_14_8_ss");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r1_14_8_su");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r1_14_8_us");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r1_14_8_uu");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r1_1_10_ss");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r1_1_10_su");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r1_1_10_us");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r1_1_10_uu");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r1_1_14_ss");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r1_1_14_su");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r1_1_14_us");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r1_1_14_uu");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r1_1_1_ss");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r1_1_1_su");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r1_1_1_us");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r1_1_1_uu");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r1_1_8_ss");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r1_1_8_su");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r1_1_8_us");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r1_1_8_uu");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r1_8_10_ss");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r1_8_10_su");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r1_8_10_us");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r1_8_10_uu");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r1_8_14_ss");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r1_8_14_su");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r1_8_14_us");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r1_8_14_uu");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r1_8_1_ss");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r1_8_1_su");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r1_8_1_us");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r1_8_1_uu");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r1_8_8_ss");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r1_8_8_su");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r1_8_8_us");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r1_8_8_uu");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r8_10_10_ss");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r8_10_10_su");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r8_10_10_us");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r8_10_10_uu");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r8_10_14_ss");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r8_10_14_su");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r8_10_14_us");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r8_10_14_uu");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r8_10_1_ss");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r8_10_1_su");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r8_10_1_us");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r8_10_1_uu");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r8_10_8_ss");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r8_10_8_su");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r8_10_8_us");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r8_10_8_uu");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r8_14_10_ss");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r8_14_10_su");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r8_14_10_us");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r8_14_10_uu");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r8_14_14_ss");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r8_14_14_su");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r8_14_14_us");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r8_14_14_uu");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r8_14_1_ss");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r8_14_1_su");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r8_14_1_us");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r8_14_1_uu");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r8_14_8_ss");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r8_14_8_su");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r8_14_8_us");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r8_14_8_uu");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r8_1_10_ss");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r8_1_10_su");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r8_1_10_us");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r8_1_10_uu");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r8_1_14_ss");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r8_1_14_su");
	$fdisplay(file, "#OUT#   E   sig_AddBinOp_r8_1_14_us");
	$fdisplay(file, "#OUT#   F   sig_AddBinOp_r8_1_14_uu");
	$fdisplay(file, "#OUT#   G   sig_AddBinOp_r8_1_1_ss");
	$fdisplay(file, "#OUT#   H   sig_AddBinOp_r8_1_1_su");
	$fdisplay(file, "#OUT#   I   sig_AddBinOp_r8_1_1_us");
	$fdisplay(file, "#OUT#   J   sig_AddBinOp_r8_1_1_uu");
	$fdisplay(file, "#OUT#   K   sig_AddBinOp_r8_1_8_ss");
	$fdisplay(file, "#OUT#   L   sig_AddBinOp_r8_1_8_su");
	$fdisplay(file, "#OUT#   M   sig_AddBinOp_r8_1_8_us");
	$fdisplay(file, "#OUT#   N   sig_AddBinOp_r8_1_8_uu");
	$fdisplay(file, "#OUT#   O   sig_AddBinOp_r8_8_10_ss");
	$fdisplay(file, "#OUT#   P   sig_AddBinOp_r8_8_10_su");
	$fdisplay(file, "#OUT#   Q   sig_AddBinOp_r8_8_10_us");
	$fdisplay(file, "#OUT#   R   sig_AddBinOp_r8_8_10_uu");
	$fdisplay(file, "#OUT#   S   sig_AddBinOp_r8_8_14_ss");
	$fdisplay(file, "#OUT#   T   sig_AddBinOp_r8_8_14_su");
	$fdisplay(file, "#OUT#   U   sig_AddBinOp_r8_8_14_us");
	$fdisplay(file, "#OUT#   V   sig_AddBinOp_r8_8_14_uu");
	$fdisplay(file, "#OUT#   W   sig_AddBinOp_r8_8_1_ss");
	$fdisplay(file, "#OUT#   X   sig_AddBinOp_r8_8_1_su");
	$fdisplay(file, "#OUT#   Y   sig_AddBinOp_r8_8_1_us");
	$fdisplay(file, "#OUT#   Z   sig_AddBinOp_r8_8_1_uu");
	$fdisplay(file, "#OUT#   A   sig_AddBinOp_r8_8_8_ss");
	$fdisplay(file, "#OUT#   B   sig_AddBinOp_r8_8_8_su");
	$fdisplay(file, "#OUT#   C   sig_AddBinOp_r8_8_8_us");
	$fdisplay(file, "#OUT#   D   sig_AddBinOp_r8_8_8_uu");
	$fdisplay(file, "#OUT#");
	$fdisplay(file, {"#OUT# ", "A", "/--------B", "/------------C", "/------D", "E", "/--------F", "/------------G", "/------H", " ", "#", " ", "/--------I", "/--------J", "/--------K", "/--------L", "/--------M", "/--------N", "/--------O", "/--------P", "/--------Q", "/--------R", "/--------S", "/--------T", "/--------U", "/--------V", "/--------W", "/--------X", "/--------Y", "/--------Z", "/--------A", "/--------B", "/--------C", "/--------D", "/--------E", "/--------F", "/--------G", "/--------H", "/--------I", "/--------J", "/--------K", "/--------L", "/--------M", "/--------N", "/--------O", "/--------P", "/--------Q", "/--------R", "/--------S", "/--------T", "/--------U", "/--------V", "/--------W", "/--------X", "/--------Y", "/--------Z", "/--------A", "/--------B", "/--------C", "/--------D", "/--------E", "/--------F", "/--------G", "/--------H", "/--------I", "/--------J", "/--------K", "/--------L", "/--------M", "/--------N", "/--------O", "/--------P", "/--------Q", "/--------R", "/--------S", "/--------T", "/------------U", "/------------V", "/------------W", "/------------X", "/------------Y", "/------------Z", "/------------A", "/------------B", "/------------C", "/------------D", "/------------E", "/------------F", "/------------G", "/------------H", "/------------I", "/------------J", "/------------K", "/------------L", "/------------M", "/------------N", "/------------O", "/------------P", "/------------Q", "/------------R", "/------------S", "/------------T", "/------------U", "/------------V", "/------------W", "/------------X", "/------------Y", "/------------Z", "/------------A", "/------------B", "/------------C", "/------------D", "/------------E", "/------------F", "/------------G", "/------------H", "/------------I", "/------------J", "/------------K", "/------------L", "/------------M", "/------------N", "/------------O", "/------------P", "/------------Q", "/------------R", "/------------S", "/------------T", "/------------U", "/------------V", "/------------W", "/------------X", "/------------Y", "/------------Z", "/------------A", "/------------B", "/------------C", "/------------D", "/------------E", "/------------F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "/------S", "/------T", "/------U", "/------V", "/------W", "/------X", "/------Y", "/------Z", "/------A", "/------B", "/------C", "/------D", "/------E", "/------F", "/------G", "/------H", "/------I", "/------J", "/------K", "/------L", "/------M", "/------N", "/------O", "/------P", "/------Q", "/------R", "/------S", "/------T", "/------U", "/------V", "/------W", "/------X", "/------Y", "/------Z", "/------A", "/------B", "/------C", "/------D", "/------E", "/------F", "/------G", "/------H", "/------I", "/------J", "/------K", "/------L", "/------M", "/------N", "/------O", "/------P", "/------Q", "/------R", "/------S", "/------T", "/------U", "/------V", "/------W", "/------X", "/------Y", "/------Z", "/------A", "/------B", "/------C", "/------D"});
end
endtask

task AddBinOp_test;
begin
	$fdisplay(file, "#OUT#\n#OUT# ==== AddBinOp ====");
	AddBinOp_reset;
	for (i=0; i<1000; i=i+1) begin
		if (i % 20 == 0) AddBinOp_print_header;
		#100; AddBinOp_update_data;
		#100; AddBinOp_update_clock;
		#100; AddBinOp_print_status;
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
	AddBinOp_test;
	$fclose(file);
	$finish;
end

endmodule
