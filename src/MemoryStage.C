#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Instruction.h"
#include "Stage.h"
#include "W.h"
#include "M.h"
/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool MemoryStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * mdreg = pipeRegs->getMemoryReg();
   PipeReg * wreg = pipeRegs->getWritebackReg();

   uint64_t stat = mdreg->get(M_STAT);
   uint64_t icode = mdreg->get(M_ICODE);
   uint64_t dste = mdreg->get(M_DSTE);
   uint64_t dstm = mdreg->get(M_DSTM);
   uint64_t valE = mdreg->get(M_VALE);
   uint64_t valA = mdreg->get(M_VALA);
   uint64_t address = mem_addr(mdreg);
   m_valM = 0;
   bool error = false;
   if (mem_read(mdreg))
   {
      m_valM = mem->getLong(address, error);
   }
   if (mem_write(mdreg))
   {
      mem->putLong(valA, address, error);
   }
   setWInput(wreg, stat, icode, valE, m_valM, dste, dstm);
   return false;
   
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void MemoryStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * wreg = pipeRegs->getWritebackReg();  
   wreg->normal();   
}

void MemoryStage::setWInput(PipeReg * wreg, uint64_t stat, uint64_t icode,
                            uint64_t valE, uint64_t valM, uint64_t dstE, 
                            uint64_t dstM)
{
   wreg->set(W_STAT, stat);
   wreg->set(W_ICODE, icode);

   wreg->set(W_VALE, valE);
   wreg->set(W_VALM, valM);

   wreg->set(W_DSTE, dstE);
   wreg->set(W_DSTM, dstM);
}


uint64_t MemoryStage::mem_addr(PipeReg * mdreg)
{
   uint64_t icode = mdreg->get(M_ICODE);
   if (icode == Instruction::IRMMOVQ || icode == Instruction::IPUSHQ 
      || icode == Instruction::ICALL || icode == Instruction::IMRMOVQ)
      {
         return mdreg->get(M_VALE);
      }
   if (icode == Instruction::IPOPQ || icode == Instruction::IRET)
   {
      return mdreg->get(M_VALA);
   }
   return 0;
}


bool MemoryStage::mem_read(PipeReg * mdreg)
{
   uint64_t icode = mdreg->get(M_ICODE);
   if (icode == Instruction::IMRMOVQ || icode == Instruction::IPOPQ
      || icode == Instruction::IRET)
      {
         return 1;
      }
   return 0;
}

/*
//HCL for Mem Write component
bool mem_write = M_icode in { IRMMOVQ, IPUSHQ, ICALL };*/
bool MemoryStage::mem_write(PipeReg * mdreg)
{
   uint64_t icode = mdreg->get(M_ICODE);
   if (icode == Instruction::IRMMOVQ || icode == Instruction::IPUSHQ
      || icode == Instruction::ICALL)
      {
         return 1;
      }
   return 0;
}