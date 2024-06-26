#include "PipeRegArray.h"
#include "Stage.h"

#ifndef DECODESTAGE_H
#define DECODESTAGE_H
class DecodeStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
      bool E_bubble;
      uint64_t d_srcAFun(PipeReg * dreg);
      uint64_t d_srcBFun(PipeReg * dreg);
      uint64_t d_dstEFun(PipeReg * dreg);
      uint64_t d_dstMFun(PipeReg * dreg);
      uint64_t selFwdAFun(uint64_t ra);
      uint64_t selFwdAFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcA);
      uint64_t FwdBFun(PipeReg * mreg, PipeReg * wreg, uint64_t srcB);

      void setEInput(PipeReg * Ereg, uint64_t stat, 
         uint64_t icode, uint64_t ifun, uint64_t valA,
         uint64_t valC, uint64_t valB,uint64_t dstE,
         uint64_t dstM,uint64_t srcA, uint64_t srcB); 
      
      uint64_t calculateControlSignals(PipeReg * ereg);
   

public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
