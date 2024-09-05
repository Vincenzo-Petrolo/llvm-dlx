static inline unsigned getOR1KRegisterNumbering(unsigned Reg) {
  switch(Reg) {
    case DLX::R0  : return 0;
    case DLX::R1  : return 1;
    case DLX::R2  : return 2;
    case DLX::R3  : return 3;
    case DLX::R4  : return 4;
    case DLX::R5  : return 5;
    case DLX::R6  : return 6;
    case DLX::R7  : return 7;
    case DLX::R8  : return 8;
    case DLX::R9  : return 9;
    case DLX::R10 : return 10;
    case DLX::R11 : return 11;
    case DLX::R12 : return 12;
    case DLX::R13 : return 13;
    case DLX::R14 : return 14;
    case DLX::R15 : return 15;
    case DLX::R16 : return 16;
    case DLX::R17 : return 17;
    case DLX::R18 : return 18;
    case DLX::R19 : return 19;
    case DLX::R20 : return 20;
    case DLX::R21 : return 21;
    case DLX::R22 : return 22;
    case DLX::R23 : return 23;
    case DLX::R24 : return 24;
    case DLX::R25 : return 25;
    case DLX::R26 : return 26;
    case DLX::R27 : return 27;
    case DLX::R28 : return 28;
    case DLX::R29 : return 29;
    case DLX::R30 : return 30;
    case DLX::R31 : return 31;
    default: llvm_unreachable("Unknown register number");
  }
}

