#include "ScriptPCH.h"
#include "SpellScript.h"

/*######
## TW_npc_argent_squire
######*/

enum eArgentSquire
{
    //QUEST_THE_ASPIRANT_S_CHALLENGE_H                    = 13680,
    //QUEST_THE_ASPIRANT_S_CHALLENGE_A                    = 13679,

    //QUEST_THE_VALIANT_S_CHALLENGE_SM                    = 13731,
    //QUEST_THE_VALIANT_S_CHALLENGE_UC                    = 13729,
    //QUEST_THE_VALIANT_S_CHALLENGE_TB                    = 13728,
    //QUEST_THE_VALIANT_S_CHALLENGE_SJ                    = 13727,
    //QUEST_THE_VALIANT_S_CHALLENGE_OG                    = 13726,
    //QUEST_THE_VALIANT_S_CHALLENGE_DA                    = 13725,
    //QUEST_THE_VALIANT_S_CHALLENGE_EX                    = 13724,
    //QUEST_THE_VALIANT_S_CHALLENGE_GN                    = 13723,
    //QUEST_THE_VALIANT_S_CHALLENGE_IF                    = 13713,
    //QUEST_THE_VALIANT_S_CHALLENGE_SW                    = 13699,

    //QUEST_THE_BLACK_KNGIHT_S_FALL                       = 13664,

    //NPC_SQUIRE_DAVID                                    = 33447,
    NPC_SQUIRE_DANNY = 33518,
    NPC_SQUIRE_CAVIN = 33522,

    //NPC_ARGENT_VALIANT                                  = 33448,
    NPC_ARGENT_CHAMPION = 33707,
    NPC_BLACK_KNIGHT = 33785,

    //GOSSIP_TEXTID_SQUIRE                                = 14407,

    //SPELL_AURA_ARGENT_CHARGER                           = 63663, // Summon Tournament Argent Charger
    SPELL_AURA_TOURNAMENT_MOUNT = 63034  // Player On Tournament Mount
};

class TW_npc_argent_squire : public CreatureScript
{
    public:
    TW_npc_argent_squire() : CreatureScript("TW_npc_argent_squire") { }

    struct TW_npc_argent_squireAI : public ScriptedAI
    {
        TW_npc_argent_squireAI(Creature* creature) : ScriptedAI(creature) { }

        void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            if (me->GetEntry() == NPC_SQUIRE_DANNY)
            {
                switch (action)
                {
                    case 0:
                        player->CLOSE_GOSSIP_MENU();
                        me->SummonCreature(NPC_ARGENT_CHAMPION, 8534.675781f, 1069.993042f, 552.022827f, 1.274804f);
                        break;
                }
            }

