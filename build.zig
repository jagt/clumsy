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

    const arch_tag = @tagName(arch);
    const conf_tag = @tagName(conf);
    const sign_tag = @tagName(windivert_sign);
    const windivert_dir = b.fmt("WinDivert-2.2.0-{s}", .{sign_tag});

    debug.print("- arch: {s}, conf: {s}, sign: {s}\n", .{@tagName(arch), @tagName(conf), @tagName(windivert_sign)});

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

    const exe = b.addExecutable("clumsy", null);

    switch (conf) {
        .Debug => {
            exe.setBuildMode(std.builtin.Mode.Debug);
            exe.subsystem = std.Target.SubSystem.Console;
        },
        .Release => {
            exe.setBuildMode(std.builtin.Mode.ReleaseSafe);
            exe.subsystem = std.Target.SubSystem.Windows;
        },
        .Ship => {
            exe.setBuildMode(std.builtin.Mode.ReleaseFast);
            exe.subsystem = std.Target.SubSystem.Windows;
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
    exe.addCSourceFile("src/bandwidth.c", &[_][]const u8{});
    exe.addCSourceFile("src/divert.c", &[_][]const u8{});
    exe.addCSourceFile("src/drop.c", &[_][]const u8{});
    exe.addCSourceFile("src/duplicate.c", &[_][]const u8{});
    exe.addCSourceFile("src/elevate.c", &[_][]const u8{});
    exe.addCSourceFile("src/lag.c", &[_][]const u8{});
    exe.addCSourceFile("src/main.c", &[_][]const u8{});
    exe.addCSourceFile("src/ood.c", &[_][]const u8{});
    exe.addCSourceFile("src/packet.c", &[_][]const u8{});
    exe.addCSourceFile("src/reset.c", &[_][]const u8{});
    exe.addCSourceFile("src/tamper.c", &[_][]const u8{});
    exe.addCSourceFile("src/throttle.c", &[_][]const u8{});
    exe.addCSourceFile("src/utils.c", &.{""});
    exe.addCSourceFile("src/automation.c", &[_][]const u8{});
    exe.addCSourceFile("src/length.c", &[_][]const u8{});
    exe.addCSourceFile("src/logging.c", &[_][]const u8{});
    exe.addCSourceFile("src/mingw_compat.c", &[_][]const u8{});
    exe.addCSourceFile("src/profiles.c", &[_][]const u8{});
    exe.addCSourceFile("src/protocol.c", &[_][]const u8{});
    exe.addCSourceFile("src/stats.c", &[_][]const u8{});
    exe.addCSourceFile("src/tls.c", &[_][]const u8{});

    if (arch == .x86)
        exe.addCSourceFile("etc/chkstk.s", &[_][]const u8{});

    exe.addIncludePath(b.fmt("external/{s}/include", .{windivert_dir}));

    const iupLib = switch (arch) {
        .x64 => "external/iup-3.30_Win64_mingw6_lib",
        .x86 => "external/iup-3.30_Win32_mingw6_lib",
    };

    exe.addIncludePath(b.pathJoin(&.{iupLib, "include"}));
    exe.addObjectFile(b.pathJoin(&.{iupLib, "libiup.a"}));

    exe.linkLibC();
    exe.addLibraryPath(b.fmt("external/{s}/{s}", .{windivert_dir, arch_tag}));
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