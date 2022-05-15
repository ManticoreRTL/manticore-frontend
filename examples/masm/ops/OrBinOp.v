module OrBinOp(input [1 - 1 : 0] a1,
input [1 - 1 : 0] b1,
input [8 - 1 : 0] a8,
input [8 - 1 : 0] b8,
input [10 - 1 : 0] a10,
input [10 - 1 : 0] b10,
input [14 - 1 : 0] a14,
input [14 - 1 : 0] b14, 
output [1 - 1 : 0] r1_1_1_uu,
output [1 - 1 : 0] r1_1_1_su,
output [1 - 1 : 0] r1_1_1_us,
output [1 - 1 : 0] r1_1_1_ss,
output [8 - 1 : 0] r8_1_1_uu,
output [8 - 1 : 0] r8_1_1_su,
output [8 - 1 : 0] r8_1_1_us,
output [8 - 1 : 0] r8_1_1_ss,
output [10 - 1 : 0] r10_1_1_uu,
output [10 - 1 : 0] r10_1_1_su,
output [10 - 1 : 0] r10_1_1_us,
output [10 - 1 : 0] r10_1_1_ss,
output [14 - 1 : 0] r14_1_1_uu,
output [14 - 1 : 0] r14_1_1_su,
output [14 - 1 : 0] r14_1_1_us,
output [14 - 1 : 0] r14_1_1_ss,
output [1 - 1 : 0] r1_1_8_uu,
output [1 - 1 : 0] r1_1_8_su,
output [1 - 1 : 0] r1_1_8_us,
output [1 - 1 : 0] r1_1_8_ss,
output [8 - 1 : 0] r8_1_8_uu,
output [8 - 1 : 0] r8_1_8_su,
output [8 - 1 : 0] r8_1_8_us,
output [8 - 1 : 0] r8_1_8_ss,
output [10 - 1 : 0] r10_1_8_uu,
output [10 - 1 : 0] r10_1_8_su,
output [10 - 1 : 0] r10_1_8_us,
output [10 - 1 : 0] r10_1_8_ss,
output [14 - 1 : 0] r14_1_8_uu,
output [14 - 1 : 0] r14_1_8_su,
output [14 - 1 : 0] r14_1_8_us,
output [14 - 1 : 0] r14_1_8_ss,
output [1 - 1 : 0] r1_1_10_uu,
output [1 - 1 : 0] r1_1_10_su,
output [1 - 1 : 0] r1_1_10_us,
output [1 - 1 : 0] r1_1_10_ss,
output [8 - 1 : 0] r8_1_10_uu,
output [8 - 1 : 0] r8_1_10_su,
output [8 - 1 : 0] r8_1_10_us,
output [8 - 1 : 0] r8_1_10_ss,
output [10 - 1 : 0] r10_1_10_uu,
output [10 - 1 : 0] r10_1_10_su,
output [10 - 1 : 0] r10_1_10_us,
output [10 - 1 : 0] r10_1_10_ss,
output [14 - 1 : 0] r14_1_10_uu,
output [14 - 1 : 0] r14_1_10_su,
output [14 - 1 : 0] r14_1_10_us,
output [14 - 1 : 0] r14_1_10_ss,
output [1 - 1 : 0] r1_1_14_uu,
output [1 - 1 : 0] r1_1_14_su,
output [1 - 1 : 0] r1_1_14_us,
output [1 - 1 : 0] r1_1_14_ss,
output [8 - 1 : 0] r8_1_14_uu,
output [8 - 1 : 0] r8_1_14_su,
output [8 - 1 : 0] r8_1_14_us,
output [8 - 1 : 0] r8_1_14_ss,
output [10 - 1 : 0] r10_1_14_uu,
output [10 - 1 : 0] r10_1_14_su,
output [10 - 1 : 0] r10_1_14_us,
output [10 - 1 : 0] r10_1_14_ss,
output [14 - 1 : 0] r14_1_14_uu,
output [14 - 1 : 0] r14_1_14_su,
output [14 - 1 : 0] r14_1_14_us,
output [14 - 1 : 0] r14_1_14_ss,
output [1 - 1 : 0] r1_8_1_uu,
output [1 - 1 : 0] r1_8_1_su,
output [1 - 1 : 0] r1_8_1_us,
output [1 - 1 : 0] r1_8_1_ss,
output [8 - 1 : 0] r8_8_1_uu,
output [8 - 1 : 0] r8_8_1_su,
output [8 - 1 : 0] r8_8_1_us,
output [8 - 1 : 0] r8_8_1_ss,
output [10 - 1 : 0] r10_8_1_uu,
output [10 - 1 : 0] r10_8_1_su,
output [10 - 1 : 0] r10_8_1_us,
output [10 - 1 : 0] r10_8_1_ss,
output [14 - 1 : 0] r14_8_1_uu,
output [14 - 1 : 0] r14_8_1_su,
output [14 - 1 : 0] r14_8_1_us,
output [14 - 1 : 0] r14_8_1_ss,
output [1 - 1 : 0] r1_8_8_uu,
output [1 - 1 : 0] r1_8_8_su,
output [1 - 1 : 0] r1_8_8_us,
output [1 - 1 : 0] r1_8_8_ss,
output [8 - 1 : 0] r8_8_8_uu,
output [8 - 1 : 0] r8_8_8_su,
output [8 - 1 : 0] r8_8_8_us,
output [8 - 1 : 0] r8_8_8_ss,
output [10 - 1 : 0] r10_8_8_uu,
output [10 - 1 : 0] r10_8_8_su,
output [10 - 1 : 0] r10_8_8_us,
output [10 - 1 : 0] r10_8_8_ss,
output [14 - 1 : 0] r14_8_8_uu,
output [14 - 1 : 0] r14_8_8_su,
output [14 - 1 : 0] r14_8_8_us,
output [14 - 1 : 0] r14_8_8_ss,
output [1 - 1 : 0] r1_8_10_uu,
output [1 - 1 : 0] r1_8_10_su,
output [1 - 1 : 0] r1_8_10_us,
output [1 - 1 : 0] r1_8_10_ss,
output [8 - 1 : 0] r8_8_10_uu,
output [8 - 1 : 0] r8_8_10_su,
output [8 - 1 : 0] r8_8_10_us,
output [8 - 1 : 0] r8_8_10_ss,
output [10 - 1 : 0] r10_8_10_uu,
output [10 - 1 : 0] r10_8_10_su,
output [10 - 1 : 0] r10_8_10_us,
output [10 - 1 : 0] r10_8_10_ss,
output [14 - 1 : 0] r14_8_10_uu,
output [14 - 1 : 0] r14_8_10_su,
output [14 - 1 : 0] r14_8_10_us,
output [14 - 1 : 0] r14_8_10_ss,
output [1 - 1 : 0] r1_8_14_uu,
output [1 - 1 : 0] r1_8_14_su,
output [1 - 1 : 0] r1_8_14_us,
output [1 - 1 : 0] r1_8_14_ss,
output [8 - 1 : 0] r8_8_14_uu,
output [8 - 1 : 0] r8_8_14_su,
output [8 - 1 : 0] r8_8_14_us,
output [8 - 1 : 0] r8_8_14_ss,
output [10 - 1 : 0] r10_8_14_uu,
output [10 - 1 : 0] r10_8_14_su,
output [10 - 1 : 0] r10_8_14_us,
output [10 - 1 : 0] r10_8_14_ss,
output [14 - 1 : 0] r14_8_14_uu,
output [14 - 1 : 0] r14_8_14_su,
output [14 - 1 : 0] r14_8_14_us,
output [14 - 1 : 0] r14_8_14_ss,
output [1 - 1 : 0] r1_10_1_uu,
output [1 - 1 : 0] r1_10_1_su,
output [1 - 1 : 0] r1_10_1_us,
output [1 - 1 : 0] r1_10_1_ss,
output [8 - 1 : 0] r8_10_1_uu,
output [8 - 1 : 0] r8_10_1_su,
output [8 - 1 : 0] r8_10_1_us,
output [8 - 1 : 0] r8_10_1_ss,
output [10 - 1 : 0] r10_10_1_uu,
output [10 - 1 : 0] r10_10_1_su,
output [10 - 1 : 0] r10_10_1_us,
output [10 - 1 : 0] r10_10_1_ss,
output [14 - 1 : 0] r14_10_1_uu,
output [14 - 1 : 0] r14_10_1_su,
output [14 - 1 : 0] r14_10_1_us,
output [14 - 1 : 0] r14_10_1_ss,
output [1 - 1 : 0] r1_10_8_uu,
output [1 - 1 : 0] r1_10_8_su,
output [1 - 1 : 0] r1_10_8_us,
output [1 - 1 : 0] r1_10_8_ss,
output [8 - 1 : 0] r8_10_8_uu,
output [8 - 1 : 0] r8_10_8_su,
output [8 - 1 : 0] r8_10_8_us,
output [8 - 1 : 0] r8_10_8_ss,
output [10 - 1 : 0] r10_10_8_uu,
output [10 - 1 : 0] r10_10_8_su,
output [10 - 1 : 0] r10_10_8_us,
output [10 - 1 : 0] r10_10_8_ss,
output [14 - 1 : 0] r14_10_8_uu,
output [14 - 1 : 0] r14_10_8_su,
output [14 - 1 : 0] r14_10_8_us,
output [14 - 1 : 0] r14_10_8_ss,
output [1 - 1 : 0] r1_10_10_uu,
output [1 - 1 : 0] r1_10_10_su,
output [1 - 1 : 0] r1_10_10_us,
output [1 - 1 : 0] r1_10_10_ss,
output [8 - 1 : 0] r8_10_10_uu,
output [8 - 1 : 0] r8_10_10_su,
output [8 - 1 : 0] r8_10_10_us,
output [8 - 1 : 0] r8_10_10_ss,
output [10 - 1 : 0] r10_10_10_uu,
output [10 - 1 : 0] r10_10_10_su,
output [10 - 1 : 0] r10_10_10_us,
output [10 - 1 : 0] r10_10_10_ss,
output [14 - 1 : 0] r14_10_10_uu,
output [14 - 1 : 0] r14_10_10_su,
output [14 - 1 : 0] r14_10_10_us,
output [14 - 1 : 0] r14_10_10_ss,
output [1 - 1 : 0] r1_10_14_uu,
output [1 - 1 : 0] r1_10_14_su,
output [1 - 1 : 0] r1_10_14_us,
output [1 - 1 : 0] r1_10_14_ss,
output [8 - 1 : 0] r8_10_14_uu,
output [8 - 1 : 0] r8_10_14_su,
output [8 - 1 : 0] r8_10_14_us,
output [8 - 1 : 0] r8_10_14_ss,
output [10 - 1 : 0] r10_10_14_uu,
output [10 - 1 : 0] r10_10_14_su,
output [10 - 1 : 0] r10_10_14_us,
output [10 - 1 : 0] r10_10_14_ss,
output [14 - 1 : 0] r14_10_14_uu,
output [14 - 1 : 0] r14_10_14_su,
output [14 - 1 : 0] r14_10_14_us,
output [14 - 1 : 0] r14_10_14_ss,
output [1 - 1 : 0] r1_14_1_uu,
output [1 - 1 : 0] r1_14_1_su,
output [1 - 1 : 0] r1_14_1_us,
output [1 - 1 : 0] r1_14_1_ss,
output [8 - 1 : 0] r8_14_1_uu,
output [8 - 1 : 0] r8_14_1_su,
output [8 - 1 : 0] r8_14_1_us,
output [8 - 1 : 0] r8_14_1_ss,
output [10 - 1 : 0] r10_14_1_uu,
output [10 - 1 : 0] r10_14_1_su,
output [10 - 1 : 0] r10_14_1_us,
output [10 - 1 : 0] r10_14_1_ss,
output [14 - 1 : 0] r14_14_1_uu,
output [14 - 1 : 0] r14_14_1_su,
output [14 - 1 : 0] r14_14_1_us,
output [14 - 1 : 0] r14_14_1_ss,
output [1 - 1 : 0] r1_14_8_uu,
output [1 - 1 : 0] r1_14_8_su,
output [1 - 1 : 0] r1_14_8_us,
output [1 - 1 : 0] r1_14_8_ss,
output [8 - 1 : 0] r8_14_8_uu,
output [8 - 1 : 0] r8_14_8_su,
output [8 - 1 : 0] r8_14_8_us,
output [8 - 1 : 0] r8_14_8_ss,
output [10 - 1 : 0] r10_14_8_uu,
output [10 - 1 : 0] r10_14_8_su,
output [10 - 1 : 0] r10_14_8_us,
output [10 - 1 : 0] r10_14_8_ss,
output [14 - 1 : 0] r14_14_8_uu,
output [14 - 1 : 0] r14_14_8_su,
output [14 - 1 : 0] r14_14_8_us,
output [14 - 1 : 0] r14_14_8_ss,
output [1 - 1 : 0] r1_14_10_uu,
output [1 - 1 : 0] r1_14_10_su,
output [1 - 1 : 0] r1_14_10_us,
output [1 - 1 : 0] r1_14_10_ss,
output [8 - 1 : 0] r8_14_10_uu,
output [8 - 1 : 0] r8_14_10_su,
output [8 - 1 : 0] r8_14_10_us,
output [8 - 1 : 0] r8_14_10_ss,
output [10 - 1 : 0] r10_14_10_uu,
output [10 - 1 : 0] r10_14_10_su,
output [10 - 1 : 0] r10_14_10_us,
output [10 - 1 : 0] r10_14_10_ss,
output [14 - 1 : 0] r14_14_10_uu,
output [14 - 1 : 0] r14_14_10_su,
output [14 - 1 : 0] r14_14_10_us,
output [14 - 1 : 0] r14_14_10_ss,
output [1 - 1 : 0] r1_14_14_uu,
output [1 - 1 : 0] r1_14_14_su,
output [1 - 1 : 0] r1_14_14_us,
output [1 - 1 : 0] r1_14_14_ss,
output [8 - 1 : 0] r8_14_14_uu,
output [8 - 1 : 0] r8_14_14_su,
output [8 - 1 : 0] r8_14_14_us,
output [8 - 1 : 0] r8_14_14_ss,
output [10 - 1 : 0] r10_14_14_uu,
output [10 - 1 : 0] r10_14_14_su,
output [10 - 1 : 0] r10_14_14_us,
output [10 - 1 : 0] r10_14_14_ss,
output [14 - 1 : 0] r14_14_14_uu,
output [14 - 1 : 0] r14_14_14_su,
output [14 - 1 : 0] r14_14_14_us,
output [14 - 1 : 0] r14_14_14_ss);
assign r1_1_1_uu = a1 | b1;
assign r1_1_1_su = $signed(a1) | b1;
assign r1_1_1_uu = a1 | $signed(b1);
assign r1_1_1_uu = $signed(a1) | $signed(b1);
assign r8_1_1_uu = a1 | b1;
assign r8_1_1_su = $signed(a1) | b1;
assign r8_1_1_uu = a1 | $signed(b1);
assign r8_1_1_uu = $signed(a1) | $signed(b1);
assign r10_1_1_uu = a1 | b1;
assign r10_1_1_su = $signed(a1) | b1;
assign r10_1_1_uu = a1 | $signed(b1);
assign r10_1_1_uu = $signed(a1) | $signed(b1);
assign r14_1_1_uu = a1 | b1;
assign r14_1_1_su = $signed(a1) | b1;
assign r14_1_1_uu = a1 | $signed(b1);
assign r14_1_1_uu = $signed(a1) | $signed(b1);
assign r1_1_8_uu = a1 | b8;
assign r1_1_8_su = $signed(a1) | b8;
assign r1_1_8_uu = a1 | $signed(b8);
assign r1_1_8_uu = $signed(a1) | $signed(b8);
assign r8_1_8_uu = a1 | b8;
assign r8_1_8_su = $signed(a1) | b8;
assign r8_1_8_uu = a1 | $signed(b8);
assign r8_1_8_uu = $signed(a1) | $signed(b8);
assign r10_1_8_uu = a1 | b8;
assign r10_1_8_su = $signed(a1) | b8;
assign r10_1_8_uu = a1 | $signed(b8);
assign r10_1_8_uu = $signed(a1) | $signed(b8);
assign r14_1_8_uu = a1 | b8;
assign r14_1_8_su = $signed(a1) | b8;
assign r14_1_8_uu = a1 | $signed(b8);
assign r14_1_8_uu = $signed(a1) | $signed(b8);
assign r1_1_10_uu = a1 | b10;
assign r1_1_10_su = $signed(a1) | b10;
assign r1_1_10_uu = a1 | $signed(b10);
assign r1_1_10_uu = $signed(a1) | $signed(b10);
assign r8_1_10_uu = a1 | b10;
assign r8_1_10_su = $signed(a1) | b10;
assign r8_1_10_uu = a1 | $signed(b10);
assign r8_1_10_uu = $signed(a1) | $signed(b10);
assign r10_1_10_uu = a1 | b10;
assign r10_1_10_su = $signed(a1) | b10;
assign r10_1_10_uu = a1 | $signed(b10);
assign r10_1_10_uu = $signed(a1) | $signed(b10);
assign r14_1_10_uu = a1 | b10;
assign r14_1_10_su = $signed(a1) | b10;
assign r14_1_10_uu = a1 | $signed(b10);
assign r14_1_10_uu = $signed(a1) | $signed(b10);
assign r1_1_14_uu = a1 | b14;
assign r1_1_14_su = $signed(a1) | b14;
assign r1_1_14_uu = a1 | $signed(b14);
assign r1_1_14_uu = $signed(a1) | $signed(b14);
assign r8_1_14_uu = a1 | b14;
assign r8_1_14_su = $signed(a1) | b14;
assign r8_1_14_uu = a1 | $signed(b14);
assign r8_1_14_uu = $signed(a1) | $signed(b14);
assign r10_1_14_uu = a1 | b14;
assign r10_1_14_su = $signed(a1) | b14;
assign r10_1_14_uu = a1 | $signed(b14);
assign r10_1_14_uu = $signed(a1) | $signed(b14);
assign r14_1_14_uu = a1 | b14;
assign r14_1_14_su = $signed(a1) | b14;
assign r14_1_14_uu = a1 | $signed(b14);
assign r14_1_14_uu = $signed(a1) | $signed(b14);
assign r1_8_1_uu = a8 | b1;
assign r1_8_1_su = $signed(a8) | b1;
assign r1_8_1_uu = a8 | $signed(b1);
assign r1_8_1_uu = $signed(a8) | $signed(b1);
assign r8_8_1_uu = a8 | b1;
assign r8_8_1_su = $signed(a8) | b1;
assign r8_8_1_uu = a8 | $signed(b1);
assign r8_8_1_uu = $signed(a8) | $signed(b1);
assign r10_8_1_uu = a8 | b1;
assign r10_8_1_su = $signed(a8) | b1;
assign r10_8_1_uu = a8 | $signed(b1);
assign r10_8_1_uu = $signed(a8) | $signed(b1);
assign r14_8_1_uu = a8 | b1;
assign r14_8_1_su = $signed(a8) | b1;
assign r14_8_1_uu = a8 | $signed(b1);
assign r14_8_1_uu = $signed(a8) | $signed(b1);
assign r1_8_8_uu = a8 | b8;
assign r1_8_8_su = $signed(a8) | b8;
assign r1_8_8_uu = a8 | $signed(b8);
assign r1_8_8_uu = $signed(a8) | $signed(b8);
assign r8_8_8_uu = a8 | b8;
assign r8_8_8_su = $signed(a8) | b8;
assign r8_8_8_uu = a8 | $signed(b8);
assign r8_8_8_uu = $signed(a8) | $signed(b8);
assign r10_8_8_uu = a8 | b8;
assign r10_8_8_su = $signed(a8) | b8;
assign r10_8_8_uu = a8 | $signed(b8);
assign r10_8_8_uu = $signed(a8) | $signed(b8);
assign r14_8_8_uu = a8 | b8;
assign r14_8_8_su = $signed(a8) | b8;
assign r14_8_8_uu = a8 | $signed(b8);
assign r14_8_8_uu = $signed(a8) | $signed(b8);
assign r1_8_10_uu = a8 | b10;
assign r1_8_10_su = $signed(a8) | b10;
assign r1_8_10_uu = a8 | $signed(b10);
assign r1_8_10_uu = $signed(a8) | $signed(b10);
assign r8_8_10_uu = a8 | b10;
assign r8_8_10_su = $signed(a8) | b10;
assign r8_8_10_uu = a8 | $signed(b10);
assign r8_8_10_uu = $signed(a8) | $signed(b10);
assign r10_8_10_uu = a8 | b10;
assign r10_8_10_su = $signed(a8) | b10;
assign r10_8_10_uu = a8 | $signed(b10);
assign r10_8_10_uu = $signed(a8) | $signed(b10);
assign r14_8_10_uu = a8 | b10;
assign r14_8_10_su = $signed(a8) | b10;
assign r14_8_10_uu = a8 | $signed(b10);
assign r14_8_10_uu = $signed(a8) | $signed(b10);
assign r1_8_14_uu = a8 | b14;
assign r1_8_14_su = $signed(a8) | b14;
assign r1_8_14_uu = a8 | $signed(b14);
assign r1_8_14_uu = $signed(a8) | $signed(b14);
assign r8_8_14_uu = a8 | b14;
assign r8_8_14_su = $signed(a8) | b14;
assign r8_8_14_uu = a8 | $signed(b14);
assign r8_8_14_uu = $signed(a8) | $signed(b14);
assign r10_8_14_uu = a8 | b14;
assign r10_8_14_su = $signed(a8) | b14;
assign r10_8_14_uu = a8 | $signed(b14);
assign r10_8_14_uu = $signed(a8) | $signed(b14);
assign r14_8_14_uu = a8 | b14;
assign r14_8_14_su = $signed(a8) | b14;
assign r14_8_14_uu = a8 | $signed(b14);
assign r14_8_14_uu = $signed(a8) | $signed(b14);
assign r1_10_1_uu = a10 | b1;
assign r1_10_1_su = $signed(a10) | b1;
assign r1_10_1_uu = a10 | $signed(b1);
assign r1_10_1_uu = $signed(a10) | $signed(b1);
assign r8_10_1_uu = a10 | b1;
assign r8_10_1_su = $signed(a10) | b1;
assign r8_10_1_uu = a10 | $signed(b1);
assign r8_10_1_uu = $signed(a10) | $signed(b1);
assign r10_10_1_uu = a10 | b1;
assign r10_10_1_su = $signed(a10) | b1;
assign r10_10_1_uu = a10 | $signed(b1);
assign r10_10_1_uu = $signed(a10) | $signed(b1);
assign r14_10_1_uu = a10 | b1;
assign r14_10_1_su = $signed(a10) | b1;
assign r14_10_1_uu = a10 | $signed(b1);
assign r14_10_1_uu = $signed(a10) | $signed(b1);
assign r1_10_8_uu = a10 | b8;
assign r1_10_8_su = $signed(a10) | b8;
assign r1_10_8_uu = a10 | $signed(b8);
assign r1_10_8_uu = $signed(a10) | $signed(b8);
assign r8_10_8_uu = a10 | b8;
assign r8_10_8_su = $signed(a10) | b8;
assign r8_10_8_uu = a10 | $signed(b8);
assign r8_10_8_uu = $signed(a10) | $signed(b8);
assign r10_10_8_uu = a10 | b8;
assign r10_10_8_su = $signed(a10) | b8;
assign r10_10_8_uu = a10 | $signed(b8);
assign r10_10_8_uu = $signed(a10) | $signed(b8);
assign r14_10_8_uu = a10 | b8;
assign r14_10_8_su = $signed(a10) | b8;
assign r14_10_8_uu = a10 | $signed(b8);
assign r14_10_8_uu = $signed(a10) | $signed(b8);
assign r1_10_10_uu = a10 | b10;
assign r1_10_10_su = $signed(a10) | b10;
assign r1_10_10_uu = a10 | $signed(b10);
assign r1_10_10_uu = $signed(a10) | $signed(b10);
assign r8_10_10_uu = a10 | b10;
assign r8_10_10_su = $signed(a10) | b10;
assign r8_10_10_uu = a10 | $signed(b10);
assign r8_10_10_uu = $signed(a10) | $signed(b10);
assign r10_10_10_uu = a10 | b10;
assign r10_10_10_su = $signed(a10) | b10;
assign r10_10_10_uu = a10 | $signed(b10);
assign r10_10_10_uu = $signed(a10) | $signed(b10);
assign r14_10_10_uu = a10 | b10;
assign r14_10_10_su = $signed(a10) | b10;
assign r14_10_10_uu = a10 | $signed(b10);
assign r14_10_10_uu = $signed(a10) | $signed(b10);
assign r1_10_14_uu = a10 | b14;
assign r1_10_14_su = $signed(a10) | b14;
assign r1_10_14_uu = a10 | $signed(b14);
assign r1_10_14_uu = $signed(a10) | $signed(b14);
assign r8_10_14_uu = a10 | b14;
assign r8_10_14_su = $signed(a10) | b14;
assign r8_10_14_uu = a10 | $signed(b14);
assign r8_10_14_uu = $signed(a10) | $signed(b14);
assign r10_10_14_uu = a10 | b14;
assign r10_10_14_su = $signed(a10) | b14;
assign r10_10_14_uu = a10 | $signed(b14);
assign r10_10_14_uu = $signed(a10) | $signed(b14);
assign r14_10_14_uu = a10 | b14;
assign r14_10_14_su = $signed(a10) | b14;
assign r14_10_14_uu = a10 | $signed(b14);
assign r14_10_14_uu = $signed(a10) | $signed(b14);
assign r1_14_1_uu = a14 | b1;
assign r1_14_1_su = $signed(a14) | b1;
assign r1_14_1_uu = a14 | $signed(b1);
assign r1_14_1_uu = $signed(a14) | $signed(b1);
assign r8_14_1_uu = a14 | b1;
assign r8_14_1_su = $signed(a14) | b1;
assign r8_14_1_uu = a14 | $signed(b1);
assign r8_14_1_uu = $signed(a14) | $signed(b1);
assign r10_14_1_uu = a14 | b1;
assign r10_14_1_su = $signed(a14) | b1;
assign r10_14_1_uu = a14 | $signed(b1);
assign r10_14_1_uu = $signed(a14) | $signed(b1);
assign r14_14_1_uu = a14 | b1;
assign r14_14_1_su = $signed(a14) | b1;
assign r14_14_1_uu = a14 | $signed(b1);
assign r14_14_1_uu = $signed(a14) | $signed(b1);
assign r1_14_8_uu = a14 | b8;
assign r1_14_8_su = $signed(a14) | b8;
assign r1_14_8_uu = a14 | $signed(b8);
assign r1_14_8_uu = $signed(a14) | $signed(b8);
assign r8_14_8_uu = a14 | b8;
assign r8_14_8_su = $signed(a14) | b8;
assign r8_14_8_uu = a14 | $signed(b8);
assign r8_14_8_uu = $signed(a14) | $signed(b8);
assign r10_14_8_uu = a14 | b8;
assign r10_14_8_su = $signed(a14) | b8;
assign r10_14_8_uu = a14 | $signed(b8);
assign r10_14_8_uu = $signed(a14) | $signed(b8);
assign r14_14_8_uu = a14 | b8;
assign r14_14_8_su = $signed(a14) | b8;
assign r14_14_8_uu = a14 | $signed(b8);
assign r14_14_8_uu = $signed(a14) | $signed(b8);
assign r1_14_10_uu = a14 | b10;
assign r1_14_10_su = $signed(a14) | b10;
assign r1_14_10_uu = a14 | $signed(b10);
assign r1_14_10_uu = $signed(a14) | $signed(b10);
assign r8_14_10_uu = a14 | b10;
assign r8_14_10_su = $signed(a14) | b10;
assign r8_14_10_uu = a14 | $signed(b10);
assign r8_14_10_uu = $signed(a14) | $signed(b10);
assign r10_14_10_uu = a14 | b10;
assign r10_14_10_su = $signed(a14) | b10;
assign r10_14_10_uu = a14 | $signed(b10);
assign r10_14_10_uu = $signed(a14) | $signed(b10);
assign r14_14_10_uu = a14 | b10;
assign r14_14_10_su = $signed(a14) | b10;
assign r14_14_10_uu = a14 | $signed(b10);
assign r14_14_10_uu = $signed(a14) | $signed(b10);
assign r1_14_14_uu = a14 | b14;
assign r1_14_14_su = $signed(a14) | b14;
assign r1_14_14_uu = a14 | $signed(b14);
assign r1_14_14_uu = $signed(a14) | $signed(b14);
assign r8_14_14_uu = a14 | b14;
assign r8_14_14_su = $signed(a14) | b14;
assign r8_14_14_uu = a14 | $signed(b14);
assign r8_14_14_uu = $signed(a14) | $signed(b14);
assign r10_14_14_uu = a14 | b14;
assign r10_14_14_su = $signed(a14) | b14;
assign r10_14_14_uu = a14 | $signed(b14);
assign r10_14_14_uu = $signed(a14) | $signed(b14);
assign r14_14_14_uu = a14 | b14;
assign r14_14_14_su = $signed(a14) | b14;
assign r14_14_14_uu = a14 | $signed(b14);
assign r14_14_14_uu = $signed(a14) | $signed(b14);
endmodule