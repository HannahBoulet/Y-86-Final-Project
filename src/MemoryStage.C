#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "PipeRegField.h"
#include "PipeReg.h"
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
   uint64_t iCND = mdreg->get(M_CND);
   uint64_t dste = mdreg->get(M_DSTE);
   uint64_t dstm = mdreg->get(M_DSTM);
   uint64_t valE = mdreg->get(M_VALE);
   uint64_t valA = mdreg->get(M_VALA);
   setWInput(wreg, stat, icode, valE, valA, dste, dstm);
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
   PipeReg * mreg = pipeRegs->getMemoryReg();  
   mreg->normal();   
}
//Extra helpers

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


