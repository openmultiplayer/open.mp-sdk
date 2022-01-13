#pragma once

#include <chrono>
#include <list>
#include <sdk.hpp>
#include <vehicle_params.hpp>

struct IPlayer;

struct VehicleSpawnData {
    int modelID;
    Vector3 position;
    float zRotation;
    int colour1;
    int colour2;
    Seconds respawnDelay;
    bool siren;
};

struct UnoccupiedVehicleUpdate {
    uint8_t seat;
    Vector3 position;
    Vector3 velocity;
};

enum VehicleSCMEvent : uint32_t {
    VehicleSCMEvent_SetPaintjob = 1,
    VehicleSCMEvent_AddComponent,
    VehicleSCMEvent_SetColour,
    VehicleSCMEvent_EnterExitModShop
};

struct VehicleModelInfo {
    Vector3 Size;
    Vector3 FrontSeat;
    Vector3 RearSeat;
    Vector3 PetrolCap;
    Vector3 FrontWheel;
    Vector3 RearWheel;
    Vector3 MidWheel;
    float FrontBumperZ;
    float RearBumperZ;
};

enum VehicleComponentSlot {
    VehicleComponent_None = -1,
    VehicleComponent_Spoiler = 0,
    VehicleComponent_Hood = 1,
    VehicleComponent_Roof = 2,
    VehicleComponent_SideSkirt = 3,
    VehicleComponent_Lamps = 4,
    VehicleComponent_Nitro = 5,
    VehicleComponent_Exhaust = 6,
    VehicleComponent_Wheels = 7,
    VehicleComponent_Stereo = 8,
    VehicleComponent_Hydraulics = 9,
    VehicleComponent_FrontBumper = 10,
    VehicleComponent_RearBumper = 11,
    VehicleComponent_VentRight = 12,
    VehicleComponent_VentLeft = 13,
};

enum VehicleVelocitySetType : uint8_t {
    VehicleVelocitySet_Normal = 0,
    VehicleVelocitySet_Angular
};

enum VehicleModelInfoType {
    VehicleModelInfo_Size = 1,
    VehicleModelInfo_FrontSeat,
    VehicleModelInfo_RearSeat,
    VehicleModelInfo_PetrolCap,
    VehicleModelInfo_WheelsFront,
    VehicleModelInfo_WheelsRear,
    VehicleModelInfo_WheelsMid,
    VehicleModelInfo_FrontBumperZ,
    VehicleModelInfo_RearBumperZ
};

struct VehicleDriverSyncPacket {
    int PlayerID;
    uint16_t VehicleID;
    uint16_t LeftRight;
    uint16_t UpDown;
    uint16_t Keys;
    GTAQuat Rotation;
    Vector3 Position;
    Vector3 Velocity;
    float Health;
    Vector2 PlayerHealthArmour;
    uint8_t Siren;
    uint8_t LandingGear;
    uint16_t TrailerID;
    bool HasTrailer;

    union {
        uint8_t AdditionalKeyWeapon;
        struct {
            uint8_t WeaponID : 6;
            uint8_t AdditionalKey : 2;
        };
    };

    union {
        uint32_t HydraThrustAngle;
        float TrainSpeed;
    };
};

struct VehiclePassengerSyncPacket {
    int PlayerID;
    int VehicleID;

    union {
        uint16_t DriveBySeatAdditionalKeyWeapon;
        struct {
            uint8_t SeatID : 2;
            uint8_t DriveBy : 6;
            uint8_t WeaponID : 6;
            uint8_t AdditionalKey : 2;
        };
    };

    Vector2 HealthArmour;
    uint16_t LeftRight;
    uint16_t UpDown;
    uint16_t Keys;
    Vector3 Position;
};

struct VehicleUnoccupiedSyncPacket {
    int VehicleID;
    int PlayerID;
    uint8_t SeatID;
    Vector3 Roll;
    Vector3 Rotation;
    Vector3 Position;
    Vector3 Velocity;
    Vector3 AngularVelocity;
    float Health;
};

struct VehicleTrailerSyncPacket {
    int VehicleID;
    int PlayerID;
    Vector3 Position;
    Vector4 Quat;
    Vector3 Velocity;
    Vector3 TurnVelocity;
};

