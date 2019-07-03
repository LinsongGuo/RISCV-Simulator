#include <bits/stdc++.h>
using namespace std;

namespace NAME{
	const char REG_string[40][15] = {
      "REG_ZERO", "REG_RA", "REG_SP",     "REG_GP",  "REG_TP", "REG_T0",
      "REG_T1",   "REG_T2", "REG_S0",     "REG_S1",  "REG_A0", "REG_A1",
      "REG_A2",   "REG_A3", "REG_A4",     "REG_A5",  "REG_A6", "REG_A7",
      "REG_S2",   "REG_S3", "REG_S4",     "REG_S5",  "REG_S6", "REG_S7",
      "REG_S8",   "REG_S9", "REG_S10",    "REG_S11", "REG_T3", "REG_T4",
      "REG_T5",   "REG_T6", "UNKNOWN_REG"
  };
  const char INST_string[40][15] = {
      "LUI",  "AUIPC", "JAL",  "JALR", "BEQ",   "BNE",         "BLT", "BGE",
      "BLTU", "BGEU",  "LB",   "LH",   "LW",    "LBU",         "LHU", "SB",
      "SH",   "SW",    "ADDI", "SLTI", "SLTIU", "XORI",        "ORI", "ANDI",
      "SLLI", "SRLI",  "SRAI", "ADD",  "SUB",   "SLL",         "SLT", "SLTU",
      "XOR",  "SRL",   "SRA",  "OR",   "AND",   "UNKNOWN_INST"
  };
}

class simulator {
private:
  const uint32_t bin_30_25 =
      (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 25);
  const uint32_t bin_11_8 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8);
  const uint32_t bin_11_7 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 7);
  const uint32_t bin_19_12 = (1 << 19) | (1 << 18) | (1 << 17) | (1 << 16) |
                             (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12);
  const uint32_t bin_30_21 = (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) |
                             (1 << 26) | (1 << 25) | (1 << 24) | (1 << 23) |
                             (1 << 22) | (1 << 21);
  const uint32_t bin_24_20 =
      (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21) | (1 << 20);
  const uint32_t bin_31_11 = (-1) ^ ((1 << 11) - 1);
  const uint32_t bin_31_12 = (-1) ^ ((1 << 12) - 1);
  const uint32_t bin_31_20 = (-1) ^ ((1 << 20) - 1);
  const uint32_t bin_31_25 = (-1) ^ ((1 << 25) - 1);
  const uint32_t bin_24_21 = (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21);

  int reg[32], pc;
  uint8_t mem[0x200ff];
  char buffer[10];
  enum REG {
    REG_ZERO,
    REG_RA,
    REG_SP,
    REG_GP,
    REG_TP,
    REG_T0,
    REG_T1,
    REG_T2,
    REG_S0,
    REG_S1,
    REG_A0,
    REG_A1,
    REG_A2,
    REG_A3,
    REG_A4,
    REG_A5,
    REG_A6,
    REG_A7,
    REG_S2,
    REG_S3,
    REG_S4,
    REG_S5,
    REG_S6,
    REG_S7,
    REG_S8,
    REG_S9,
    REG_S10,
    REG_S11,
    REG_T3,
    REG_T4,
    REG_T5,
    REG_T6,
    UNKNOWN_REG
  };
  enum INST {
    LUI,
    AUIPC,
    JAL,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    SB,
    SH,
    SW,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    UNKNOWN_INST
  };
  struct Inst {
    INST opt;
    REG rs1, rs2, rd;
    int operand, imm;
    uint32_t funct3, funct7;
    Inst(INST _opt = UNKNOWN_INST, REG _rs1 = UNKNOWN_REG,
         REG _rs2 = UNKNOWN_REG, REG _rd = UNKNOWN_REG, int _operand = 0,
         int _imm = 0, uint32_t _funct3 = -1, uint32_t _funct7 = -1)
        : opt(_opt), rs1(_rs1), rs2(_rs2), rd(_rd), operand(_operand),
          imm(_imm), funct3(_funct3), funct7(_funct7) {
      ;
    }
  };

