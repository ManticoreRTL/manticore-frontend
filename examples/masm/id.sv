module Identity(input wire [31:0] a, output logic [31:0] b);

    logic [31:0] out;
    always_comb begin
        out = a;
        out = out;
        b = out;
    end
endmodule