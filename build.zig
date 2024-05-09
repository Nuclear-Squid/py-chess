const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const c_optimize_flag = switch (optimize) {
        .Debug        => "-Og",
        .ReleaseSafe  => "-O3",
        .ReleaseFast  => "-Ofast",
        .ReleaseSmall => "-Oz",
    };

    const c_src_dir = "backend";
    const lib = b.addSharedLibrary(.{
        .name = "chess",
        .target = target,
        .optimize = optimize,
    });

    const allocator = std.heap.page_allocator;

    var src_dir = try std.fs.openIterableDirAbsolute(b.pathFromRoot(c_src_dir), .{});
    defer src_dir.close();
    var src_dir_walker = try src_dir.walk(allocator);
    defer src_dir_walker.deinit();

    var c_source_files = std.ArrayList([]const u8).init(allocator);
    defer {
        for (c_source_files.items) |item| {
            b.allocator.free(item);
        }
        c_source_files.deinit();
    }

    while (try src_dir_walker.next()) |entry| {
        if (entry.kind == .file and entry.path[entry.path.len - 1] == 'c') {
            const new_file_path = try c_source_files.addOne();
            new_file_path.* = b.pathJoin(&.{ c_src_dir, entry.path });
        }
    }

    lib.addCSourceFiles(c_source_files.items, &.{ c_optimize_flag, "-fPIC" });
    lib.addIncludePath(.{ .path = std.os.getenv("CPYTHON_HEADER_PATH").? });
    lib.linkLibC();

    b.installArtifact(lib);

    const run_cmd = b.addSystemCommand(&.{ "python", "frontend/main.py" });
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
