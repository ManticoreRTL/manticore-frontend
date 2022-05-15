
module AndMod(input wire a, input wire b, output wire c);
    assign c = a & b;
endmodule
module WithGenerate(input wire [3:0] a, input wire [3:0] b, output wire [3:0] c);
    genvar i;
    generate
        for (i = 0 ; i < 4; i = i + 1) begin
            AndMod dut(a[i], b[i], c[i]);
        end
    endgenerate
endmodule