#include "PipeRegArray.h"
#include "ExecuteStage.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Stage.h"
#include "E.h"
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
bool ExecuteStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * ereg = pipeRegs->getExecuteReg();
   PipeReg * mreg = pipeRegs->getMemoryReg();

   uint64_t stat = ereg->get(E_STAT);
   uint64_t icode = ereg->get(E_ICODE);
   uint64_t ifun = ereg->get(E_IFUN);
   uint64_t valC = ereg->get(E_VALC);
   uint64_t valA = ereg->get(E_VALA);
   uint64_t valB = ereg->get(E_VALB);
   uint64_t dste = ereg->get(E_DSTE);
   uint64_t dstm = ereg->get(E_DSTM);
   uint64_t srcA = ereg->get(E_SRCA);
   uint64_t srcB = ereg->get(E_SRCB);

   ereg->set(e_valE, E_VALC);
   setMInput(mreg, stat, icode, ifun, valC, valA, dste, dstm);

   return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void ExecuteStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * mreg  = pipeRegs->getMemoryReg();
   mreg->normal();
}

void ExecuteStage::setMInput(PipeReg * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, 
                     uint64_t valE, uint64_t valA,
                     uint64_t dstE, uint64_t dstM)

{
    mreg->set(M_STAT, stat);
    mreg->set(M_ICODE, icode);
    mreg->set(M_CND, cnd);
    mreg->set(M_VALE, e_valE);
    mreg->set(M_VALA, valA);
    mreg->set(M_DSTE, dstE);
    mreg->set(M_DSTM, dstM);
}
