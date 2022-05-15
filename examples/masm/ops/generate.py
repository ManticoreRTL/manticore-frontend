from re import sub
import subprocess


def generate(name, op):


    width = [1, 8, 10, 14]
    inputs = []
    outputs = []
    stmts = []

    for w1 in width:
        inputs.append(
            f"input [{w1} - 1 : 0] a{w1}"
        )
        inputs.append(
            f"input [{w1} - 1 : 0] b{w1}"
        )
        for w2 in width:
            for w3 in width:

                outputs.append(
                    f"output [{w3} - 1 : 0] r{w3}_{w1}_{w2}_uu"
                )
                outputs.append(
                    f"output [{w3} - 1 : 0] r{w3}_{w1}_{w2}_su"
                )
                outputs.append(
                    f"output [{w3} - 1 : 0] r{w3}_{w1}_{w2}_us"
                )
                outputs.append(
                    f"output [{w3} - 1 : 0] r{w3}_{w1}_{w2}_ss"
                )
                stmts.append(
                    f"assign r{w3}_{w1}_{w2}_uu = a{w1} {op} b{w2};"
                )
                stmts.append(
                    f"assign r{w3}_{w1}_{w2}_su = $signed(a{w1}) {op} b{w2};"
                )
                stmts.append(
                    f"assign r{w3}_{w1}_{w2}_uu = a{w1} {op} $signed(b{w2});"
                )
                stmts.append(
                    f"assign r{w3}_{w1}_{w2}_uu = $signed(a{w1}) {op} $signed(b{w2});"
                )

    inpstr = ",\n".join(inputs)
    outpstr = ",\n".join(outputs)
    assigns = "\n".join(stmts)
    code = f"module {name}({inpstr}, \n{outpstr});\n{assigns}\nendmodule"

    with open(f"{name}.v", 'w') as fp:
        fp.write(code)
    with open(f"{name}.ys", 'w') as fp:
        fp.write(f"read_verilog -sv {name}.v; write_rtlil {name}.rtl; test_autotb {name}_tb.v")
    subprocess.run(
        ["yosys", "-s", f"{name}.ys"]
    )
    subprocess.run(
        ["iverilog",
         f"{name}_tb.v",
         f"{name}.v",
         "-s",
         "testbench",
         "-o",
         f"{name}.vvp"
        ]
    )
    subprocess.run(
        ["vvp",
        f"{name}.vvp",
        f"+OUT={name}.txt",
        f"+VCD={name}.vcd"
        ]
    )

if __name__ == "__main__":


    # generate("AndBinOp", "&")
    # generate("OrBinOp", "|")
    # generate("XorBinOp", "^")
    # generate("XnorBinOp", "~^")

    # generate("ShlBinOp", "<<")
    # generate("SshlBinOp", "<<<")

    # generate("ShrBinOp", ">>")
    # generate("SshrBinOp", ">>>")

    # generate("LandBinOp", "&&")
    # generate("LorBinOp", "||")


    # generate("EqxBinOp", "===")
    # generate("NexBinOp", "!==")


    generate("LtBinOp", "<")
    generate("LeBinOp", "<=")
    generate("EqBinOp", "==")
    generate("NeBinOp", "!=")
    generate("GtBinOp", ">")
    generate("GeBinOp", ">=")

    # generate("AddBinOp", "+")
    # generate("SubBinOp", "-")





