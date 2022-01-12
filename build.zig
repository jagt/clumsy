const std = @import("std");
const Builder = std.build.Builder;
const Pkg = std.build.Pkg;
const Step = std.build.Step;
const debug = std.debug;
const Allocator = std.mem.Allocator;
const CrossTarget = std.zig.CrossTarget;

const ClumsyArch = enum { x86, x64 };
const ClumsyConf = enum { Debug, Release };

pub fn build(b: *std.build.Builder) void {
    const arch = b.option(ClumsyArch, "arch", "x86, x64") orelse .x64;
    const conf = b.option(ClumsyConf, "conf", "Debug, Release") orelse .Debug;
    const windowsKitBinRoot = b.option([]const u8, "windowsKitBinRoot", "Windows SDK Bin root") orelse "C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0";

    const archTag = @tagName(arch);
    const confTag = @tagName(conf);

    debug.print("- arch: {s}, conf: {s}\n", .{@tagName(arch), @tagName(conf)});
    debug.print("- windowsKitBinRoot: {s}\n", .{windowsKitBinRoot});
    _ = std.fs.realpathAlloc(b.allocator, windowsKitBinRoot) catch @panic("windowsKitBinRoot not found");

    const tuple = b.fmt("{s}_{s}", .{archTag, confTag});
    b.exe_dir = b.fmt("{s}/{s}", .{b.install_path, tuple});

    debug.print("- out: {s}\n", .{b.exe_dir});

    const tmpPath = b.fmt("tmp/{s}", .{tuple});

    b.makePath(tmpPath) catch @panic("unable to create tmp directory");

    b.installFile(b.fmt("external/WinDivert-2.2.0-A/{s}/WinDivert.dll", .{archTag}), b.fmt("{s}/WinDivert.dll", .{tuple}));
    switch (arch) {
        .x64 => b.installFile(b.fmt("external/WinDivert-2.2.0-A/{s}/WinDivert64.sys", .{archTag}), b.fmt("{s}/WinDivert64.sys", .{tuple})),
        .x86 => b.installFile(b.fmt("external/WinDivert-2.2.0-A/{s}/WinDivert32.sys", .{archTag}), b.fmt("{s}/WinDivert32.sys", .{tuple})),
    }

    b.installFile("etc/config.txt", b.fmt("{s}/config.txt", .{tuple}));
    b.installFile("LICENSE", b.fmt("{s}/License.txt", .{tuple}));

    const resObjPath = b.fmt("tmp/{s}/clumsy_res.obj", .{tuple});

    //  check `rc` is on path, warn about VCVars
    const rcExe = b.findProgram(&.{
        "rc",
    }, &.{
        b.pathJoin(&.{windowsKitBinRoot, @tagName(arch)}),
    }) catch @panic("unable to find `rc.exe`, check your windowsKitBinRoot");

    const cmd = b.addSystemCommand(&.{
        rcExe,
        "/nologo",
        "/d",
        "NDEBUG",
        "/d",
        switch (arch) {
            .x64 => "X64",
            .x86 => "X86",
        },
        "/r",
        "/fo",
        resObjPath,
        "etc/clumsy.rc",
    });

    const exe = b.addExecutable("clumsy", null);

    switch (conf) {
        .Debug => {
            exe.setBuildMode(.Debug);
        },
        .Release => {
            exe.setBuildMode(.ReleaseFast);
        },
    }
    const triple  = switch (arch) {
        .x64 => "x86_64-windows-gnu",
        .x86 => "i386-windows-gnu",
    };

    const selectedTarget = CrossTarget.parse(.{
        .arch_os_abi = triple,
    }) catch unreachable;

    exe.setTarget(selectedTarget);

    exe.step.dependOn(&cmd.step);
    exe.addObjectFile(resObjPath);
    exe.addCSourceFile("src/bandwidth.c", &.{""});
    exe.addCSourceFile("src/divert.c", &.{""});
    exe.addCSourceFile("src/drop.c", &.{""});
    exe.addCSourceFile("src/duplicate.c", &.{""});
    exe.addCSourceFile("src/elevate.c", &.{""});
    exe.addCSourceFile("src/lag.c", &.{""});
    exe.addCSourceFile("src/main.c", &.{""});
    exe.addCSourceFile("src/ood.c", &.{""});
    exe.addCSourceFile("src/packet.c", &.{""});
    exe.addCSourceFile("src/reset.c", &.{""});
    exe.addCSourceFile("src/tamper.c", &.{""});
    exe.addCSourceFile("src/throttle.c", &.{""});
    exe.addCSourceFile("src/utils.c", &.{""});
    exe.addCSourceFile("src/utils.c", &.{""});

    if (arch == .x86)
        exe.addCSourceFile("etc/chkstk.s", &.{""});

    exe.addIncludeDir("external/WinDivert-2.2.0-A/include");

    const iupLib = switch (arch) {
        .x64 => "external/iup-3.30_Win64_mingw6_lib",
        .x86 => "external/iup-3.30_Win32_mingw6_lib",
    };

    exe.addIncludeDir(b.pathJoin(&.{iupLib, "include"}));
    exe.addCSourceFile(b.pathJoin(&.{iupLib, "libiup.a"}), &.{""});

    exe.linkLibC();
    exe.addLibPath(b.fmt("external/WinDivert-2.2.0-A/{s}", .{archTag}));
    exe.linkSystemLibrary("WinDivert");
    exe.linkSystemLibrary("comctl32");
    exe.linkSystemLibrary("Winmm");
    exe.linkSystemLibrary("ws2_32");
    exe.linkSystemLibrary("kernel32");
    exe.linkSystemLibrary("gdi32");
    exe.linkSystemLibrary("comdlg32");
    exe.linkSystemLibrary("uuid");
    exe.linkSystemLibrary("ole32");
    exe.install();
}
