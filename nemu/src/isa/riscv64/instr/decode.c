#include "../local-include/reg.h"
#include "cpu/decode.h"
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
  static word_t zero_null;
  zero_null = 0;
  op->preg = (is_write && val == 0) ? &zero_null : &gpr(val);
}

static inline def_DopHelper(csr){
  bool is_write = flag;
  static word_t zero_null;
  zero_null = 0;
  op->preg = (is_write && val == 0) ? &zero_null : &csr(val);
}

static inline def_DHelper(R) {
  decode_op_r(s,id_src1,s->isa.instr.r.rs1,false);
  decode_op_r(s,id_src2,s->isa.instr.r.rs2,false);
  decode_op_r(s,id_dest,s->isa.instr.r.rd,true);
}

static inline def_DHelper(I) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, false);
  decode_op_i(s, id_src2, s->isa.instr.i.simm11_0,false);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, true);
}

static inline def_DHelper(U) {
  decode_op_i(s, id_src1, s->isa.instr.u.simm31_12 << 12, false);
  decode_op_r(s, id_dest, s->isa.instr.u.rd, true);
}

static inline def_DHelper(B){
  sword_t simm = (s->isa.instr.b.imm4_1 << 1)  | \
                  (s->isa.instr.b.imm10_5 << 5) | \
                  (s->isa.instr.b.imm11 << 11)  | \
                  (s->isa.instr.b.simm12 << 12) ;
  decode_op_i(s,id_dest,simm,false);
  decode_op_r(s,id_src1,s->isa.instr.b.rs1,false);
  decode_op_r(s,id_src2,s->isa.instr.b.rs2,false);
}

static inline def_DHelper(S) {
  decode_op_r(s, id_src1, s->isa.instr.s.rs1, false);
  sword_t simm = (s->isa.instr.s.simm11_5 << 5) | s->isa.instr.s.imm4_0;
  decode_op_i(s, id_src2, simm, false);
  decode_op_r(s, id_dest, s->isa.instr.s.rs2, true);
}

static inline def_DHelper(J){
  decode_op_r(s, id_dest, s->isa.instr.j.rd, true);
  
  rtlreg_t simm = (s->isa.instr.j.imm10_1 << 1) | \
                  (s->isa.instr.j.imm11 << 11) | \
                  (s->isa.instr.j.imm19_12 << 12) | \
                  (s->isa.instr.j.simm20 << 20);
  
  decode_op_i(s,id_src1,simm,false);
  decode_op_r(s,id_dest,s->isa.instr.j.rd, true);
}

static inline def_DHelper(CSR){
  decode_op_r(s,id_dest,s->isa.instr.i.rd,true);
  decode_op_r(s,id_src1,s->isa.instr.i.rs1,false);
  decode_op_csr(s,id_src2,s->isa.instr.i.simm11_0,true);
}

def_THelper(ri){
  def_INSTR_TAB("?0????? ????? ????? 101 ????? ????? ??",srli);
  def_INSTR_TAB("?1????? ????? ????? 101 ????? ????? ??",srai);
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",addi);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",slli);
  def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??",xori);
  def_INSTR_TAB("??????? ????? ????? 110 ????? ????? ??",ori);
  def_INSTR_TAB("??????? ????? ????? 111 ????? ????? ??",andi);

  def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??",slti);
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??",sltiu);
  return EXEC_ID_inv;
}

def_THelper(rr){
  def_INSTR_TAB("0000000 ????? ????? 000 ????? ????? ??",add);
  def_INSTR_TAB("0100000 ????? ????? 000 ????? ????? ??",sub);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",sll);
  def_INSTR_TAB("0000000 ????? ????? 101 ????? ????? ??",srl);
  def_INSTR_TAB("0100000 ????? ????? 101 ????? ????? ??",sra);
  def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??",xor);
  def_INSTR_TAB("??????? ????? ????? 110 ????? ????? ??",or);
  def_INSTR_TAB("??????? ????? ????? 111 ????? ????? ??",and);
  
  def_INSTR_TAB("0000001 ????? ????? 000 ????? ????? ??",mul);
  def_INSTR_TAB("0000001 ????? ????? 001 ????? ????? ??",mulh);
  def_INSTR_TAB("0000001 ????? ????? 010 ????? ????? ??",mulhsu);
  def_INSTR_TAB("0000001 ????? ????? 011 ????? ????? ??",mulhu);

  def_INSTR_TAB("0000001 ????? ????? 100 ????? ????? ??",div);
  def_INSTR_TAB("0000001 ????? ????? 101 ????? ????? ??",divu);
  def_INSTR_TAB("0000001 ????? ????? 110 ????? ????? ??",rem);
  def_INSTR_TAB("0000001 ????? ????? 111 ????? ????? ??",remu);
  
  def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??",slt);
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??",sltu);
  return EXEC_ID_inv;
}

