module UnaryOps(
    input [3:0] a,
    output[3:0] not_,
    output [3:0] pos_,
    output [3:0] neg_,

    output [5:0] not_w,
    output [5:0] pos_w,
    output [5:0] neg_w,


    output[3:0] not_signed,
    output [3:0] pos_signed,
    output [3:0] neg_signed,

    output[5:0] not_signed_w,
    output [5:0] pos_signed_w,
    output [5:0] neg_signed_w,

    output [3:0] reduce_and_,
    output [3:0] reduce_or_,
    output [3:0] reduce_xor_,
    output [3:0] reduce_xnor_,
    output [3:0] reduce_bool_,
    output [3:0] logic_not_,

    output [3:0] reduce_and_signed,
    output [3:0] reduce_or_signed,
    output [3:0] reduce_xor_signed,
    output [3:0] reduce_xnor_signed,
    output [3:0] reduce_bool_signed,
    output [3:0] logic_not_signed,

    output [0:0] reduce_and_bit,
    output [0:0] reduce_or_bit,
    output [0:0] reduce_xor_bit,
    output [0:0] reduce_xnor_bit,
    output [0:0] reduce_bool_bit,
    output [0:0] logic_not_bit,


    output [0:0] reduce_xor_bit_bit,
    output [0:0] reduce_xnor_bit_bit
    );

    assign not_ = ~a;
    assign pos_ = +a;
    assign neg_ = -a;

    assign not_w = ~a;
    assign pos_w = +a;
    assign neg_w = -a;


    assign not_signed = ~$signed(a);
    assign pos_signed = +$signed(a);
    assign neg_signed = -$signed(a);

    assign not_signed_w = ~$signed(a);
    assign pos_signed_w = +$signed(a);
    assign neg_signed_w = -$signed(a);

    assign reduce_and_ = &a;
    assign reduce_or_ = |a;
    assign reduce_xor_ = ^a;
    assign reduce_xnor_ = ~^a;
    assign reduce_bool_ = a ? 1'b1 : 1'b0;
    assign logic_not_ = !a ? 1'b1 : 1'b0;


    assign reduce_and_signed = &$signed(a);
    assign reduce_or_signed = |$signed(a);
    assign reduce_xor_signed = ^$signed(a);
    assign reduce_xnor_signed = ~^$signed(a);
    assign reduce_bool_signed = $signed(a) ? 1'b1 : 1'b0;
    assign logic_not_signed = !$signed(a) ? 1'b1 : 1'b0;

    assign reduce_and_bit = &a;
    assign reduce_or_bit = |a;
    assign reduce_xor_bit = ^a;
    assign reduce_xnor_bit = ~^a;
    assign reduce_bool_bit = a ? 1'b1 : 1'b0;
    assign logic_not_bit = !a ? 1'b1 : 1'b0;


    assign reduce_xor_bit_bit = ^a[0];
    assign reduce_xnor_bit_bit = ~^a[0];

endmodule