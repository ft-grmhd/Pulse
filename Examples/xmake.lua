option("examples", { description = "Build the examples", default = false })

if has_config("examples") then
	set_group("Examples")
	includes("*/xmake.lua")
end

