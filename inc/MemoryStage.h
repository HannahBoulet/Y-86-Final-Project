#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Stage.h"

#ifndef MEMORYSTAGE_H
#define MEMORYSTAGE_H
class MemoryStage: public Stage
{
   private:
      void setWInput(PipeReg * wreg, uint64_t stat, uint64_t icode,
                            uint64_t valE, uint64_t valM, uint64_t dstE, 
                            uint64_t dstM);
      uint64_t mem_addr(uint64_t icode, uint64_t valA, uint64_t valE);
      bool mem_read(uint64_t icode);
      bool mem_write(uint64_t icode);


   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
