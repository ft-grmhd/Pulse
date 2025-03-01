option("vulkan-tests", { description = "Build Vulkan tests", default = false })

add_repositories("nazara-engine-repo https://github.com/NazaraEngine/xmake-repo")

add_requires("nzsl >=2023.12.31", { configs = { shared = false, nzslc = true } })

if is_cross() then
	add_requires("nzsl~host", { kind = "binary", host = true })
end

-- Yoinked from NZSL xmake repo
rule("find_nzsl")
	on_config(function(target)
		import("core.project.project")
		import("core.tool.toolchain")
		import("lib.detect.find_tool")

		local envs
		if is_plat("windows") then
			local msvc = target:toolchain("msvc")
			if msvc and msvc:check() then
				envs = msvc:runenvs()
			end
		elseif is_plat("mingw") then
			local mingw = target:toolchain("mingw")
			if mingw and mingw:check() then
				envs = mingw:runenvs()
			end
		end
		target:data_set("nzsl_envs", envs)

		local nzsl = project.required_package("nzsl~host") or project.required_package("nzsl")
		local nzsldir
		if nzsl then
			nzsldir = path.join(nzsl:installdir(), "bin")
			local osenvs = os.getenvs()
			envs = envs or {}
			for env, values in pairs(nzsl:get("envs")) do
				local flatval = path.joinenv(values)
				local oldenv = envs[env] or osenvs[env]
				if not oldenv or oldenv == "" then
					envs[env] = flatval
				elseif not oldenv:startswith(flatval) then
					envs[env] = flatval .. path.envsep() .. oldenv
				end
			end
		end

		local nzsla = find_tool("nzsla", { version = true, paths = nzsldir, envs = envs })
		local nzslc = find_tool("nzslc", { version = true, paths = nzsldir, envs = envs })

		target:data_set("nzsla", nzsla)
		target:data_set("nzslc", nzslc)
		target:data_set("nzsl_runenv", envs)
	end)
rule_end()

rule("compile_shaders")
	set_extensions(".nzsl")
	add_deps("find_nzsl")

	before_buildcmd_file(function(target, batchcmds, shaderfile, opt)
		local outputdir = target:data("nzsl_includedirs")
		local nzslc = target:data("nzslc")
		local runenvs = target:data("nzsl_runenv")
		assert(nzslc, "nzslc not found! please install nzsl package with nzslc enabled")

		batchcmds:show_progress(opt.progress, "${color.build.object}compiling.shader %s", shaderfile)
		local argv = { "--compile=spv-header", "--optimize" }
		if outputdir then
			batchcmds:mkdir(outputdir)
			table.insert(argv, "--output=" .. outputdir)
		end

		local kind = target:data("plugin.project.kind") or ""
		if kind:match("vs") then
			table.insert(argv, "--log-format=vs")
		end

		table.insert(argv, shaderfile)

		batchcmds:vrunv(nzslc.program, argv, { curdir = ".", envs = runenvs })

		local outputfile = path.join(outputdir or path.directory(shaderfile), path.basename(shaderfile) .. ".spv.h")

		batchcmds:add_depfiles(shaderfile)
		batchcmds:add_depvalues(nzslc.version)
		batchcmds:set_depmtime(os.mtime(outputfile))
		batchcmds:set_depcache(target:dependfile(outputfile))
	end)
rule_end()

if has_config("vulkan-tests") then
	set_group("VulkanTests")
	add_requires("unity_test")

	if is_plat("linux") then
		add_requires("libbacktrace")
	end

	target("VulkanUnitTests")
		set_kind("binary")
		add_deps("pulse_gpu")
		add_rules("compile_shaders")
		add_files("**.c")
		add_files("**.nzsl")
		add_packages("unity_test")
		if is_plat("linux") then
			add_packages("libbacktrace")
			set_extension(".x86_64")
		end
	target_end()
end
