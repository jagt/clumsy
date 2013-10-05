-- currently only supports vs2010.
-- and premake4 4.4 doesn't have vs2012 support yet. but seems auto upgrade works fine

if _ACTION == 'clean' then
    os.rmdir('./build')
    os.rmdir('./bin')
end

solution('clumsy')
    location("./build")
    configurations({'Debug'})
    platforms({'x32', 'x64'})

    project('clumsy')
        language("C") -- not really a C++ project, just visual studio c subset sucks too hard
        includedirs({'external/WinDivert-1.0.5-MSVC/include'})
        files({'src/**.c', 'src/**.h'})
        -- flags({'ExtraWarnings', 'StaticRuntime', 'Symbols'})
        flags({'ExtraWarnings', 'Symbols'})
        links({'WinDivert', 'iup', 'comctl32', 'Winmm', 'ws2_32'}) 
        --links({'gdi32', 'comdlg32', 'uuid', 'ole32'}) -- covered by default in vs already

        configuration('Debug')
            defines({'_DEBUG'})
            kind("ConsoleApp")

        configuration("vs*")
            defines({"_CRT_SECURE_NO_WARNINGS"})
            buildoptions({'/wd"4214"'})

        configuration('x32')
            includedirs({'external/iup-3.8_Win32_dll11_lib/include'})
            libdirs({
                'external/WinDivert-1.0.5-MSVC/x86',
                'external/iup-3.8_Win32_dll11_lib'
                })

        configuration('x64')
            includedirs({'external/iup-3.8_Win64_dll11_lib/include'})
            libdirs({
                'external/WinDivert-1.0.5-MSVC/amd64',
                'external/iup-3.8_Win64_dll11_lib'
                })

        configuration({"Debug", "x32"})
            local x32_debug_bin = 'bin/debug/x32'
            targetdir(x32_debug_bin)
            debugdir(x32_debug_bin)
            -- cwd is ./build

            postbuildcommands({
                "robocopy ../external/WinDivert-1.0.5-MSVC/x86/ ../"   .. x32_debug_bin .. '  *.dll *.sys *.inf > robolog.txt',
                "robocopy ../external/iup-3.8_Win32_dll11_lib ../"   .. x32_debug_bin .. ' iup.dll >> robolog.txt',
                "exit /B 0"
            })

        configuration({"Debug", "x64"})
            local x64_debug_bin = 'bin/debug/x64'
            targetdir(x64_debug_bin)
            debugdir(x64_debug_bin)
            postbuildcommands({
                "robocopy ../external/WinDivert-1.0.5-MSVC/amd64/ ../"   .. x64_debug_bin .. '  *.dll *.sys *.inf > robolog.txt',
                "robocopy ../external/iup-3.8_Win64_dll11_lib ../"   .. x64_debug_bin ..  ' iup.dll >> robolog.txt',
                "exit /B 0"
            })
