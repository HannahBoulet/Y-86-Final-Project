#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Stage.h"

#ifndef FETCHSTAGE_H
#define FETCHSTAGE_H
class FetchStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
      bool f_stall;
      bool d_stall;
      bool D_bubble;
      uint64_t selectPC(PipeReg * freg, PipeReg * mreg, PipeReg * wreg);
      bool need_regids(uint64_t f_icode);
      bool need_valC(uint64_t f_icode);
      uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t valP);
      uint64_t PCincrement(uint64_t f_pc, bool needRegIds, bool needValC);
      void getRegIds(uint64_t f_pc, bool needRegIds, uint64_t &rA, uint64_t &rB);
      uint64_t f_stat(bool mem_error, bool i_valid, uint64_t f_icode);

      uint64_t buildValC(uint64_t f_pc, bool needRegIds, bool needvalC);
      bool instr_valid(uint64_t icode);


      void setDInput(PipeReg * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);

      bool F_stall(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg);
      bool D_stall(PipeReg * ereg);
      bool getD_bubble(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg);

      void calculateControlSignals(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg);
   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
