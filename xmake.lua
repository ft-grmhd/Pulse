-- Copyright (C) 2024 kanel
-- This file is part of "Pulse"
-- For conditions of distribution and use, see copyright notice in LICENSE

local backends = {
	Vulkan = {
		option = "vulkan",
		default = true,
		packages = { "vulkan-headers", "vulkan-memory-allocator" },
		custom = function()
			add_defines("VK_NO_PROTOTYPES")
			add_files("Sources/Backends/Vulkan/**.cpp")
		end
	},
	Metal = {
		option = "metal",
		default = is_plat("macosx", "iphoneos"),
		custom = function()
			add_files("Sources/Backends/Metal/**.m")
		end
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
set_languages("c17", "cxx20")
set_encodings("utf-8")
set_warnings("allextra")

set_objectdir("build/Objs/$(os)_$(arch)")
set_targetdir("build/Bin/$(os)_$(arch)")
set_rundir("build/Bin/$(os)_$(arch)")
set_dependir("build/.deps")

set_optimize("fastest")

option("unitybuild", { description = "Build the library using unity build", default = false })

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

	if has_config("unitybuild") then
		add_rules("c.unity_build", { batchsize = 6 })
	end

	for name, module in pairs(backends) do
		if has_config(module.option) then
			if module.packages then
				add_packages(table.unpack(module.packages))
			end

			add_defines("PULSE_ENABLE_" .. name:upper() .. "_BACKEND")

			add_headerfiles("Sources/Backends/" .. name .. "/**.h", { prefixdir = "private", install = false })
			add_headerfiles("Sources/Backends/" .. name .. "/**.inl", { prefixdir = "private", install = false })

			add_files("Sources/Backends/" .. name .. "/**.c")

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

includes("Xmake/**.lua")
includes("Examples/*.lua")
includes("Tests/Vulkan/*.lua")