/// A vehicle interface
struct IVehicle : public IEntity {

    /// Set the inital spawn data of the vehicle
    virtual void setSpawnData(const VehicleSpawnData& data) = 0;

    /// Checks if player has the vehicle streamed in for themselves
    virtual bool isStreamedInForPlayer(const IPlayer& player) const = 0;

    /// Streams in the vehicle for a specific player
    virtual void streamInForPlayer(IPlayer& player) = 0;

    /// Streams out the vehicle for a specific player
    virtual void streamOutForPlayer(IPlayer& player) = 0;

    /// Set the vehicle's colour
    virtual void setColour(int col1, int col2) = 0;

    /// Get the vehicle's colour
    virtual Pair<int, int> getColour() const = 0;

    /// Set the vehicle's health
    virtual void setHealth(float Health) = 0;

    /// Get the vehicle's health
    virtual float getHealth() = 0;

    /// Update the vehicle from a sync packet
    virtual bool updateFromDriverSync(const VehicleDriverSyncPacket& vehicleSync, IPlayer& player) = 0;

    /// Update the vehicle from a passenger sync packet
    virtual bool updateFromPassengerSync(const VehiclePassengerSyncPacket& passengerSync, IPlayer& player) = 0;

    /// Update the vehicle from an unoccupied sync packet
    virtual bool updateFromUnoccupied(const VehicleUnoccupiedSyncPacket& unoccupiedSync, IPlayer& player) = 0;

    /// Update the vehicle from a trailer sync packet
    virtual bool updateFromTrailerSync(const VehicleTrailerSyncPacket& unoccupiedSync, IPlayer& player) = 0;

    /// Returns the current driver of the vehicle
    virtual IPlayer* getDriver() = 0;

    /// Returns the passengers of the vehicle
    virtual const FlatHashSet<IPlayer*>& getPassengers() = 0;

    /// Sets the vehicle's number plate
    virtual void setPlate(StringView plate) = 0;

    /// Get the vehicle's number plate
    virtual const StringView getPlate() = 0;

    /// Sets the vehicle's damage status
    virtual void setDamageStatus(int PanelStatus, int DoorStatus, uint8_t LightStatus, uint8_t TyreStatus, IPlayer* vehicleUpdater = nullptr) = 0;

    /// Gets the vehicle's damage status
    virtual void getDamageStatus(int& PanelStatus, int& DoorStatus, int& LightStatus, int& TyreStatus) = 0;

    /// Sets the vehicle's paintjob
    virtual void setPaintJob(int paintjob) = 0;

    /// Gets the vehicle's paintjob
    virtual int getPaintJob() = 0;

    /// Adds a component to the vehicle.
    virtual void addComponent(int component) = 0;

    /// Gets the vehicle's component in a designated slot
    virtual int getComponentInSlot(int slot) = 0;

    /// Removes a vehicle's component.
    virtual void removeComponent(int component) = 0;

    /// Puts the player inside this vehicle.
    virtual void putPlayer(IPlayer& player, int SeatID) = 0;

    /// Removes the player from the vehicle.
    virtual void removePlayer(IPlayer& player) = 0;

    /// Set the vehicle's Z angle.
    virtual void setZAngle(float angle) = 0;

    /// Gets the vehicle's Z angle.
    virtual float getZAngle() = 0;

    // Set the vehicle's parameters.
    virtual void setParams(const VehicleParams& params) = 0;

    // Set the vehicle's parameters for a specific player.
    virtual void setParamsForPlayer(IPlayer& player, const VehicleParams& params) = 0;

    // Get the vehicle's parameters.
    virtual VehicleParams const& getParams() = 0;

    /// Checks if the vehicle is dead.
    virtual bool isDead() = 0;

    /// Respawns the vehicle.
    virtual void respawn() = 0;

    /// Get the vehicle's respawn delay.
    virtual Seconds getRespawnDelay() = 0;

    /// Checks if the vehicle is respawning.
    virtual bool isRespawning() = 0;

    // Sets (links) the vehicle to an interior.
    virtual void setInterior(int InteriorID) = 0;

    // Gets the vehicle's interior.
    virtual int getInterior() = 0;

    /// Attaches a vehicle as a trailer to this vehicle.
    virtual void attachTrailer(IVehicle& trailer) = 0;