            if (me->GetEntry() == NPC_SQUIRE_CAVIN)
            {
                switch (action)
                {
                    case 0:
                        player->CLOSE_GOSSIP_MENU();
                        if (TempSummon* temp = me->SummonCreature(NPC_BLACK_KNIGHT, 8480.357f, 962.554f, 547.376f, 3.106686f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
                            temp->AI()->SetGUID(player->GetGUID());
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new TW_npc_argent_squireAI(creature);
    }
};

/*######
## TW_npc_argent_champion
######*/

enum eArgentChampion
{
    SPELL_CHARGE = 63010,
    SPELL_SHIELD_BREAKER = 65147,
    SPELL_DEFEND = 62719,
    SPELL_THRUST = 62544,
    SPELL_ARGENT_CHAMPION_CREDIT = 63516,

    //NPC_ARGENT_VALIANT           = 33448,
    //NPC_ARGENT_CHAMPION          = 33707,
};

class TW_npc_argent_champion : public CreatureScript
{
    public:
    TW_npc_argent_champion() : CreatureScript("TW_npc_argent_champion") { }

    struct TW_npc_argent_championAI : public ScriptedAI
    {
        TW_npc_argent_championAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->GetMotionMaster()->MovePoint(0, 8557.131836f, 1109.635742f, 556.787476f);
            creature->SetHomePosition(8557.131836f, 1109.635742f, 556.787476f, 1.27f);
            creature->setFaction(35); //wrong faction in db?
        }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        uint32 uiShieldTimer;
        uint32 uiThrustTimer;
        bool bCharge;

        void Reset() override
        {
            uiChargeTimer = 12000;
            uiShieldBreakerTimer = 10000;
            uiShieldTimer = 4000;
            uiThrustTimer = 2000;
            bCharge = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            for (uint8 i = 0; i < 3; ++i)
                DoCast(me, SPELL_DEFEND, true);
        }

        void MovementInform(uint32 uiType, uint32 /*pointId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            // charge after moving away from the victim
            if (me->IsInCombat() && me->GetVictim() && bCharge)
            {
                me->GetMotionMaster()->Clear();
                // but only after rangecheck
                if (me->GetDistance(me->GetVictim()) > 5.0f && me->GetDistance(me->GetVictim()) <= 30.0f)
                    DoCastVictim(SPELL_CHARGE);
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                uiChargeTimer = 7000;
                bCharge = false;
            }
            else
                me->setFaction(14);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
        {
            if (uiDamage >= me->GetHealth() && pDoneBy->GetTypeId() == TYPEID_PLAYER)
            {
                uiDamage = 0;
                pDoneBy->CastSpell(pDoneBy, SPELL_ARGENT_CHAMPION_CREDIT, true);
                me->setFaction(35);
                me->DespawnOrUnsummon(5000);
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                EnterEvadeMode();
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiShieldTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_DEFEND);
                uiShieldTimer = 4000;
            }
            else uiShieldTimer -= uiDiff;

            if (uiChargeTimer <= uiDiff && !bCharge)
            {
                // directly charge if range is ok
                if (me->GetDistance(me->GetVictim()) > 5.0f && me->GetDistance(me->GetVictim()) <= 30.0f)
                    DoCastVictim(SPELL_CHARGE);
                else
                {
                    // move away for charge...
                    float angle = me->GetAngle(me->GetVictim());
                    float x = me->GetPositionX() + 20.0f * cos(angle);
                    float y = me->GetPositionY() + 20.0f * sin(angle);
                    me->GetMotionMaster()->MovePoint(0, x, y, me->GetPositionZ());
                    bCharge = true;
                }
            }
            else uiChargeTimer -= uiDiff;

            // prevent shieldbreaker while moving away, npc is not facing player at that time
            if (bCharge)
                return;

            if (uiShieldBreakerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHIELD_BREAKER);
                uiShieldBreakerTimer = 10000;
            }
            else uiShieldBreakerTimer -= uiDiff;

            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                if (uiThrustTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_THRUST);
                    uiThrustTimer = 2000;
                }
                else uiThrustTimer -= uiDiff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new TW_npc_argent_championAI(creature);
    }
};

/*#####
## TW_npc_argent_faction_rider
######*/

enum eArgentFactionRiders
{
    NPC_EXODAR_VALIANT = 33562,
    NPC_DARNASSUS_VALIANT = 33559,
    NPC_GNOMEREGAN_VALIANT = 33558,
    NPC_IRONFORGE_VALIANT = 33564,
    NPC_STORMWIND_VALIANT = 33561,
    NPC_SILVERMOON_VALIANT = 33382,
    NPC_THUNDER_BLUFF_VALIANT = 33383,
    NPC_UNDERCITY_VALIANT = 33384,
    NPC_ORGRIMMAR_VALIANT = 33306,
    NPC_SENJIN_VALIANT = 33285,
    NPC_EXODAR_CHAMPION = 33739,
    NPC_DARNASSUS_CHAMPION = 33738,
    NPC_STORMWIND_CHAMPION = 33747,
    NPC_IRONFORGE_CHAMPION = 33743,
    NPC_GNOMEREGAN_CHAMPION = 33740,
    NPC_SILVERMOON_CHAMPION = 33746,
    NPC_THUNDER_BLUFF_CHAMPION = 33748,
    NPC_ORGRIMMAR_CHAMPION = 33744,
    NPC_SENJIN_CHAMPION = 33745,
    NPC_UNDERCITY_CHAMPION = 33749,

    QUEST_AMONG_CHAMPIONS_H_DK = 13814,
    QUEST_AMONG_CHAMPIONS_H = 13811,
    QUEST_AMONG_CHAMPIONS_A_DK = 13793,
    QUEST_AMONG_CHAMPIONS_A = 13790,
    QUEST_GRAND_MELEE_SM = 13787,
    QUEST_GRAND_MELEE_UC = 13782,
    QUEST_GRAND_MELEE_TB = 13777,
    QUEST_GRAND_MELEE_SJ = 13772,
    QUEST_GRAND_MELEE_OG = 13767,
    QUEST_GRAND_MELEE_DA = 13761,
    QUEST_GRAND_MELEE_EX = 13756,
    QUEST_GRAND_MELEE_GN = 13750,
    QUEST_GRAND_MELEE_IF = 13745,
    QUEST_GRAND_MELEE_SW = 13665,

    SPELL_BESTED_DARNASSUS = 64805,
    SPELL_BESTED_EXODAR = 64808,
    SPELL_BESTED_GNOMEREGAN = 64809,
    SPELL_BESTED_IRONFORGE = 64810,
    SPELL_BESTED_ORGRIMMAR = 64811,
    SPELL_BESTED_SENJIN = 64812,
    SPELL_BESTED_SILVERMOON = 64813,
    SPELL_BESTED_STORMWIND = 64814,
    SPELL_BESTED_THUNDER_BLUFF = 64815,
    SPELL_BESTED_UNDERCITY = 64816,
    SPELL_MOUNTED_MELEE_VICTORY_C = 63596,
    SPELL_MOUNTED_MELEE_VICTORY_V = 62724,
    SPELL_READYJOUST_POSE_EFFECT = 64723,

    ITEM_MARK_OF_CHAMPION = 45500,
    ITEM_MARK_OF_VALIANT = 45127,

    EVENT_START = 1,

    TYPE_VALIANT_ALLIANCE = 1,
    TYPE_VALIANT_HORDE = 2,
    TYPE_CHAMPION = 3,
    TYPE_OTHER = 4,

    DATA_PLAYER = 1,
    DATA_TYPE = 2,
};

#define GOSSIP_FACTION_RIDER_1 "I am ready to fight!"

class TW_npc_argent_faction_rider : public CreatureScript
{
    public:
    TW_npc_argent_faction_rider() : CreatureScript("TW_npc_argent_faction_rider") { }

    struct TW_npc_argent_faction_riderAI : public ScriptedAI
    {
        TW_npc_argent_faction_riderAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        uint32 uiShieldTimer;
        uint32 uiThrustTimer;
        bool bCharge;
        Position arenaCenter;

        ObjectGuid challengeeGUID;

        void sGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/)
        {
            player->PlayerTalkClass->SendCloseGossip();
            SetGuidData(DATA_PLAYER, player->GetGUID());
            me->GetAI()->DoAction(EVENT_START);
        }

        void Reset() override
        {
            me->m_CombatDistance = 100.0f;
            me->setFaction(35);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            DoCast(me, SPELL_READYJOUST_POSE_EFFECT, true);

            uiChargeTimer = 7000;
            uiShieldBreakerTimer = 10000;
            uiShieldTimer = 4000;
            uiThrustTimer = 2000;
            bCharge = false;

            challengeeGUID.Clear();

            if (GetCustomType() == TYPE_CHAMPION)
                arenaCenter.Relocate(8428.757f, 945.349f, 544.675f);
            else if (GetCustomType() == TYPE_VALIANT_ALLIANCE)
                arenaCenter.Relocate(8656.402f, 722.827f, 547.523f);
            else if (GetCustomType() == TYPE_VALIANT_HORDE)
                arenaCenter.Relocate(8334.375f, 721.165f, 553.702f);
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_TYPE)
                return GetCustomType();
            return 0;
        }

        void SetGuidData(uint32 type, ObjectGuid data)
        {
            if (type == DATA_PLAYER)
                challengeeGUID = data;
        }

        void DoAction(int32 type) override
        {
            if (type == EVENT_START)
            {
                // check valid player
                Player* challengee = ObjectAccessor::GetPlayer(*me, challengeeGUID);
                if (!challengee)
                    return;

                // check for cooldown
                bool playerCooldown;
                switch (me->GetEntry())
                {
                    case NPC_EXODAR_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_EXODAR);
                        break;
                    case NPC_DARNASSUS_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_DARNASSUS);
                        break;
                    case NPC_STORMWIND_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_STORMWIND);
                        break;
                    case NPC_IRONFORGE_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_IRONFORGE);
                        break;
                    case NPC_GNOMEREGAN_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_GNOMEREGAN);
                        break;
                    case NPC_SILVERMOON_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_SILVERMOON);
                        break;
                    case NPC_THUNDER_BLUFF_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_THUNDER_BLUFF);
                        break;
                    case NPC_ORGRIMMAR_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_ORGRIMMAR);
                        break;
                    case NPC_SENJIN_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_SENJIN);
                        break;
                    case NPC_UNDERCITY_CHAMPION:
                        playerCooldown = challengee->HasAura(SPELL_BESTED_UNDERCITY);
                        break;
                    default:
                        playerCooldown = false;
                        break;
                }

                if (playerCooldown)
                    return;

                // remove gossip flag
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                // remove pose aura, otherwise no walking animation
                me->RemoveAura(SPELL_READYJOUST_POSE_EFFECT);

                // move towards arena center
                float angle = me->GetAngle(&arenaCenter);
                float x = me->GetPositionX() + 22.0f * cos(angle);
                float y = me->GetPositionY() + 22.0f * sin(angle);
                me->GetMotionMaster()->MovePoint(0, x, y, me->GetPositionZ());
            }

        }

        uint32 GetCustomType() const
        {
            switch (me->GetEntry())
            {
                case NPC_EXODAR_CHAMPION:
                case NPC_DARNASSUS_CHAMPION:
                case NPC_STORMWIND_CHAMPION:
                case NPC_IRONFORGE_CHAMPION:
                case NPC_GNOMEREGAN_CHAMPION:
                case NPC_SILVERMOON_CHAMPION:
                case NPC_THUNDER_BLUFF_CHAMPION:
                case NPC_ORGRIMMAR_CHAMPION:
                case NPC_SENJIN_CHAMPION:
                case NPC_UNDERCITY_CHAMPION:
                    return TYPE_CHAMPION;
                case NPC_EXODAR_VALIANT:
                case NPC_DARNASSUS_VALIANT:
                case NPC_GNOMEREGAN_VALIANT:
                case NPC_IRONFORGE_VALIANT:
                case NPC_STORMWIND_VALIANT:
                    return TYPE_VALIANT_ALLIANCE;
                case NPC_SILVERMOON_VALIANT:
                case NPC_THUNDER_BLUFF_VALIANT:
                case NPC_UNDERCITY_VALIANT:
                case NPC_ORGRIMMAR_VALIANT:
                case NPC_SENJIN_VALIANT:
                    return TYPE_VALIANT_HORDE;
                default:
                    return TYPE_OTHER;
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            uint8 stackAmount;
            if (GetCustomType() == TYPE_CHAMPION)
                stackAmount = 3;
            else
                stackAmount = 2;

            for (uint8 i = 0; i < stackAmount; ++i)
                DoCast(me, SPELL_DEFEND, true);
        }

        void MovementInform(uint32 uiType, uint32 /*pointId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            // charge after moving away from the victim
            if (me->IsInCombat() && me->GetVictim() && bCharge)
            {
                me->GetMotionMaster()->Clear();
                // but only after rangecheck
                if (me->GetDistance(me->GetVictim()) > 5.0f && me->GetDistance(me->GetVictim()) <= 30.0f)
                    DoCastVictim(SPELL_CHARGE);
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                uiChargeTimer = GetCustomType() == TYPE_CHAMPION ? 6500 : 7500;
                bCharge = false;
            }
            else
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->setFaction(14);

                if (Player* player = ObjectAccessor::GetPlayer(*me, challengeeGUID))
                {
                    if (player->GetVehicle())
                        AttackStart(player->GetVehicleBase());
                    else
                        AttackStart(player);
                }
            }
        }

        void DamageTaken(Unit* who, uint32& damage) override
        {
            if (damage >= me->GetHealth() && who->GetTypeId() == TYPEID_PLAYER/* && !bDefeated*/)
            {
                damage = 0;
                GrantCredit(who);
                me->setFaction(35);
                me->DespawnOrUnsummon(5000);
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                EnterEvadeMode();
            }
        }

        void GrantCredit(Unit* who)
        {
            Player* player;
            if (!(player = who->ToPlayer()))
                return;

            switch (GetCustomType())
            {
                case TYPE_CHAMPION:
                {
                    who->CastSpell(who, SPELL_MOUNTED_MELEE_VICTORY_C, true);
                    uint32 creditSpell = 0;
                    switch (me->GetEntry())
                    {
                    case NPC_EXODAR_CHAMPION:
                        creditSpell = SPELL_BESTED_EXODAR;
                        break;
                    case NPC_DARNASSUS_CHAMPION:
                        creditSpell = SPELL_BESTED_DARNASSUS;
                        break;
                    case NPC_STORMWIND_CHAMPION:
                        creditSpell = SPELL_BESTED_STORMWIND;
                        break;
                    case NPC_IRONFORGE_CHAMPION:
                        creditSpell = SPELL_BESTED_IRONFORGE;
                        break;
                    case NPC_GNOMEREGAN_CHAMPION:
                        creditSpell = SPELL_BESTED_GNOMEREGAN;
                        break;
                    case NPC_SILVERMOON_CHAMPION:
                        creditSpell = SPELL_BESTED_SILVERMOON;
                        break;
                    case NPC_THUNDER_BLUFF_CHAMPION:
                        creditSpell = SPELL_BESTED_THUNDER_BLUFF;
                        break;
                    case NPC_ORGRIMMAR_CHAMPION:
                        creditSpell = SPELL_BESTED_ORGRIMMAR;
                        break;
                    case NPC_SENJIN_CHAMPION:
                        creditSpell = SPELL_BESTED_SENJIN;
                        break;
                    case NPC_UNDERCITY_CHAMPION:
                        creditSpell = SPELL_BESTED_UNDERCITY;
                        break;
                    }
                    who->CastSpell(who, creditSpell, false);
                    who->CastSpell(who, creditSpell, false); // second cast for criteria check...which is checked before aura is applied...HILARIOUS!
                    break;
                }
                case TYPE_VALIANT_ALLIANCE:
                case TYPE_VALIANT_HORDE:
                {
                     who->CastSpell(who, SPELL_MOUNTED_MELEE_VICTORY_V, true);
                     break;
                }
            }

        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiShieldTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_DEFEND);
                uiShieldTimer = GetCustomType() == TYPE_CHAMPION ? 3500 : 4500;
            }
            else uiShieldTimer -= uiDiff;

            if (uiChargeTimer <= uiDiff && !bCharge)
            {
                // directly charge if range is ok
                if (me->GetDistance(me->GetVictim()) > 5.0f && me->GetDistance(me->GetVictim()) <= 30.0f)
                {
                    DoCastVictim(SPELL_CHARGE);
                    uiChargeTimer = GetCustomType() == TYPE_CHAMPION ? 6500 : 7500;
                }
                else
                {
                    // move away for charge...
                    float angle = me->GetAngle(me->GetVictim());
                    float x = me->GetPositionX() + 20.0f * cos(angle);
                    float y = me->GetPositionY() + 20.0f * sin(angle);
                    me->GetMotionMaster()->MovePoint(0, x, y, me->GetPositionZ());
                    bCharge = true;
                }
            }
            else uiChargeTimer -= uiDiff;

            if (uiShieldBreakerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHIELD_BREAKER);
                uiShieldBreakerTimer = GetCustomType() == TYPE_CHAMPION ? 9000 : 10000;
            }
            else uiShieldBreakerTimer -= uiDiff;

            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                if (uiThrustTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_THRUST);
                    uiThrustTimer = GetCustomType() == TYPE_CHAMPION ? 1800 : 2000;
                }
                else uiThrustTimer -= uiDiff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new TW_npc_argent_faction_riderAI(creature);
    }
};

