module Accum (
    input wire [31:0] in,
    input wire en,
    input wire clock,
    input wire reset,
    output wire [31:0] out
);

  logic [31:0] acc;

  always @(posedge clock) begin
    if (reset) acc <= 0;
    else if (en) acc <= acc + in;

    // assert (acc <= 32'd100);
    // else $error("overflow");
  end

  assign out = acc;

endmodule


module Tester(input wire __clock__);


  logic reset;
  wire [31:0] acc;
  logic [31:0] cycle = 0;
  localparam CYCLES = 256;
  logic [4:0] reset_cycles = 3;
  logic [31:0] values [CYCLES - 1 : 0];
  logic [31:0] gold [CYCLES : 0];
  integer ix;

  initial begin

    $readmemh("inputs.dat", values);
    $readmemh("gold_out.dat", gold);
    // for (ix = 0; ix < 16; ix = ix + 1) begin
    //   values[ix] = ix + 1;
    //   gold[ix + 1] = values[ix] + gold[ix - 1];
    // end
  end


  always @(posedge __clock__) begin
    if (reset_cycles == 0)
      reset <= 1'b0;
    else begin
      reset <= 1'b1;
      reset_cycles <= reset_cycles - 1;
    end
  end

  always @(posedge __clock__) begin
    if (reset != 1) begin
      cycle <= cycle + 1;
      if (cycle < CYCLES) begin
        $masm_expect(acc == gold[cycle], "Expected %d but got %d", gold[cycle], acc);
      end
    end
  end

  Accum dut(
    .in(values[cycle]),
    .en(1'b1),
    .clock(__clock__),
    .reset(reset),
    .out(acc)
  );
  // always_ff
    // $masm_expect(val < 10);


endmodule



