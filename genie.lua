-- genie, https://github.com/bkaradzic/GENie
-- known working version
-- https://github.com/bkaradzic/bx/blob/51f25ba638b9cb35eb2ac078f842a4bed0746d56/tools/bin/windows/genie.exe

if _ACTION == 'clean' then
    os.rmdir('./build')
    os.rmdir('./bin')
    os.rmdir('./obj_vs')
    os.rmdir('./obj_ninja')
end

if _ACTION == 'ninja' then
    -- need a msys2 with clang
    premake.gcc.cc   = 'C:/msys64/mingw64/bin/clang'
    premake.gcc.cxx  = 'C:/msys64/mingw64/bin/clang++'
    premake.gcc.ar   = 'C:/msys64/mingw64/bin/llvm-ar'
    premake.llvm = true
end

local LIB_DIVERT_VC11 = 'external/WinDivert-2.2.0-A'
local LIB_DIVERT_MINGW = 'external/WinDivert-2.2.0-A'
local LIB_IUP_WIN32_VC11 = 'external/iup-3.30_Win32_dll16_lib'
local LIB_IUP_WIN64_VC11 = 'external/iup-3.30_Win64_dll16_lib'
local LIB_IUP_WIN32_MINGW = 'external/iup-3.30_Win32_mingw6_lib'
local LIB_IUP_WIN64_MINGW = 'external/iup-3.30_Win64_mingw6_lib'

local ROOT = os.getcwd()
print(ROOT)

solution('clumsy')
    location("./build")
    configurations({'Debug', 'Release'})
    platforms({'x32', 'x64'})

    project('clumsy')
        language("C")
        files({'src/**.c', 'src/**.h'})
        links({'WinDivert', 'iup', 'comctl32', 'Winmm', 'ws2_32'}) 
        if string.match(_ACTION, '^vs') then -- only vs can include rc file in solution
            files({'./etc/clumsy.rc'})
        elseif _ACTION == 'ninja' then
            files({'./etc/clumsy.rc'})
        end

        configuration('Debug')
			flags({'ExtraWarnings', 'Symbols'})
            defines({'_DEBUG'})
            kind("ConsoleApp")

        configuration('Release')
			flags({"Optimize"})            
			flags({'Symbols'}) -- keep the debug symbols for development
            defines({'NDEBUG'})
            kind("WindowedApp")

        configuration("ninja")
            links({'kernel32', 'gdi32', 'comdlg32', 'uuid', 'ole32'}) -- additional libs
            buildoptions({'-Wno-missing-braces', '--std=c99'}) -- suppress a bug in gcc warns about {0} initialization
            --linkoptions({'--std=c90'})
            -- notice that tdm-gcc use static runtime by default
            objdir('obj_ninja')

        configuration("vs*")
            defines({"_CRT_SECURE_NO_WARNINGS"})
            flags({'NoManifest'})
            kind("WindowedApp") -- We don't need the console window in VS as we use OutputDebugString().
            buildoptions({'/wd"4214"'})
			linkoptions({'/ENTRY:"mainCRTStartup" /SAFESEH:NO'})
			-- characterset("MBCS")
            includedirs({LIB_DIVERT_VC11 .. '/include'})
            objdir('obj_vs')

        configuration({'x32', 'vs*'})
            -- defines would be passed to resource compiler for whatever reason
            -- and ONLY can be put here not under 'configuration('x32')' or it won't work
            defines({'X32'})
            includedirs({LIB_IUP_WIN32_VC11 .. '/include'})
            libdirs({
                LIB_DIVERT_VC11 .. '/x86',
                LIB_IUP_WIN32_VC11 .. ''
                })

        configuration({'x64', 'vs*'})
            defines({'X64'})
            includedirs({LIB_IUP_WIN64_VC11 .. '/include'})
            libdirs({
                LIB_DIVERT_VC11 .. '/x64',
                LIB_IUP_WIN64_VC11 .. ''
                })

        configuration({'x32', 'ninja'})
            defines({'X32'}) -- defines would be passed to resource compiler for whatever reason
            includedirs({LIB_DIVERT_MINGW .. '/include',
                LIB_IUP_WIN32_MINGW .. '/include'})
            libdirs({
                LIB_DIVERT_MINGW .. '/x86',
                LIB_IUP_WIN32_MINGW .. ''
                })
            resoptions({'-O coff', '-F pe-i386'}) -- mingw64 defaults to x64

        configuration({'x64', 'ninja'})
            defines({'X64'})
            includedirs({LIB_DIVERT_MINGW .. '/include',
                LIB_IUP_WIN64_MINGW .. '/include'})
            libdirs({
                LIB_DIVERT_MINGW .. '/x64',
                LIB_IUP_WIN64_MINGW .. ''
                })

        local function set_bin(platform, config, arch)
            local platform_str
            if platform == 'vs*' then
                platform_str = 'vs'
            else
                platform_str = platform
            end
            local subdir = ROOT .. '/bin/' .. platform_str .. '/' .. config .. '/' .. arch
            local divert_lib, iup_lib
            if platform == 'vs*' then 
                if arch == 'x64' then
                    divert_lib = ROOT .. '/' .. LIB_DIVERT_VC11  .. '/x64/'
                    iup_lib = ROOT .. '/' .. LIB_IUP_WIN64_VC11 .. ''
                else
                    divert_lib = ROOT ..'/' .. LIB_DIVERT_VC11 .. '/x86/'
                    iup_lib = ROOT ..'/' .. LIB_IUP_WIN32_VC11 .. ''
                end
            elseif platform == 'ninja' then
                if arch == 'x64' then
                    divert_lib = ROOT .. '/' .. LIB_DIVERT_MINGW .. '/x64/'
                    iup_lib = ROOT .. '/' .. LIB_IUP_WIN64_MINGW .. ''
                else
                    divert_lib = ROOT .. '/' .. LIB_DIVERT_MINGW .. '/x86/'
                    iup_lib = ROOT .. '/' .. LIB_IUP_WIN32_MINGW .. ''
                end
            end
            configuration({platform, config, arch})
                targetdir(subdir)
                debugdir(subdir)
                if platform == 'vs*' then
                    postbuildcommands({
                        "robocopy " .. divert_lib .." " .. subdir .. '  *.dll *.sys >> robolog.txt',
                        "robocopy " .. iup_lib .. " "  .. subdir .. ' iup.dll >> robolog.txt',
                        "robocopy " .. ROOT .. "/etc/ "   .. subdir .. ' config.txt >> robolog.txt',
                        "exit /B 0"
                    })
                elseif platform == 'ninja' then 
                    postbuildcommands({
                        -- robocopy returns non 0 will fail make
                        'cp ' .. divert_lib .. "WinDivert* " .. subdir,
                        'cp ' .. ROOT .. "/etc/config.txt " .. subdir,
                    })
                end
        end

        set_bin('vs*', 'Debug', "x32")
        set_bin('vs*', 'Debug', "x64")
        set_bin('vs*', 'Release', "x32")
        set_bin('vs*', 'Release', "x64")
        set_bin('ninja', 'Debug', "x32")
        set_bin('ninja', 'Debug', "x64")
        set_bin('ninja', 'Release', "x32")
        set_bin('ninja', 'Release', "x64")