enum eBlackKnight
{
    SPELL_BK_CHARGE = 63003,
    //SPELL_SHIELD_BREAKER        = 65147,
    SPELL_DARK_SHIELD = 64505,
    SPELL_DEFEND_AURA_PERIODIC = 64223 // 10sec

};

#define YELL_ATTACK_PHASE_1_END         "Get off that horse and fight me man-to-man!"
#define YELL_ATTACK_PHASE_2             "I will not fail you, master!"

class TW_npc_the_black_knight : public CreatureScript
{
    public:
    TW_npc_the_black_knight() : CreatureScript("TW_npc_the_black_knight") { }

    struct TW_npc_the_black_knightAI : public ScriptedAI
    {
        TW_npc_the_black_knightAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        ObjectGuid guidAttacker;
        uint32 uireattackTimer;

        bool chargeing;

        bool mountDuel;
        bool handDuel;

        void Reset() override
        {
            me->m_CombatDistance = 100.0f; // lawl, copied from zuldrak.cpp
            uiChargeTimer = 7000;
            uiShieldBreakerTimer = 10000;
            uireattackTimer = 10000;
            me->setFaction(35);
            mountDuel = false;
            handDuel = false;
        }

        void SetGUID(ObjectGuid guid, int32) override
        {
            guidAttacker = guid;
            me->Mount(28652);
            me->GetMotionMaster()->MovePoint(0, 8430.947f, 966.5736f, 546.3145f);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->DespawnOrUnsummon(5000);
        }

