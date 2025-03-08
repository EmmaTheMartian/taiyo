const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // libhoshi
    const lib_hoshi = b.addSharedLibrary(.{ .name = "hoshi", .target = target, .optimize = optimize });
    lib_hoshi.addCSourceFiles(.{ .files = &.{
        "src/hoshi/chunk.c",
        "src/hoshi/debug.c",
        "src/hoshi/memory.c",
        "src/hoshi/value.c",
        "src/hoshi/vm.c",
    } });
    lib_hoshi.linkLibC();
    b.installArtifact(lib_hoshi);

    // hoshi
    const hoshi_exe = b.addExecutable(.{ .name = "hoshi", .target = target, .optimize = optimize });
    hoshi_exe.addCSourceFile(.{ .file = b.path("src/hoshi/main.c") });
    hoshi_exe.linkLibrary(lib_hoshi);
    hoshi_exe.linkLibC();
    b.installArtifact(hoshi_exe);
    const run_hoshi_exe = b.addRunArtifact(hoshi_exe);

    const run_hoshi_step = b.step("run-hoshi", "Run Hoshi");
    run_hoshi_step.dependOn(&run_hoshi_exe.step);

    // libtaiyo

    // taiyo

    // additional steps
}
