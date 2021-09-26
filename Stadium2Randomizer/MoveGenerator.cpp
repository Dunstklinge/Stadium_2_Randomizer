#include "stdafx.h"
#include "MoveGenerator.h"

#include "GlobalConfigs.h"
#include "GlobalRandom.h"

MoveGenerator::MoveGenerator()
{
}


MoveGenerator::~MoveGenerator()
{
}

GameInfo::Move MoveGenerator::Generate(const GameInfo::Move& from)
{
	GameInfo::Move ret = from;
	int oldRating = RateMove(from);
	
	if (randomType) {
		std::uniform_int_distribution<int> dist(0, 16);
		int type = dist(Random::Generator);
		if (type > GameInfo::ROCK) type += 1;
		if (type > GameInfo::STEEL) type += 0x0A;
		ret.type = type;
	}

	if (randomSecondary && (randomBp && ret.basePower > 0 || !randomBp && ret.basePower > 1)) {
		std::uniform_int_distribution<int> percentDist(0, 99);
		int rand = percentDist(Random::Generator);
		if (ret.effectId == 0 && rand < gainSecondaryEffectChance || ret.effectId != 0 && rand >= looseSecondaryEffectChance) {
			if (randomEffectChance) {
				std::uniform_int_distribution<int> dist(0, Aux().secChanceEffects.size() + Aux().secNonChanceEffects.size() - 1);
				int index = dist(Random::Generator);
				if (index < Aux().secChanceEffects.size()) {
					ret.effectId = Aux().secChanceEffects[index];
					ret.effectChance = ecDist(Random::Generator);
				}
				else {
					ret.effectId = Aux().secNonChanceEffects[index - Aux().secChanceEffects.size()];
					ret.effectChance = 0;
				}
			}
			else if (GameInfo::MoveEffectInfos[ret.effectId].flags & GameInfo::MoveEffectInfo::CHANCE_PARAMETER) {
				std::uniform_int_distribution<int> dist(0, Aux().secChanceEffects.size() - 1);
				ret.effectId = Aux().secChanceEffects[dist(Random::Generator)];
			}
			else {
				std::uniform_int_distribution<int> dist(0, Aux().secNonChanceEffects.size() - 1);
				ret.effectId = Aux().secNonChanceEffects[dist(Random::Generator)];
			}
		}
		else if (ret.effectId != 0 && rand < looseSecondaryEffectChance) {
			ret.effectId = 0;
			ret.effectChance = 0;
		}
	}
	if (randomStatusMoveEffect && ret.basePower == 0) {
		std::uniform_int_distribution<int> dist(0, Aux().primEffects.size() - 1);
		ret.effectId = Aux().primEffects[dist(Random::Generator)];
	}


	if (randomAcc) {
		int acc = accDist(Random::Generator);
		ret.accuracy = acc;
	}
	if (randomPp) {
		int pp = ppDist(Random::Generator);
		ret.pp = pp;
	}
	if (randomBp && (GameInfo::MoveEffectInfos[ret.effectId].flags & GameInfo::MoveEffectInfo::ATTACK_EFFECT)) {
		if (stayCloseToBp) {
			int diff = ret.basePower * 0.1;
			bpDist.SetMinMax(ret.basePower - diff, ret.basePower + diff);
			int bp = bpDist(Random::Generator);
			ret.basePower = ret.basePower + bp;
		}
		else {
			int bp = bpDist(Random::Generator);
			ret.basePower = bp;
		}
	}

	if (balanced) {
		//potentially scewing the rng if balance was requested
		bool fixed = false;

		MoveEffectValue::Value* effectValue = nullptr;
		auto it = GlobalConfig::MoveEffectValues.data.find((GameInfo::MoveEffect)ret.effectId);
		if (it != GlobalConfig::MoveEffectValues.data.end()) {
			effectValue = &it->second;
			fixed = ((effectValue->affects & MoveEffectValue::Value::USE_VALUE));
		}
		
		if (!fixed) {
			//try multiple times if more options are available
			int nTries = randomAcc * 3 + randomPp * 2 + (randomSecondary || randomStatusMoveEffect) * 5 + randomBp * 5;
			while (nTries-- > 0) {
				int newRating = RateMove(ret);
				double changePP = 0, changeBP = 0, changeAcc = 0, changeEffectChance = 0;
				if (newRating < 60 || newRating > 150) {
					bool reduce = newRating > 150;
					if (effectValue && randomEffectChance && (GameInfo::MoveEffectInfos[from.effectId].flags & GameInfo::MoveEffectInfo::CHANCE_PARAMETER)) {
						if (effectValue->affects & effectValue->FLAT_BONUS)
							changeEffectChance = effectValue->a;
						else if (effectValue->affects & effectValue->MULT_BP)
							changeEffectChance = effectValue->m * ret.basePower;
					}
					if (reduce) {
						if (randomPp && ret.pp > 20) {
							if (ret.pp > 10)
								changePP = 20;
							else
								changePP = 5;
						}
						if (randomAcc && ret.accuracy > 200) {
							changeAcc = std::clamp(ret.accuracy - 200, 0, 40);
						}
						if (randomBp) {
							changeBP = 30;
						}
					}
					else/*(strengthen)*/ {
						if (randomPp && ret.pp < 15) {
							if (ret.pp > 10)
								changePP = 10;
							else
								changePP = (10 - ret.pp) * 10;
						}
						if (randomAcc && ret.accuracy < 255) {
							changeAcc = 20;
						}
						if (randomBp) {
							changeBP = 30;
						}
					}
					std::uniform_real_distribution dist(0.0, changePP + changeBP + changeAcc + changeEffectChance);
					double choice = dist(Random::Generator);
					double ifTotal = 0;
					if (choice < (ifTotal += changePP)) {
						double changePercent = reduce ? Random::GetDouble(0.6, 0.9) : Random::GetDouble(1.2, 1.5);
						ret.pp = std::clamp<uint8_t>(ret.pp * changePercent, 2, 63);
					}
					else if (choice < (ifTotal += changeBP)) {
						double changePercent = reduce ? Random::GetDouble(0.7, 0.9) : Random::GetDouble(1.2, 2.0);
						ret.basePower = std::clamp<uint8_t>(ret.basePower * changePercent, 10, 200);
					}
					else if (choice < (ifTotal += changeAcc)) {
						double changePercent = reduce ? Random::GetDouble(0.7, 0.9) : Random::GetDouble(1.1, 1.4);
						ret.accuracy = std::clamp<uint8_t>(ret.accuracy * changePercent, 50, 255);
					}
					else {
						double changePercent = reduce ? Random::GetDouble(0.4, 0.9) : Random::GetDouble(1.2, 2.0);
						ret.effectChance = std::clamp<uint8_t>(ret.effectChance * changePercent, 2, 63);
					}
				}
				else break;
			}
		}
	}

	return ret;
}

MoveGenerator::AuxMaps& MoveGenerator::Aux()
{
	static AuxMaps auxData;
	return auxData;
}

MoveGenerator::AuxMaps::AuxMaps()
{
	using namespace GameInfo;
	

	for (int i = 0; i < _countof(MoveEffectInfos); i++) {
		MoveEffect id = (MoveEffect)i;
		const MoveEffectInfo& info = MoveEffectInfos[i];
		if (info.flags & MoveEffectInfo::ATTACK_EFFECT) {
			if (info.flags & MoveEffectInfo::CHANCE_PARAMETER) {
				secChanceEffects.push_back(id);
			}
			else {
				secNonChanceEffects.push_back(id);
			}
		}
		if (info.flags & MoveEffectInfo::STATUS_EFFECT) {
			primEffects.push_back(id);
		}
	}
}
