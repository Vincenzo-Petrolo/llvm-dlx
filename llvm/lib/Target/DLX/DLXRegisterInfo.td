//===-- DLXRegisterInfo.td - DLX Register defs ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TRT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the RISC-W register file
//===----------------------------------------------------------------------===//

let Namespace = "DLX" in {
class DLXReg<bits<5> Enc, string n, list<string> alt = []> : Register<n> {
  let HWEncoding{4-0} = Enc;
  let AltNames = alt;
}
} // end Namespace

def R0  : DLXReg<0, "r0", ["zero"]>, DwarfRegNum<[0]>;
def R1  : DLXReg<1, "r1", ["t6"]>, DwarfRegNum<[1]>;
def R2  : DLXReg<2, "r2", ["sp"]>, DwarfRegNum<[2]>;
def R3  : DLXReg<3, "r3", ["gp"]>, DwarfRegNum<[3]>;
def R4  : DLXReg<4, "r4", ["tp"]>, DwarfRegNum<[4]>;
def R5  : DLXReg<5, "r5", ["t0"]>, DwarfRegNum<[5]>;
def R6  : DLXReg<6, "r6", ["t1"]>, DwarfRegNum<[6]>;
def R7  : DLXReg<7, "r7", ["t2"]>, DwarfRegNum<[7]>;
def R8  : DLXReg<8, "r8", ["s0", "fp"]>, DwarfRegNum<[8]>;
def R9  : DLXReg<9, "r9", ["s1"]>, DwarfRegNum<[9]>;
def R10 : DLXReg<10,"r10", ["a0"]>, DwarfRegNum<[10]>;
def R11 : DLXReg<11,"r11", ["a1"]>, DwarfRegNum<[11]>;
def R12 : DLXReg<12,"r12", ["a2"]>, DwarfRegNum<[12]>;
def R13 : DLXReg<13,"r13", ["a3"]>, DwarfRegNum<[13]>;
def R14 : DLXReg<14,"r14", ["a4"]>, DwarfRegNum<[14]>;
def R15 : DLXReg<15,"r15", ["a5"]>, DwarfRegNum<[15]>;
def R16 : DLXReg<16,"r16", ["a6"]>, DwarfRegNum<[16]>;
def R17 : DLXReg<17,"r17", ["a7"]>, DwarfRegNum<[17]>;
def R18 : DLXReg<18,"r18", ["s2"]>, DwarfRegNum<[18]>;
def R19 : DLXReg<19,"r19", ["s3"]>, DwarfRegNum<[19]>;
def R20 : DLXReg<20,"r20", ["s4"]>, DwarfRegNum<[20]>;
def R21 : DLXReg<21,"r21", ["s5"]>, DwarfRegNum<[21]>;
def R22 : DLXReg<22,"r22", ["s6"]>, DwarfRegNum<[22]>;
def R23 : DLXReg<23,"r23", ["s7"]>, DwarfRegNum<[23]>;
def R24 : DLXReg<24,"r24", ["s8"]>, DwarfRegNum<[24]>;
def R25 : DLXReg<25,"r25", ["s9"]>, DwarfRegNum<[25]>;
def R26 : DLXReg<26,"r26", ["s10"]>, DwarfRegNum<[26]>;
def R27 : DLXReg<27,"r27", ["s11"]>, DwarfRegNum<[27]>;
def R28 : DLXReg<28,"r28", ["t3"]>, DwarfRegNum<[28]>;
def R29 : DLXReg<29,"r29", ["t4"]>, DwarfRegNum<[29]>;
def R30 : DLXReg<30,"r30", ["t5"]>, DwarfRegNum<[30]>;
def R31 : DLXReg<31,"r31", ["ra"]>, DwarfRegNum<[31]>;

// The order of registers represents the preferred allocation sequence.
// Registers are listed in the order caller-save, callee-save, specials.
def GPR : RegisterClass<"DLX", [i32], 32, (add
    (sequence "R%u", 10, 17),
    (sequence "R%u", 5, 7),
    (sequence "R%u", 28, 31),
    (sequence "R%u", 8, 9),
    (sequence "R%u", 18, 27),
    (sequence "R%u", 0, 4)
  )>;

def SP : RegisterClass<"DLX", [i32], 32, (add R2)>;