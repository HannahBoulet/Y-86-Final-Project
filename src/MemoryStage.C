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
   //uint64_t iCND = mdreg->get(M_CND);
   uint64_t dste = mdreg->get(M_DSTE);
   uint64_t dstm = mdreg->get(M_DSTM);
   uint64_t valE = mdreg->get(M_VALE);
   uint64_t valA = mdreg->get(M_VALA);
   setWInput(wreg, stat, icode, valE, valA, dste, dstm);
   return false;

   // call Addr method to obtain the address used to access memory
   uint64_t address = mem_addr(mdreg);

   // if the mem_read method returns true, use Memory class to read a long
   bool error = false;
   if (mem_read(mdreg))
   {
      m_valM = mem->getLong(address, error);
   }

   //  If the mem_write returns true use your Memory class to write M_valA to memory
   if (mem_write(mdreg))
   {
      mem->putLong(mdreg->get(M_VALA), address, error);
   }
   
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
   wreg->set(W_VALM, 0);

   wreg->set(W_DSTE, dstE);
   wreg->set(W_DSTM, dstM);
}

//HCL for Addr component
/*
word mem_addr = [
   M_icode in { IRMMOVQ, IPUSHQ, ICALL, IMRMOVQ } : M_valE;
   M_icode in { IPOPQ, IRET } : M_valA;
   1: 0;  
];
*/
uint64_t MemoryStage::mem_addr(PipeReg * mdreg)
{
   if (mdreg->get(M_ICODE) == Instruction::IRMMOVQ || mdreg->get(M_ICODE) == Instruction::IPUSHQ 
      || mdreg->get(M_ICODE) == Instruction::ICALL || mdreg->get(M_ICODE) == Instruction::IMRMOVQ)
      {
         return mdreg->get(M_VALE);
      }
   if (mdreg->get(M_ICODE) == Instruction::IPOPQ || mdreg->get(M_ICODE) == Instruction::IRET)
   {
      return mdreg->get(M_VALA);
   }
   return 0;
}

/*
//HCL for Mem Read component
bool mem_read = M_icode in { IMRMOVQ, IPOPQ, IRET };*/
bool MemoryStage::mem_read(PipeReg * mdreg)
{
   if (mdreg->get(M_ICODE) == Instruction::IMRMOVQ || mdreg->get(M_ICODE) == Instruction::IPOPQ
      || mdreg->get(M_ICODE) == Instruction::IRET)
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
   if (mdreg->get(M_ICODE) == Instruction::IRMMOVQ || mdreg->get(M_ICODE) == Instruction::IPUSHQ
      || mdreg->get(M_ICODE) == Instruction::ICALL)
      {
         return 1;
      }
   return 0;
}