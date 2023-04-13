const std = @import("std");
const shaders = @import("./resources.zig").shaders;

const OutputType = enum { header, dependencies };

fn usage(exec: []const u8) noreturn {
    std.log.err(
        \\usage: {s} <-h|-d>
        \\    -h: print header file
        \\    -d: print Makefile dependencies
    , .{exec});
    std.process.exit(1);
}

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();

    const args = try std.process.argsAlloc(std.heap.page_allocator);
    defer std.process.argsFree(std.heap.page_allocator, args);
    if (args.len != 2) {
        usage(args[0]);
    }
    const output_type = if (std.mem.eql(u8, args[1], "-h"))
        OutputType.header
    else if (std.mem.eql(u8, args[1], "-d"))
        OutputType.dependencies
    else
        usage(args[0]);

    switch (output_type) {
        .header => {
            try stdout.print("#include <cstdlib>\n", .{});
            for (shaders) |shader| {
                for (shader.types) |shader_type| {
                    try stdout.print("extern const char {s}_{s}[];\n", .{ shader.name, @tagName(shader_type) });
                    try stdout.print("extern const size_t {s}_{s}_len;\n", .{ shader.name, @tagName(shader_type) });
                }
            }
        },
        .dependencies => {
            try stdout.print("src/resources.o: src/resources.zig", .{});
            for (shaders) |shader| {
                for (shader.types) |shader_type| {
                    try stdout.print(" resources/{s}.{s}", .{ shader.name, @tagName(shader_type) });
                }
            }
            try stdout.print("\n", .{});
        },
    }
}
