#include "ScriptPCH.h"
#include "SpellScript.h"

enum WeakAlcohol
{
    SPELL_SUMMON_PINK_ELEKK_GUARDIAN = 50180,
    SPELL_PINK_ELEKK_MOUNT           = 49908
};

class TW_spell_item_draenic_pale_ale : public SpellScriptLoader
{
    public:
        TW_spell_item_draenic_pale_ale() : SpellScriptLoader("TW_spell_item_draenic_pale_ale") { }

        class TW_spell_item_draenic_pale_ale_SpellScript : public SpellScript
        {
            PrepareSpellScript(TW_spell_item_draenic_pale_ale_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SUMMON_PINK_ELEKK_GUARDIAN) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_PINK_ELEKK_GUARDIAN))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                // If player doesn't already have the mount aura roll for chance of it being applied 
                if (Player* caster = GetCaster()->ToPlayer())
                    if (!caster->HasAura(SPELL_PINK_ELEKK_MOUNT))
                       if (roll_chance_i(33)) // pure guess on 33%
                       {
                           PreventHitDefaultEffect(effIndex);
                           // prevent client crashes from stacking mounts
                           caster->RemoveAurasByType(SPELL_AURA_MOUNTED);
                           caster->CastSpell(caster, SPELL_PINK_ELEKK_MOUNT, true, GetCastItem());
                       }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(TW_spell_item_draenic_pale_ale_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new TW_spell_item_draenic_pale_ale_SpellScript();
        }
};

class TW_spell_landmine_knockback : public SpellScriptLoader
{
    public:
        TW_spell_landmine_knockback() : SpellScriptLoader("TW_spell_landmine_knockback") { }

        class TW_spell_landmine_knockback_SpellScript : public SpellScript
        {
            PrepareSpellScript(TW_spell_landmine_knockback_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Player* target = GetHitPlayer())
                {
                    Aura const* aura = GetHitAura();
                    if (!aura || aura->GetStackAmount() != 10)
                        return;

                    AchievementEntry const* achiev = sAchievementStore.LookupEntry(1428);
                    target->CompletedAchievement(achiev);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(TW_spell_landmine_knockback_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new TW_spell_landmine_knockback_SpellScript();
        }
};

enum SlimeSpray
{
    NPC_OOZE_SPRAY_STALKER = 37986
};

class TW_spell_rotface_slime_spray : public SpellScriptLoader
{
    public:
        TW_spell_rotface_slime_spray() : SpellScriptLoader("TW_spell_rotface_slime_spray") { }

        class TW_spell_rotface_slime_spray_SpellScript : public SpellScript
        {
            PrepareSpellScript(TW_spell_rotface_slime_spray_SpellScript);

            void ChangeOrientation()
            {
                Unit* caster = GetCaster();
                // find stalker and set caster orientation to face it
                if (Creature* target = caster->FindNearestCreature(NPC_OOZE_SPRAY_STALKER, 200.0f))
                    caster->SetOrientation(caster->GetAngle(target));
            }

            void Register()
            {
                BeforeCast += SpellCastFn(TW_spell_rotface_slime_spray_SpellScript::ChangeOrientation);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new TW_spell_rotface_slime_spray_SpellScript();
        }
};

void AddSC_custom_scripts()
{
    new TW_spell_item_draenic_pale_ale();
    new TW_spell_landmine_knockback();
    new TW_spell_rotface_slime_spray();
}
