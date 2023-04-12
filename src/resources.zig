const std = @import("std");

comptime {
	for (.{"buffered", "rectangle", "sprite", "blur", "reshader", "multiplier"}) |shader_name| {
		const types = if (std.mem.eql(u8, shader_name, "reshader"))
			.{"vert"}
		else if (std.mem.eql(u8, shader_name, "multiplier"))
			.{"frag"}
		else
			.{"frag", "vert"};
		for (types) |shader_type| {
			const path = "../resources/" ++ shader_name ++ "." ++ shader_type;
			const shader_data_pointer = @embedFile(path);
			const shader_data = shader_data_pointer.*;
			const shader_len  = shader_data_pointer.len;
			@export(shader_data, .{ .name = shader_name ++ "_" ++ shader_type });
			@export(shader_len,  .{ .name = shader_name ++ "_" ++ shader_type ++ "_len" });
		}
	}
}
