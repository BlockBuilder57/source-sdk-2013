if (Convars.GetStr("sv_allow_point_servercommand") != "always")
{
	error("sv_allow_point_servercommand needs to be set to always!\n")
	return;
}

IncludeScript("crispy/devtest/quotabots.nut")
IncludeScript("crispy/devtest/playerextensions.nut")
IncludeScript("crispy/devtest/weaponry.nut")

// clear unbalance limit - I don't care right now
SendToServerConsole("mp_teams_unbalance_limit 0");
// ignore waiting for players
SendToServerConsole("mp_waitingforplayers_cancel 1");
// make sure we have a clean slate of bots
SendToServerConsole("bot_kick all");
