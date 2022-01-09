const std = @import("std");
const Builder = std.build.Builder;
const Pkg = std.build.Pkg;
const Step = std.build.Step;
const debug = std.debug;


pub fn build(b: *std.build.Builder) void {
    b.installFile("external/WinDivert-2.2.0-A/x64/WinDivert.dll", "bin/WinDivert.dll");
    b.installFile("external/WinDivert-2.2.0-A/x64/WinDivert64.sys", "bin/WinDivert64.sys");

    const exe = b.addExecutable("clumsy", null);

    //  switch to rc.exe?? at least it's not so fucking stupid
    // const abs_rc = std.fs.cwd().realpathAlloc(b.allocator);


    //  TODO setup candidate for windres/rc.exe from a few places
    //       windres need to be working in its directory
    //  TODO mkdir p
    // const cmd = b.addSystemCommand(&[_][]const u8{
    //     "C:/msys64/mingw64/bin/llvm-windres.exe",
    //      "etc/clumsy.rc",
    //     "-O",
    //     "coff",
    //     "-o",
    //     "tmp/x64/release/clumsy_res_x64.obj",
    //     "-DNDEBUG",
    //     "-DX64",
    // });
    // cmd.cwd = "C:/msys64/mingw64/bin";

    const cmd = b.addSystemCommand(&[_][]const u8{
        "rc",
        "/nologo",
        "/d",
        "NDEBUG",
        "/d",
        "X64",
        "/r",
        "/fo",
        "tmp/x64/release/clumsy_res_x64.obj",
        "etc/clumsy.rc",
    });

    exe.step.dependOn(&cmd.step);

    exe.addObjectFile("tmp/x64/release/clumsy_res_x64.obj");

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