    /// Detaches a vehicle from this vehicle.
    virtual void detachTrailer() = 0;

    /// Checks if the current vehicle is a trailer.
    virtual bool isTrailer() const = 0;

    /// Get the current vehicle's attached trailer.
    virtual IVehicle* getTrailer() const = 0;

    /// Fully repair the vehicle.
    virtual void repair() = 0;

    /// Adds a train carriage to the vehicle (ONLY FOR TRAINS).
    virtual void addCarriage(IVehicle* carriage, int pos) = 0;
    virtual void updateCarriage(Vector3 pos, Vector3 veloc) = 0;
    virtual StaticArray<IVehicle*, 3> getCarriages() = 0;

    /// Sets the velocity of the vehicle.
    virtual void setVelocity(Vector3 velocity) = 0;

    /// Gets the current velocity of the vehicle.
    virtual Vector3 getVelocity() = 0;

    /// Sets the angular velocity of the vehicle.
    virtual void setAngularVelocity(Vector3 velocity) = 0;

    /// Gets the current angular velocity of the vehicle.
    virtual Vector3 getAngularVelocity() = 0;

    /// Gets the current model ID of the vehicle.
    virtual int getModel() = 0;

    /// Gets the current landing gear state from a ID_VEHICLE_SYNC packet from the latest driver.
    virtual uint8_t getLandingGearState() = 0;
};

/// A vehicle event handler
struct VehicleEventHandler {
    virtual void onVehicleStreamIn(IVehicle& vehicle, IPlayer& player) { }
    virtual void onVehicleStreamOut(IVehicle& vehicle, IPlayer& player) { }
    virtual void onVehicleDeath(IVehicle& vehicle, IPlayer& player) { }
    virtual void onPlayerEnterVehicle(IPlayer& player, IVehicle& vehicle, bool passenger) { }
    virtual void onPlayerExitVehicle(IPlayer& player, IVehicle& vehicle) { }
    virtual void onVehicleDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) { }
    virtual bool onVehiclePaintJob(IPlayer& player, IVehicle& vehicle, int paintJob) { return true; }
    virtual bool onVehicleMod(IPlayer& player, IVehicle& vehicle, int component) { return true; }
    virtual bool onVehicleRespray(IPlayer& player, IVehicle& vehicle, int colour1, int colour2) { return true; }
    virtual void onEnterExitModShop(IPlayer& player, bool enterexit, int interiorID) { }
    virtual void onVehicleSpawn(IVehicle& vehicle) { }
    virtual bool onUnoccupiedVehicleUpdate(IVehicle& vehicle, IPlayer& player, UnoccupiedVehicleUpdate const updateData) { return true; }
    virtual bool onTrailerUpdate(IPlayer& player, IVehicle& trailer) { return true; }
    virtual bool onVehicleSirenStateChange(IPlayer& player, IVehicle& vehicle, uint8_t sirenState) { return true; }
};

/// A vehicle pool
static const UUID VehicleComponent_UUID = UUID(0x3f1f62ee9e22ab19);
struct IVehiclesComponent : public IPoolComponent<IVehicle, VEHICLE_POOL_SIZE> {
    PROVIDE_UUID(VehicleComponent_UUID)

    /// Get the number of model instances for each model
    virtual StaticArray<uint8_t, MAX_VEHICLE_MODELS>& models() = 0;

    virtual IVehicle* create(int modelID, Vector3 position, float Z = 0.0f, int colour1 = -1, int colour2 = -1, Seconds respawnDelay = Seconds(-1), bool addSiren = false) = 0;
    virtual IVehicle* create(const VehicleSpawnData& data) = 0;

    virtual IEventDispatcher<VehicleEventHandler>& getEventDispatcher() = 0;
};

/// Player vehicle data
static const UUID SomePlayerData_UUID = UUID(0xa960485be6c70fb2);
struct IPlayerVehicleData : public IPlayerData {
    PROVIDE_UUID(SomePlayerData_UUID)

    /// Get the player's vehicle
    /// Returns nullptr if they aren't in a vehicle
    virtual IVehicle* getVehicle() = 0;

    /// Get the player's seat
    /// Returns -1 if they aren't in a vehicle.
    virtual int getSeat() const = 0;
};
