-- currently only supports vs2010.
-- and premake4 4.4 doesn't have vs2012 support yet. but seems auto upgrade works fine

if _ACTION == 'clean' then
    os.rmdir('./build')
    os.rmdir('./bin')
end

solution('clumsy')
    location("./build")
    configurations({'Debug', 'Release'})
    platforms({'x32', 'x64'})

    project('clumsy')
        language("C")
        includedirs({'external/WinDivert-1.0.5-MSVC/include'})
        files({'src/**.c', 'src/**.h'})
        links({'WinDivert', 'iup', 'comctl32', 'Winmm', 'ws2_32'}) 
        files({'./etc/clumsy.rc'})

        configuration('Debug')
            flags({'ExtraWarnings', 'Symbols'})
            defines({'_DEBUG'})
            kind("ConsoleApp")

        configuration('Release')
            flags({'Optimize'})
            defines({'NDEBUG'})
            kind("WindowedApp")

        configuration("gmake")
            links({'gdi32', 'comdlg32', 'uuid', 'ole32'}) -- additional libs
            -- notice that tdm-gcc use static runtime by default

        configuration("vs*")
            defines({"_CRT_SECURE_NO_WARNINGS"})
            flags({'NoManifest'})
            buildoptions({'/wd"4214"'})

        configuration({'x32', 'vs*'})
            defines({'X32'}) -- defines would be passed to resource compiler for whatever reason
            includedirs({'external/iup-3.8_Win32_dll11_lib/include'})
            libdirs({
                'external/WinDivert-1.0.5-MSVC/x86',
                'external/iup-3.8_Win32_dll11_lib'
                })

        configuration({'x64', 'vs*'})
            defines({'X64'})
            includedirs({'external/iup-3.8_Win64_dll11_lib/include'})
            libdirs({
                'external/WinDivert-1.0.5-MSVC/amd64',
                'external/iup-3.8_Win64_dll11_lib'
                })

        local function set_bin(platform, config, arch)
            local platform_str
            if platform == 'vs*' then
                platform_str = 'vs'
            end
            local subdir = 'bin/' .. platform_str .. '/' .. config .. '/' .. arch
            local divert_lib, iup_lib
            if platform == 'vs*' then 
                if arch == 'x64' then
                    divert_lib = '../external/WinDivert-1.0.5-MSVC/amd64/'
                    iup_lib = '../external/iup-3.8_Win64_dll11_lib'
                else
                    divert_lib = '../external/WinDivert-1.0.5-MSVC/x86/'
                    iup_lib = '../external/iup-3.8_Win32_dll11_lib'
                end
            end
            configuration({platform, config, arch})
                targetdir(subdir)
                debugdir(subdir)
                if platform == 'vs*' then
                    postbuildcommands({
                        "robocopy " .. divert_lib .." ../"   .. subdir .. '  *.dll *.sys *.inf > robolog.txt',
                        "robocopy " .. iup_lib .. " ../"   .. subdir .. ' iup.dll >> robolog.txt',
                        "exit /B 0"
                    })
                end
        end

        set_bin('vs*', 'Debug', "x32")
        set_bin('vs*', 'Debug', "x64")
        set_bin('vs*', 'Release', "x32")
        set_bin('vs*', 'Release', "x64")

