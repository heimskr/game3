const std = @import("std");

const ShaderType = enum { frag, vert };

const Shader = struct {
	name: []const u8,
	types: []const ShaderType = &.{ .frag, .vert },
};

/// public so that resources_generator can access this
pub const shaders = [_]Shader{
	.{ .name = "buffered" },
	.{ .name = "rectangle" },
	.{ .name = "sprite" },
	.{ .name = "blur" },
	.{ .name = "reshader", .types = &.{.vert} },
	.{ .name = "multiplier" },
};

comptime {
	for (shaders) |shader| {
		for (shader.types) |shader_type| {
			const path = "../resources/" ++ shader.name ++ "." ++ @tagName(shader_type);

			const shader_data_pointer = @embedFile(path);
			const shader_data = shader_data_pointer.*;
			const shader_len = shader_data_pointer.len;
			@export(shader_data, .{ .name = shader.name ++ "_" ++ @tagName(shader_type) });
			@export(shader_len, .{ .name = shader.name ++ "_" ++ @tagName(shader_type) ++ "_len" });
		}
	}
}
