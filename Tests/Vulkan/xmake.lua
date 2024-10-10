option("vulkan-tests", { description = "Build Vulkan tests", default = false })

if has_config("vulkan-tests") then
	set_group("VulkanTests")
	add_requires("unity_test")

	target("VulkanUnitTests")
		set_kind("binary")
		add_deps("pulse_gpu")
		add_files("**.c")
		add_packages("unity_test")
		if is_plat("linux") then
			set_extension(".x86_64")
		end
	target_end()
end
