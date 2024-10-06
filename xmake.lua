-- Copyright (C) 2024 kanel
-- This file is part of "Pulse"
-- For conditions of distribution and use, see copyright notice in LICENSE

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

add_rules("mode.debug", "mode.release")

add_includedirs("Includes")
set_languages("c99", "cxx20")

set_objectdir("build/Objs/$(os)_$(arch)")
set_targetdir("build/Bin/$(os)_$(arch)")
set_rundir("build/Bin/$(os)_$(arch)")
set_dependir("build/.deps")

set_optimize("fastest")

target("pulse_gpu")
	set_kind("$(kind)")
	add_defines("PULSE_BUILD")
	add_headerfiles("Includes/*.hpp)")
	add_headerfiles("Sources/**.h", { prefixdir = "private", install = false })
	add_headerfiles("Sources/**.inl", { prefixdir = "private", install = false })
	add_files("Sources/**.c")
	add_files("Sources/**.cpp")
	on_load(function(target)
		if target:kind() == "static" then
			target:add("defines", "PULSE_STATIC", { public = true })
		end
	end)
target_end()
