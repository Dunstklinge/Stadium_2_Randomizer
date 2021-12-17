#include "stdafx.h"
#include "GameContext.h"


double GameContext::LowestMoveRating() const
{
    if (isnan(calcCache.lowestMoveRating)) {

        using GameInfo::Move;
        double lowest = RateMove(moveList[0]);
        for (const Move* it = moveList + 1; it != moveListEnd; it++) {
            double rate = RateMove(*it);
            if (rate < lowest) lowest = rate;
        }

        calcCache.lowestMoveRating = lowest;
    }
    return calcCache.lowestMoveRating;
}

double GameContext::HighestMoveRating() const
{
    if (isnan(calcCache.highestMoveRating)) {

        using GameInfo::Move;
        double highest = RateMove(moveList[0]);
        for (const Move* it = moveList + 1; it != moveListEnd; it++) {
            double rate = RateMove(*it);
            if (rate > highest) highest = rate;
        }

        calcCache.highestMoveRating = highest;
    }
    return calcCache.highestMoveRating;
}

double GameContext::LowestBst() const
{
    if (isnan(calcCache.lowestBst)) {

        using GameInfo::Pokemon;
        double lowest = pokeList[0].CalcBST();
        for (const Pokemon* it = pokeList + 1; it != pokeListEnd; it++) {
            double bst = it->CalcBST();
            if (bst < lowest) lowest = bst;
        }

        calcCache.lowestBst = lowest;
    }
    return calcCache.lowestBst;
}

double GameContext::HighestBst() const
{
    if (isnan(calcCache.highestBst)) {

        using GameInfo::Pokemon;
        double highest = pokeList[0].CalcBST();
        for (const Pokemon* it = pokeList + 1; it != pokeListEnd; it++) {
            double bst = it->CalcBST();
            if (bst > highest) highest = bst;
        }

        calcCache.highestBst = highest;
    }
    return calcCache.highestBst;
}
