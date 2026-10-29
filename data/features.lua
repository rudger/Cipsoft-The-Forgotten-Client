-- WARNING:
-- This file controls real protocol packet parsing.
-- Use only true/false values. Restart the client after changing this file.
--
-- true  = the client reads/sends this protocol feature and shows its UI.
-- false = the client does not read/send this protocol feature and hides its UI.
--
-- Important:
-- If your server still sends bytes for a feature that you set to false, packets can
-- desync and the client can freeze, debug, or disconnect. The client and server
-- must agree.
--
-- Profiles:
--   cip772        = tibia-gameserver-unified (CipSoft 7.72, login 7171 / game 7172).
--                   Needs the 7.72 Tibia.dat / Tibia.spr in data/Assets.
--   tfs860_custom = Mateus TFS 1.8 downgrade 8.60.
-- Select a profile with selectedProfile or per server in serverProfiles.

ClientFeatures = {
    selectedProfile = "cip772",

    default = {
        protocol = 772,
        host = "127.0.0.1",
        port = 7171,
    },

    serverProfiles = {
        ["127.0.0.1:7171"] = "cip772",
        ["localhost:7171"] = "cip772",
    },

    profiles = {
        cip772 = {
            description = "CipSoft 7.72 (tibia-gameserver-unified)",
            protocol = 772,
            clientVersion = 772,
            fileVersion = 772,

            -- Use the client's built-in 7.72 features: numeric account number,
            -- RSA + XTEA, no checksum, no 8.x extensions.
            inheritDefaults = true,

            features = {
                -- Quests button in the inventory and Ctrl+U.
                -- The server does not answer quest log requests yet.
                GameQuestLog = true,

                -- "!" button (Receive hint) in the top right corner of the game
                -- window, as in the 7.72 client. Needs the 7.72 Tibia.pic.
                -- Shown only, it does nothing yet.
                GameHintButton = false,

                -- Buttons next to the game window to open and close extra sidebars.
                -- false hides them and only the standard sidebar is used.
                GameExtraSidebars = false,

                -- Blue mana bar under the health bar of your own character.
                -- Adds "Show player mana bar" to General Options to turn it on/off.
                GameManaBar = true,

                -- Speed in the skills window, under Capacity.
                GameSkillsSpeed = true,

                -- NPC names and health bars in blue (screen and battle list).
                -- NPCs are recognised by their creature id (0x80000000 and up).
                GameBlueNPCs = true,

                -- Time left of items with a duration (rings, torches, ...) in the
                -- inventory and backpacks, e.g. "18m". The server must send it: set
                -- send_item_duration = true in the server config.cfg, otherwise the
                -- client disconnects.
                GameItemDuration = true,

                -- Loot frames around items in containers by their value (the most
                -- an NPC pays): grey, green (1k), blue (10k), purple (100k), yellow
                -- (1kk). "Show loot frames" in General Options turns them on/off.
                -- The server must send it: set send_item_values = true in the
                -- server config.cfg, otherwise the client disconnects.
                GameItemValues = true,

                -- Pigeon icon in the inventory while standing in a protection zone.
                -- The server must send it: set send_pz_icon = true in the server
                -- config.cfg, otherwise the client disconnects.
                GamePlayerIconsU16 = true,

                -- Real liquid type for fluids (mana fluid and wine are both purple),
                -- so hotkeys can tell them apart. The server must send it: set
                -- send_liquid_type = true in the server config.cfg, otherwise
                -- fluids show the wrong color.
                GameNewFluids = true,

                -- Stamina (42:00 hours) in the skills window, under Food. The server
                -- must send it: set send_stamina = true in the server config.cfg,
                -- otherwise the client disconnects.
                GamePlayerStamina = true,

                -- Outfit window with a list of outfits and names (old and new 7.92
                -- outfits). The server must send it: set send_outfit_list = true in
                -- the server config.cfg, otherwise the client disconnects.
                GameNewOutfitProtocol = true,

                -- Food time in the skills window, under Speed (0:05 = 5 minutes).
                -- The server must send it: set send_food_time = true in the
                -- server config.cfg, otherwise the client disconnects.
                GameRegenerationTime = true,

                -- Level of the speaker with chat/private messages: "Name [level]: text"
                -- (Console Options -> "Show levels" must be on). The server must send it:
                -- set send_message_level = true in the server config.cfg, otherwise the
                -- client disconnects.
                GameMessageLevel = true,
            }
        },

        tfs860_custom = {
            description = "TFS 1.8 downgrade 8.60 custom protocol",
            protocol = 860,
            clientVersion = 860,
            fileVersion = 860,

            -- false below really disables the feature. The server must match.
            inheritDefaults = true,

            features = {
                -- Core 8.60 protocol.
                GameUpdateTile = false,
                GameLoginPacketEncryption = true,
                GameRSA1024 = true,
                GameMessageStatements = true,
                GameLooktypeU16 = true,
                GameNewFluids = true,
                GameMessageLevel = true,
                GamePlayerStateU16 = true,
                GamePlayerAddons = true,
                GamePlayerStamina = true,
                GameNewOutfitProtocol = true,
                GameWritableDate = true,
                GameNpcInterface = true,
                GameProtocolChecksum = true,
                GameAccountNames = true,
                GameChallengeOnLogin = true,
                GameDoubleFreeCapacity = true,
                GameTileAddThingWithStackpos = true,
                GameCreatureEmblems = true,
                GameAttackSeq = true,
                GameQuestLog = true,

                -- Your TFS 1.8 custom 8.60 features.
                -- Example: setting GamePlayerMounts = false hides Mount/Dismount
                -- and blocks the server feature packet from enabling mounts again.
                GamePlayerMounts = false,
                GameMagicEffectU16 = false,
                GameDistanceEffectU16 = false,
                GameDoubleHealth = false,
                GameOfflineTrainingTime = false,
                GameSkillsBase = false,
                GameBaseSkillU16 = false,
                GameDoubleSkills = false,
                GameAdditionalSkills = false,
                GameIdleAnimations = false,
                GameEnhancedAnimations = false,
                GameExtendedClientPing = false,
                GameSpritesU32 = false,
                GameDoublePlayerGoodsMoney = false,
                GameCreatureIcons = false,
                GamePurseSlot = false,
                GamePrey = false,
                GameThingUpgradeClassification = false,

                -- Keep these false unless your server code really sends them.
                GameDeathPenalty = false,
                GameDoubleExperience = false,
                GameSpellList = false,
                GameChatPlayerList = false,
                GameTotalCapacity = false,
                GameRegenerationTime = false,
                GameItemAnimationPhases = false,
                GameEnvironmentEffects = false,
                GameNpcNameOnTrade = false,
                GamePlayerMarket = false,
                GamePing = false,
                GameDoubleShopBuyAmount = false,
                GameDoubleShopSellAmount = false,
                GameMinimapRemoveMark = false,
                GameItemsU16 = false,
                GameLookAtCreature = false,
                GameAdditionalVipInfo = false,
                GamePreviewState = false,
                GameClientVersion = false,
                GameKeepConnectionAfterDeath = false,
                GameLoginPending = false,
                GameVipStatus = false,
                GameNewSpeedLaw = false,
                GameBrowseField = false,
                GameContainerPagination = false,
                GamePVPMode = false,
                GameThingMarks = false,
                GameCreatureType = false,
                GameHideNpcNames = false,
                GamePremiumExpiration = false,
                GameUnjustifiedPoints = false,
                GameExperienceBonus = false,
                GameDeathType = false,
                GameOGLInformation = false,
                GameContentRevision = false,
                GameAuthenticator = false,
                GameSessionKey = false,
                GameEquipHotkey = false,
                GameIngameStore = false,
                GameWrappable = false,
                GameStoreInbox = false,
                GameIngameStoreServiceType = false,
                GameIngameStoreHighlights = false,
                GameDetailedExperienceBonus = false,
                GameStoreHighlights2 = false,
                GameNewFilesStructure = false,
                GameClientSendServerName = false,
                GameImbuing = false,
                GameVipGroups = false,
                GameInspection = false,
                GameSequencedPackets = false,
                GameBlessDialog = false,
                GameQuestTracker = false,
                GameTime = false,
                GameCompedium = false,
                GamePlayerStateU32 = false,
                GameRewardWall = false,
                GameAnalytics = false,
                GameQuickLoot = false,
                GameExtendedCapacity = false,
                GameCyclopediaMonsters = false,
                GameStash = false,
                GameCyclopediaMapAndDetails = false,
                GameDoublePercentSkills = false,
                GameTournaments = false,
                GameAccountEmail = false,
            }
        },
    }
}
