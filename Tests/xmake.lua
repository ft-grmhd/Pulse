local Backend = {
	VULKAN = 1,
	OPENGL = 2,
	OPENGL_ES = 3,
}

local nzsl_included = false

function nzsl(backend)
	if not nzsl_included then
		add_repositories("nazara-engine-repo https://github.com/NazaraEngine/xmake-repo")
		add_requires("nzsl", { configs = { shared = false, nzslc = true } })
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

		nzsl_included = true
	end

	local name = ""
	if backend == Backend.VULKAN then
		name = "vulkan"
	elseif backend == Backend.OPENGL_ES then
		name = "opengl_es"
	else
		name = "opengl"
	end

	rule("nzsl_compile_shaders_" .. name)
		set_extensions(".nzsl")
		add_deps("find_nzsl")
		before_buildcmd_file(function(target, batchcmds, shaderfile, opt)
			local outputdir = target:data("nzsl_includedirs")
			local nzslc = target:data("nzslc")
			local runenvs = target:data("nzsl_runenv")
			assert(nzslc, "nzslc not found! please install nzsl package with nzslc enabled")
			batchcmds:show_progress(opt.progress, "${color.build.object}compiling.shader %s", shaderfile)
			local argv = {}
			if backend == Backend.VULKAN then
				argv = { "--compile=spv-header" }
			elseif backend == Backend.OPENGL_ES then
				argv = { "--compile=glsl,glsl-header", "--gl-version", "310", "--gl-es", "--gl-bindingmap" }
			else
				argv = { "--compile=glsl,glsl-header", "--gl-version", "310", "--gl-bindingmap" }
			end
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
			local ext = ""
			if backend == Backend.VULKAN then
				ext = ".spv.h"
			else
				ext = ".glsl.h"
			end
			local outputfile = path.join(outputdir or path.directory(shaderfile), path.basename(shaderfile) .. ext)
			batchcmds:add_depfiles(shaderfile)
			batchcmds:add_depvalues(nzslc.version)
			batchcmds:set_depmtime(os.mtime(outputfile))
			batchcmds:set_depcache(target:dependfile(outputfile))
		end)
	rule_end()
end

local tests = {
	Vulkan = {
		option = "vulkan",
		global_custom = function()
			nzsl(Backend.VULKAN)
		end,
		custom = function()
			add_rules("nzsl_compile_shaders_vulkan")
			add_packages("nzsl")
			add_files("**.nzsl")
		end
	},
	OpenGL = {
		option = "opengl",
		global_custom = function()
			nzsl(Backend.OPENGL)
		end,
		custom = function()
			add_rules("nzsl_compile_shaders_opengl")
			add_packages("nzsl")
			add_files("**.nzsl")
		end
	},
	OpenGLES = {
		option = "opengl-es",
		global_custom = function()
			nzsl(Backend.OPENGL_ES)
		end,
		custom = function()
			add_rules("nzsl_compile_shaders_opengl_es")
			add_packages("nzsl")
			add_files("**.nzsl")
		end
	},
	WebGPU = {
		option = "webgpu",
		global_custom = function()
			rule("wgsl_compile_shaders")
				set_extensions(".wgsl")
				before_buildcmd_file(function(target, batchcmds, shaderfile, opt)
					batchcmds:show_progress(opt.progress, "${color.build.object}compiling.shader %s", shaderfile)
					io.writefile(shaderfile .. ".h", [[
// Generated File
#undef WGSL_SOURCE
#define WGSL_SOURCE(...) #__VA_ARGS__
const uint8_t SHADER_NAME[] = WGSL_SOURCE(

]] .. io.readfile(shaderfile) .. [[

);]])
					end)
			rule_end()
		end,
		custom = function()
			add_rules("wgsl_compile_shaders")
			add_files("**.wgsl")
		end
	}
}

if is_plat("linux") then
	add_requires("libbacktrace")
end
add_requires("unity_test")
-- add_requireconfs("unity_test", { configs = { cflags = "-D UNITY_OUTPUT_COLOR" }})

for name, module in table.orderpairs(tests) do
	if module.option then
		option(module.option .. "-tests", { description = "Enables " .. name .. " tests", default = false })
	end
end

for name, module in pairs(tests) do
	if has_config(module.option .. "-tests") then
		if module.global_custom then
			module.global_custom()
		end
		if module.packages then
			add_requires(table.unpack(module.packages))
		end

		target(name .. "UnitTests")
			set_kind("binary")
			add_deps("pulse_gpu")
			add_packages("unity_test")
			add_files("**.c")
			add_defines(string.upper(name) .. "_ENABLED")
			if module.custom then
				module.custom()
			end
			if is_plat("linux") then
				add_packages("libbacktrace")
				set_extension(".x86_64")
				add_defines("BACKTRACE")
			elseif is_plat("windows") then
				add_defines("WINTRACE")
				add_cflags("/Zi")
				add_ldflags("/DEBUG")
				add_links("dbghelp")
			end
		target_end()
	end
end
