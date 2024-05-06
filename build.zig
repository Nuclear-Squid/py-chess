const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const OptMode = std.builtin.OptimizeMode;
    const c_optimize_flag = switch (optimize) {
        OptMode.Debug        => "-Og",
        OptMode.ReleaseSafe  => "-O3",
        OptMode.ReleaseFast  => "-Ofast",
        OptMode.ReleaseSmall => "-Oz",
    };

    // const lib = b.addExecutable(.{
    const lib = b.addSharedLibrary(.{
        .name = "chess",
        .target = target,
        .optimize = optimize,
    });

    const allocator = std.heap.page_allocator;

    var src_dir = try std.fs.openIterableDirAbsolute(b.pathFromRoot("backend"), .{});
    defer src_dir.close();
    var src_dir_walker = try src_dir.walk(allocator);
    defer src_dir_walker.deinit();

    const c_flags = [_][]const u8 { c_optimize_flag, "-fPIC" };
    var c_source_files = std.ArrayList([]const u8).init(allocator);
    defer {
        for (c_source_files.items) |item| {
            allocator.free(item);
        }
        c_source_files.deinit();
    }

    while (try src_dir_walker.next()) |entry| {
        if (entry.kind == std.fs.IterableDir.Entry.Kind.file
            and entry.path[entry.path.len - 1] == 'c')
        {
            const new_file_path = try c_source_files.addOne();
            new_file_path.* = try std.fmt.allocPrint(allocator, "backend/{s}", .{ entry.path });
        }
    }

    lib.addCSourceFiles(c_source_files.items, &c_flags);
    lib.linkLibC();

    b.installArtifact(lib);

    const run_cmd = b.addSystemCommand(&[_][]const u8 { "python", "frontend/main.py" });

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
