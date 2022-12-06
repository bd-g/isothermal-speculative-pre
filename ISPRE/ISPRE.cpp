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

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <unordered_set>

using namespace llvm;

#define DEBUG_TYPE "ispre"

namespace ISPRE {
struct ISPREPass : public FunctionPass {
    static char ID;
    static constexpr double THRESHOLD = 0.8;
    ISPREPass() : FunctionPass(ID) {}

    int calculateHotColdNodes(Function &F, std::map<StringRef, double> &freqs,
                              std::vector<StringRef> &hotNodes, std::vector<StringRef> &coldNodes) {
        BlockFrequencyInfo &bfi = getAnalysis<BlockFrequencyInfoWrapperPass>().getBFI();

        int maxCount = -1;
        for (BasicBlock &BB : F) {
            int freq = bfi.getBlockProfileCount(&BB).getValue();
            freqs[BB.getName()] = freq;
            if (freq > maxCount) {
                maxCount = freq;
            }
        }

        for (auto i = freqs.begin(); i != freqs.end(); i++) {
            i->second = i->second / maxCount;
            if (i->second > THRESHOLD) {
                hotNodes.push_back(i->first);
            } else {
                coldNodes.push_back(i->first);
            }
        }

        return maxCount;
    }

    void calculateHotColdEdges(Function &F, std::map<StringRef, double> &freqs,
                               std::vector<std::pair<StringRef, StringRef>> &hotEdges,
                               std::vector<std::pair<StringRef, StringRef>> &coldEdges,
                               int maxCount) {
        BranchProbabilityInfo &bpi = getAnalysis<BranchProbabilityInfoWrapperPass>().getBPI();
        for (BasicBlock &BB : F) {
            for (BasicBlock *successor : successors(&BB)) {
                BranchProbability edgeProb = bpi.getEdgeProbability(&BB, successor);
                const uint64_t val = (uint64_t)(freqs[BB.getName()] * maxCount);
                int edgeProb2 = edgeProb.scale(val);
                double scaled = (double)edgeProb2 / maxCount;
                if (scaled > THRESHOLD) {
                    hotEdges.push_back(std::make_pair(BB.getName(), successor->getName()));
                } else {
                    coldEdges.push_back(std::make_pair(BB.getName(), successor->getName()));
                }
            }
        }
    }

    void calculateIngressEdges(std::vector<std::pair<StringRef, StringRef>> &coldEdges,
                               std::vector<StringRef> &hotNodes, std::vector<StringRef> &coldNodes,
                               std::vector<std::pair<StringRef, StringRef>> &ingressEdges) {
        for (auto i : coldEdges) {
            if (std::count(coldNodes.begin(), coldNodes.end(), i.first) &&
                std::count(hotNodes.begin(), hotNodes.end(), i.second)) {
                ingressEdges.push_back(i);
            }
        }
    }

    void printIngressEdges(std::vector<std::pair<StringRef, StringRef>> &ingressEdges) {
        errs() << "\n*************" << '\n';

        errs() << "--Ingress Edges--" << '\n';
        for (auto i : ingressEdges) {
            errs() << i.first << " - " << i.second << '\n';
        }
        errs() << "*************\n" << '\n';
    }

    void printHotColdNodes(std::vector<StringRef> &hotNodes, std::vector<StringRef> &coldNodes) {
        errs() << "\n*************" << '\n';

        errs() << "--Hot Nodes--" << '\n';
        for (auto i : hotNodes) {
            errs() << i << '\n';
        }

        errs() << "\n--Cold Nodes--" << '\n';
        for (auto i : coldNodes) {
            errs() << i << '\n';
        }
        errs() << "*************\n" << '\n';
    }

    void printHotColdEdges(std::vector<std::pair<StringRef, StringRef>> &hotEdges,
                           std::vector<std::pair<StringRef, StringRef>> &coldEdges) {
        errs() << "\n*************" << '\n';
        errs() << "--Hot Edges--" << '\n';
        for (auto i : hotEdges) {
            errs() << i.first << " - " << i.second << '\n';
        }

        errs() << "\n--Cold Edges--" << '\n';
        for (auto i : coldEdges) {
            errs() << i.first << " - " << i.second << '\n';
        }

        errs() << "*************\n" << '\n';
    }

