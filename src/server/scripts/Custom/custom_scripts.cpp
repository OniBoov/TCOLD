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

void AddSC_custom_scripts()
{
    new TW_spell_item_draenic_pale_ale();
}
