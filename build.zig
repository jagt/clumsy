const std = @import("std");
const Builder = std.build.Builder;
const Pkg = std.build.Pkg;
const Step = std.build.Step;
const debug = std.debug;
const Allocator = std.mem.Allocator;
const CrossTarget = std.zig.CrossTarget;

const ClumsyArch = enum { x86, x64 };
const ClumsyConf = enum { Debug, Release, Ship };
const ClumsyWinDivertSign = enum { A, B, C };

pub fn build(b: *std.build.Builder) void {
    const arch = b.option(ClumsyArch, "arch", "x86, x64") orelse .x64;
    const conf = b.option(ClumsyConf, "conf", "Debug, Release") orelse .Debug;
    const windivert_sign = b.option(ClumsyWinDivertSign, "sign", "A, B, C") orelse .A;
    const windows_kit_bin_root = b.option([]const u8, "windows_kit_bin_root", "Windows SDK Bin root") orelse "C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0";

    const arch_tag = @tagName(arch);
    const conf_tag = @tagName(conf);
    const sign_tag = @tagName(windivert_sign);
    const windivert_dir = b.fmt("WinDivert-2.2.0-{s}", .{sign_tag});

    debug.print("- arch: {s}, conf: {s}, sign: {s}\n", .{@tagName(arch), @tagName(conf), @tagName(windivert_sign)});
    debug.print("- windows_kit_bin_root: {s}\n", .{windows_kit_bin_root});
    _ = std.fs.realpathAlloc(b.allocator, windows_kit_bin_root) catch @panic("windows_kit_bin_root not found");

    const prefix = b.fmt("{s}_{s}_{s}", .{arch_tag, conf_tag, sign_tag});
    b.exe_dir = b.fmt("{s}/{s}", .{b.install_path, prefix});

    debug.print("- out: {s}\n", .{b.exe_dir});

    const tmp_path = b.fmt("tmp/{s}", .{prefix});
    b.makePath(tmp_path) catch @panic("unable to create tmp directory");

    b.installFile(b.fmt("external/{s}/{s}/WinDivert.dll", .{windivert_dir, arch_tag}), b.fmt("{s}/WinDivert.dll", .{prefix}));
    switch (arch) {
        .x64 => b.installFile(b.fmt("external/{s}/{s}/WinDivert64.sys", .{windivert_dir, arch_tag}), b.fmt("{s}/WinDivert64.sys", .{prefix})),
        .x86 => b.installFile(b.fmt("external/{s}/{s}/WinDivert32.sys", .{windivert_dir, arch_tag}), b.fmt("{s}/WinDivert32.sys", .{prefix})),
    }

    b.installFile("etc/config.txt", b.fmt("{s}/config.txt", .{prefix}));
    if (conf == .Ship)
        b.installFile("LICENSE", b.fmt("{s}/License.txt", .{prefix}));

    const res_obj_path = b.fmt("{s}/clumsy_res.obj", .{tmp_path});

    const rc_exe = b.findProgram(&.{
        "rc",
    }, &.{
        b.pathJoin(&.{windows_kit_bin_root, @tagName(arch)}),
    }) catch @panic("unable to find `rc.exe`, check your windows_kit_bin_root");

    const archFlag = switch (arch) {
        .x86 => "X86",
        .x64 => "X64",
    };
    const cmd = b.addSystemCommand(&.{
        rc_exe,
        "/nologo",
        "/d",
        "NDEBUG",
        "/d",
        archFlag,
        "/r",
        "/fo",
        res_obj_path,
        "etc/clumsy.rc",
    });

    const exe = b.addExecutable("clumsy", null);

    switch (conf) {
        .Debug => {
            exe.setBuildMode(.Debug);
            exe.subsystem = .Console;
        },
        .Release => {
            exe.setBuildMode(.ReleaseSafe);
            exe.subsystem = .Windows;
        },
        .Ship => {
            exe.setBuildMode(.ReleaseFast);
            exe.subsystem = .Windows;
        },
    }
    const triple  = switch (arch) {
        .x64 => "x86_64-windows-gnu",
        .x86 => "i386-windows-gnu",
    };

    const target = CrossTarget.parse(.{
        .arch_os_abi = triple,
    }) catch unreachable;
    exe.setTarget(target);

    exe.step.dependOn(&cmd.step);
    exe.addObjectFile(res_obj_path);
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

    exe.addIncludeDir(b.fmt("external/{s}/include", .{windivert_dir}));

    const iupLib = switch (arch) {
        .x64 => "external/iup-3.30_Win64_mingw6_lib",
        .x86 => "external/iup-3.30_Win32_mingw6_lib",
    };

    exe.addIncludeDir(b.pathJoin(&.{iupLib, "include"}));
    exe.addCSourceFile(b.pathJoin(&.{iupLib, "libiup.a"}), &.{""});

    exe.linkLibC();
    exe.addLibPath(b.fmt("external/{s}/{s}", .{windivert_dir, arch_tag}));
    exe.linkSystemLibrary("WinDivert");
    exe.linkSystemLibrary("comctl32");
    exe.linkSystemLibrary("Winmm");
    exe.linkSystemLibrary("ws2_32");
    exe.linkSystemLibrary("kernel32");
    exe.linkSystemLibrary("gdi32");
    exe.linkSystemLibrary("comdlg32");
    exe.linkSystemLibrary("uuid");
    exe.linkSystemLibrary("ole32");

    const exe_install_step = b.addInstallArtifact(exe);  
    if (conf == .Ship)
    {
        const remove_pdb_step = RemoveOutFile.create(b, "clumsy.pdb");
        remove_pdb_step.step.dependOn(&exe_install_step.step);
        b.getInstallStep().dependOn(&remove_pdb_step.step);
    }
    else
    {
        b.getInstallStep().dependOn(&exe_install_step.step);
    }

    const clean_all = b.step("clean", "purge zig-cache and zig-out");
    clean_all.dependOn(&b.addRemoveDirTree(b.install_path).step);
    //  TODO can't clean cache atm since build.exe is in it
    // clean_all.dependOn(&b.addRemoveDirTree("zig-cache").step);
}

pub const RemoveOutFile = struct {
    step: Step,
    builder: *Builder,
    rel_path: []const u8,

    pub fn create(builder: *Builder, rel_path: []const u8) *@This() {
        const self = builder.allocator.create(@This()) catch unreachable;
        self.* = . {
            .step = Step.init(.custom, builder.fmt("RemoveOutFile {s}", .{rel_path}), builder.allocator, make),
            .builder = builder,
            .rel_path = rel_path,
        };
        return self;
    }

    fn make(step: *Step) anyerror!void {
        const self = @fieldParentPtr(RemoveOutFile, "step", step);
        const out_dir = try std.fs.openDirAbsolute(self.builder.exe_dir, .{});
        try out_dir.deleteFile(self.rel_path);
    }
};