option("tests", { description = "Build tests", default = false })

if has_config("tests") then
	set_group("Tests")
	includes("*/xmake.lua")
end
