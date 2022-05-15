module BitReplicate(input wire a, input wire b, output wire [3:0] c,
    output wire [2:0] d, output wire [2:0] e);
    assign c = {a, a, b, a};
    assign d = {b, a, b} + {a, b, a};
    assign e = {b, a, b} + {b, b, b};
endmodule