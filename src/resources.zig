const std = @import("std");

const Shader = struct {
	name: []const u8,
	has_frag: bool = true,
	has_vert: bool = true,
};

const shaders = [_]Shader{
	.{ .name = "buffered" },
	.{ .name = "rectangle" },
	.{ .name = "sprite" },
	.{ .name = "blur" },
	.{ .name = "reshader", .has_frag = false },
	.{ .name = "multiplier", .has_vert = false },
};

comptime {
	for (shaders) |shader| {
		var types: []const []const u8 = &[_][]const u8{};
		if (shader.has_frag) {
			types = types ++ [_][]const u8{"frag"};
		}
		if (shader.has_vert) {
			types = types ++ [_][]const u8{"vert"};
		}
		for (types) |shader_type| {
			const path = "../resources/" ++ shader.name ++ "." ++ shader_type;

			const shader_data_pointer = @embedFile(path);
			const shader_data = shader_data_pointer.*;
			const shader_len = shader_data_pointer.len;
			@export(shader_data, .{ .name = shader.name ++ "_" ++ shader_type });
			@export(shader_len, .{ .name = shader.name ++ "_" ++ shader_type ++ "_len" });
		}
	}
}
