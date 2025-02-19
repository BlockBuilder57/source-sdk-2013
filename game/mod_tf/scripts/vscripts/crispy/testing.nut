/*::MaxPlayers <- MaxClients().tointeger()

for (local i = 1; i <= MaxPlayers ; i++)
{
    local player = PlayerInstanceFromIndex(i)
    if (player == null) continue
    printl(player)
}*/

local player = GetListenServerHost()
local rocket = SpawnEntityFromTable("tf_projectile_rocket",
{
    // This is a table of keyvalues, which is the same way as keyvalues that are defined in Hammer
    // Key         Value
    origin       = player.EyePosition() + player.EyeAngles().Forward() * 32
    angles       = player.EyeAngles()
    teamnum      = 3
})
rocket.SetOwner(player) // make it not collide with owner and give proper kill credits
rocket.SetAbsVelocity(player.EyeAngles().Forward() * 250)