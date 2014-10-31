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

enum Squire
{
    ACH_PONY_UP = 3736,

    NPC_ARGENT_GRUNT = 33239,
    NPC_ARGENT_SQUIRE = 33238,

    SPELL_SQUIRE_MOUNT_CHECK = 67039,
    SPELL_STORMWIND_PENNANT = 62727,
    SPELL_SENJIN_PENNANT = 63446,
    SPELL_DARNASSUS_PENNANT = 63443,
    SPELL_EXODAR_PENNANT = 63439,
    SPELL_UNDERCITY_PENNANT = 63441,
    SPELL_GNOMEREAGAN_PENNANT = 63442,
    SPELL_IRONFORGE_PENNANT = 63440,
    SPELL_ORGRIMMAR_PENNANT = 63444,
    SPELL_SILVERMOON_PENNANT = 63438,
    SPELL_THUNDERBLUFF_PENNANT = 63445,
    SPELL_SQUIRE_TIRED = 67401,
    SPELL_GRUNT_TIRED = 68852,
    SPELL_SQUIRE_BANK = 67368,
    SPELL_SQUIRE_SHOP = 67377,
    SPELL_SQUIRE_POSTMAN = 67376,
    SPELL_GRUNT_BANK = 68849,
    SPELL_GRUNT_POSTMAN = 68850,
    SPELL_GRUNT_SHOP = 68851,
    SPELL_PLAYER_TIRED = 67334
};

class TW_npc_vanity_argent_squire : public CreatureScript
{
    public:
        TW_npc_vanity_argent_squire() : CreatureScript("TW_npc_vanity_argent_squire") { }

        struct TW_npc_vanity_argent_squireAI : public ScriptedAI
        {
            TW_npc_vanity_argent_squireAI(Creature* creature) : ScriptedAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                curPennant = 0;
                entry = me->GetEntry();
                Bank = false;
                Shop = false;
                Mail = false;
            }

            void Reset()
            {
                Initialize();

                if (Aura* tired = me->GetOwner()->GetAura(SPELL_PLAYER_TIRED))
                {
                    int32 duration = tired->GetDuration();
                    Aura* petTired = me->AddAura(entry == NPC_ARGENT_SQUIRE ? SPELL_SQUIRE_TIRED : SPELL_GRUNT_TIRED, me);
                    petTired->SetDuration(duration);
                }

                if (Unit* owner = me->GetOwner())
                    me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            }

            void UpdateAI(uint32 diff)
            {
                if (!me->HasAura(SPELL_SQUIRE_MOUNT_CHECK))
                    if (me->GetOwner()->ToPlayer()->HasAchieved(ACH_PONY_UP))
                        DoCast(me, SPELL_SQUIRE_MOUNT_CHECK, true);

                if (me->HasAura(entry == NPC_ARGENT_SQUIRE ? SPELL_SQUIRE_TIRED : SPELL_GRUNT_TIRED))
                    if (Bank || Shop || Mail)
                        me->DespawnOrUnsummon();
            }

