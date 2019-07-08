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
const char STATE[5][10] = {"empty", "finish", "unfinish"};
} // namespace NAME

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
  enum State { empty, finish, unfinish };
  int reg[32], pc, now, next, branch_skip, ex_rs1_val, ex_rs2_val, mem_rs2_val;
  bool pauseIF, pauseID, pauseEX, pauseMEM;
  uint8_t mem[0x200ff], history_table[8];
  char buffer[10];

  struct pipeline {
    INST opt;
    uint32_t code;
    REG rd, rs1, rs2;
    int32_t npc, operand, imm, ALU;
    State state;
    pipeline() {
      opt = UNKNOWN_INST;
      code = npc = operand = imm = ALU = 0;
      rd = rs1 = rs2 = UNKNOWN_REG;
      state = empty;
    }
  } IF[2], ID[2], EX[2], MEM[2], WB[2];

public:
  /*--------------------------construct simulator------------------------*/
  simulator() {
    pc = 0;
    branch_skip = 0xffffffff;
    now = 0, next = 1;
    pauseIF = pauseID = pauseEX = pauseMEM = false;
    for (int i = 0; i < 32; ++i)
      reg[i] = 0;
    for (int i = 0; i < 8; ++i)
      history_table[i] = 0;
  }

  /*-------------------------read instruction-----------------------------*/
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

  /*-----------------------branch prediction----------------------------*/
  bool get_prediction(const uint32_t &code) {
    return (history_table[(code >> 12) & 7] >> 1) & 1;
  }
  void update_prediction(const uint32_t &code, const bool &jump) {
    uint8_t &tmp = history_table[(code >> 12) & 7];
    tmp &= 3;
    if (tmp == 3)
      tmp = jump ? 3 : 2;
    else if (tmp == 2)
      tmp = jump ? 3 : 0;
    else if (tmp == 1)
      tmp = jump ? 3 : 0;
    else
      tmp = jump ? 1 : 0;
  }

  /*----------------------immediate operand------------------------------*/
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

  /*-------------------------instruction fetch-----------------------------*/
  void fetch() {
    uint32_t code;
    memcpy(&code, mem + pc, 4);
    if (code == 0x00c68223)
      IF[now].state = empty;
    else {
      IF[now].state = finish;
      IF[now].code = code;
      IF[now].npc = pc + 4;
      if ((code & 0x7f) == 0x63) {
        if (get_prediction(code))
          pc += operand_B(code);
        else
          pc += 4;
      } else if ((code & 0x7f) == 0x6f) {
        pc += operand_J(code);
      } else
        pc += 4;
    }
  }

  /*-------------------------instruction decode------------------------------*/
  void decode() {
    uint32_t code = ID[now].code;
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
      break;
    case 0x67:
      opt = JALR;
      rd = REG((code >> 7) & 0x1f);
      rs1 = REG((code >> 15) & 0x1f);
      imm = (code >> 20);
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
    ID[now].opt = opt;
    ID[now].rs1 = rs1;
    ID[now].rs2 = rs2;
    ID[now].rd = rd;
    ID[now].imm = imm;
    ID[now].operand = operand;
    ID[now].state = finish;
    //  printf("decode: %s %s %s %s %x\n", NAME::INST_string[opt],
    //  NAME::REG_string[rs1],
    //      NAME::REG_string[rs2], NAME::REG_string[rd], operand);
  }

  /*--------------------------instruction execute----------------------------*/
  void LUI_exe() { EX[now].ALU = EX[now].operand; }
  void AUIPC_exe() { EX[now].ALU = EX[now].npc + EX[now].operand - 4; }
  void JAL_exe() { EX[now].ALU = EX[now].npc; }
  void JALR_exe() {
    EX[now].ALU = EX[now].npc;
    branch_skip = (ex_rs1_val + EX[now].operand) & (-2);
  }
  void BEQ_exe() {
    bool jump = (ex_rs1_val == ex_rs2_val);
    if (get_prediction(EX[now].code)) {
      if (!jump)
        branch_skip = EX[now].npc;
    } else {
      if (jump)
        branch_skip = EX[now].npc + EX[now].operand - 4;
    }
    update_prediction(EX[now].code, jump);
  }
  void BNE_exe() {
    bool jump = (ex_rs1_val != ex_rs2_val);

    if (get_prediction(EX[now].code)) {
      if (!jump)
        branch_skip = EX[now].npc;
    } else {
      if (jump)
        branch_skip = EX[now].npc + EX[now].operand - 4;
    }
    update_prediction(EX[now].code, jump);
  }
  void BLT_exe() {
    bool jump = (ex_rs1_val < ex_rs2_val);
    if (get_prediction(EX[now].code)) {
      if (!jump)
        branch_skip = EX[now].npc;
    } else {
      if (jump)
        branch_skip = EX[now].npc + EX[now].operand - 4;
    }
    update_prediction(EX[now].code, jump);
  }
  void BGE_exe() {
    bool jump = (ex_rs1_val >= ex_rs2_val);
    if (get_prediction(EX[now].code)) {
      if (!jump)
        branch_skip = EX[now].npc;
    } else {
      if (jump)
        branch_skip = EX[now].npc + EX[now].operand - 4;
    }
    update_prediction(EX[now].code, jump);
  }
  void BLTU_exe() {
    bool jump = (uint32_t(ex_rs1_val) < uint32_t(ex_rs2_val));
    if (get_prediction(EX[now].code)) {
      if (!jump)
        branch_skip = EX[now].npc;
    } else {
      if (jump)
        branch_skip = EX[now].npc + EX[now].operand - 4;
    }
    update_prediction(EX[now].code, jump);
  }
  void BGEU_exe() {
    bool jump = (uint32_t(ex_rs1_val) >= uint32_t(ex_rs2_val));
    if (get_prediction(EX[now].code)) {
      if (!jump)
        branch_skip = EX[now].npc;
    } else {
      if (jump)
        branch_skip = EX[now].npc + EX[now].operand - 4;
    }
    update_prediction(EX[now].code, jump);
  }
  void LB_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void LBU_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void LH_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void LHU_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void LW_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void SB_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void SH_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void SW_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void ADDI_exe() { EX[now].ALU = ex_rs1_val + EX[now].operand; }
  void SLTI_exe() { EX[now].ALU = ex_rs1_val < EX[now].operand; }
  void SLTIU_exe() {
    EX[now].ALU = uint32_t(ex_rs1_val) < uint32_t(EX[now].operand);
  }
  void XORI_exe() { EX[now].ALU = ex_rs1_val ^ EX[now].operand; }
  void ORI_exe() { EX[now].ALU = ex_rs1_val | EX[now].operand; }
  void ANDI_exe() { EX[now].ALU = ex_rs1_val & EX[now].operand; }
  void SLLI_exe() { EX[now].ALU = ex_rs1_val << EX[now].imm; }
  void SRLI_exe() {
    EX[now].ALU = uint32_t(ex_rs1_val) >> uint32_t(EX[now].imm);
  }
  void SRAI_exe() { EX[now].ALU = ex_rs1_val >> EX[now].imm; }
  void ADD_exe() { EX[now].ALU = ex_rs1_val + ex_rs2_val; }
  void SUB_exe() { EX[now].ALU = ex_rs1_val - ex_rs2_val; }
  void SLL_exe() { EX[now].ALU = ex_rs1_val << (ex_rs2_val & 0x1f); }
  void SLT_exe() { EX[now].ALU = ex_rs1_val < ex_rs2_val; }
  void SLTU_exe() { EX[now].ALU = uint32_t(ex_rs1_val) < uint32_t(ex_rs2_val); }
  void XOR_exe() { EX[now].ALU = ex_rs1_val ^ ex_rs2_val; }
  void SRL_exe() {
    EX[now].ALU = uint32_t(ex_rs1_val) >> uint32_t((ex_rs2_val & 0x1f));
  }
  void SRA_exe() { EX[now].ALU = ex_rs1_val >> (ex_rs2_val & 0x1f); }
  void OR_exe() { EX[now].ALU = ex_rs1_val | ex_rs2_val; }
  void AND_exe() { EX[now].ALU = ex_rs1_val & ex_rs2_val; }
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
    static int exe_cnt = 0;
    (this->*func_exe[EX[now].opt])();
    //   printf("%s %d %d %d %d %x %x %x %x\n",
    //         NAME::INST_string[EX[now].opt],  ++exe_cnt,EX[now].rs1,
    //         EX[now].rs2,
    //  EX[now].rd, ex_rs1_val, ex_rs2_val, EX[now].operand, EX[now].ALU);
    //  printf("%s %d %x %x %x %x\n",
    //          NAME::INST_string[EX[now].opt],  ++exe_cnt, EX[now].npc - 4,
    //          EX[now].code, ex_rs1_val, EX[now].operand);
    EX[now].state = finish;
  }

  /*--------------------------instruction memoryaccess-----------------------*/
  void LB_mem() {
    int8_t tmp;
    memcpy(&tmp, mem + MEM[now].ALU, 1);
    MEM[now].ALU = tmp;
  }
  void LBU_mem() {
    uint8_t tmp;
    memcpy(&tmp, mem + MEM[now].ALU, 1);
    MEM[now].ALU = tmp;
  }
  void LH_mem() {
    int16_t tmp;
    memcpy(&tmp, mem + MEM[now].ALU, 2);
    MEM[now].ALU = tmp;
  }
  void LHU_mem() {
    uint16_t tmp;
    memcpy(&tmp, mem + MEM[now].ALU, 2);
    MEM[now].ALU = tmp;
  }
  void LW_mem() {
    int tmp;
    memcpy(&tmp, mem + MEM[now].ALU, 4);
    MEM[now].ALU = tmp;
  }
  void SB_mem() { memcpy(mem + MEM[now].ALU, &mem_rs2_val, 1); }
  void SH_mem() { memcpy(mem + MEM[now].ALU, &mem_rs2_val, 2); }
  void SW_mem() { memcpy(mem + MEM[now].ALU, &mem_rs2_val, 4); }
  void memoryAccess() {
    static int time_clock = 0;
    ++time_clock;
    if (time_clock % 3) {
      MEM[now].state = unfinish;
      return;
    }
    // printf("memoryaccess: %s\n", NAME::INST_string[MEM[now].opt]);
    switch (MEM[now].opt) {
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
    MEM[now].state = finish;
  }

  /*---------------------------instruction writeback-------------------------*/
  void writeBack() {
    if (WB[now].rd != UNKNOWN_REG) {
      reg[WB[now].rd] = WB[now].ALU;
      WB[now].state = finish;
      // printf("WB:%s %s %x\n",
      // NAME::INST_string[WB[now].opt],NAME::REG_string[WB[now].rd],
      // WB[now].ALU);
    }
  }
  void next_loop() {
    pauseIF = pauseID = pauseEX = pauseMEM = false;
    /*-----------------------------WB[next]----------------------------*/
    if (MEM[now].state == finish) {
      WB[next] = MEM[now];
      WB[next].state = unfinish;
    } else
      WB[next].state = empty;

    /*-----------------------------MEM[next]----------------------------*/
    if (MEM[now].state == unfinish) {
      MEM[next] = MEM[now];
      if (MEM[next].opt == SB || MEM[next].opt == SH || MEM[next].opt == SW)
        mem_rs2_val = reg[MEM[next].rs2];
      if (EX[now].state == finish) {
        if (branch_skip != 0xffffffff) {
          EX[next] = EX[now];
          ID[next].state = empty;
          IF[next].state = unfinish;
          pc = branch_skip;
          pauseEX = pauseID = true;
        } else {
          EX[next] = EX[now];
          pauseEX = true;
          if (ID[now].state != finish && IF[now].state == finish) {
            ID[next] = IF[now];
            IF[next].state = unfinish;
          } else
            pauseID = pauseIF = true;
        }
        return;
      }
    } else if (EX[now].state == finish) {
      MEM[next] = EX[now];
      MEM[next].state = unfinish;
      if (MEM[next].opt == SB || MEM[next].opt == SH || MEM[next].opt == SW) {
        if (MEM[now].state != empty && MEM[now].rd == MEM[next].rs2)
          mem_rs2_val = MEM[now].ALU;
        else
          mem_rs2_val = reg[MEM[next].rs2];
      }
    } else {
      MEM[next].state = empty;
    }

    /*-----------------------------branch skip----------------------------*/
    if (branch_skip != 0xffffffff) {
      EX[next].state = empty;
      ID[next].state = empty;
      IF[next].state = unfinish;
      pc = branch_skip;
    } else {
      /*-----------------------------EX[next]----------------------------*/
      if (EX[now].state == unfinish) {
        EX[next] = EX[now];
        if (EX[next].rs1 != UNKNOWN_REG) {
          ex_rs1_val = reg[EX[next].rs1];
          if (MEM[now].state != empty && MEM[now].rd == EX[next].rs1) {
            if (MEM[now].state == finish) {
              ex_rs1_val = MEM[now].ALU;
            } else
              pauseEX = pauseID = pauseIF = true;
          }
        }
        if (EX[next].rs2 != UNKNOWN_REG) {
          ex_rs2_val = reg[EX[next].rs2];
          if (MEM[now].state != empty && MEM[now].rd == EX[next].rs2) {
            if (MEM[now].state == finish)
              ex_rs2_val = MEM[now].ALU;
            else
              pauseEX = pauseID = pauseIF = true;
          }
        }
      } else if (ID[now].state == finish) {
        EX[next] = ID[now];
        EX[next].state = unfinish;
        if (EX[next].rs1 != UNKNOWN_REG) {
          ex_rs1_val = reg[EX[next].rs1];
          if (MEM[now].state != empty && MEM[now].rd == EX[next].rs1) {
            if (MEM[now].state == finish)
              ex_rs1_val = MEM[now].ALU;
            else
              pauseEX = pauseID = pauseIF = true;
          }
          if (EX[now].state != empty && EX[now].rd == EX[next].rs1) {
            INST opt = EX[now].opt;
            if (opt == LB || opt == LBU || opt == LH || opt == LHU || opt == LW)
              pauseEX = pauseID = pauseIF = true;
            else if (opt != SB && opt != SH && opt != SW)
              ex_rs1_val = EX[now].ALU;
          }
        }
        if (EX[next].rs2 != UNKNOWN_REG) {
          ex_rs2_val = reg[EX[next].rs2];
          if (MEM[now].state != empty && MEM[now].rd == EX[next].rs2) {
            if (MEM[now].state == finish)
              ex_rs2_val = MEM[now].ALU;
            else
              pauseEX = pauseID = pauseIF = true;
          }
          if (EX[now].state != empty && EX[now].rd == EX[next].rs2) {
            INST opt = EX[now].opt;
            if (opt == LB || opt == LBU || opt == LH || opt == LHU ||
                opt == LW || EX[now].state == unfinish)
              pauseEX = pauseID = pauseIF = true;
            else if (opt != SB && opt != SH && opt != SW)
              ex_rs2_val = EX[now].ALU;
          }
        }
      } else {
        EX[next].state = empty;
      }

      /*-----------------------------ID[next]----------------------------*/
      if (ID[now].state == unfinish) {
        ID[next] = ID[now];
      } else if (IF[now].state == finish) {
        ID[next] = IF[now];
        ID[next].state = unfinish;
      } else {
        ID[next].state = empty;
      }

      /*-----------------------------IF[next]----------------------------*/
      if (IF[now].state == empty)
        IF[next].state = empty;
      else
        IF[next].state = unfinish;
    }
  }
  void solve() {
    read();
    IF[now].state = unfinish;
    while (true) {
      branch_skip = 0xffffffff;
      // printf("pc : %x\n", pc);
      if (WB[now].state != empty) {
        writeBack();
        reg[REG_ZERO] = 0;
        /* for (int i = 0; i < 32; ++i)
           printf("%x ", reg[i]);
         printf("\n");*/
      }

      if (MEM[now].state != empty && !pauseMEM) {
        if (MEM[now].rs2 == REG_ZERO)
          mem_rs2_val = 0;
        memoryAccess();
      }

      if (EX[now].state != empty && !pauseEX) {
        if (EX[now].rs1 == REG_ZERO)
          ex_rs1_val = 0;
        if (EX[now].rs2 == REG_ZERO)
          ex_rs2_val = 0;
        execute();
      }

      if (ID[now].state != empty && !pauseID)
        decode();

      if (IF[now].state != empty && !pauseIF)
        fetch();

      //	for(int i = 0; i < 32; ++i) printf("%x\n", reg[i]);
      //  printf("state: %s %s %s %s %s\n", NAME::STATE[IF[now].state],
      //   NAME::STATE[ID[now].state], NAME::STATE[EX[now].state],
      //  NAME::STATE[MEM[now].state], NAME::STATE[WB[now].state]);
      if (IF[now].state == empty && ID[now].state == empty &&
          EX[now].state == empty && MEM[now].state == empty &&
          WB[now].state == empty)
        break;
      next_loop();
      next ^= 1, now ^= 1;
      //  puts("-------------------------------------------------------------------"
      //    "------------");
    }
    printf("%d\n", reg[REG_A0] & 0xff);
  }
};

int main() {
  simulator Simulator;
  Simulator.solve();
  return 0;
}
