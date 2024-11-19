namespace core {

    enum TagIndex : int {
        UNDEFINED = 0,

        // Player (Megaman)
        PLAYER = 1,

        // Enemies
        METALL = 2,
        BUNBY_HEAD = 3,
        BUNBY_TANK = 4,
        BOSS = 5,
        BOSS_SHIELD = 6,

        // Megaman Weapon
        COMMON_BULLET = 7,
        CHARGE_X1 = 8,
        CHARGE_X2 = 9,

        // Enemies Weapon
        METALL_BULLET = 10,
        BOSS_BLADE = 11,

        // Background
        PLATFORM = 12,
        BOUNDARY = 13,
        DEADEND = 14,

        // Enemies Sensor
        AWAKE_SENSOR = 15,
        DEADEND_SENSOR = 16,

        // Trigger Points
        TRIGGER_POINT = 17,
        BOSS_MOVE_POINT = 18,
        BLADE_BOUNDARY = 19,

        COUNT
    };

    enum EnemyDamage : int {
        METALL_DMG = 2,
        METALL_BULLET_DMG = 1,
        BUNBY_DMG = 2,
        BOSS_DMG = 2,
        BOSS_BLADE_DMG = 4
    };

    enum class CategoryBits : int {
        PLAYER = 0x0001,
        ENEMY = 0x0002,
        BOUNDARY = 0x0004,
        ENEMY_PROJECTILE = 0x0008,
        PLAYER_PROJECTILE = 0x0010,
        PLATFORM = 0x0020,
        ENEMY_AWAKE_SENSOR = 0x0040,
        DEADEND_SENSOR = 0x0080,
        TRIGGER_POINT = 0x0100,
        BOSS_MOVE_POINT = 0x0200,
        DEADEND = 0x0400,
        BLADE_BOUNDARY = 0x0800
    };
}
