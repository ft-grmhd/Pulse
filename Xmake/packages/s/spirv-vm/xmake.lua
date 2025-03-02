package("spirv-vm")
	set_homepage("https://github.com/dfranx/SPIRV-VM/")
	set_description("A Virtual machine for executing SPIR-V.")
	set_license("MIT")

	add_urls("https://github.com/dfranx/SPIRV-VM.git")

	on_install(function (package)
		os.cp("inc/spvm", package:installdir("include"))
		io.writefile("xmake.lua", [[
			add_rules("mode.debug", "mode.release")
			target("spirv-vm")
				set_kind("$(kind)")
				add_files("src/**.c")
				add_includedirs("inc/")
				--add_headerfiles("inc/**.h")
		]])
		import("package.tools.xmake").install(package)
	end)

	on_test(function (package)
		assert(package:check_csnippets({test = [[
			#include <spvm/context.h>
			void test()
			{
				spvm_context_t ctx = spvm_context_initialize();
				spvm_context_deinitialize(ctx);
			}
		]]}))
	end)
