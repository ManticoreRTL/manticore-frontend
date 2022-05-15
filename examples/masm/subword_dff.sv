module SubwordDff(input wire clk, input wire rst, input wire en,
    input wire [3:0] a, input wire [3:0] b,
    output reg [7:0] c, output reg [3:0] d, output [3:0] f, output g, output h,
    output[1:0] x);
    reg [3:0] freg = 4'd4;

    assign {g, h} = {rst, en};
    always @(posedge clk) begin
        if (rst) begin
            {d , c} <= 0;
            c <= 0;
            freg <= 4'b1011;
        end else if (en) begin
            {d, c[3 : 0]} <= {(b - a), a};
            c[7: 4] <= b;
            freg <= a;
            // d <= (b - a);
        end
    end
    assign f = freg;
    assign x[0] = a;
endmodule