        void EnterCombat(Unit* /*attacker*/) override
        {
            DoCast(me, SPELL_DEFEND_AURA_PERIODIC, true);
            if (Aura* aur = me->AddAura(SPELL_DEFEND, me))
                aur->ModStackAmount(1);
        }

        void MovementInform(uint32 uiType, uint32 uiId) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (uiId == 0)
            {
                if (Player* plr = ObjectAccessor::GetPlayer(*me, guidAttacker))
                {
                    me->SetMaxHealth(50000);
                    me->SetHealth(50000);

                    me->SetFacingToObject(plr);
                    Position pos = me->GetPosition();
                    me->SetHomePosition(pos);

                    mountDuel = true;
                    handDuel = false;
                    me->setFaction(14);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

                    if (plr->GetVehicleBase())
                        AttackStart(plr->GetVehicleBase()); // if player is mounted attack the mount
                    else
                        AttackStart(plr); // if by any reason player unmounts,attack him
                }
            }
            else if (uiId == 1)
            {

                chargeing = false;

                DoCastVictim(SPELL_BK_CHARGE);
                if (me->GetVictim())
                    me->GetMotionMaster()->MoveChase(me->GetVictim());

            }
            else if (uiId == 2)
            {
                if (Player* plr = ObjectAccessor::GetPlayer(*me, guidAttacker))
                {
                    if (plr->GetVehicleBase())
                        AttackStart(plr->GetVehicleBase()); // if player is mounted attack the mount
                    else
                        AttackStart(plr); // if by any reason player unmounts,attack him
                }
            }
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
        {
            if (pDoneBy && pDoneBy->GetGUID() != guidAttacker)
                uiDamage = 0;

            if (handDuel)
                return;
            if (!mountDuel)
                return;

            if (uiDamage > me->GetHealth() && pDoneBy->GetTypeId() == TYPEID_PLAYER)
            {
                uiDamage = 0;
                mountDuel = false;
                me->SetHealth(50000);
                me->Dismount();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAllAuras();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
                if (Player* plr = ObjectAccessor::GetPlayer(*me, guidAttacker))
                    me->Yell(YELL_ATTACK_PHASE_1_END, LANG_UNIVERSAL, plr);
                uireattackTimer = 10000;
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (mountDuel)
            {
                if (uiChargeTimer <= uiDiff)
                {
                    chargeing = true;
                    float x, y, z;
                    me->GetNearPoint(me, x, y, z, 1.0f, 15.0f, float(M_PI * 2 * rand_norm()));
                    me->GetMotionMaster()->MovePoint(1, x, y, z);

                    uiChargeTimer = 7000;
                }
                else uiChargeTimer -= uiDiff;

                if (uiShieldBreakerTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_SHIELD_BREAKER);
                    uiShieldBreakerTimer = 10000;
                }
                else uiShieldBreakerTimer -= uiDiff;

                if (me->isAttackReady())
                {
                    DoCast(me->GetVictim(), SPELL_THRUST, true);
                    me->resetAttackTimer();
                }
            }
            else if (handDuel)
            {
                if (uiShieldBreakerTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_DARK_SHIELD);
                    uiShieldBreakerTimer = 30000;
                }
                else uiShieldBreakerTimer -= uiDiff;

                DoMeleeAttackIfReady();
            }
            else
            {
                if (uireattackTimer <= uiDiff)
                {
                    handDuel = true;
                    if (me->GetVictim())
                        me->GetMotionMaster()->MoveChase(me->GetVictim());
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                    if (Player* plr = ObjectAccessor::GetPlayer(*me, guidAttacker))
                    if (plr->GetVehicleBase())
                        plr->ExitVehicle();

                    me->SetMaxHealth(12500);
                    me->SetHealth(12500);
                    if (Player* plr = ObjectAccessor::GetPlayer(*me, guidAttacker))
                        me->Yell(YELL_ATTACK_PHASE_2, LANG_UNIVERSAL, plr);
                    uireattackTimer = 99999999;
                }
                else uireattackTimer -= uiDiff;
            }
        }
    };

    CreatureAI *GetAI(Creature* creature) const override
    {
        return new TW_npc_the_black_knightAI(creature);
    }
};

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

            void UpdateAI(uint32 /*diff*/)
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