def_THelper(riw){
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",addiw);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",slliw);
  def_INSTR_TAB("?0????? ????? ????? 101 ????? ????? ??",srliw);
  def_INSTR_TAB("?1????? ????? ????? 101 ????? ????? ??",sraiw);
  
  return EXEC_ID_inv;
}

def_THelper(rrw){
  def_INSTR_TAB("0000000 ????? ????? 000 ????? ????? ??",addw);
  def_INSTR_TAB("0100000 ????? ????? 000 ????? ????? ??",subw);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",sllw);
  def_INSTR_TAB("0000000 ????? ????? 101 ????? ????? ??",srlw);
  def_INSTR_TAB("0100000 ????? ????? 101 ????? ????? ??",sraw);

  def_INSTR_TAB("0000001 ????? ????? 000 ????? ????? ??",mulw);
  def_INSTR_TAB("0000001 ????? ????? 100 ????? ????? ??",divw);
  def_INSTR_TAB("0000001 ????? ????? 101 ????? ????? ??",divuw);
  def_INSTR_TAB("0000001 ????? ????? 110 ????? ????? ??",remw);
  def_INSTR_TAB("0000001 ????? ????? 111 ????? ????? ??",remuw);
  return EXEC_ID_inv;
}

def_THelper(load) {
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",lb);
  def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??",lbu);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",lh);
  def_INSTR_TAB("??????? ????? ????? 101 ????? ????? ??",lhu);
  def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??",lw);
  def_INSTR_TAB("??????? ????? ????? 110 ????? ????? ??",lwu);
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??",ld);
  return EXEC_ID_inv;
}

def_THelper(store) {
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",sb);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",sh);
  def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??",sw);
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??",sd);
  return EXEC_ID_inv;
}

def_THelper(cb){
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",beq);
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",bne);
  def_INSTR_TAB("??????? ????? ????? 100 ????? ????? ??",blt);
  def_INSTR_TAB("??????? ????? ????? 101 ????? ????? ??",bge);
  def_INSTR_TAB("??????? ????? ????? 110 ????? ????? ??",bltu);
  def_INSTR_TAB("??????? ????? ????? 111 ????? ????? ??",bgeu);
  return EXEC_ID_inv; 
}

def_THelper(system){
  def_INSTR_TAB("??????? ????? ????? 001 ????? ????? ??",csrrw);
  def_INSTR_TAB("??????? ????? ????? 010 ????? ????? ??",csrrs);
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??",csrrc);
  return EXEC_ID_inv;
}

def_THelper(main) {
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01101 11", U     , lui);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00101 11", U     , auipc); // calculation
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00000 11", I     , load); // load

  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01000 11", S     , store); // store
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00100 11", I     , ri);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00110 11", I     , riw);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01100 11", R     , rr);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01110 11", R     , rrw);
  
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11011 11", J     , jal);  // control
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11001 11", I     , jalr);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11000 11", B     , cb);

  def_INSTR_TAB("0000000 00000 00000 000 00000 11100 11"          , ecall);
  def_INSTR_TAB("0011000 00010 00000 000 00000 11100 11"          , mret);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11100 11", CSR    , system);

  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 11010 11",         nemu_trap); // trap

  return table_inv(s);
};

int isa_fetch_decode(Decode *s) {
  s->isa.instr.val = instr_fetch(&s->snpc, 4);
  int idx = table_main(s);
  return idx;
}
