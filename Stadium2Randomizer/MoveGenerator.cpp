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

static constexpr const char* EffectChanceWildcard = "<may>";
static constexpr const char* EffectDescriptions[] = {
	"No added effect."/* NO_EFFECT */,
	"Inflicts Sleep."/* SLEEP_STATUS */,
	"<may> cause Poisoning."/* POISON_CHANCE */,
	"Drains Health."/* DRAIN_HEALTH */,
	"<may> cause Burn."/* BURN_CHANCE */,
	"<may> cause Freeze."/* FREEZE_CHANCE */,
	"<may> cause paralysis."/* PARALYSIS_CHANCE */,
	"The user faints."/* KABOOM */,
	"Drains Health, but only works on sleeping targets."/* DREAM_EATER_EFFECT */,
	"Strikes back with the opponent's last move. Low priority."/* MIRROR_MOVE_EFFECT */,
	"Rasies ATTACK."/* ATTACK_PLUS_ONE */,
	"Raises DEFENSE."/* DEFENSE_PLUS_ONE */,
	"Raises SPEED."/* SPEED_PLUS_ONE */,
	"Raises Special Attack."/* SPA_PLUS_ONE */,
	"Raises SPCL.DEF."/* SPD_PLUS_ONE */,
	"Raises Accuracy."/* ACCURACY_PLUS_ONE */,
	"Raises Evasion."/* EVASION_PLUS_ONE */,
	"Cant miss."/* ALWAYS_HITS */,
	"Lowers Opponents ATTACK."/* ATTACK_MINUS_ONE */,
	"Lowers Opponents DEFENSE."/* DEFENSE_MINUS_ONE */,
	"Lowers Opponents SPEED."/* SPEED_MINUS_ONE */,
	"Lowers Opponents Special Attack."/* SPA_MINUS_ONE */,
	"Lowers Opponents SPCL.DEF."/* SPD_MINUS_ONE */,
	"Lowers Opponents Accuracy."/* ACCURACY_MINUS_ONE */,
	"Lowers Opponents Evasion."/* EVASION_MINUS_ONE */,
	"Eleminates all stat changes."/* HAZE_EFFECT */,
	"Waits 2-3 turns; hits back double."/* BIDE_EFFECT */,
	"Works 2-3 turns, then confuses user."/* THRASHING_ABOUT */,
	"Phases."/* PHASING */,
	"Hits 2-5 times."/* MULTI_STRIKE */,
	"Change user's type to a move's type. "/* CONVERSION_EFFECT */,
	"<may> Flinch."/* FLINCH_CHANCE */,
	"Restores 50% of max health."/* RECOVER_HEALTH */,
	"Inflicts bad poisoning."/* BAD_POISON_STATUS */,
	"Scatters coins."/* PAY_DAY_MONEY */,
	"Halfs incoming Special Damage on your team for five turns."/* LIGHT_SCREEN_EFFECT */,
	"<may> cause Freeze, Burn or paralysis"/* TRI_ATTACK_EFFECT */,
	"???"/* UNKNOWN_EFFECT_1 */,
	"Always K.Os if it hits."/* ONE_HIT_KO */,
	"Move Attacks on the second turn. Increased critical hit ratio."/* RAZOR_WIND_EFFECT */,
	"Always inflicts damage equal to half the opponents current health."/* SUPER_FANG_EFFECT */,
	"Damage ignores attacks and defenses."/* DAMAGE_IGNORES_STATS */,
	"Traps the target and does 1/16 health as damage per turn, for 2-5 turns."/* BINDING */,
	"???"/* UNKNOWN_EFFECT_2 */,
	"Hits twice."/* HITS_TWICE */,
	"Inflicts recoil damage if it misses."/* RECOIL_WHEN_MISSING */,
	"Prevents stat reduction."/* MIST_EFFECT */,
	"Increaes critical hit rate."/* FOCUS_ENERGY_EFFECT */,
	"Does recoil damage to user."/* RECOIL */,
	"Inflicts Confusion."/* CONFUSE */,
	"Sharply raises ATTACK."/* ATTACK_PLUS_TWO */,
	"Sharply raises DEFENSE."/* DEFENSE_PLUS_TWO */,
	"Sharply raises SPEED."/* SPEED_PLUS_TWO */,
	"Sharply raises SPCL.ATK."/* SPA_PLUS_TWO */,
	"Sharply raises SPCL.DEF."/* SPD_PLUS_TWO */,
	"Sharply raises Accuracy."/* ACCURACY_PLUS_TWO */,
	"Sharply raises Evasion."/* EVASION_PLUS_TWO */,
	"Transforms into opponent."/* TRANSFORM_EFFECT */,
	"Harshly lowers Opponents ATTACK."/* ATTACK_MINUS_TWO */,
	"Harshly lowers Opponents DEFENSE."/* DEFENSE_MINUS_TWO */,
	"Harshly lowers Opponents SPEED."/* SPEED_MINUS_TWO */,
	"Harshly lowers Opponents SPCL.ATK."/* SPA_MINUS_TWO */,
	"Harshly lowers Opponents SPCL.DEF."/* SPD_MINUS_TWO */,
	"Harshly lowers Opponents Accuracy."/* ACCURACY_MINUS_TWO */,
	"Harshly lowers Opponents Evasion."/* EVASION_MINUS_TWO */,
	"Harshly lowers Opponents Attack."/* REFLECT_EFFECT */,
	"Inflicts Poison."/* POISON_STATUS */,
	"Inflicts Paralysis."/* PARALYSIS_STATUS */,
	"<may> lower Opponents ATTACK."/* ATTACK_MINUS_ONE_CHANCE */,
	"<may> lower Opponents DEFENSE."/* DEFENSE_MINUS_ONE_CHANCE */,
	"<may> lower Opponents SPEED."/* SPEED_MINUS_ONE_CHANCE */,
	"<may> lower Opponents SPCL.ATK."/* SPA_MINUS_ONE_CHANCE */,
	"<may> lower Opponents SPCL.DEF."/* SPD_MINUS_ONE_CHANCE */,
	"<may> lower Opponents Accuracy."/* ACCURACY_MINUS_ONE_CHANCE */,
	"<may> lower Opponents Evasion."/* EVASION_MINUS_ONE_CHANCE */,
	"Move Attacks on the second turn. Increased critical hit ratio."/* SKY_ATTACK_EFFECT */,
	"<may> cause Confusion."/* CONFUSE_CHANCE */,
	"Hits twice. <may> cause Poison."/* HITS_TWICE_POISON_CHANCE */,
	"???"/* UNKNOWN_EFFECT_3 */,
	"Uses 1/4 of the user's maximum HP to create a substitute that takes the opponent's attacks."/* SUBSTITUTE_EFFECT */,
	"Recharges after use."/* RECHARGE_AFTER_USE */,
	"A non-stop attack move. The user's ATTACK power increases every time it sustains damage."/* RAGE_EFFECT */,
	"A move for learning one of the opponent's moves, for use during that battle only."/* MIMIC_EFFECT */,
	"Invokes a random move."/* METRONOME_EFFECT */,
	"Steals HP from the foe on every turn."/* LEECH_SEED_EFFECT */,
	"Does nothing."/* SPLASH_EFFECT */,
	"Disables the last move used by the opponent."/* DISABLE_EFFECT */,
	"Does damage equal to the users damage, ignoring attack and defense."/* DAMAGE_BY_LEVEL_EFFECT */,
	"Does random damage depending on the users level, ignoring attack and defense."/* PSYWAVE_EFFECT */,
	"Returns a physical blow double."/* COUNTER_EFFECT */,
	"Forces the target to continue to use the move it used last for the next two to six turns."/* ENCORE_EFFECT */,
	"A move that adds the HPs of the user and the target, then divides the total between them."/* PAIN_SPLIT_EFFECT */,
	"Has a one-in-three chance of making the target flinch. Can be used only by a sleeping POKÈMON."/* SNORE_EFFECT */,
	"A move that changes the user's type into one that is resistant to the opponent's last move."/* CONVERSION_TWO_EFFECT */,
	"The user's next attack will never miss."/* NEXT_MOVE_HITS */,
	"A move that allows the user to copy a move used by an opponent and learn that move."/* SKETCH_EFFECT */,
	"???"/* UNKNOWN_EFFECT_4 */,
	"Randomly chooses one of the user's moves. Can be used only by a sleeping POKÈMON."/* SLEEP_TALK_EFFECT */,
	"A move that causes the opponent to faint if the user faints after using this move."/*DESTINY_BOND_EFFECT * / ,
	"An attack that increases in power if the user's HP is low. It could turn the tide in battle."/* BP_BASED_ON_HEALTH */,
	"A vengeful move that slightly reduces the PP of the opponent's last move."/* SPITE_EFFECT */,
	"A move that adjusts ATTACK so that the foe has one HP left.  Can't cause the foe to faint."/* FALSE_SWIPE_EFFECT */,
	"Eliminates all status problems for the entire party."/* HEAL_BELL_EFFECT */,
	"High priority."/* HIGH_PRIORITY */,
	"Hits up to 3 times; each hit inflicts increasing damage."/* TRIPLE_KICK_EFFECT */,
	"<may> steal an item held by the target."/* ITEM_STEAL_CHANCE */,
	"Prevent the target from escaping until the attacking POKÈMON is gone."/* CANT_ESCAPE */,
	"A move that makes a sleeping target have bad dreams. The victim will steadily lose HP."/* NIGHTMARE_EFFECT */,
	"<may> cause Burn. If the user is frozen, using this move will thaw it out."/* FLAME_WHEEL_EFFECT */,
	"Raises ATTACK and DEFENSE at the expense of SPEED. It works differently for the GHOST type."/* CURSE_EFFECT */,
	"???"/* UNKNOWN_EFFECT_5 */,
	"Completely foils an opponent's attack. If used consecutively, its success rate decreases."/* PROTECT_EFFECT */,
	"An attack that scatters SPIKES and injures opponent POKÈMON when they switch."/* SPIKES_EFFECT */,
	"Neutralizes the foe's evasive- ness. Also makes the GHOST type vulnerable to physical attacks."/* FORESIGHT_EFFECT */,
	"Causes both the user and the opponent to faint in three turns."/* PERISH_SONG_EFFECT */,
	"An attack that creates a sand- storm. The effect causes damage to both combatants."/* SANDSTORM_EFFECT */,
	"Always leaves the user with at least one HP. Success rate decreases if used repeatedly."/* ENDURE_EFFECT */,
	"Attacks for five turns, growing more powerful each time. Power returns to normal if it misses."/* ROLLOUT_EFFECT */,
	"A move that infuriates and  confuses the opponent, but it increases the target's ATTACK."/* SWAGGER_EFFECT */,
	"Grows more powerful with every hit. Returns to normal if it misses."/* FURY_CUTTER_EFFECT */,
	"Infatuates targets, making it hard for them to attack foes of the opposite gender."/* ATTRACT_EFFECT */,
	"The tamer the user, the more powerful the move."/* RETURN_EFFECT */,
	"A move that inflicts major damage but may restore the target's HP."/* PRESENT_EFFECT */,
	"The more the user dislikes its trainer, the more powerful the move."/* FRUSTRATION_EFFECT */,
	"A mystic power that protects the user from status problems for five turns."/* SAFEGUARD_EFFECT */,
	"<may> cause Burn. If the user is frozen, using this move will thaw it out."/* SACRED_FIRE_EFFECT */,
	"The power of the attack will vary each time it is used."/* MAGNITUDE_EFFECT */,
	"A move that transfers the user's status changes to the next POKÈMON when switching."/* BATON_PASS_EFFECT */,
	"Inflicts major damage if the target switches out in the same turn."/* PURSUIT_EFFECT */,
	"User escapes from restraining moves."/* RAPID_SPIN_EFFECT */,
	"???"/* UNKNOWN_EFFECT_6 */,
	"???"/* UNKNOWN_EFFECT_7 */,
	"Restores Health depending on weather and time of day."/* MORNING_SUN_EFFECT */,
	"Restores Health depending on weather and time of day."/* SYNTHESIS_EFFECT */,
	"Restores Health depending on weather and time of day."/* MOONLIGHT_EFFECT */,
	"A peculiar move that changes type and power depending on the POKÈMON using it."/* HIDDEN_POWER_EFFECT */,
	"Summons rain for five turns. While it is raining, the power of WATER-type moves increases.@"/* RAIN_DANCE_EFFECT */,
	"Makes the weather sunny for five turns. When sunny, FIRE- type moves are more powerful."/* SUNNY_DAY_EFFECT */,
	"<may> raise users ATTACK."/* ATTACK_PLUS_ONE_CHANCE */,
	"<may> raise users DEFENSE."/* DEFENSE_PLUS_ONE_CHANCE */,
	"<may> cause Rainbow Boost."/* RAINBOW_CHANCE */,
	"???"/* UNKNOWN_EFFECT_7 */,
	"A move that allows the user to consume half of its own HP in order to maximize its ATTACK."/* BELLY_DRUM_EFFECT */,
	"Self-hypnosis move that copies the foe's stats changes, then applies them to the user."/* PSYCH_UP_EFFECT */,
	"The foe will receive double the damage the user sustained from a special attack. High accuracy."/* MIRROR_COAT_EFFECT */,
	"In the first turn, the attacker tucks its head. The next turn, it head-butts at full steam."/* SKULL_BASH_EFFECT */,
	"<may> Flinch. Always hits airborne targets."/* TWISTER_EFFECT */,
	"Always hits targets underground."/* HITS_DIGGING */,
	"Strikes the target two turns after it is used."/* FUTURE_SIGHT_EFFECT */,
	"Always hits airborne targets."/* HITS_FLYING */,
	"Increased strength and accuracy against minimzed targets."/* HITS_MINIMIZED */,
	"Energy is absorbed in the first turn, then fired on the next."/* SOLAR_BEAM_EFFECT */,
	"<may> cause paralysis. Does not miss while in rain or if the target is airborne."/* THUNDER_EFFECT */,
	"A special move for instantly escaping from wild POKÈMON. Useful in the wild only."/* TELEPORT_EFFECT */,
	"The user's fellow party POKÈMON appear to pummel the target. "/* BEAT_UP_EFFECT */,
	"Charges, then attacks. The user evades most attacks during the first turn."/* SEMI_INVUL */,
	"Raises the users DEFENSE. Boosts power of ROLLOUT."/* DEFENSE_CURL_EFFECT */,
};
constexpr const char* EffectChanceDescription(double percent) {
	if (percent < 5) {
		return "Very rarely";
	}
	if (percent < 15) {
		return "Rarely";
	}
	if (percent < 30) {
		return "Sometimes";
	}
	if (percent < 50) {
		return "Often";
	}
	if (percent < 80) {
		return "Usually";
	}
	if (percent < 100) {
		return "Almost always";
	}
	return "Always";

}

GameInfo::Move MoveGenerator::Generate(const GameInfo::Move& from, int moveId)
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

	if (generateDescription) {
		using namespace GameInfo;
		if (MoveEffectInfos[ret.effectId].flags & MoveEffectInfo::CHANCE_PARAMETER) {
			std::string descr = EffectDescriptions[ret.effectId];
			if (preciseDescription) {
				size_t idx = descr.find(EffectChanceWildcard);
				if (idx != descr.npos) {
					double chancePercent = (ret.effectChance / 255.0) * 100;
					std::string chance = std::to_string(int(std::round(chancePercent))) + "% chance to";
					descr.replace(idx, strlen(EffectChanceWildcard), chance);
				}
			}
			else {
				size_t idx = descr.find(EffectChanceWildcard);
				if (idx != descr.npos) {
					double chancePercent = (ret.effectChance / 255.0) * 100;
					descr.replace(idx, strlen(EffectChanceWildcard), EffectChanceDescription(chancePercent));
				}
			}
			textChanges.AddChange(TableInfo::MOVE_DESCRIPTIONS, moveId, std::move(descr));
		}
		else {
			textChanges.AddChange(TableInfo::MOVE_DESCRIPTIONS, moveId, EffectDescriptions[ret.effectId]);
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
