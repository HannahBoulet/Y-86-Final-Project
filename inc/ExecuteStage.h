#include "PipeRegArray.h"
#include "Stage.h"

#ifndef EXECUTESTAGE_H
#define EXECUTESTAGE_H
class ExecuteStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
      void setMInput(PipeReg * Ereg, uint64_t stat, uint64_t icode, uint64_t cnd, 
                     uint64_t valE, uint64_t valA,
                     uint64_t dstE, uint64_t dstM);
      uint64_t aluA(PipeReg * ereg);
      uint64_t aluB(PipeReg * ereg);
      uint64_t aluFun(PipeReg * ereg);
      bool set_cc(PipeReg * ereg);
      uint64_t set_dstE(PipeReg * ereg);






   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