// 60192 - Freezing Arrow
class TW_spell_hun_freezing_arrow : public SpellScriptLoader
{
public:
    TW_spell_hun_freezing_arrow() : SpellScriptLoader("TW_spell_hun_freezing_arrow") { }

    class TW_spell_hun_freezing_arrow_SpellScript : public SpellScript
    {
        PrepareSpellScript(TW_spell_hun_freezing_arrow_SpellScript);

        bool Validate(SpellInfo const* spellInfo) override
        {
            if (!sSpellMgr->GetSpellInfo(spellInfo->Effects[0].TriggerSpell))
                return false;
            return true;
        }

        void TriggerTrap(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            uint32 trigger_spell = GetSpellInfo()->Effects[effIndex].TriggerSpell;
            Position pos = GetHitDest()->GetPosition();
            GetCaster()->ToPlayer()->RemoveSpellCooldown(trigger_spell);
            GetCaster()->CastSpell(pos.m_positionX, pos.m_positionY, pos.m_positionZ, trigger_spell, true);
        }

        void HandleHit(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(TW_spell_hun_freezing_arrow_SpellScript::TriggerTrap, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            OnEffectHit += SpellEffectFn(TW_spell_hun_freezing_arrow_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new TW_spell_hun_freezing_arrow_SpellScript();
    }
};

enum EarthLiving
{
    SPELL_BLESSING_OF_THE_ETERNALS = 51554
};

// -51940 - Earthliving Weapon (Passive) 
class TW_spell_sha_eathliving_passive : public SpellScriptLoader
{
public:
    TW_spell_sha_eathliving_passive() : SpellScriptLoader("TW_spell_sha_eathliving_passive") { }

    class TW_spell_sha_eathliving_passive_AuraScript : public AuraScript
    {
        PrepareAuraScript(TW_spell_sha_eathliving_passive_AuraScript);

        bool Validate(SpellInfo const* spellInfo) override
        {
            if (!sSpellMgr->GetSpellInfo(spellInfo->Effects[0].TriggerSpell))
                return false;
            return true;
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            uint32 chance = 20;
            if (Player* modOwner = GetCaster()->ToPlayer())
                modOwner->ApplySpellMod(GetSpellInfo()->Id, SPELLMOD_CHANCE_OF_SUCCESS, chance);

            if (AuraEffect const* eternal = GetCaster()->GetAuraEffectOfRankedSpell(SPELL_BLESSING_OF_THE_ETERNALS, EFFECT_1))
                if (eventInfo.GetProcTarget()->HealthBelowPctDamaged(35, eventInfo.GetHealInfo()->GetHitHeal()))
                    chance += eternal->GetAmount();

            return roll_chance_i(chance);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(TW_spell_sha_eathliving_passive_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new TW_spell_sha_eathliving_passive_AuraScript();
    }
};

// 74399 - Charge (LK Valk)
class TW_spell_icc_valk_charge : public SpellScriptLoader
{
public:
    TW_spell_icc_valk_charge() : SpellScriptLoader("TW_spell_icc_valk_charge") { }

    class TW_spell_icc_valk_charge_SpellScript : public SpellScript
    {
        PrepareSpellScript(TW_spell_icc_valk_charge_SpellScript);

        void ChargeDest(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            Position pos = GetHitDest()->GetPosition();
            float angle = GetCaster()->GetRelativeAngle(pos.GetPositionX(), pos.GetPositionY());
            float dist = GetCaster()->GetDistance2d(pos.GetPositionX(), pos.GetPositionY());
            pos = GetCaster()->GetFirstCollisionPosition(dist, angle, pos.m_positionZ);

            GetCaster()->GetMotionMaster()->MoveCharge(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(TW_spell_icc_valk_charge_SpellScript::ChargeDest, EFFECT_0, SPELL_EFFECT_CHARGE_DEST);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new TW_spell_icc_valk_charge_SpellScript();
    }
};

class TW_npc_gen_movable_cannon_hack : public CreatureScript
{
    public:
        TW_npc_gen_movable_cannon_hack() : CreatureScript("TW_npc_gen_movable_cannon_hack") { }

        struct TW_npc_gen_movable_cannon_hackAI : public PassiveAI
        {
            TW_npc_gen_movable_cannon_hackAI(Creature* creature) : PassiveAI(creature) { }

            void OnCharmed(bool /*apply*/) override
            {
                me->SetControlled(false, UNIT_STATE_ROOT);
                me->SetControlled(true, UNIT_STATE_ROOT);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new TW_npc_gen_movable_cannon_hackAI(creature);
        }
};

enum TauntFlagSpells
{
    SPELL_TAUNT_FLAG_SUMMON = 51657,

    EMOTE_TAUNT = 28008
};

class TauntFlagFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
            return target->ToPlayer()->IsAlive();
        else
            return true;
    }
};

class TW_spell_taunt_flag : public SpellScriptLoader
{
public:
    TW_spell_taunt_flag() : SpellScriptLoader("TW_spell_taunt_flag") { }

    class TW_spell_taunt_flag_SpellScript : public SpellScript
    {
        PrepareSpellScript(TW_spell_taunt_flag_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_TAUNT_FLAG_SUMMON))
                return false;
            return true;
        }

        SpellCastResult CheckIfCorpseNear()
        {
            Unit* caster = GetCaster();
            float max_range = GetSpellInfo()->Effects[0].CalcRadius();
            WorldObject* result = NULL;
            // search for nearby enemy corpse in range
            Trinity::AnyDeadUnitSpellTargetInRangeCheck check(caster, max_range, GetSpellInfo(), TARGET_CHECK_ENEMY);
            Trinity::WorldObjectSearcher<Trinity::AnyDeadUnitSpellTargetInRangeCheck> searcher(caster, result, check);
            caster->GetMap()->VisitFirstFound(caster->m_positionX, caster->m_positionY, max_range, searcher);
            if (!result)
                return SPELL_FAILED_BAD_TARGETS;
            return SPELL_CAST_OK;
        }

        void SelectTarget(std::list<WorldObject*>& targets)
        {
            targets.remove_if(TauntFlagFilter());
            targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_TAUNT_FLAG_SUMMON));
            if (targets.empty())
                return;

            WorldObject* _target = Trinity::Containers::SelectRandomContainerElement(targets);
            targets.clear();
            targets.push_back(_target);
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->TextEmote(EMOTE_TAUNT, GetHitUnit());
            GetCaster()->CastSpell(GetHitUnit(), SPELL_TAUNT_FLAG_SUMMON, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(TW_spell_taunt_flag_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnCheckCast += SpellCheckCastFn(TW_spell_taunt_flag_SpellScript::CheckIfCorpseNear);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(TW_spell_taunt_flag_SpellScript::SelectTarget, EFFECT_0, TARGET_CORPSE_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new TW_spell_taunt_flag_SpellScript();
    }
};

enum LandmineKnockbackAchievement
{
    SPELL_LANDMINE_KNOCKBACK_ACHIEMENT = 57064,
};

class spell_gen_landmine_knockback : public SpellScriptLoader
{
public:
    spell_gen_landmine_knockback() : SpellScriptLoader("spell_gen_landmine_knockback") { }

    class spell_gen_landmine_knockback_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gen_landmine_knockback_SpellScript);

        bool Validate(SpellInfo const* /*spell*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_LANDMINE_KNOCKBACK_ACHIEMENT))
                return false;
            return true;
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Player* target = GetHitPlayer())
            {
                Aura const* aura = GetHitAura();
                if (!aura || aura->GetStackAmount() != 10)
                    return;
                // spell 57064 Landmine Knockback Achievement seems to not trigger the achieve..so using hack
                AchievementEntry const* achiev = sAchievementStore.LookupEntry(1428);
                target->CompletedAchievement(achiev);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_gen_landmine_knockback_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gen_landmine_knockback_SpellScript();
    }
};

enum PutricideTrapData
{
    SPELL_GIANT_INSECT_SWARM = 70475,
    DATA_INSECT_SWARM_EVENT  = 43,
    EVENT_SUMMON_INSECTS     = 1,
    EVENT_FLESH_EATING_BITE  = 2,
    NPC_FLESH_EATING_INSECT  = 37782,
    ACTION_START_TRAP_EVENT  = 20,
    ACTION_END_TRAP_EVENT    = 21,
    SPELL_FLESH_EATING_BITE  = 72967
};

class TW_npc_putricide_insect_trap : public CreatureScript
{
    public:
        TW_npc_putricide_insect_trap() : CreatureScript("TW_npc_putricide_insect_trap") { }

    struct TW_npc_putricide_insect_trapAI : public ScriptedAI
    {
        TW_npc_putricide_insect_trapAI(Creature* creature) : ScriptedAI(creature), _summons(me)
        { 
            _instance = creature->GetInstanceScript();
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            creature->SetDisplayId(11686);
        }

        void DoAction(int32 actionId) override
        {
            if (actionId == ACTION_START_TRAP_EVENT)
            {
                me->AI()->DoCastAOE(SPELL_GIANT_INSECT_SWARM);
                _events.ScheduleEvent(EVENT_SUMMON_INSECTS, 2000);
            }
            else if (actionId == ACTION_END_TRAP_EVENT)
            {
                _summons.DespawnAll();
                me->InterruptNonMeleeSpells(true);
                EnterEvadeMode();
                me->SetVisible(false);
            }
        }

        void EnterCombat(Unit* /* who */) override
        {
            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) != DONE)
                SummonInsects(me);
        }

        bool CanAIAttack(Unit const* /*who*/) const override
        {
            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) == DONE)
                return false;

            return true;
        }

