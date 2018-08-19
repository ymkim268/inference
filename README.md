# inference

## problem statement:
* first order logic inference based on resolution
* determine true/false for given queries and knowledge base
* for each query, determine if it can be inferred from knwoledge base or not

## input.txt format:
* 1st line: NQ num of queries
* NQ lines: queries
* NQ + 1 line: NS num of given sentences in knowledge base
* NS lines: knowledge base sentences

## output.txt format:
* NQ lines: true/false answer to the NQ queries

## note:
* each query will be in Pred(Const) or `~`Pred(Const) format
* variables are lowercase, predicates are case-sensitive that begin with uppercase
* each predicate at least one argument
* no existential quantifier, so no need for Skolemization to elimate them