#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Stage.h"
#include "D.h"
#include "E.h"

/*
will use the values of the pipeline register fields in 
the D pipeline register and determine the values to 
be stored in the E pipeline register. The doClockHigh 
method stores those values in the E pipeline register.
*/

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool DecodeStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * dreg = pipeRegs->getDecodeReg();
   PipeReg * ereg = pipeRegs->getExecuteReg();

   uint64_t stat = dreg->get(D_STAT);
   uint64_t icode = dreg->get(D_ICODE);
   uint64_t ifun = dreg->get(D_IFUN);
   uint64_t rA = dreg->get(D_RA);
   uint64_t rB = dreg->get(D_RB);
   uint64_t valC = dreg->get(D_VALC);
   uint64_t valP = dreg->get(D_VALP);

    // Call setEinput with the obtained values
   setEInput(ereg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void DecodeStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * ereg = pipeRegs->getExecuteReg();  
   ereg->normal();
}



//Extra Helpers!
/*
 Note that it has a parameter for each pipe register field within the
E pipeline register and will have a call to the set method for each of 
these. The fields of the E pipeline register are identified by the #defines in E.h. 
 (Note: you'll need to add the setEinput declaration to DecodeStage.h)
 asking abby in a bit about this
 
 */
void DecodeStage::setEInput(PipeReg * Ereg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP)
{
   Ereg->set(E_STAT, stat);
   Ereg->set(E_ICODE, icode);
   Ereg->set(E_IFUN, ifun);

   Ereg->set(E_DSTE, rA);
   Ereg->set(E_DSTM, rB);
   Ereg->set(E_SRCA, rA);
   Ereg->set(E_SRCB, rB);

   Ereg->set(E_VALC, valC);
   Ereg->set(E_VALA, 0);

}