public:
  simulator() { pc = 0x00000000; }
  void read() {
    int p = 0;
    while (scanf("%s", buffer) != EOF) {
      if (buffer[0] == '@') {
        sscanf(buffer + 1, "%x", &p);
      } else {
        uint32_t m0, m1, m2, m3;
        sscanf(buffer, "%x", &m0);
        scanf("%x %x %x", &m1, &m2, &m3);
        unsigned int tmp = (m3 << 24) | (m2 << 16) | (m1 << 8) | m0;
        memcpy(mem + p, &tmp, sizeof(tmp));
        p += 4;
      }
    }
  }
  void fetch(uint32_t &code) { memcpy(&code, mem + pc, 4); }
  int operand_I(const uint32_t &code) {
    int res = code >> 20;
    if (code >> 31) {
      res |= bin_31_11;
    }
    return res;
  }
  int operand_S(const uint32_t &code) {
    int res = (code >> 7) & 1;
    res |= (((code & bin_11_8) >> 8) << 1);
    res |= (((code & bin_30_25) >> 25) << 5);
    if (code >> 31)
      res |= bin_31_11;
    return res;
  }
  int operand_B(const uint32_t &code) {
    int res = (((code & bin_11_8) >> 8) << 1);
    res |= (((code & bin_30_25) >> 25) << 5);
    res |= (((code >> 7) & 1U) << 11);
    if (code >> 31)
      res |= bin_31_12;
    return res;
  }
  int operand_U(const uint32_t &code) { return code & bin_31_12; }
  int operand_J(const uint32_t &code) {
    int res = (((code & bin_24_21) >> 21) << 1);
    res |= (((code & bin_30_25) >> 25) << 5);
    res |= (((code >> 20) & 1) << 11);
    res |= (code & bin_19_12);
    if (code >> 31)
      res |= bin_31_20;
    return res;
  }
  Inst decode(const uint32_t &code) {
    uint32_t opcode = code & 0x7f, funct3 = (code >> 12) & 0x7, funct7 = -1;
    int imm = 0, operand = 0;
    REG rs1 = UNKNOWN_REG, rs2 = UNKNOWN_REG, rd = UNKNOWN_REG;
    INST opt = UNKNOWN_INST;
    switch (opcode) {
    case 0x37:
      opt = LUI;
      rd = REG((code >> 7) & 0x1f);
      imm = (code >> 12) << 12;
      funct3 = -1;
      operand = operand_U(code);
      break;
    case 0x17:
      opt = AUIPC;
      rd = REG((code >> 7) & 0x1f);
      imm = (code >> 12) << 12;
      funct3 = -1;
      operand = operand_U(code);
      break;
    case 0x6F:
      opt = JAL;
      rd = REG((code >> 7) & 0x1f);
      imm = ((code >> 31) << 20) | (((code & bin_30_21) >> 21) << 1) |
            (((code & (1 << 20)) >> 20) << 11) |
            (((code & bin_19_12) >> 12) << 12);
      funct3 = -1;
      operand = operand_J(code);
      break;
    case 0x67:
      opt = JALR;
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      imm = (code >> 20);
      funct3 = -1;
      operand = operand_I(code);
      break;
    case 0x63:
      rs1 = REG((code >> 15) & 0x1f);
      rs2 = REG((code >> 20) & 0x1f);
      imm = ((code >> 31) << 12) | (((code & bin_30_25) >> 25) << 5) |
            (((code & bin_11_8) >> 8) << 1) | (((code >> 7) & 1) << 11);
      operand = operand_B(code);
      switch (funct3) {
      case 0x0:
        opt = BEQ;
        break;
      case 0x1:
        opt = BNE;
        break;
      case 0x4:
        opt = BLT;
        break;
      case 0x5:
        opt = BGE;
        break;
      case 0x6:
        opt = BLTU;
        break;
      case 0x7:
        opt = BGEU;
        break;
      }
      break;
    case 0x3:
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      imm = (code >> 20);
      operand = operand_I(code);
      switch (funct3) {
      case 0x0:
        opt = LB;
        break;
      case 0x1:
        opt = LH;
        break;
      case 0x2:
        opt = LW;
        break;
      case 0x4:
        opt = LBU;
        break;
      case 0x5:
        opt = LHU;
        break;
      }
      break;
    case 0x23:
      rs1 = REG((code >> 15) & 0x1f);
      rs2 = REG((code >> 20) & 0x1f);
      imm = (((code & bin_31_25) >> 25) << 5) | ((code & bin_11_7) >> 7);
      operand = operand_S(code);
      switch (funct3) {
      case 0x0:
        opt = SB;
        break;
      case 0x1:
        opt = SH;
        break;
      case 0x2:
        opt = SW;
        break;
      }
      break;
    case 0x13:
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      operand = operand_I(code);
      switch (funct3) {
      case 0x0:
        imm = code >> 20;
        opt = ADDI;
        break;
      case 0x2:
        imm = code >> 20;
        opt = SLTI;
        break;
      case 0x3:
        imm = code >> 20;
        opt = SLTIU;
        break;
      case 0x4:
        imm = code >> 20;
        opt = XORI;
        break;
      case 0x6:
        imm = code >> 20;
        opt = ORI;
        break;
      case 0x7:
        imm = code >> 20;
        opt = ANDI;
        break;
      case 0x1:
        imm = (code & bin_24_20) >> 20;
        funct7 = code >> 25;
        opt = SLLI;
        break;
      case 0x5:
      	funct7 = code >> 25;
        switch (funct7) {
        case 0x0:
          imm = (code & bin_24_20) >> 20;
          opt = SRLI;
          break;
        case 0x20:
          imm = (code & bin_24_20) >> 20;
          opt = SRAI;
          break;
        }
        break;
      }
      break;
    case 0x33:      
      //printf("%x  %x  case 0x33\n", code, funct3);
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      rs2 = REG((code >> 20) & 0x1f);
      funct7 = code >> 25;
      switch (funct3) {
      case 0x0:
        switch (funct7) {
        case 0x0:
          opt = ADD;
          break;
        case 0x20:
          opt = SUB;
          break;
        }
        break;
      case 0x1:
        opt = SLL;
        break;
      case 0x2:
        opt = SLT;
        break;
      case 0x3:
        opt = SLTU;
        break;
      case 0x4:
        opt = XOR;
        break;
      case 0x5:
        switch (funct7) {
        case 0x0:
          opt = SRL;
          break;
        case 0x20:
          opt = SRA;
          break;
        }
        break;
      case 0x6:
        opt = OR;
        break;
      case 0x7:
        opt = AND;
        break;
      }
      break;
    }
    return Inst(opt, rs1, rs2, rd, operand, imm, funct3, funct7);
  }

  void LUI_exe(const Inst &inst) {
    reg[inst.rd] = inst.operand;
    pc += 4;
  }
  void AUIPC_exe(const Inst &inst) {
    reg[inst.rd] = pc + inst.operand;
    pc += 4;
  }
  void JAL_exe(const Inst &inst) {
    reg[inst.rd] = pc + 4;
    pc += inst.operand;
  }
  void JALR_exe(const Inst &inst) {
    reg[inst.rd] = pc + 4;
    pc = (reg[inst.rs1] + inst.operand) & (-2);
  }
  void BEQ_exe(const Inst &inst) {
    if (reg[inst.rs1] == reg[inst.rs2]) {
      pc += inst.operand;
    } else
      pc += 4;
  }
  void BNE_exe(const Inst &inst) {
    if (reg[inst.rs1] != reg[inst.rs2]) {
      pc += inst.operand;
    } else
      pc += 4;
  }
  void BLT_exe(const Inst &inst) {
    if (reg[inst.rs1] < reg[inst.rs2]) {
      pc += inst.operand;
    } else
      pc += 4;
  }
  void BGE_exe(const Inst &inst) {
    if (reg[inst.rs1] >= reg[inst.rs2]) {
      pc += inst.operand;
    } else
      pc += 4;
  }
  void BLTU_exe(const Inst &inst) {
    if (uint32_t(reg[inst.rs1] < uint32_t(reg[inst.rs2]))) {
      pc += inst.operand;
    } else
      pc += 4;
  }
  void BGEU_exe(const Inst &inst) {
    if (uint32_t(reg[inst.rs1]) >= uint32_t(reg[inst.rs2])) {
      pc += inst.operand;
    } else
      pc += 4;
  }

  void LB_exe(const Inst &inst) {
    int8_t tmp;
    memcpy(&tmp, mem + reg[inst.rs1] + inst.operand, 1);
    reg[inst.rd] = tmp;
    pc += 4;
  }
  void LBU_exe(const Inst &inst) {
    uint8_t tmp;
    memcpy(&tmp, mem + reg[inst.rs1] + inst.operand, 1);
    reg[inst.rd] = tmp;
    pc += 4;
  }
  void LH_exe(const Inst &inst) {
    int16_t tmp;
    memcpy(&tmp, mem + reg[inst.rs1] + inst.operand, 2);
    reg[inst.rd] = tmp;
    pc += 4;
  }
  void LHU_exe(const Inst &inst) {
    uint16_t tmp;
    memcpy(&tmp, mem + reg[inst.rs1] + inst.operand, 2);
    reg[inst.rd] = tmp;
    pc += 4;
  }
  void LW_exe(const Inst &inst) {
    memcpy(&reg[inst.rd], mem + reg[inst.rs1] + inst.operand, 4);
    pc += 4;
  }
  void SB_exe(const Inst &inst) {
    uint8_t tmp = reg[inst.rs2];
    //printf("SB_exe %d %d %x %x %x %x\n", inst.rs1, inst.rs2, tmp, reg[inst.rs1], inst.operand, reg[inst.rs1] + inst.operand);
    memcpy(mem + reg[inst.rs1] + inst.operand, &tmp, 1);
    pc += 4;
  }
  void SH_exe(const Inst &inst) {
    uint16_t tmp = reg[inst.rs2];
    memcpy(mem + reg[inst.rs1] + inst.operand, &tmp, 2);
    pc += 4;
  }
  void SW_exe(const Inst &inst) {
  	memcpy(mem + reg[inst.rs1] + inst.operand, &reg[inst.rs2], 4);
    pc += 4;
  }
  void ADDI_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] + inst.operand;
    pc += 4;
  }
  void SLTI_exe(const Inst &inst) {
    reg[inst.rd] = (reg[inst.rs1] < inst.operand);
    pc += 4;
  }
  void SLTIU_exe(const Inst &inst) {
    reg[inst.rd] = (uint32_t(reg[inst.rs1]) < uint32_t(inst.operand));
    pc += 4;
  }
  void XORI_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] ^ inst.operand;
    pc += 4;
  }
  void ORI_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] | inst.operand;
    pc += 4;
  }
  void ANDI_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] & inst.operand;
    pc += 4;
  }
  void SLLI_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] << inst.imm;
    pc += 4;
  }
  void SRLI_exe(const Inst &inst) {
    reg[inst.rd] = uint32_t(reg[inst.rs1]) >> uint32_t(inst.imm);
    pc += 4;
  }
  void SRAI_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] >> inst.imm;
    pc += 4;
  }
  void ADD_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] + reg[inst.rs2];
    pc += 4;
  }
  void SUB_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] - reg[inst.rs2];
    pc += 4;
  }
  void SLL_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] << (reg[inst.rs2] & 0x1f);
    pc += 4;
  }
  void SLT_exe(const Inst &inst) {
    reg[inst.rd] = (reg[inst.rs1] < reg[inst.rs2]);
    pc += 4;
  }
  void SLTU_exe(const Inst &inst) {
    reg[inst.rd] = (uint32_t(reg[inst.rs1]) < uint32_t(reg[inst.rs2]));
    pc += 4;
  }
  void XOR_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] ^ reg[inst.rs2];
    pc += 4;
  }
  void SRL_exe(const Inst &inst) {
    reg[inst.rd] = uint32_t(reg[inst.rs1]) >> uint32_t((reg[inst.rs2] & 0x1f));
    pc += 4;
  }
  void SRA_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] >> (reg[inst.rs2] & 0x1f);
    pc += 4;
  }
  void OR_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] | reg[inst.rs2];
    pc += 4;
  }
  void AND_exe(const Inst &inst) {
    reg[inst.rd] = reg[inst.rs1] & reg[inst.rs2];
    pc += 4;
  }
  void (simulator::*func[50])(const Inst &inst) = {
      &simulator::LUI_exe,  &simulator::AUIPC_exe, &simulator::JAL_exe,
      &simulator::JALR_exe, &simulator::BEQ_exe,   &simulator::BNE_exe,
      &simulator::BLT_exe,  &simulator::BGE_exe,   &simulator::BLTU_exe,
      &simulator::BGEU_exe, &simulator::LB_exe,    &simulator::LH_exe,
      &simulator::LW_exe,   &simulator::LBU_exe,   &simulator::LHU_exe,
      &simulator::SB_exe,   &simulator::SH_exe,    &simulator::SW_exe,
      &simulator::ADDI_exe, &simulator::SLTI_exe,  &simulator::SLTIU_exe,
      &simulator::XORI_exe, &simulator::ORI_exe,   &simulator::ANDI_exe,
      &simulator::SLLI_exe, &simulator::SRLI_exe,  &simulator::SRAI_exe,
      &simulator::ADD_exe,  &simulator::SUB_exe,   &simulator::SLL_exe,
      &simulator::SLT_exe,  &simulator::SLTU_exe,  &simulator::XOR_exe,
      &simulator::SRL_exe,  &simulator::SRA_exe,   &simulator::OR_exe,
      &simulator::AND_exe
  };
  void print(Inst inst) {
  	  printf("inst: %s %s %s %s %x %d \n", NAME::INST_string[inst.opt],
           NAME::REG_string[inst.rs1], NAME::REG_string[inst.rs2], NAME::REG_string[inst.rd],
           inst.operand, inst.operand);
  	  cout << bitset<32>(inst.operand) <<' '<<bitset<8>(inst.funct3)<<' '<< bitset<8>(inst.funct7)<<endl;  
  }

  void execute(const Inst &inst) { 
  	(this->*func[inst.opt])(inst); 
  }
  
  
  void solve() {
    read();
    uint32_t code = 0;
    while (true) {
      fetch(code);
      if (code == 0x00c68223) break;
      Inst inst = decode(code);   
      execute(inst);
      reg[REG_ZERO] = 0;
    }
    printf("%d\n", reg[REG_A0] & 0xff);
  }
};

int main() {
  simulator Simulator;
  Simulator.solve();
  return 0;
}
