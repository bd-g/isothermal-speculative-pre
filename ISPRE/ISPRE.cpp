//===----------------------------------------------------------------------===//
//
//  INSERT DESCRIPTION OF FILE HERE
//
////===----------------------------------------------------------------------===//
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
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

#include <map>
#include <string>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "ispre"

namespace ISPRE {
struct ISPREPass : public FunctionPass {
    static char ID;
    ISPREPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {

        std::map<StringRef, double> freqs;
        std::vector<StringRef> hotNodes;
        std::vector<StringRef> coldNodes;
        std::vector<std::pair<StringRef, StringRef>> hotEdges;
        std::vector<std::pair<StringRef, StringRef>> coldEdges;

        BlockFrequencyInfo &bfi = getAnalysis<BlockFrequencyInfoWrapperPass>().getBFI();
        BranchProbabilityInfo &bpi = getAnalysis<BranchProbabilityInfoWrapperPass>().getBPI();

        int maxCount = -1;
        for (BasicBlock &BB : F) {
            int freq = bfi.getBlockFreq(&BB).getFrequency();
            freqs[BB.getName()] = freq;
            if (freq > maxCount) {
                maxCount = freq;
            }
        }

        errs() << maxCount << " count" << '\n';
        for (auto i = freqs.begin(); i != freqs.end(); i++) {
            i->second = i->second / maxCount;
            if (i->second > 0.8) {
                hotNodes.push_back(i->first);
            } else {
                coldNodes.push_back(i->first);
            }
        }

        for (BasicBlock &BB : F) {
            for (BasicBlock *successor : successors(&BB)) {
                BranchProbability edgeProb = bpi.getEdgeProbability(&BB, successor);
                const uint64_t val = (uint64_t)(freqs[BB.getName()] * maxCount);
                int edgeProb2 = edgeProb.scale(val);
                double scaled = (double)edgeProb2 / maxCount;
                if (scaled > .8) {
                    hotEdges.push_back(std::make_pair(BB.getName(), successor->getName()));
                } else {
                    coldEdges.push_back(std::make_pair(BB.getName(), successor->getName()));
                }
            }
        }

        errs() << "*************" << '\n';

        errs() << "--Hot Nodes--" << '\n';
        for (auto i : hotNodes) {
            errs() << i << '\n';
        }

        errs() << "--Cold Nodes--" << '\n';
        for (auto i : coldNodes) {
            errs() << i << '\n';
        }

        errs() << "--Hot Edges--" << '\n';
        for (auto i : hotEdges) {
            errs() << i.first << " - " << i.second << '\n';
        }

        errs() << "--Cold Edges--" << '\n';
        for (auto i : coldEdges) {
            errs() << i.first << " - " << i.second << '\n';
        }

        errs() << "*************" << '\n';

        return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.addRequired<BranchProbabilityInfoWrapperPass>();
        AU.addRequired<BlockFrequencyInfoWrapperPass>();
        AU.addRequired<LoopInfoWrapperPass>();
    }

  private:
    static int dummyVar;
};
} // namespace ISPRE

char ISPRE::ISPREPass::ID = 0;
static RegisterPass<ISPRE::ISPREPass>
    X("ispre", "Isothermal Speculative Partial Redundancy Elimination", false, false);
