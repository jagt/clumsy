const std = @import("std");
const Builder = std.build.Builder;
const Pkg = std.build.Pkg;
const Step = std.build.Step;
const debug = std.debug;
const Allocator = std.mem.Allocator;

const ClumsyArch = enum { x86, x64 };
const ClumsyConf = enum { Debug, Release };

pub fn build(b: *std.build.Builder) void {
    const arch = b.option(ClumsyArch, "arch", "x86, x64") orelse .x64;
    const conf = b.option(ClumsyConf, "conf", "Debug, Release") orelse .Debug;

    debug.print("building: {s} {s}\n", .{arch, conf});

    //  path format helper
    const fmt = struct {
        allocator: Allocator,
        _arch: ClumsyArch,
        _conf: ClumsyConf,

        //  zig format doesn't allow unused argument
        pub fn archConf(self: *const @This(), comptime fmt: []const u8) []u8 {
            return std.fmt.allocPrint(self.allocator, fmt, .{ .arch = @tagName(self._arch), .conf = @tagName(self._conf)}) catch unreachable;
        }

        pub fn arch(self: *const @This(), comptime fmt: []const u8) []u8 {
            return std.fmt.allocPrint(self.allocator, fmt, .{ .arch = @tagName(self._arch) }) catch unreachable;
        }

        pub fn conf(self: *const @This(), comptime fmt: []const u8) []u8 {
            return std.fmt.allocPrint(self.allocator, fmt, .{ .conf = @tagName(self._conf) }) catch unreachable;
        }
    }{ .allocator = b.allocator, ._arch = arch, ._conf = conf };

    const tmpPath = fmt.archConf("tmp/{[arch]s}/{[conf]s}");

    b.makePath(tmpPath) catch @panic("unable to create tmp directory");

    b.installFile(fmt.arch("external/WinDivert-2.2.0-A/{[arch]s}/WinDivert.dll"), "bin/WinDivert.dll");
    b.installFile(fmt.arch("external/WinDivert-2.2.0-A/{[arch]s}/WinDivert64.sys"), "bin/WinDivert64.sys");
    b.installFile("etc/config.txt", "bin/config.txt");
    b.installFile("LICENSE", "bin/License.txt");

    const resObjPath = fmt.archConf("tmp/{[arch]s}/{[conf]s}/clumsy_res.obj");

    //  fix up `PATH` 
    if (b.env_map.get("PATH") == null)
    {
        var foundPath = false;
        var it = b.env_map.iterator();
        while (it.next()) |kv| {
            if (std.ascii.eqlIgnoreCase(kv.key_ptr.*, "PATH")) {
                foundPath = true;
                b.env_map.put("PATH", kv.value_ptr.*) catch unreachable;
            }
        }
        if (!foundPath) @panic("no `PATH` in env");
    }

    //  check `rc` is on path, warn about VCVars
    const rcExe = b.findProgram(&[_][]const u8{
        "rc",
    }, &[_][]const u8{}) catch @panic("unable to find `rc.exe`. make sure you've run VCVars.bat");

    const cmd = b.addSystemCommand(&[_][]const u8{
        rcExe,
        "/nologo",
        "/d",
        "NDEBUG",
        "/d",
        "X64",
        "/r",
        "/fo",
        resObjPath,
        "etc/clumsy.rc",
    });

    const exe = b.addExecutable("clumsy", null);
    exe.step.dependOn(&cmd.step);

    exe.addObjectFile(resObjPath);

    exe.addCSourceFile("src/bandwidth.c", &[_][]const u8{""});
    exe.addCSourceFile("src/divert.c", &[_][]const u8{""});
    exe.addCSourceFile("src/drop.c", &[_][]const u8{""});
    exe.addCSourceFile("src/duplicate.c", &[_][]const u8{""});
    exe.addCSourceFile("src/elevate.c", &[_][]const u8{""});
    exe.addCSourceFile("src/lag.c", &[_][]const u8{""});
    exe.addCSourceFile("src/main.c", &[_][]const u8{""});
    exe.addCSourceFile("src/ood.c", &[_][]const u8{""});
    exe.addCSourceFile("src/packet.c", &[_][]const u8{""});
    exe.addCSourceFile("src/reset.c", &[_][]const u8{""});
    exe.addCSourceFile("src/tamper.c", &[_][]const u8{""});
    exe.addCSourceFile("src/throttle.c", &[_][]const u8{""});
    exe.addCSourceFile("src/utils.c", &[_][]const u8{""});

    exe.setBuildMode(b.standardReleaseOptions());
    exe.setTarget(b.standardTargetOptions(.{}));

    exe.addIncludeDir("external/WinDivert-2.2.0-A/include");
    exe.addIncludeDir("external/iup-3.30_Win64_mingw6_lib/include");

    exe.addCSourceFile("external/iup-3.30_Win64_mingw6_lib/libiup.a", &[_][]const u8{""});

    exe.linkLibC();
    exe.addLibPath("external/WinDivert-2.2.0-A/x64");
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

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());

    //  `b.step` creates a new top level step
    //  then it depends on exe.run,so running hello would trigger run
    const hello_step = b.step("hello", "this is a hello step");
    hello_step.dependOn(&run_cmd.step);

    const clumsy_step = ClumsyStep.create(b, "wow");
    hello_step.dependOn(&clumsy_step.step);

    debug.print("builder args: {s}\n", .{b.args});
}

pub const ClumsyStep = struct {
    step: Step,
    builder: *Builder,

    pub fn create(builder: *Builder, data: []const u8) *@This() {
        const self = builder.allocator.create(@This()) catch unreachable;
        self.* = . {
            .step = Step.init(.custom, builder.fmt("clumsyStep {s}", .{data}), builder.allocator, make),
            .builder = builder,
        };
        return self;
    }

    fn make(_: *Step) anyerror!void {
        debug.print("{s}", .{"hello clumsyStep"});
    }
};
