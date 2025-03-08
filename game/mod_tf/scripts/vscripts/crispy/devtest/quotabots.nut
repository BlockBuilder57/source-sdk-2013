::QuotaBots <- [];
::QuotaFakeClients <- function(num)
{
	printl("Preparing " + num + " fake clients");

	local toAdd = num - QuotaBots.len()

	// kick any extra bots
	for (local i = 0; i < MaxClients().tointeger() && toAdd < 0; i++)
	{
		local player = PlayerInstanceFromIndex(i);
		if (player == null || !player.IsFakeClient()) continue;

		local userid = NetProps.GetPropIntArray(Entities.FindByClassname(null, "tf_player_manager"), "m_iUserID", player.entindex())
		SendToServerConsole("kickid " + userid);
		toAdd++;
	}

	#SendToServerConsole("bot_kick all");

	/*for (local i = 0; i < num; i++)
	{
		//printl("spawning bot?");
		SendToServerConsole("bot -name QuotaBot" + i);
	}

	for (local i = 0; i < MaxClients().tointeger(); i++)
	{
		local player = PlayerInstanceFromIndex(i);
		if (player == null || !player.IsFakeClient()) continue;
		printl(player);
		QuotaBots.append(player);
	}*/

	for (local i = 0; i < toAdd; i++)
	{
		//printl("spawning bot?");
		QuotaBots.append(CreateFakeClient("QuotaBot" + QuotaBots.len(), -1, -1));
	}

	return QuotaBots
}

function OnGameEvent_player_disconnect(params)
{
	if (!params.bot)
		return;

	local player = GetPlayerFromUserID(params.userid);
	if (!player)
		return;

	local idx = QuotaBots.find(player);
	if (idx != null)
	{
		printl("Removing player #" + idx);
		QuotaBots.remove(idx);
	}
}

__CollectGameEventCallbacks(this)

printl("Loaded quota bots")