            void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action)
            {
                player->PlayerTalkClass->SendCloseGossip();
                switch (action)
                {
                    case 0:
                        if (!Bank)
                        {
                            DoCast(me, entry == NPC_ARGENT_SQUIRE ? SPELL_SQUIRE_BANK : SPELL_GRUNT_BANK, true);
                            Bank = true;
                            player->AddAura(SPELL_PLAYER_TIRED, player);
                            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER);
                        }
                        player->GetSession()->SendShowBank(player->GetGUID());
                        break;
                    case 1:
                        if (!Shop)
                        {
                            DoCast(me, entry == NPC_ARGENT_SQUIRE ? SPELL_SQUIRE_SHOP : SPELL_GRUNT_SHOP, true);
                            Shop = true;
                            player->AddAura(SPELL_PLAYER_TIRED, player);
                        }
                        player->GetSession()->SendListInventory(me->GetGUID());
                        break;
                    case 2:
                        if (!Mail)
                        {
                            DoCast(me, entry == NPC_ARGENT_SQUIRE ? SPELL_SQUIRE_POSTMAN : SPELL_GRUNT_POSTMAN, true);
                            Mail = true;
                            player->AddAura(SPELL_PLAYER_TIRED, player);
                            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR);
                            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_MAILBOX);
                        }
                        player->GetSession()->SendShowMailBox(me->GetGUID());
                        break;
                    case 3: //Darnassus/Darkspear Pennant
                    case 4: //Exodar/Forsaken Pennant
                    case 5: //Gnomereagan/Orgrimmar Pennant
                    case 6: //Ironforge/Silvermoon Pennant
                    case 7: //Stormwind/Thunder Bluff Pennant
                        me->RemoveAurasDueToSpell(curPennant);
                        if (entry == NPC_ARGENT_SQUIRE)
                        {
                            switch (action)
                            {
                                case 3: //Darnassus
                                    DoCast(me, SPELL_DARNASSUS_PENNANT, true);
                                    curPennant = SPELL_DARNASSUS_PENNANT;
                                    break;
                                case 4: //Exodar
                                    DoCast(me, SPELL_EXODAR_PENNANT, true);
                                    curPennant = SPELL_EXODAR_PENNANT;
                                    break;
                                case 5: //Gnomereagan
                                    DoCast(me, SPELL_GNOMEREAGAN_PENNANT, true);
                                    curPennant = SPELL_GNOMEREAGAN_PENNANT;
                                    break;
                                case 6: //Ironforge
                                    DoCast(me, SPELL_IRONFORGE_PENNANT, true);
                                    curPennant = SPELL_IRONFORGE_PENNANT;
                                    break;
                                case 7: //Stormwind
                                    DoCast(me, SPELL_STORMWIND_PENNANT, true);
                                    curPennant = SPELL_STORMWIND_PENNANT;
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            switch (action)
                            {
                                case 3: //Darkspear Pennant
                                    DoCast(me, SPELL_SENJIN_PENNANT, true);
                                    curPennant = SPELL_SENJIN_PENNANT;
                                    break;
                                case 4: //Forsaken Pennant
                                    DoCast(me, SPELL_UNDERCITY_PENNANT, true);
                                    curPennant = SPELL_UNDERCITY_PENNANT;
                                    break;
                                case 5: //Orgrimmar Pennant
                                    DoCast(me, SPELL_ORGRIMMAR_PENNANT, true);
                                    curPennant = SPELL_ORGRIMMAR_PENNANT;
                                    break;
                                case 6: //Silvermoon Pennant
                                    DoCast(me, SPELL_SILVERMOON_PENNANT, true);
                                    curPennant = SPELL_SILVERMOON_PENNANT;
                                    break;
                                case 7: //Thunder Bluff Pennant
                                    DoCast(me, SPELL_THUNDERBLUFF_PENNANT, true);
                                    curPennant = SPELL_THUNDERBLUFF_PENNANT;
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                }
            }
    private:
        uint32 curPennant;
        uint32 entry;
        bool Bank;
        bool Shop;
        bool Mail;

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new TW_npc_vanity_argent_squireAI(creature);
    }
};

class TW_spell_vanity_mount_check : public SpellScriptLoader
{
    public:
        TW_spell_vanity_mount_check() : SpellScriptLoader("TW_spell_vanity_mount_check") { }

        class TW_spell_vanity_mount_check_AuraScript : public AuraScript
        {
            PrepareAuraScript(TW_spell_vanity_mount_check_AuraScript);

            void HandleEffectPeriodic(AuraEffect const * /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetOwner()->IsMounted())
                        caster->Mount(29736);
                    else if (caster->IsMounted())
                        caster->Dismount();
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(TW_spell_vanity_mount_check_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new TW_spell_vanity_mount_check_AuraScript();
        }
};

enum VanityPetSpells
{
    SPELL_ROCKET_BOT_PASSIVE = 45266,
    SPELL_ROCKET_BOT_ATTACK = 45269
};

class TW_spell_vanity_pet_focus : public SpellScriptLoader
{
    public:
        TW_spell_vanity_pet_focus(const char* name, uint32 _passiveId = 0) : SpellScriptLoader(name), passiveId(_passiveId) { }

        class TW_spell_vanity_pet_focus_SpellScript : public SpellScript
        {
            PrepareSpellScript(TW_spell_vanity_pet_focus_SpellScript);

            public:
                TW_spell_vanity_pet_focus_SpellScript(int32 _passiveId) : SpellScript(), passiveId(_passiveId) { }

                bool Validate(SpellInfo const* spellInfo) override
                {
                    if (!sSpellMgr->GetSpellInfo(spellInfo->Effects[EFFECT_0].BasePoints))
                        return false;
                    if (!sSpellMgr->GetSpellInfo(SPELL_ROCKET_BOT_ATTACK))
                        return false;
                    return true;
                }

                void FilterTargets(std::list<WorldObject*>& targets)
                {
                    if (passiveId == SPELL_ROCKET_BOT_PASSIVE)
                        targets.remove_if(Trinity::UnitAuraCheck(false, SPELL_ROCKET_BOT_PASSIVE));

                    if (targets.empty())
                        return;

                    WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                    targets.clear();
                    targets.push_back(target);
                }

                void HandleScript(SpellEffIndex /*effIndex*/)
                {
                    // bit of a work around the orange clockwork robot doesn't fire 49058 correctly
                    if (passiveId == SPELL_ROCKET_BOT_PASSIVE)
                        GetCaster()->CastSpell(GetHitUnit(), SPELL_ROCKET_BOT_ATTACK, true);
                    else
                        GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
                }