        void UpdateAI(uint32 diff) override
        {
            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) != IN_PROGRESS)
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_SUMMON_INSECTS)
                {
                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 25.0f, true))
                        SummonInsects(victim);

                    _events.ScheduleEvent(EVENT_SUMMON_INSECTS, 3000);
                }
            }
        }

        void JustSummoned(Creature* summon) override
        {
            _summons.Summon(summon);
            summon->SetReactState(REACT_AGGRESSIVE);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            _summons.Despawn(summon);
        }

        void SummonInsects(Unit* victim)
        {
            for (uint32 i = 0; i < 5; ++i)
                if (Creature* insect = me->SummonCreature(NPC_FLESH_EATING_INSECT, victim->GetPositionX() + float(irand(-7, 7)), victim->GetPositionY() + float(irand(-7, 7)), victim->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
                    if (victim != me)
                        insect->AI()->AttackStart(victim);

        }

        private:
            EventMap _events;
            InstanceScript* _instance;
            SummonList _summons;
    };

    CreatureAI *GetAI(Creature* creature) const override
    {
        return new TW_npc_putricide_insect_trapAI(creature);
    }
};

class TW_npc_flesh_eating_insect : public CreatureScript
{
    public:
         TW_npc_flesh_eating_insect() : CreatureScript("TW_npc_flesh_eating_insect") { }

