module Counter(
    input wire __clock__
);
    localparam TEST_SIZE = 32;
    logic [31:0] counter;
    logic [31:0] expected_values [TEST_SIZE - 1 : 0];
    initial begin
        counter = 0;
        $readmemh("expected.dat", expected_values);
    end

    always @(posedge __clock__) begin
        if (counter < TEST_SIZE) begin
            // assert(counter == expected_values[counter]);
            counter <= counter + 1;
        end else begin
            // $stop;
        end

    end

    always_comb begin
        assert(counter == expected_values[counter]);
    end

endmodule