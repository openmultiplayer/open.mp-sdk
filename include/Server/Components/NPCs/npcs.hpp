#pragma once

#include <sdk.hpp>

enum NPCMoveType
{
	NPCMoveType_None,
	NPCMoveType_Walk,
	NPCMoveType_Jog,
	NPCMoveType_Sprint
};

struct INPC : public IExtensible, public IEntity
{
	/// Get player instance of NPC.
	virtual IPlayer* getPlayer() = 0;

	/// Spawn NPC.
	virtual void spawn() = 0;

	/// Move NPC to a specified location.
	virtual bool move(Vector3 position, NPCMoveType moveType) = 0;

	/// Stop NPC from moving.
	virtual void stopMove() = 0;

	/// Set NPC skin
	virtual void setSkin(int model) = 0;

	/// Check if a player is streamed in for the current NPC
	virtual bool isStreamedInForPlayer(const IPlayer& other) const = 0;

	/// Get the NPC which are streamed in for this player
	virtual const FlatPtrHashSet<IPlayer>& streamedForPlayers() const = 0;

	/// Set NPC interior, it doesn't do much but keeps a record internally to use getter functions later
	virtual void setInterior(unsigned int interior) = 0;

	/// Get NPC interior, just the value that is stored internally
	virtual unsigned int getInterior() const = 0;

	/// Get NPC velocity
	virtual Vector3 getVelocity() const = 0;

	/// Set NPC velocity
	virtual void setVelocity(Vector3 position, bool update = false) = 0;

	/// Set health
	virtual void setHealth(float health) = 0;

	/// Get health
	virtual float getHealth() const = 0;

	/// Set armour
	virtual void setArmour(float armour) = 0;

	/// Get armour
	virtual float getArmour() const = 0;

	/// Check if NPC is dead
	virtual bool isDead() const = 0;

	/// Set weapon
	virtual void setWeapon(uint8_t weapon) = 0;

	/// Get weapon
	virtual uint8_t getWeapon() const = 0;

	/// Set ammo
	virtual void setAmmo(int ammo) = 0;

	/// Get ammo
	virtual int getAmmo() const = 0;

	/// Set weapon skill level
	virtual void setWeaponSkillLevel(PlayerWeaponSkill weaponSkill, int level) = 0;

	/// Get weapon skill level
	virtual int getWeaponSkillLevel(PlayerWeaponSkill weaponSkill) const = 0;

	/// Set keys
	virtual void setKeys(uint16_t upAndDown, uint16_t leftAndRight, uint16_t keys) = 0;

	/// Get keys
	virtual void getKeys(uint16_t& upAndDown, uint16_t& leftAndRight, uint16_t& keys) const = 0;

	/// Melee attack
	virtual void meleeAttack(int time, bool secondaryMeleeAttack = false) = 0;

	/// Stop melee attack
	virtual void stopMeleeAttack() = 0;

	/// Check if npc is doing melee attack
	virtual bool isMeleeAttacking() const = 0;

	/// Set fighting style
	virtual void setFightingStyle(PlayerFightingStyle style) = 0;

	/// Get fighting style
	virtual PlayerFightingStyle getFightingStyle() const = 0;

	/// Set state to reloading
	virtual void enableReloading(bool toggle) = 0;

	/// Check if NPC is reloading
	virtual bool isReloadEnabled() const = 0;

	/// Check if reloading
	virtual bool isReloading() const = 0;

	/// Enable or disable infinite ammo
	virtual void enableInfiniteAmmo(bool enable) = 0;

	/// Check if NPC is reloading
	virtual bool isInfiniteAmmoEnabled() const = 0;

	/// Get weapon state
	virtual PlayerWeaponState getWeaponState() const = 0;

	/// Set ammo in current clip
	virtual void setAmmoInClip(int ammo) = 0;

	/// Get ammo in current clip
	virtual int getAmmoInClip() const = 0;

	/// Trigger a weapon shot
	virtual void shoot(int hitId, PlayerBulletHitType hitType, uint8_t weapon, const Vector3& endPoint, const Vector3& offset, bool isHit, uint8_t betweenCheckFlags) = 0;

	/// Check if shooting
	virtual bool isShooting() const = 0;

};

struct NPCEventHandler
{
	virtual void onNPCFinishMove(INPC& npc) {};
	virtual void onNPCCreate(INPC& npc) {};
	virtual void onNPCDestroy(INPC& npc) {};
	virtual void onNPCSpawn(INPC& npc) {};
	virtual void onNPCWeaponStateChange(INPC& npc, PlayerWeaponState newState, PlayerWeaponState oldState) {};
	virtual bool onNPCTakeDamage(INPC& npc, IPlayer& damager, float damage, uint8_t weapon, BodyPart bodyPart) { return true; };
	virtual bool onNPCGiveDamage(INPC& npc, IPlayer& damaged, float damage, uint8_t weapon, BodyPart bodyPart) { return true; };
	virtual void onNPCDeath(INPC& npc, IPlayer* killer, int reason) {};
	virtual bool onNPCShotMissed(INPC& npc, const PlayerBulletData& bulletData) { return true; }
	virtual bool onNPCShotPlayer(INPC& npc, IPlayer& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onNPCShotNPC(INPC& npc, INPC& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onNPCShotVehicle(INPC& npc, IVehicle& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onNPCShotObject(INPC& npc, IObject& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onNPCShotPlayerObject(INPC& npc, IPlayerObject& target, const PlayerBulletData& bulletData) { return true; }
};

static const UID NPCComponent_UID = UID(0x3D0E59E87F4E90BC);
struct INPCComponent : public IPool<INPC>, public INetworkComponent
{
	PROVIDE_UID(NPCComponent_UID)

	/// Get the npc event dispatcher
	virtual IEventDispatcher<NPCEventHandler>& getEventDispatcher() = 0;

	/// Create a controllable NPC
	virtual INPC* create(StringView name) = 0;

	/// Destroy an NPC. We need this because it's more than just an entity removal from a pool
	virtual void destroy(INPC& npc) = 0;
};