    struct TW_npc_flesh_eating_insectAI : public ScriptedAI
    {
        TW_npc_flesh_eating_insectAI(Creature* creature) : ScriptedAI(creature)  
        { 
            _instance = creature->GetInstanceScript();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _events.ScheduleEvent(EVENT_FLESH_EATING_BITE, 2000);
        }

        void DamageTaken(Unit* /*done_by*/, uint32&) override { }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) != IN_PROGRESS)
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_FLESH_EATING_BITE)
                {
                    DoCast(me, SPELL_FLESH_EATING_BITE, true);
                    _events.ScheduleEvent(EVENT_FLESH_EATING_BITE, 3000);
                }
            }

            DoMeleeAttackIfReady();
        }

        private:
            EventMap _events;
            InstanceScript* _instance;
    };

    CreatureAI *GetAI(Creature* creature) const override
    {
        return new TW_npc_flesh_eating_insectAI(creature);
    }
};

void AddSC_custom_scripts()
{
    new TW_npc_argent_squire();
    new TW_npc_argent_champion();
    new TW_npc_argent_faction_rider();
    new TW_npc_the_black_knight();
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
    new TW_spell_hun_freezing_arrow();
    new TW_spell_sha_eathliving_passive();
    new TW_spell_icc_valk_charge();
    new TW_npc_gen_movable_cannon_hack();
    new TW_spell_taunt_flag();
    new spell_gen_landmine_knockback();
    new TW_npc_putricide_insect_trap();
    new TW_npc_flesh_eating_insect();
}
