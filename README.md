A few notes:
1. Currently xUses, Gens, Kills works only for expression type x op y where op can be any operator like shift right/left, arithmetic, etc. As per discussion, I have ignored sef updates, stores, loads, branch and branch ends. So far no special handling for x=Constant and compares. x and/or y can be modified before or after e.
2. xUses: Current code looks within BB and gets operands of e. It then looks for loads before e and gets the source of loads. It then checks if there are any stores into this source of loads before e. If yes, then e is not added to xUses set.
3. Gens: Same as above except that it looks for loads before e and stores after e.
4. Kills: If e is not in gens and not in xUses, then it has been added to kills.
5. Maybe helpful if we can first get this basic case working end to end (x op y).
