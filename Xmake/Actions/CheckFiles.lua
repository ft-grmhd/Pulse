task("check-files")

set_menu({
	usage = "xmake check-files [options]",
	description = "Check every file for consistency (can fix some errors)",
	options =
	{
		{'f', "fix", "k", nil, "Attempt to automatically fix files." }
	}
})

on_run(function()
	import("core.base.option")

	local file_lines = {}
	local updated_files = {}
	local function GetFile(file_path)
		file_path = path.translate(file_path)

		local lines = file_lines[file_path]
		if not lines then
			lines = table.to_array(io.lines(file_path))
			if not lines then
				os.raise("failed to open " .. file_path)
			end

			file_lines[file_path] = lines
		end

		return lines
	end

	local function UpdateFile(file_path, lines)
		file_path = path.translate(file_path)
		if lines then
			file_lines[file_path] = lines
		end
		updated_files[file_path] = true
	end

	local checks = {}

	-- Remove empty lines at the beginning of files
	table.insert(checks, {
		Name = "empty lines",
		Check = function()
			local files = table.join(
				os.files("Sources/**.h"),
				os.files("Sources/**.inl"),
				os.files("Sources/**.c"),
				os.files("Sources/**.m"),
				os.files("Sources/**.cpp")
			)

			local fixes = {}

			for _, file_path in pairs(files) do
				local lines = GetFile(file_path)

				for i = 1, #lines do
					if not lines[i]:match("^%s*$") then
						if i ~= 1 then
							print(file_path .. " starts with empty lines")

							table.insert(fixes, {
								File = file_path,
								Func = function(lines)
									for j = 1, i - 1 do
										table.remove(lines, 1)
									end

									UpdateFile(file_path, lines)
								end
							})
						end

						break
					end
				end
			end

			return fixes
		end
	})

	-- Check copyright date and format
	table.insert(checks, {
		Name = "copyright",
		Check = function()
			local files = table.join(
				os.files("Sources/**.h"),
				os.files("Sources/**.inl"),
				os.files("Sources/**.c"),
				os.files("Sources/**.m"),
				os.files("Sources/**.cpp")
			)

			local current_year = os.date("%Y")
			local fixes = {}

			-- Headers
			for _, file_path in pairs(files) do
				local lines = GetFile(file_path)

				local has_copyright
				local should_fix = false

				local year, authors = lines[1]:match("^// Copyright %(C%) (Y?E?A?R?%d*) kanel$")
				has_copyright = year ~= nil

				if has_copyright then
					if year ~= current_year then
						print(file_path .. ": copyright year error")
						sould_fix = true
					end

					if lines[2] ~= "// This file is part of \"Pulse\"" then
						print(file_path .. ": copyright project error")
						sould_fix = true
					end

					if lines[3] ~= "// For conditions of distribution and use, see copyright notice in LICENSE" then
						print(file_path .. ": copyright file reference error")
						sould_fix = true
					end
				else
					print(file_path .. ": copyright not found")
					sould_fix = true
				end

				if sould_fix then
					table.insert(fixes, {
						File = file_path,
						Func = function(lines)
							local copyright_lines = {
								"// Copyright (C) " .. current_year .. " kanel",
								"// This file is part of \"Pulse\"",
								"// For conditions of distribution and use, see copyright notice in LICENSE"
							}

							if has_copyright then
								for i, line in ipairs(copyright_lines) do
									lines[i] = line
								end
							else
								for i, line in ipairs(copyright_lines) do
									table.insert(lines, i, line)
								end
								table.insert(lines, #copyright_lines + 1, "")
							end

							return lines
						end
					})
				end

				::skip::
			end

			return fixes
		end
	})

	local sould_fix = option.get("fix") or false

	for _, check in pairs(checks) do
		print("Running " .. check.Name .. " check...")

		local fixes = {}
		table.join2(fixes, check.Check())

		if sould_fix then
			for _, fix in pairs(fixes) do
				print("Fixing " .. fix.File)
				UpdateFile(fix.File, fix.Func(assert(file_lines[fix.File])))
			end
		end
	end

	for file_path, _ in pairs(updated_files) do
		local lines = assert(file_lines[file_path])
		if lines[#lines] ~= "" then
			table.insert(lines, "")
		end

		print("Saving changes to " .. file_path)
		io.writefile(file_path, table.concat(lines, "\n"))
	end
end)
