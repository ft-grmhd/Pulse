target("LoadingPulse")
	add_deps("pulse_gpu")
	if is_plat("linux") then
		set_extension(".x86_64")
	end
	add_files("main.c")
	set_targetdir("build/")
target_end()
