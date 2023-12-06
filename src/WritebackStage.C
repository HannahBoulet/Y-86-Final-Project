#include "PipeRegArray.h"
#include "WritebackStage.h"
#include "Instruction.h"
#include "Memory.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "PipeRegArray.h"
#include "Stage.h"
#include "Status.h"
#include "W.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool WritebackStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * wreg = pipeRegs->getWritebackReg();
   return wreg->get(W_STAT) != Status::SAOK;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void WritebackStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * wreg = pipeRegs->getWritebackReg();
   uint64_t valE = wreg->get(W_VALE);
   uint64_t valM = wreg->get(W_VALM);
   uint64_t w_dstM = wreg->get(W_DSTM);
   uint64_t w_dste = wreg->get(W_DSTE);
   bool error;
   rf->writeRegister(valE, w_dste, error);
   rf->writeRegister(valM, w_dstM, error);
}
