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

    bool runOnFunction(Function &F) override
    {

      std::map<StringRef, double> freqs;
      std::vector<StringRef> hotNodes;
      std::vector<StringRef> coldNodes;
      std::vector<std::pair<StringRef, StringRef>> hotEdges;
      std::vector<std::pair<StringRef, StringRef>> coldEdges;

      BlockFrequencyInfo &bfi = getAnalysis<BlockFrequencyInfoWrapperPass>().getBFI();
      BranchProbabilityInfo &bpi = getAnalysis<BranchProbabilityInfoWrapperPass>().getBPI();
      // for(const auto& elem : hotNodes)
      // {
      //   errs() << elem.first << " " << elem.second << "\n";
      // }
      int maxCount = -1;
      for (BasicBlock &BB : F)
      {
        int freq = bfi.getBlockFreq(&BB).getFrequency();
        freqs[BB.getName()] = freq;
        if (freq > maxCount)
        {
          maxCount = freq;
        }
      }

      errs() << maxCount << " count" << '\n';
      for (auto i = freqs.begin(); i != freqs.end(); i++)
      {
        i->second = i->second / maxCount;
        if (i->second > 0.8)
        {
          hotNodes.push_back(i->first);
        }
        else
        {
          coldNodes.push_back(i->first);
        }
      }

      for (BasicBlock &BB : F)
      {
        for (BasicBlock *successor : successors(&BB))
        {
          BranchProbability edgeProb = bpi.getEdgeProbability(&BB, successor);
          const uint64_t val = (uint64_t)(freqs[BB.getName()] * maxCount);
          int edgeProb2 = edgeProb.scale(val);
          double scaled = (double)edgeProb2 / maxCount;
          if (scaled > .8)
          {
            hotEdges.push_back(std::make_pair(BB.getName(), successor->getName()));
          }
          else
          {
            coldEdges.push_back(std::make_pair(BB.getName(), successor->getName()));
          }
        }
      }

      errs() << "------------" << '\n';

      errs() << "--Hot Nodes--" << '\n';
      for (auto i : hotNodes)
      {
        errs() << i << '\n';
      }
      errs() << "--Cold Nodes--" << '\n';
      for (auto i : coldNodes)
      {
        errs() << i << '\n';
      }
      errs() << "--Hot Edges--" << '\n';
      for (auto i : hotEdges)
      {
        errs() << i.first << " - " << i.second << '\n';
      }
      errs() << "--Cold Edges--" << '\n';
      for (auto i : coldEdges)
      {
        errs() << i.first << " - " << i.second << '\n';
      }

      // for(const auto& elem : freqs)
      // {
      //   errs() << elem.first << " " << elem.second << "\n";
      // }

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override
    {
      AU.addRequired<BranchProbabilityInfoWrapperPass>();
      AU.addRequired<BlockFrequencyInfoWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
    }

  private:
  };
} // end of namespace Correctness

char ISPRE::ISPREpass::ID = 0;
static RegisterPass<ISPRE::ISPREpass> X("ispre", "Frequent Loop Invariant Code Motion for correctness test", false, false);
