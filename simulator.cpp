#include <bits/stdc++.h>
using namespace std;

namespace NAME {
const char REG_string[40][15] = {
    "REG_ZERO", "REG_RA", "REG_SP",     "REG_GP",  "REG_TP", "REG_T0",
    "REG_T1",   "REG_T2", "REG_S0",     "REG_S1",  "REG_A0", "REG_A1",
    "REG_A2",   "REG_A3", "REG_A4",     "REG_A5",  "REG_A6", "REG_A7",
    "REG_S2",   "REG_S3", "REG_S4",     "REG_S5",  "REG_S6", "REG_S7",
    "REG_S8",   "REG_S9", "REG_S10",    "REG_S11", "REG_T3", "REG_T4",
    "REG_T5",   "REG_T6", "UNKNOWN_REG"};
const char INST_string[40][15] = {
    "LUI",  "AUIPC", "JAL",  "JALR", "BEQ",   "BNE",         "BLT", "BGE",
    "BLTU", "BGEU",  "LB",   "LH",   "LW",    "LBU",         "LHU", "SB",
    "SH",   "SW",    "ADDI", "SLTI", "SLTIU", "XORI",        "ORI", "ANDI",
    "SLLI", "SRLI",  "SRAI", "ADD",  "SUB",   "SLL",         "SLT", "SLTU",
    "XOR",  "SRL",   "SRA",  "OR",   "AND",   "UNKNOWN_INST"};
}

