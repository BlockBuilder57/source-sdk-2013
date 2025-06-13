printl("Math tests");

local original = Vector(1, 0, 0);
local target = Vector(-1, 0, 0);
local angles = QAngle(0, 180, 0);

local res = Math.RotatePosition(Vector(0,0,0), angles, original);
res.Norm();

printl(res.Dot(target).tostring())
if (res.Dot(target) <= 0.9) {
	printl("res dot target was wrong!" + res.tostring() + target.tostring());
}