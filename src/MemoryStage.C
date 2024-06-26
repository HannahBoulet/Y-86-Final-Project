#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Instruction.h"
#include "Stage.h"
#include "Status.h"
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
   m_valM = 0;
   bool mem_error = false;
   PipeReg * mdreg = pipeRegs->getMemoryReg();
   PipeReg * wreg = pipeRegs->getWritebackReg();
   Stage::m_stat = mdreg->get(M_STAT);
   uint64_t icode = mdreg->get(M_ICODE);
   uint64_t dste = mdreg->get(M_DSTE);
   uint64_t dstm = mdreg->get(M_DSTM);
   uint64_t valE = mdreg->get(M_VALE);
   uint64_t valA = mdreg->get(M_VALA);
   uint64_t address = mem_addr(icode, valA, valE);

   if (mem_read(icode))
   {
      m_valM = mem->getLong(address, mem_error);
   }
   if (mem_write(icode))
   {
      mem->putLong(valA, address, mem_error);
   }
   if(mem_error)
   {
      m_stat = Status::SADR;
   }
   setWInput(wreg, m_stat, icode, valE, m_valM, dste, dstm);
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

/**
* mem_addr
*
* Obtains the address that is used to access memory.
*
* @param: icode - the instruction code representing the type of instruction
* @param: valA - value from register A
* @param: valE - value from register E
* @return: the selected memory address based on the icode.
*/
uint64_t MemoryStage::mem_addr(uint64_t icode, uint64_t valA,  uint64_t valE)
{
   if (icode == Instruction::IRMMOVQ || icode == Instruction::IPUSHQ 
      || icode == Instruction::ICALL || icode == Instruction::IMRMOVQ)
      {
         return valE;
      }
   if (icode == Instruction::IPOPQ || icode == Instruction::IRET)
   {
      return valA;
   }
   return 0;
}

/**
* mem_read 
*
* Examines the icode to determine if it corresponds to a memory read operation.
*
* @param: icode - the instruction code representing the type of instruction
* @return: true if the instruction requires a memory read, false otherwise.
*/
bool MemoryStage::mem_read(uint64_t icode)
{
   return (icode == Instruction::IMRMOVQ || icode == Instruction::IPOPQ
      || icode == Instruction::IRET);
}

/**
* mem_write
*
* Examines icode to determine if it corresponds to a memory write operation.
*
* @param: icode - the instruction code representing the type of instruction
* @return: true if memory write, false otherwise
*/
bool MemoryStage::mem_write(uint64_t icode)
{
   return (icode == Instruction::IRMMOVQ || icode == Instruction::IPUSHQ
      || icode == Instruction::ICALL);
}
