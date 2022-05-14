#include "../local-include/reg.h"
#include <cpu/ifetch.h>
#include <isa-all-instr.h>

def_all_THelper();

static uint32_t get_instr(Decode *s) {
  return s->isa.instr.val;
}

// decode operand helper
#define def_DopHelper(name) \
  void concat(decode_op_, name) (Decode *s, Operand *op, word_t val, bool flag)

static inline def_DopHelper(i) {
  op->imm = val;
}

static inline def_DopHelper(r) {
  bool is_write = flag;
  static word_t zero_null = 0;
  op->preg = (is_write && val == 0) ? &zero_null : &gpr(val);
}

static inline def_DHelper(I) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, false);
  rtlreg_t simm = s->isa.instr.i.simm11_0; 

  if(simm & 1<<11)
    simm=c_sext(simm,8);

  decode_op_i(s, id_src2,simm,false);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, true);

}

static inline def_DHelper(U) {
  decode_op_i(s, id_src1, (sword_t)s->isa.instr.u.simm31_12 << 12, false);
  decode_op_r(s, id_dest, s->isa.instr.u.rd, true);
}

static inline def_DHelper(S) {
  decode_op_r(s, id_src1, s->isa.instr.s.rs1, false);
  sword_t simm = (s->isa.instr.s.simm11_5 << 5) | s->isa.instr.s.imm4_0;
  decode_op_i(s, id_src2, simm, false);
  decode_op_r(s, id_dest, s->isa.instr.s.rs2, false);
}

static inline def_DHelper(J){
  decode_op_r(s, id_dest, s->isa.instr.j.rd, true);
  
  rtlreg_t simm = ((s->isa.instr.j.offset1_10 << 1) | \
                 (s->isa.instr.j.offset11 << 11) | \
                 (s->isa.instr.j.offset12_19 << 12) | \
                 (s->isa.instr.j.offset20 << 20));

  if(s->isa.instr.j.offset20) simm = c_sext(simm,8);

  decode_op_i(s,id_src1,simm,false);
  decode_op_r(s,id_dest,s->isa.instr.j.rd, false);
}

def_THelper(ci){
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",addi);
  
  return EXEC_ID_inv;
}


def_THelper(load) {
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", ld);
  return EXEC_ID_inv;
}

def_THelper(store) {
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", sd);
  return EXEC_ID_inv;
}

def_THelper(main) {
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01101 11", U     , lui);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00000 11", I     , load); // load

  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01000 11", S     , store); // store
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00101 11", U     , auipc); // calculation
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00100 11", I     , ci);
  
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11011 11", J     , jal);  // control
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11001 11", I     , jalr);

  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 11010 11",         nemu_trap); // trap

  return table_inv(s);
};

int isa_fetch_decode(Decode *s) {
  s->isa.instr.val = instr_fetch(&s->snpc, 4);
  int idx = table_main(s);
  return idx;
}
