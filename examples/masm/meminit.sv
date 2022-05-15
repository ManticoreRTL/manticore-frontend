module MemoryInitTest (
    input wire [11 : 0] addr,
    input wire [12 : 0] addr2,
    input wire [63 : 0] din,
    input wire [63 : 0] din2,
    input wire [7  : 0] wstrb,
    input wire wen,
    input wire ren,
    input wire en,
    output logic [63 : 0] dout,
    input wire clk
);

    logic [63:0] storage [0 : 1 << 12];
    (*keep*)
    logic rword;


    initial begin
        rword = 1'b1;
    end
    integer i;
    always @(posedge clk) begin
        if (en)
            rword <= (addr == addr2);
        if (wen) begin
                storage[addr2] <= din2;
        end
        if (wen & en) begin
            for (i = 0; i < 7; i = i + 1) begin
                if (wstrb[i]) { storage[addr][i * 8 + 4 +: 4], storage[addr][i * 8 +: 4] } <= din[i * 8 +: 8];
            end

            // rword = storage[addr];
            // for (i = 0; i < 8; i = i + 1) begin
            //         if (wstrb[i]) rword[i * 8 +: 8] = din[i * 8 +: 8];
            // end
            // storage[addr] <= rword;
        end
        if (ren & en) begin
            // $display("Reading %d", sotrage[addr]);
            dout <= storage[addr];
        end
        storage[addr][63 -: 8] <= din[63 -: 8];
    end

    // assign dout = storage[addr];
    // assign dout[0] = 0;

    initial begin
        // storage[0][0 +: 8] = 8'd32;
        // storage[0][16 +: 8] = 8'hff;
        // storage[1] = 64'd1232;
        // storage[2] = 64'd12;

        // storage[1] = 64'd15;
    end


endmodule