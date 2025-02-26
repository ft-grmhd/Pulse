option("examples", { description = "Build the examples", default = false })

if has_config("examples") then
	set_group("Examples")
	if not is_plat("wasm") then
		includes("Vulkan/xmake.lua")
	end
	includes("WebGPU/xmake.lua")
end
