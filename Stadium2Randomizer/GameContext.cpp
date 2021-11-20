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
