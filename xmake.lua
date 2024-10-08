-- Copyright (C) 2024 kanel
-- This file is part of "Pulse"
-- For conditions of distribution and use, see copyright notice in LICENSE

local backends = {
	Vulkan = {
		option = "vulkan",
		default = true,
		has_cpp_files = true,
		packages = { "vulkan-headers", "vulkan-memory-allocator" },
		custom = function()
			add_defines("VK_NO_PROTOTYPES")
		end
	},
	D3D11 = {
		option = "d3d11",
		default = is_plat("windows", "mingw"),
		has_cpp_files = false,
	}
}

local sanitizers = {
	asan = "address",
	lsan = "leak",
	tsan = "thread",
}

for opt, policy in table.orderpairs(sanitizers) do
	option(opt, { description = "Enable " .. opt, default = false })
	if has_config(opt) then
		set_policy("build.sanitizer." .. policy, true)
	end
end

for name, module in table.orderpairs(backends) do
	if module.option then
		option(module.option, { description = "Enables the " .. name .. " backend", default = module.default })
	end
end

add_rules("mode.debug", "mode.release")

add_includedirs("Includes")
set_languages("c99", "cxx20")
set_encodings("utf-8")
set_warnings("allextra")

set_objectdir("build/Objs/$(os)_$(arch)")
set_targetdir("build/Bin/$(os)_$(arch)")
set_rundir("build/Bin/$(os)_$(arch)")
set_dependir("build/.deps")

set_optimize("fastest")

for name, module in pairs(backends) do
	if has_config(module.option) then
		if module.packages then
			add_requires(table.unpack(module.packages))
		end
	end
end

target("pulse_gpu")
	set_kind("$(kind)")
	add_defines("PULSE_BUILD")
	add_headerfiles("Sources/*.h", { prefixdir = "private", install = false })
	add_headerfiles("Sources/*.inl", { prefixdir = "private", install = false })
	
	add_files("Sources/*.c")

	for name, module in pairs(backends) do
		if has_config(module.option) then
			if module.packages then
				add_packages(table.unpack(module.packages))
			end

			add_defines("PULSE_ENABLE_" .. name:upper() .. "_BACKEND")

			add_headerfiles("Sources/Backends/" .. name .. "/**.h", { prefixdir = "private", install = false })
			add_headerfiles("Sources/Backends/" .. name .. "/**.inl", { prefixdir = "private", install = false })

			add_files("Sources/Backends/" .. name .. "/**.c")
			if module.has_cpp_files then
				add_files("Sources/Backends/" .. name .. "/**.cpp")
			end

			if module.custom then
				module.custom()
			end
		end
	end

	on_load(function(target)
		if target:kind() == "static" then
			target:add("defines", "PULSE_STATIC", { public = true })
		end
	end)
target_end()

includes("Tests/*.lua")
