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
	 /* NO_EFFECT */	"No added effect."
	,/* SLEEP_STATUS */	"Inflicts Sleep."
	,/* POISON_CHANCE */	"<may> cause Poisoning."
	,/* DRAIN_HEALTH */	"Drains Health."
	,/* BURN_CHANCE */	"<may> cause Burn."
	,/* FREEZE_CHANCE */	"<may> cause Freeze."
	,/* PARALYSIS_CHANCE */	"<may> cause paralysis."
	,/* KABOOM */	"The user faints."
	,/* DREAM_EATER_EFFECT */	"Drains Health, but only works on sleeping targets."
	,/* MIRROR_MOVE_EFFECT */	"Strikes back with the opponent's last move. Low priority."
	,/* ATTACK_PLUS_ONE */	"Rasies ATTACK."
	,/* DEFENSE_PLUS_ONE */	"Raises DEFENSE."
	,/* SPEED_PLUS_ONE */	"Raises SPEED."
	,/* SPA_PLUS_ONE */	"Raises Special Attack."
	,/* SPD_PLUS_ONE */	"Raises SPCL.DEF."
	,/* ACCURACY_PLUS_ONE */	"Raises Accuracy."
	,/* EVASION_PLUS_ONE */	"Raises Evasion."
	,/* ALWAYS_HITS */	"Cant miss."
	,/* ATTACK_MINUS_ONE */	"Lowers Opponents ATTACK."
	,/* DEFENSE_MINUS_ONE */	"Lowers Opponents DEFENSE."
	,/* SPEED_MINUS_ONE */	"Lowers Opponents SPEED."
	,/* SPA_MINUS_ONE */	"Lowers Opponents Special Attack."
	,/* SPD_MINUS_ONE */	"Lowers Opponents SPCL.DEF."
	,/* ACCURACY_MINUS_ONE */	"Lowers Opponents Accuracy."
	,/* EVASION_MINUS_ONE */	"Lowers Opponents Evasion."
	,/* HAZE_EFFECT */	"Eleminates all stat changes."
	,/* BIDE_EFFECT */	"Waits 2-3 turns; hits back double."
	,/* THRASHING_ABOUT */	"Works 2-3 turns, then confuses user."
	,/* PHASING */	"Phases."
	,/* MULTI_STRIKE */	"Hits 2-5 times."
	,/* CONVERSION_EFFECT */	"Change user's type to a move's type. "
	,/* FLINCH_CHANCE */	"<may> Flinch."
	,/* RECOVER_HEALTH */	"Restores 50% of max health."
	,/* BAD_POISON_STATUS */	"Inflicts bad poisoning."
	,/* PAY_DAY_MONEY */	"Scatters coins."
	,/* LIGHT_SCREEN_EFFECT */	"Halfs incoming Special Damage on your team for five turns."
	,/* TRI_ATTACK_EFFECT */	"<may> cause Freeze, Burn or paralysis"
	,/* UNKNOWN_EFFECT_1 */	"???"
	,/* ONE_HIT_KO */	"Always K.Os if it hits."
	,/* RAZOR_WIND_EFFECT */	"Move Attacks on the second turn. Increased critical hit ratio."
	,/* SUPER_FANG_EFFECT */	"Always inflicts damage equal to half the opponents current health."
	,/* DAMAGE_IGNORES_STATS */	"Damage ignores attacks and defenses."
	,/* BINDING */	"Traps the target and does 1/16 health as damage per turn, for 2-5 turns."
	,/* UNKNOWN_EFFECT_2 */	"???"
	,/* HITS_TWICE */	"Hits twice."
	,/* RECOIL_WHEN_MISSING */	"Inflicts recoil damage if it misses."
	,/* MIST_EFFECT */	"Prevents stat reduction."
	,/* FOCUS_ENERGY_EFFECT */	"Increaes critical hit rate."
	,/* RECOIL */	"Does recoil damage to user."
	,/* CONFUSE */	"Inflicts Confusion."
	,/* ATTACK_PLUS_TWO */	"Sharply raises ATTACK."
	,/* DEFENSE_PLUS_TWO */	"Sharply raises DEFENSE."
	,/* SPEED_PLUS_TWO */	"Sharply raises SPEED."
	,/* SPA_PLUS_TWO */	"Sharply raises SPCL.ATK."
	,/* SPD_PLUS_TWO */	"Sharply raises SPCL.DEF."
	,/* ACCURACY_PLUS_TWO */	"Sharply raises Accuracy."
	,/* EVASION_PLUS_TWO */	"Sharply raises Evasion."
	,/* TRANSFORM_EFFECT */	"Transforms into opponent."
	,/* ATTACK_MINUS_TWO */	"Harshly lowers Opponents ATTACK."
	,/* DEFENSE_MINUS_TWO */	"Harshly lowers Opponents DEFENSE."
	,/* SPEED_MINUS_TWO */	"Harshly lowers Opponents SPEED."
	,/* SPA_MINUS_TWO */	"Harshly lowers Opponents SPCL.ATK."
	,/* SPD_MINUS_TWO */	"Harshly lowers Opponents SPCL.DEF."
	,/* ACCURACY_MINUS_TWO */	"Harshly lowers Opponents Accuracy."
	,/* EVASION_MINUS_TWO */	"Harshly lowers Opponents Evasion."
	,/* REFLECT_EFFECT */	"Harshly lowers Opponents Attack."
	,/* POISON_STATUS */	"Inflicts Poison."
	,/* PARALYSIS_STATUS */	"Inflicts Paralysis."
	,/* ATTACK_MINUS_ONE_CHANCE */	"<may> lower Opponents ATTACK."
	,/* DEFENSE_MINUS_ONE_CHANCE */	"<may> lower Opponents DEFENSE."
	,/* SPEED_MINUS_ONE_CHANCE */	"<may> lower Opponents SPEED."
	,/* SPA_MINUS_ONE_CHANCE */	"<may> lower Opponents SPCL.ATK."
	,/* SPD_MINUS_ONE_CHANCE */	"<may> lower Opponents SPCL.DEF."
	,/* ACCURACY_MINUS_ONE_CHANCE */	"<may> lower Opponents Accuracy."
	,/* EVASION_MINUS_ONE_CHANCE */	"<may> lower Opponents Evasion."
	,/* SKY_ATTACK_EFFECT */	"Move Attacks on the second turn. Increased critical hit ratio."
	,/* CONFUSE_CHANCE */	"<may> cause Confusion."
	,/* HITS_TWICE_POISON_CHANCE */	"Hits twice. <may> cause Poison."
	,/* UNKNOWN_EFFECT_3 */	"???"
	,/* SUBSTITUTE_EFFECT */	"Uses 1/4 of the user's maximum HP to create a substitute that takes the opponent's attacks."
	,/* RECHARGE_AFTER_USE */	"Recharges after use."
	,/* RAGE_EFFECT */	"A non-stop attack move. The user's ATTACK power increases every time it sustains damage."
	,/* MIMIC_EFFECT */	"A move for learning one of the opponent's moves, for use during that battle only."
	,/* METRONOME_EFFECT */	"Invokes a random move."
	,/* LEECH_SEED_EFFECT */	"Steals HP from the foe on every turn."
	,/* SPLASH_EFFECT */	"Does nothing."
	,/* DISABLE_EFFECT */	"Disables the last move used by the opponent."
	,/* DAMAGE_BY_LEVEL_EFFECT */	"Does damage equal to the users damage, ignoring attack and defense."
	,/* PSYWAVE_EFFECT */	"Does random damage depending on the users level, ignoring attack and defense."
	,/* COUNTER_EFFECT */	"Returns a physical blow double."
	,/* ENCORE_EFFECT */	"Forces the target to continue to use the move it used last for the next two to six turns."
	,/* PAIN_SPLIT_EFFECT */	"A move that adds the HPs of the user and the target, then divides the total between them."
	,/* SNORE_EFFECT */	"Has a one-in-three chance of making the target flinch. Can be used only by a sleeping POKÈMON."
	,/* CONVERSION_TWO_EFFECT */	"A move that changes the user's type into one that is resistant to the opponent's last move."
	,/* NEXT_MOVE_HITS */	"The user's next attack will never miss."
	,/* SKETCH_EFFECT */	"A move that allows the user to copy a move used by an opponent and learn that move."
	,/* UNKNOWN_EFFECT_4 */	"???"
	,/* SLEEP_TALK_EFFECT */	"Randomly chooses one of the user's moves. Can be used only by a sleeping POKÈMON."
	,/*DESTINY_BOND_EFFECT */	"A move that causes the opponent to faint if the user faints after using this move."
	,/* BP_BASED_ON_HEALTH */	"An attack that increases in power if the user's HP is low. It could turn the tide in battle."
	,/* SPITE_EFFECT */	"A vengeful move that slightly reduces the PP of the opponent's last move."
	,/* FALSE_SWIPE_EFFECT */	"A move that adjusts ATTACK so that the foe has one HP left.  Can't cause the foe to faint."
	,/* HEAL_BELL_EFFECT */	"Eliminates all status problems for the entire party."
	,/* HIGH_PRIORITY */	"High priority."
	,/* TRIPLE_KICK_EFFECT */	"Hits up to 3 times; each hit inflicts increasing damage."
	,/* ITEM_STEAL_CHANCE */	"<may> steal an item held by the target."
	,/* CANT_ESCAPE */	"Prevent the target from escaping until the attacking POKÈMON is gone."
	,/* NIGHTMARE_EFFECT */	"A move that makes a sleeping target have bad dreams. The victim will steadily lose HP."
	,/* FLAME_WHEEL_EFFECT */	"<may> cause Burn. If the user is frozen, using this move will thaw it out."
	,/* CURSE_EFFECT */	"Raises ATTACK and DEFENSE at the expense of SPEED. It works differently for the GHOST type."
	,/* UNKNOWN_EFFECT_5 */	"???"
	,/* PROTECT_EFFECT */	"Completely foils an opponent's attack. If used consecutively, its success rate decreases."
	,/* SPIKES_EFFECT */	"An attack that scatters SPIKES and injures opponent POKÈMON when they switch."
	,/* FORESIGHT_EFFECT */	"Neutralizes the foe's evasive- ness. Also makes the GHOST type vulnerable to physical attacks."
	,/* PERISH_SONG_EFFECT */	"Causes both the user and the opponent to faint in three turns."
	,/* SANDSTORM_EFFECT */	"An attack that creates a sand- storm. The effect causes damage to both combatants."
	,/* ENDURE_EFFECT */	"Always leaves the user with at least one HP. Success rate decreases if used repeatedly."
	,/* ROLLOUT_EFFECT */	"Attacks for five turns, growing more powerful each time. Power returns to normal if it misses."
	,/* SWAGGER_EFFECT */	"A move that infuriates and  confuses the opponent, but it increases the target's ATTACK."
	,/* FURY_CUTTER_EFFECT */	"Grows more powerful with every hit. Returns to normal if it misses."
	,/* ATTRACT_EFFECT */	"Infatuates targets, making it hard for them to attack foes of the opposite gender."
	,/* RETURN_EFFECT */	"The tamer the user, the more powerful the move."
	,/* PRESENT_EFFECT */	"A move that inflicts major damage but may restore the target's HP."
	,/* FRUSTRATION_EFFECT */	"The more the user dislikes its trainer, the more powerful the move."
	,/* SAFEGUARD_EFFECT */	"A mystic power that protects the user from status problems for five turns."
	,/* SACRED_FIRE_EFFECT */	"<may> cause Burn. If the user is frozen, using this move will thaw it out."
	,/* MAGNITUDE_EFFECT */	"The power of the attack will vary each time it is used."
	,/* BATON_PASS_EFFECT */	"A move that transfers the user's status changes to the next POKÈMON when switching."
	,/* PURSUIT_EFFECT */	"Inflicts major damage if the target switches out in the same turn."
	,/* RAPID_SPIN_EFFECT */	"User escapes from restraining moves."
	,/* UNKNOWN_EFFECT_6 */	"???"
	,/* UNKNOWN_EFFECT_7 */	"???"
	,/* MORNING_SUN_EFFECT */	"Restores Health depending on weather and time of day."
	,/* SYNTHESIS_EFFECT */	"Restores Health depending on weather and time of day."
	,/* MOONLIGHT_EFFECT */	"Restores Health depending on weather and time of day."
	,/* HIDDEN_POWER_EFFECT */	"A peculiar move that changes type and power depending on the POKÈMON using it."
	,/* RAIN_DANCE_EFFECT */	"Summons rain for five turns. While it is raining, the power of WATER-type moves increases.@"
	,/* SUNNY_DAY_EFFECT */	"Makes the weather sunny for five turns. When sunny, FIRE- type moves are more powerful."
	,/* ATTACK_PLUS_ONE_CHANCE */	"<may> raise users ATTACK."
	,/* DEFENSE_PLUS_ONE_CHANCE */	"<may> raise users DEFENSE."
	,/* RAINBOW_CHANCE */	"<may> cause Rainbow Boost."
	,/* UNKNOWN_EFFECT_7 */	"???"
	,/* BELLY_DRUM_EFFECT */	"A move that allows the user to consume half of its own HP in order to maximize its ATTACK."
	,/* PSYCH_UP_EFFECT */	"Self-hypnosis move that copies the foe's stats changes, then applies them to the user."
	,/* MIRROR_COAT_EFFECT */	"The foe will receive double the damage the user sustained from a special attack. High accuracy."
	,/* SKULL_BASH_EFFECT */	"In the first turn, the attacker tucks its head. The next turn, it head-butts at full steam."
	,/* TWISTER_EFFECT */	"<may> Flinch. Always hits airborne targets."
	,/* HITS_DIGGING */	"Always hits targets underground."
	,/* FUTURE_SIGHT_EFFECT */	"Strikes the target two turns after it is used."
	,/* HITS_FLYING */	"Always hits airborne targets."
	,/* HITS_MINIMIZED */	"Increased strength and accuracy against minimzed targets."
	,/* SOLAR_BEAM_EFFECT */	"Energy is absorbed in the first turn, then fired on the next."
	,/* THUNDER_EFFECT */	"<may> cause paralysis. Does not miss while in rain or if the target is airborne."
	,/* TELEPORT_EFFECT */	"A special move for instantly escaping from wild POKÈMON. Useful in the wild only."
	,/* BEAT_UP_EFFECT */	"The user's fellow party POKÈMON appear to pummel the target. "
	,/* SEMI_INVUL */	"Charges, then attacks. The user evades most attacks during the first turn."
	,/* DEFENSE_CURL_EFFECT */	"Raises the users DEFENSE. Boosts power of ROLLOUT."
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
			int oldBp = from.basePower;
			auto scaling = bpDist.GetScaling().CenterAround(oldBp);
			int bp = bpDist(Random::Generator, scaling);
			ret.basePower = bp;
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
			bool changedAcc = false; //only modify accuracy once, this one gets annoying quickly
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
							else if(ret.pp > 2)
								changePP = 5;
						}
						if (randomAcc && !changedAcc && ret.accuracy > 200) {
							changedAcc = true;
							changeAcc = std::clamp(ret.accuracy - 200, 0, 40);
						}
						if (randomBp) {
							changeBP = 30;
						}
					}
					else/*(strengthen)*/ {
						if (randomPp && ret.pp < 15) {
							if (ret.pp == 63)
								changePP = 0;
							else if (ret.pp > 10)
								changePP = 10;
							else 
								changePP = (10 - ret.pp) * 10;
						}
						if (randomAcc && !changedAcc && ret.accuracy < 255) {
							changedAcc = true;
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
						ret.pp = std::clamp<int>(ret.pp * changePercent, 2, 63);
					}
					else if (choice < (ifTotal += changeBP)) {
						double changePercent = reduce ? Random::GetDouble(0.7, 0.9) : Random::GetDouble(1.2, 2.0);
						ret.basePower = std::clamp<int>(ret.basePower * changePercent, 10, 200);
					}
					else if (choice < (ifTotal += changeAcc)) {
						double changePercent = reduce ? Random::GetDouble(0.9, 0.99) : Random::GetDouble(1.1, 1.4);
						ret.accuracy = std::clamp<int>(ret.accuracy * changePercent, 3, 255);
					}
					else {
						double changePercent = reduce ? Random::GetDouble(0.4, 0.9) : Random::GetDouble(1.2, 2.0);
						ret.effectChance = std::clamp<int>(ret.effectChance * changePercent, 2, 63);
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
			textChanges.AddChange(TableInfo::MOVE_DESCRIPTIONS, moveId, AddDescriptionNewlines(std::move(descr)));
		}
		else {
			textChanges.AddChange(TableInfo::MOVE_DESCRIPTIONS, moveId, AddDescriptionNewlines(EffectDescriptions[ret.effectId]));
		}
	}

	return ret;
}

std::string MoveGenerator::AddDescriptionNewlines(std::string descr) const {
	//we estimate about 35 characters per line
	size_t lastSpace = descr.find(' ');
	size_t lastNewline = 0;
	for (size_t curr = descr.find(' ', lastSpace+1); curr != descr.npos; lastSpace = curr, curr = descr.find(' ', lastSpace+1)) {
		size_t currDist = curr - lastNewline;
		size_t lastDist = lastSpace - lastNewline;
		if (currDist >= 35) {
			descr[lastSpace] = '\n';
			lastNewline = lastSpace;
		}
	}

	return descr;
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