class simulator {
private:
  const uint32_t bin_30_25 =
      (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 25);
  const uint32_t bin_11_8 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8);
  const uint32_t bin_11_7 =
      (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 7);
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
  /*
  struct Inst {
    INST opt;
    REG rs1, rs2, rd;
    int imm, operand;
    uint32_t funct3, funct7;
    Inst(INST _opt = UNKNOWN_INST, REG _rs1 = UNKNOWN_REG,
         REG _rs2 = UNKNOWN_REG, REG _rd = UNKNOWN_REG, int _operand = 0,
         int _imm = 0, uint32_t _funct3 = -1, uint32_t _funct7 = -1)
        : opt(_opt), rs1(_rs1), rs2(_rs2), rd(_rd), operand(_operand),
          imm(_imm), funct3(_funct3), funct7(_funct7) {
      ;
    }
  }*/

  struct FlowRegister {
    INST opt;
    uint32_t ir;
    REG rd;
    int32_t npc, rs1, rs2, operand, imm, ALU;
    FlowRegister() {
      opt = UNKNOWN_INST;
      ir = npc = operand = imm = ALU = rs1 = rs2 = 0;
      rd = UNKNOWN_REG;
    }
  } IF_ID, ID_EX, EX_MEM, MEM_WB;

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

  /*--------------------instruction fetch-----------------------*/
  void fetch() {
    memcpy(&IF_ID.ir, mem + pc, 4);
    IF_ID.npc = pc + 4;
    pc += 4;
  }

  /*--------------------instruction decode----------------------*/
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
  void decode() {
    ID_EX = IF_ID;
    uint32_t code = IF_ID.ir;
    uint32_t opcode = code & 0x7f, funct3 = (code >> 12) & 0x7,
             funct7 = code >> 25;
    int imm = 0, operand = 0;
    REG rs1 = UNKNOWN_REG, rs2 = UNKNOWN_REG, rd = UNKNOWN_REG;
    INST opt = UNKNOWN_INST;
    switch (opcode) {
    case 0x37:
      opt = LUI;
      rd = REG((code >> 7) & 0x1f);
      imm = (code >> 12) << 12;
      operand = operand_U(code);
      break;
    case 0x17:
      opt = AUIPC;
      rd = REG((code >> 7) & 0x1f);
      imm = (code >> 12) << 12;
      operand = operand_U(code);
      break;
    case 0x6F:
      opt = JAL;
      rd = REG((code >> 7) & 0x1f);
      imm = ((code >> 31) << 20) | (((code & bin_30_21) >> 21) << 1) |
            (((code & (1 << 20)) >> 20) << 11) |
            (((code & bin_19_12) >> 12) << 12);
      operand = operand_J(code);
      ID_EX.ALU = IF_ID.npc;
      pc += operand - 4;
      break;
    case 0x67:
      opt = JALR;
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      imm = (code >> 20);
      operand = operand_I(code);
      ID_EX.ALU = IF_ID.npc;

      pc = (reg[rs1] + operand) & (-2);
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
        if (reg[rs1] == reg[rs2])
          pc += operand - 4;
        break;
      case 0x1:
        opt = BNE;
        if (reg[rs1] != reg[rs2])
          pc += operand - 4;
        break;
      case 0x4:
        opt = BLT;
        if (reg[rs1] < reg[rs2])
          pc += operand - 4;
        break;
      case 0x5:
        opt = BGE;
        if (reg[rs1] >= reg[rs2])
          pc += operand - 4;
        break;
      case 0x6:
        opt = BLTU;
        if (uint32_t(reg[rs1]) < uint32_t(reg[rs2]))
          pc += operand - 4;
        break;
      case 0x7:
        opt = BGEU;
        if (uint32_t(reg[rs1]) >= uint32_t(reg[rs2]))
          pc += operand - 4;
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
        opt = SLLI;
        break;
      case 0x5:
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
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      rs2 = REG((code >> 20) & 0x1f);
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
    ID_EX.opt = opt;
    ID_EX.rs1 = reg[rs1];
    ID_EX.rs2 = reg[rs2];
    ID_EX.rd = rd;
    ID_EX.imm = imm;
    ID_EX.operand = operand;
  }

  /*--------------------------instruction execute----------------------------*/
    void LUI_exe() { EX_MEM.ALU = ID_EX.operand; }
  void AUIPC_exe() { EX_MEM.ALU = ID_EX.npc + ID_EX.operand - 4; }
  void JAL_exe() { ; }
  void JALR_exe() { ; }
  void BEQ_exe() { ; }
  void BNE_exe() { ; }
  void BLT_exe() { ; }
  void BGE_exe() { ; }
  void BLTU_exe() { ; }
  void BGEU_exe() { ; }
  void LB_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void LBU_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void LH_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void LHU_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void LW_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void SB_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void SH_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void SW_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void ADDI_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.operand; }
  void SLTI_exe() { EX_MEM.ALU = (ID_EX.rs1 < ID_EX.operand); }
  void SLTIU_exe() {
    EX_MEM.ALU = (uint32_t(ID_EX.rs1) < uint32_t(ID_EX.operand));
  }
  void XORI_exe() { EX_MEM.ALU = ID_EX.rs1 ^ ID_EX.operand; }
  void ORI_exe() { EX_MEM.ALU = ID_EX.rs1 | ID_EX.operand; }
  void ANDI_exe() { EX_MEM.ALU = ID_EX.rs1 & ID_EX.operand; }
  void SLLI_exe() { EX_MEM.ALU = ID_EX.rs1 << ID_EX.imm; }
  void SRLI_exe() {
    EX_MEM.ALU = uint32_t(ID_EX.rs1) >> uint32_t(ID_EX.imm);
  }
  void SRAI_exe() { EX_MEM.ALU = ID_EX.rs1 >> ID_EX.imm; }
  void ADD_exe() { EX_MEM.ALU = ID_EX.rs1 + ID_EX.rs2; }
  void SUB_exe() { EX_MEM.ALU = ID_EX.rs1 - ID_EX.rs2; }
  void SLL_exe() { EX_MEM.ALU = ID_EX.rs1 << (ID_EX.rs2 & 0x1f); }
  void SLT_exe() { EX_MEM.ALU = (ID_EX.rs1 < ID_EX.rs2); }
  void SLTU_exe() {
    EX_MEM.ALU = (uint32_t(ID_EX.rs1) < uint32_t(ID_EX.rs2));
  }
  void XOR_exe() { EX_MEM.ALU = ID_EX.rs1 ^ ID_EX.rs2; }
  void SRL_exe() {
    EX_MEM.ALU = uint32_t(ID_EX.rs1) >> uint32_t((ID_EX.rs2 & 0x1f));
  }
  void SRA_exe() { EX_MEM.ALU = ID_EX.rs1 >> (ID_EX.rs2 & 0x1f); }
  void OR_exe() { EX_MEM.ALU = ID_EX.rs1 | ID_EX.rs2; }
  void AND_exe() { EX_MEM.ALU = ID_EX.rs1 & ID_EX.rs2; }
  void (simulator::*func_exe[50])() = {
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
      &simulator::AND_exe};
  void execute() {
    EX_MEM = ID_EX;
    (this->*func_exe[ID_EX.opt])();
  }

  /*--------------------------instruction memoryaccess-----------------------*/
  void LB_mem() {
    int8_t LB_tmp;
    memcpy(&LB_tmp, mem + EX_MEM.ALU, 1);
    MEM_WB.ALU = LB_tmp;
  }
  void LBU_mem() {
    uint8_t LBU_tmp;
    memcpy(&LBU_tmp, mem + EX_MEM.ALU, 1);
    MEM_WB.ALU = LBU_tmp;
  }
  void LH_mem() {
    int16_t LH_tmp;
    memcpy(&LH_tmp, mem + EX_MEM.ALU, 2);
    MEM_WB.ALU = LH_tmp;
  }
  void LHU_mem() {
    uint16_t LHU_tmp;
    memcpy(&LHU_tmp, mem + EX_MEM.ALU, 2);
    MEM_WB.ALU = LHU_tmp;
  }
  void LW_mem() {
    int LW_tmp;
    memcpy(&LW_tmp, mem + EX_MEM.ALU, 4);
    MEM_WB.ALU = LW_tmp;
  }
  void SB_mem() {
    uint8_t SB_tmp = EX_MEM.rs2;
    memcpy(mem + EX_MEM.ALU, &SB_tmp, 1);
  }
  void SH_mem() {
    uint16_t SH_tmp = EX_MEM.rs2;
    memcpy(mem + EX_MEM.ALU, &SH_tmp, 2);
  }
  void SW_mem() {
    int SW_tmp = EX_MEM.rs2;
    memcpy(mem + EX_MEM.ALU, &SW_tmp, 4);
  }
  void memoryAccess() {
    MEM_WB = EX_MEM;
    switch (EX_MEM.opt) {
    case LB:
      LB_mem();
      break;
    case LBU:
      LBU_mem();
      break;
    case LH:
      LH_mem();
      break;
    case LHU:
      LHU_mem();
      break;
    case LW:
      LW_mem();
      break;
    case SB:
      SB_mem();
      break;
    case SH:
      SH_mem();
      break;
    case SW:
      SW_mem();
      break;
    }
  }

  /*---------------------------instruction writeback-------------------------*/
  void writeBack() {
    if (EX_MEM.rd != UNKNOWN_REG)
      reg[EX_MEM.rd] = MEM_WB.ALU;
  }
  /*
  void print(Inst inst) {
    printf("%s rs1:%s rs2:%s rd:%s imm:%x rs1:%x rs2:%x\n",
           NAME::INST_string[inst.opt], NAME::REG_string[inst.rs1],
           NAME::REG_string[inst.rs2], NAME::REG_string[inst.rd], inst.operand,
           reg[inst.rs1], reg[inst.rs2]);
  }
*/
  void solve() {
    read();
    pc = 0;
    while (true) {
      //printf("pc : %x\n", pc);
      fetch();
      if (IF_ID.ir == 0x00c68223)
        break;
     // printf("fetch: %x\n", IF_ID.ir);
      decode();
    //  printf("decode: %x %s %x %x %x %x %d\n", ID_EX.ir, NAME::INST_string[ID_EX.opt], ID_EX.operand, ID_EX.imm, ID_EX.rs1, ID_EX.rs2, ID_EX.rd);
      execute();
   //   printf("exe: %x %x\n", ID_EX.ir, ID_EX.ALU);
      memoryAccess();
    //  printf("mem: %x %x\n", ID_EX.ir, ID_EX.ALU);
      writeBack();
     // printf("wb: %s\n", NAME::REG_string[ID_EX.rd]);
      reg[REG_ZERO] = 0;
    ///	for(int i = 0; i < 32; ++i) printf("%x\n", reg[i]);
    }
    printf("%d\n", reg[REG_A0] & 0xff);
  }
};

int main() {
  simulator Simulator;
  Simulator.solve();
  return 0;
}
