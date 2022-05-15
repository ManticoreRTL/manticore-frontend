module Accum (
    input wire [31:0] in,
    input wire en,
    input wire clock,
    input wire reset,
    output wire [31:0] out
);

  logic [31:0] acc;
  logic [31:0] xx;
  // assert(acc >= 0);
  always @(posedge clock) begin
    if (reset) acc <= 0;
    else if (en) acc <= acc + in;
      xx = acc;
      $display("%d", xx);
      xx = xx + 3;
      $display("%d", xx);
    // if (acc > 32'd100) {
      $display("Hello %d, %d\n", acc, in);
    // }
    assert (acc <= 32'd100);
  end

  assign out = acc;

endmodule


// module Tester(input wire __clock__);


//   logic reset;
//   wire [31:0] acc;
//   wire [31:0] acc_out;
//   logic [31:0] cycle = 0;
//   localparam CYCLES = 256;
//   logic [4:0] reset_cycles = 3;
//   logic [31:0] values [CYCLES - 1 : 0];
//   logic [31:0] computed [CYCLES : 0];
//   logic [31:0] gold [CYCLES : 0];
//   integer ix;

//   initial begin

//     $readmemh("inputs.dat", values);
//     values[1] = 2;
//     values[2] = 3;
//     values[3] = 4;
//     values[255] = 243;
//     values[3] = 5;
//     $readmemh("gold_out.dat", gold);


//   end


//   always @(posedge __clock__) begin
//     if (reset_cycles == 0)
//       reset <= 1'b0;
//     else begin
//       reset <= 1'b1;
//       reset_cycles <= reset_cycles - 1;
//     end
//   end

//   always @(posedge __clock__) begin
//     if (reset != 1) begin
//       cycle <= cycle + 1;
//       if (cycle < CYCLES) begin
//         computed[cycle] <= acc;
//         $masm_expect(acc == gold[cycle], "invalid acc");
//       end else begin
//         // $masm_stop;
//       end
//     end
//   end

//   Accum dut(
//     .in(values[cycle]),
//     .en(1'b1),
//     .clock(__clock__),
//     .reset(reset),
//     .out(acc)
//   );


// endmodule

// module BB (input wire cond, input wire in, output wire[31:0] out);
// endmodule