    /*
    GRETCHEN PART: (Necessity)
    Initialize In(X) to 0 for all basic blocks X
    change = 1
    while (change) do
    change = 0
    for each basic block in procedure, X, do
    old_NEEDIN = NEEDIN(X)
    NEEDOUT(X) = Union(NeedIn(Y)) for all successors Y of X
    NEEDIN(X) = NEEDOUT(X) - GEN(X) + REMOVABLE(X)
    if (old_IN != IN(X)) then
        change = 1
    */

    std::map<StringRef, std::set<StringRef>> needin_map;
    std::map<StringRef, std::set<StringRef>> needout_map;
    std::map<StringRef, std::set<StringRef>> gen_map;       // TODO
    std::map<StringRef, std::set<StringRef>> removable_map; // TODO
    std::map<StringRef, std::set<StringRef>> avout_map;     // TODO

    void compute_needin_needout(std::map<StringRef, std::set<StringRef>> &needin_map,
                                std::map<StringRef, std::set<StringRef>> &needout_map,
                                Function &F) {
        // Init NEEDIN(X) to 0 for all basic blocks X
        for (BasicBlock &BB : F) {
            std::set<StringRef> empty_set;
            needin_map[BB.getName()] = empty_set;
        }

        int change = 1;
        while (change) {
            change = 0;
            for (BasicBlock &BB : F) {
                auto bb_name = BB.getName();
                std::set<StringRef> old_needin = needin_map[bb_name];
                std::set<StringRef> needout;

                // NEEDOUT(X) = Union(NeedIn(Y)) for all successors Y of X
                for (BasicBlock *successor : successors(&BB)) {
                    std::set<StringRef> succ_needin = needin_map[successor->getName()];
                    std::set_union(needout.begin(), needout.end(), succ_needin.begin(),
                                   succ_needin.end(), std::inserter(needout, needout.end()));
                }

                // NEEDIN(X) = NEEDOUT(X) - GEN(X) + REMOVABLE(X)
                std::set<StringRef> needin;
                std::set<StringRef> gen = gen_map[bb_name];
                std::set_difference(needout.begin(), needout.end(), gen.begin(), gen.end(),
                                    std::inserter(needin, needin.end()));
                std::set<StringRef> removable = removable_map[bb_name];
                std::set_union(needin.begin(), needin.end(), removable.begin(), removable.end(),
                               std::inserter(needin, needin.end()));

                // update needin and needout
                needin_map[bb_name] = needin;
                needout_map[bb_name] = needout;
                if (old_needin != needin) {
                    change = 1;
                }
            }
        }
    }

    std::map<std::pair<StringRef, StringRef>, std::set<StringRef>> insert_map;
    void
    compute_insert_map(std::map<std::pair<StringRef, StringRef>, std::set<StringRef>> &insert_map,
                       std::vector<std::pair<StringRef, StringRef>> &ingressEdges) {
        for (auto itr = ingressEdges.begin(); itr != ingressEdges.end(); itr++) {
            StringRef u = itr->first;
            StringRef v = itr->second;
            std::set<StringRef> insert_set;
            std::set<StringRef> needin = needin_map[v];
            std::set<StringRef> avout = avout_map[u];
            std::set_difference(needin.begin(), needin.end(), avout.begin(), avout.end(),
                                std::inserter(insert_set, insert_set.end()));
            insert_map[{u, v}] = insert_set;
        }
    }

