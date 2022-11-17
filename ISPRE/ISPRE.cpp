//===-- Frequent Path Loop Invariant Code Motion Pass ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// EECS583 F22 - This pass can be used as a template for your Frequent Path LICM
//               homework assignment. The pass gets registered as "fplicm".
//
// This pass performs loop invariant code motion, attempting to remove as much
// code from the body of a loop as possible.  It does this by either hoisting
// code into the preheader block, or by sinking code to the exit blocks if it is
// safe.
//
////===----------------------------------------------------------------------===//
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/ADT/StringRef.h"

#include <map>
#include <string>
#include <vector>
/* *******Implementation Starts Here******* */
// include necessary header files
/* *******Implementation Ends Here******* */

using namespace llvm;

#define DEBUG_TYPE "ispre"

namespace ISPRE
{
  struct ISPREpass : public FunctionPass
  {
    static char ID;
    ISPREpass() : FunctionPass(ID) {}

    void getFreqPath(std::map<StringRef, int> &map, BasicBlock *B)
    {
      BranchProbabilityInfo &bpi = getAnalysis<BranchProbabilityInfoWrapperPass>().getBPI();
      for (BasicBlock *successor : successors(B))
      {
        if (bpi.getEdgeProbability(B, successor) >= BranchProbability(4, 5))
        {
          if (map.count(successor->getName()) > 0)
          {
            return;
          }
          map[successor->getName()] = 1;
          getFreqPath(map, successor);
        }
      }
    }

   bool runOnFunction(Function &F) override {
      errs() << F.getName() << "\n";
      errs() << "Biased Branch: " << "\n";
      errs() << "Biased Branch2: " << "\n";
      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override
    {
      AU.addRequired<BranchProbabilityInfoWrapperPass>();
      AU.addRequired<BlockFrequencyInfoWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
    }

  private:
    /// Little predicate that returns true if the specified basic block is in
    /// a subloop of the current one, not the current one itself.
    bool inSubLoop(BasicBlock *BB, Loop *CurLoop, LoopInfo *LI)
    {
      assert(CurLoop->contains(BB) && "Only valid if BB is IN the loop");
      return LI->getLoopFor(BB) != CurLoop;
    }
  };
} // end of namespace Correctness

char ISPRE::ISPREpass::ID = 0;
static RegisterPass<ISPRE::ISPREpass> X("ispre", "Frequent Loop Invariant Code Motion for correctness test", false, false);

