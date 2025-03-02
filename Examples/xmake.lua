option("examples", { description = "Build the examples", default = false })

if has_config("examples") then
	set_group("Examples")
	if not is_plat("wasm") and has_config("vulkan") then
		includes("Vulkan/xmake.lua")
	end
	if has_config("webgpu") then
		includes("WebGPU/xmake.lua")
	end
	if has_config("software") then
		includes("Software/xmake.lua")
	end
end