    // If expression e is of the form x=a op b, for each of the operands a and b, only look before
    // e. Get loads and their corresponding sources. For each load, look through all stores for
    // matching destination of store If found then e is killed and does not go into xUses
    void fillXUses(Function &F, std::map<StringRef, std::set<Instruction *>> &xUses) {

        for (BasicBlock &BB : F) // for each BB
        {
            for (auto &instr : BB) // for each instruction e within a block
            {
                // get operands of instr and check all instructions before this for stores into this
                // operand

                // errs() << instr << '\n';
                switch (instr.getOpcode()) {
                case Instruction::Add:
                case Instruction::Sub:
                case Instruction::Mul:
                case Instruction::UDiv:
                case Instruction::SDiv:
                case Instruction::URem:
                case Instruction::Shl:
                case Instruction::LShr:
                case Instruction::AShr:
                case Instruction::And:
                case Instruction::Or:
                case Instruction::Xor:
                case Instruction::SRem: {
                    int numOfOperands = instr.getNumOperands();
                    // errs() << numOfOperands << '\n';
                    int isThisExprKilled = 0;
                    // for each operand in e
                    for (int idx = 0; idx < numOfOperands; idx++) {
                        Value *currentOperand = instr.getOperand(idx);

                        // check previous instructions in that block
                        BasicBlock *bb = &BB;
                        for (BasicBlock::iterator k = bb->begin(); k != bb->end(); k++) {
                            if (instr.isIdenticalTo(&*k)) {

                                break;
                            } else {

                                if (k->getOpcode() == Instruction::Load) {
                                    LoadInst *li = dyn_cast<LoadInst>(k);
                                    Value *loadedFrom = li->getPointerOperand();
                                    BasicBlock::iterator kTemp = bb->begin();
                                    while (kTemp != k) {
                                        if (kTemp->getOpcode() == Instruction::Store) {
                                            StoreInst *si1 = dyn_cast<StoreInst>(kTemp);
                                            if (si1->getOperand(1) == loadedFrom) {
                                                isThisExprKilled = 1;
                                                break;
                                            }
                                        }
                                        kTemp++;
                                    }
                                }
                            }
                        }
                    }

                    if (isThisExprKilled == 0) {
                        if (xUses.find(BB.getName()) ==
                            xUses.end()) // cannot find current BB in xUses so add new entry
                        {
                            std::set<Instruction *> instructionToBeAdded;
                            instructionToBeAdded.insert(&instr);
                            xUses[BB.getName()] = instructionToBeAdded;
                        } else {
                            // add this instruction to already existing BB entry in xUses
                            std::set<Instruction *> instructionToBeAdded = xUses[BB.getName()];
                            instructionToBeAdded.insert(&instr);
                            xUses[BB.getName()] = instructionToBeAdded;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
                }
            }
        }
    }

    void printSets(std::map<StringRef, std::set<Instruction *>> &mySet, const char *currSet) {
        errs() << "*************\n";
        errs() << "Set " << currSet << '\n';
        errs() << "*************\n";

        for (auto &pair : mySet) {
            errs() << pair.first << '\n';
            std::set<Instruction *> values = pair.second;
            for (Instruction *allInstrInBB : values) {
                errs() << *allInstrInBB << '\n';
            }
            errs() << "end of block" << '\n';
            errs() << '\n';
        }
    }

    // If expression e is of the form x=a op b, for each of the operands a and b, only look after e
    // in that BB. Get loads from block starting till e, and their corresponding sources. For each
    // load, look through all stores for matching destination of store after e If found then e is
    // killed and does not go into gens
    void fillGens(Function &F, std::map<StringRef, std::set<Instruction *>> &gens) {

        for (BasicBlock &BB : F) {

            for (auto &instr : BB) {
                // errs() << instr << '\n';
                //  get operands of instr and check all instructions before this for stores into
                //  this operand

                switch (instr.getOpcode()) {

                case Instruction::Add:
                case Instruction::Sub:
                case Instruction::Mul:
                case Instruction::UDiv:
                case Instruction::SDiv:
                case Instruction::URem:
                case Instruction::Shl:
                case Instruction::LShr:
                case Instruction::AShr:
                case Instruction::And:
                case Instruction::Or:
                case Instruction::Xor:
                case Instruction::SRem: {
                    int numOfOperands = instr.getNumOperands();
                    int isThisExprKilled = 0;
                    for (int idx = 0; idx < numOfOperands; idx++) {
                        Value *currentOperand = instr.getOperand(idx);

                        BasicBlock *bb = &BB;
                        BasicBlock::iterator k = bb->begin();
                        // search through all instructions after the current instruction
                        // skip till you find current instruction and then do k++ to look at
                        // successor instructions within that BB
                        while (!instr.isIdenticalTo(&*k)) {
                            k++;
                        }
                        if (k != bb->end()) {
                            k++;
                            BasicBlock::iterator kTemp1 = bb->begin();
                            while (kTemp1 != k) {
                                if (kTemp1->getOpcode() == Instruction::Load) {
                                    LoadInst *li = dyn_cast<LoadInst>(kTemp1);
                                    Value *loadedFrom = li->getPointerOperand();

                                    for (BasicBlock::iterator kTemp = k; kTemp != bb->end();
                                         kTemp++) {
                                        if (kTemp->getOpcode() == Instruction::Store) {
                                            StoreInst *si1 = dyn_cast<StoreInst>(kTemp);
                                            if (si1->getOperand(1) == loadedFrom) {
                                                isThisExprKilled = 1;
                                                break;
                                            }
                                        }
                                    }
                                }
                                kTemp1++;
                            }
                        }
                    }

                    if (isThisExprKilled == 0) {
                        if (gens.find(BB.getName()) ==
                            gens.end()) // if the current BB doesnt have entry in gens, create it
                        {
                            std::set<Instruction *> instructionToBeAdded;
                            instructionToBeAdded.insert(&instr);
                            gens[BB.getName()] = instructionToBeAdded;
                        } else { // current BB exists in gens so get it and add e to it
                            std::set<Instruction *> instructionToBeAdded = gens[BB.getName()];
                            instructionToBeAdded.insert(&instr);
                            gens[BB.getName()] = instructionToBeAdded;
                        }
                    }
                    break;
                }

                default: {
                    break;
                }
                }
            }
        }
    }

    // for each expression e of the type x op y, get its operands and search in full function if
    // there is a store to any of them. If yes, then add to kills set if operand is of type load
    // instruction, then get it's operand and search for store with same dest if not of type load
    // instruction but of type mul,sub,add then get it's operands and search for load. Then get
    // load's operand and search for corresponding store with same dest. If found, enter into kills
    // set
    void fillKills(Function &F, std::map<StringRef, std::set<Instruction *>> &kills) {
        for (BasicBlock &BB : F) {

            for (auto &instr : BB) {
                // errs() << instr << '\n';
                switch (instr.getOpcode()) {

                case Instruction::Add:
                case Instruction::Sub:
                case Instruction::Mul:
                case Instruction::UDiv:
                case Instruction::SDiv:
                case Instruction::URem:
                case Instruction::Shl:
                case Instruction::LShr:
                case Instruction::AShr:
                case Instruction::And:
                case Instruction::Or:
                case Instruction::Xor:
                case Instruction::SRem: {
                    int numOfOperands = instr.getNumOperands();
                    for (int idx = 0; idx < numOfOperands; idx++) {
                        Value *currentOperand = instr.getOperand(idx);
                        Instruction *getOperandInstr = dyn_cast<Instruction>(
                            currentOperand); // crashing if its a constant so check if not nullptr

                        if (getOperandInstr != nullptr &&
                            getOperandInstr->getOpcode() ==
                                Instruction::Load) // get load for each operand
                        {
                            Value *loadOperand = getOperandInstr->getOperand(0);

                            for (BasicBlock &BB1 : F) {

                                for (auto &instr1 : BB1) {
                                    if (instr1.getOpcode() == Instruction::Store) {
                                        Value *storeDest = instr1.getOperand(1);
                                        if (storeDest ==
                                            loadOperand) // enter e into kill set of BB1 if store
                                                         // dest is same as load source
                                        {
                                            if (kills.find(BB1.getName()) == kills.end()) {
                                                std::set<Instruction *> instructionToBeAdded;
                                                instructionToBeAdded.insert(&instr);
                                                kills[BB1.getName()] = instructionToBeAdded;
                                            } else {
                                                std::set<Instruction *> instructionToBeAdded =
                                                    kills[BB1.getName()];
                                                instructionToBeAdded.insert(&instr);
                                                kills[BB1.getName()] = instructionToBeAdded;
                                            }
                                            break; // go to next BB and check
                                        }
                                    }
                                }
                            }
                        } else if (getOperandInstr != nullptr &&
                                   (getOperandInstr->getOpcode() == Instruction::Mul ||
                                    getOperandInstr->getOpcode() == Instruction::Add ||
                                    getOperandInstr->getOpcode() == Instruction::Sub)) {
                            int numOfOperands1 = getOperandInstr->getNumOperands();
                            for (int idx1 = 0; idx1 < numOfOperands1; idx1++) {
                                Value *currentOperand1 = getOperandInstr->getOperand(idx1);
                                Instruction *getOperandInstr1 =
                                    dyn_cast<Instruction>(currentOperand1);
                                if (getOperandInstr1 != nullptr &&
                                    getOperandInstr1->getOpcode() == Instruction::Load) {
                                    Value *loadOperand1 = getOperandInstr1->getOperand(0);

                                    for (BasicBlock &BB2 : F) {

                                        for (auto &instr2 : BB2) {
                                            if (instr2.getOpcode() == Instruction::Store) {

                                                Value *storeDest1 = instr2.getOperand(1);
                                                if (storeDest1 ==
                                                    loadOperand1) // enter e into kill set of BB1 if
                                                                  // store dest is same as load
                                                                  // source
                                                {
                                                    if (kills.find(BB2.getName()) == kills.end()) {
                                                        std::set<Instruction *>
                                                            instructionToBeAdded1;
                                                        instructionToBeAdded1.insert(&instr);
                                                        kills[BB2.getName()] =
                                                            instructionToBeAdded1;
                                                    } else {
                                                        std::set<Instruction *>
                                                            instructionToBeAdded1 =
                                                                kills[BB2.getName()];
                                                        instructionToBeAdded1.insert(&instr);
                                                        kills[BB2.getName()] =
                                                            instructionToBeAdded1;
                                                    }
                                                    break; // go to next BB and check
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
                default: {
                    break;
                }
                }
            }
        }
    }

    void fillCandidates(std::vector<StringRef> hotNodes,
                        std::map<StringRef, std::set<Instruction *>> &xUses,
                        std::set<Instruction *> &candidates) {
        for (auto &xUseBB : xUses) {
            if (std::find(hotNodes.begin(), hotNodes.end(), xUseBB.first) != hotNodes.end()) {
                std::set_union(candidates.begin(), candidates.end(), xUseBB.second.begin(),
                               xUseBB.second.end(), std::inserter(candidates, candidates.end()));
            }
        }
    }

    void printCandidates(std::set<Instruction *> candidates) {
        errs() << "***********\nCandidates\n";
        for (Instruction *candidate : candidates) {
            errs() << *candidate << '\n';
        }
    }

    void fillRemovable(std::set<Instruction *> candidates,
                       std::map<StringRef, std::set<Instruction *>> gens,
                       std::map<StringRef, std::set<Instruction *>> kills,
                       std::map<StringRef, std::set<Instruction *>> xUses,
                       std::vector<std::pair<StringRef, StringRef>> ingressEdges,
                       std::map<StringRef, std::set<Instruction *>> &avouts,
                       std::map<StringRef, std::set<Instruction *>> &avins,
                       std::map<StringRef, std::set<Instruction *>> &removables,
                       std::vector<StringRef> hotNodes, Function &F) {
        // Init AVOUT(b) to 0 for all basic blocks X
        for (BasicBlock &BB : F) {
            std::set<Instruction *> empty_set;
            avouts[BB.getName()] = empty_set;
        }

        int change = 1;
        while (change) {
            change = 0;
            for (BasicBlock &BB : F) {
                auto bb_name = BB.getName();
                std::set<Instruction *> old_avout = avouts[bb_name];
                std::set<Instruction *> new_avin;
                bool new_avin_initialized = false;

                // AVIN(b) = INTERSECTION(Candidates if ingress edge, otherwise AVOUT(p))
                for (BasicBlock *predecessor : predecessors(&BB)) {
                    std::set<Instruction *> intersect;
                    std::pair<StringRef, StringRef> edge =
                        std::make_pair(predecessor->getName(), BB.getName());
                    if (std::find(ingressEdges.begin(), ingressEdges.end(), edge) !=
                        ingressEdges.end()) {
                        intersect = candidates;
                    } else {
                        intersect = avouts[predecessor->getName()];
                    }
                    if (new_avin_initialized) {
                        std::set_intersection(new_avin.begin(), new_avin.end(), intersect.begin(),
                                              intersect.end(),
                                              std::inserter(new_avin, new_avin.end()));
                    } else {
                        new_avin = intersect;
                        new_avin_initialized = true;
                    }
                }

                // AVOUT(b) = (AVIN(b) - KILL(b)) U GEN(b)
                std::set<Instruction *> new_avout;
                std::set<Instruction *> gen = gens[bb_name];
                std::set<Instruction *> kill = kills[bb_name];
                std::set_difference(new_avin.begin(), new_avin.end(), kill.begin(), kill.end(),
                                    std::inserter(new_avout, new_avout.end()));
                std::set_union(new_avout.begin(), new_avout.end(), gen.begin(), gen.end(),
                               std::inserter(new_avout, new_avout.end()));

                // update needin and needout
                avins[bb_name] = new_avin;
                avouts[bb_name] = new_avout;
                if (old_avout != new_avout) {
                    change = 1;
                }
            }
        }

        for (BasicBlock &BB : F) {
            auto bb_name = BB.getName();
            if (std::find(hotNodes.begin(), hotNodes.end(), bb_name) != hotNodes.end()) {
                std::set<Instruction *> removable;
                std::set<Instruction *> avin = avins[bb_name];
                std::set<Instruction *> xUse = xUses[bb_name];
                std::set_intersection(avin.begin(), avin.end(), xUse.begin(), xUse.end(),
                                      std::inserter(removable, removable.end()));
                removables[bb_name] = removable;
            }
        }
    }

    bool runOnFunction(Function &F) override {
        std::map<StringRef, double> freqs;
        std::vector<StringRef> hotNodes;
        std::vector<StringRef> coldNodes;
        std::vector<std::pair<StringRef, StringRef>> hotEdges;
        std::vector<std::pair<StringRef, StringRef>> coldEdges;
        std::vector<std::pair<StringRef, StringRef>> ingressEdges;

        std::map<StringRef, std::set<Instruction *>> xUses;
        std::map<StringRef, std::set<Instruction *>> gens;
        std::map<StringRef, std::set<Instruction *>> kills;
        std::set<Instruction *> candidates;
        std::map<StringRef, std::set<Instruction *>> avins;
        std::map<StringRef, std::set<Instruction *>> avouts;
        std::map<StringRef, std::set<Instruction *>> removables;

        int maxCount = calculateHotColdNodes(F, freqs, hotNodes, coldNodes);
        calculateHotColdEdges(F, freqs, hotEdges, coldEdges, maxCount);

        printHotColdNodes(hotNodes, coldNodes);
        printHotColdEdges(hotEdges, coldEdges);

        calculateIngressEdges(coldEdges, hotNodes, coldNodes, ingressEdges);
        printIngressEdges(ingressEdges);

        fillXUses(F, xUses);
        printSets(xUses, "xUses");

        fillGens(F, gens);

        printSets(gens, "Gens");

        fillKills(F, kills);
        printSets(kills, "Kills");

        fillCandidates(hotNodes, xUses, candidates);
        printCandidates(candidates);

        fillRemovable(candidates, gens, kills, xUses, ingressEdges, avouts, avins, removables,
                      hotNodes, F);
        printSets(avins, "avins");
        printSets(avouts, "avouts");
        printSets(removables, "Removables");

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