                void Register() override
                {
                    OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(TW_spell_vanity_pet_focus_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                    OnEffectHitTarget += SpellEffectFn(TW_spell_vanity_pet_focus_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                }

            private:
                int32 passiveId;
        };

        SpellScript* GetSpellScript() const override
        {
            return new TW_spell_vanity_pet_focus_SpellScript(passiveId);
        }

    private:
        int32 passiveId;
};

enum PetAvoidance
{
    SPELL_HUNTER_ANIMAL_HANDLER  = 34453,
    SPELL_NIGHT_OF_THE_DEAD      = 55620,

    ENTRY_ARMY_OF_THE_DEAD_GHOUL = 24207
};

class TW_spell_hun_animal_handler : public SpellScriptLoader
{
    public:
        TW_spell_hun_animal_handler() : SpellScriptLoader("TW_spell_hun_animal_handler") { }

        class TW_spell_hun_animal_handler_AuraScript : public AuraScript
        {
            PrepareAuraScript(TW_spell_hun_animal_handler_AuraScript);

            bool Load() override
            {
                if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void CalculateAmountDamageDone(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (!GetCaster() || !GetCaster()->GetOwner())
                    return;
                if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
                {
                    if (AuraEffect* /* aurEff */ect = owner->GetAuraEffectOfRankedSpell(SPELL_HUNTER_ANIMAL_HANDLER, EFFECT_1))
                        amount = /* aurEff */ect->GetAmount();
                    else
                        amount = 0;
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(TW_spell_hun_animal_handler_AuraScript::CalculateAmountDamageDone, EFFECT_0, SPELL_AURA_MOD_ATTACK_POWER_PCT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new TW_spell_hun_animal_handler_AuraScript();
        }
};

class TW_spell_dk_avoidance_passive : public SpellScriptLoader
{
    public:
        TW_spell_dk_avoidance_passive() : SpellScriptLoader("TW_spell_dk_avoidance_passive") { }

        class TW_spell_dk_avoidance_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(TW_spell_dk_avoidance_passive_AuraScript);

            bool Load() override
            {
                if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void CalculateAvoidanceAmount(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* pet = GetUnitOwner())
                {
                    if (Unit* owner = pet->GetOwner())
                    {
                        // Army of the dead ghoul
                        if (pet->GetEntry() == ENTRY_ARMY_OF_THE_DEAD_GHOUL)
                            amount = -90;
                        // Night of the dead
                        else if (Aura* aur = owner->GetAuraOfRankedSpell(SPELL_NIGHT_OF_THE_DEAD))
                            amount = -aur->GetSpellInfo()->Effects[EFFECT_2].CalcValue();
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(TW_spell_dk_avoidance_passive_AuraScript::CalculateAvoidanceAmount, EFFECT_0, SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new TW_spell_dk_avoidance_passive_AuraScript();
        }
};

enum RoarOfSacrifice
{
    SPELL_ROAR_OF_SACRIFICE_DMG = 67481
};

// 53480 - Roar of Sacrifice
class TW_spell_hun_roar_of_sacrifice : public SpellScriptLoader
{
public:
    TW_spell_hun_roar_of_sacrifice() : SpellScriptLoader("TW_spell_hun_roar_of_sacrifice") { }

    class TW_spell_hun_roar_of_sacrifice_AuraScript : public AuraScript
    {
        PrepareAuraScript(TW_spell_hun_roar_of_sacrifice_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_ROAR_OF_SACRIFICE_DMG))
                return false;
            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (eventInfo.GetDamageInfo())
            {
                int32 damage = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), 20);
                GetTarget()->CastCustomSpell(SPELL_ROAR_OF_SACRIFICE_DMG, SPELLVALUE_BASE_POINT0, damage, GetCaster(), true, NULL, aurEff);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(TW_spell_hun_roar_of_sacrifice_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new TW_spell_hun_roar_of_sacrifice_AuraScript();
    }
};

void AddSC_custom_scripts()
{
    new TW_spell_item_draenic_pale_ale();
    new TW_spell_landmine_knockback();
    new TW_spell_rotface_slime_spray();
    new TW_npc_vanity_argent_squire();
    new TW_spell_vanity_mount_check();
    new TW_spell_vanity_pet_focus("TW_spell_lil_kt_focus");
    new TW_spell_vanity_pet_focus("TW_spell_willy_focus");
    new TW_spell_vanity_pet_focus("TW_spell_scorchling_focus");
    new TW_spell_vanity_pet_focus("TW_spell_toxic_wasteling_focus");
    new TW_spell_vanity_pet_focus("TW_spell_rocket_bot_focus", SPELL_ROCKET_BOT_PASSIVE);
    new TW_spell_hun_animal_handler();
    new TW_spell_dk_avoidance_passive();
    new TW_spell_hun_roar_of_sacrifice();
}
